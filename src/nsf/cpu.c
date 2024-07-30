#include "cpu.h"
#include "nsf/debug.h"
#include "nsf_internal.h"

#include <stdio.h>
#include <string.h>

#define CPUFLAG_NEGATIVE          (1 << 7)
#define CPUFLAG_OVERFLOW          (1 << 6)
#define CPUFLAG_RESERVED          (1 << 5)
#define CPUFLAG_BREAK_CMD         (1 << 4)
#define CPUFLAG_DECIMAL           (1 << 3)
#define CPUFLAG_INTERRUPT_DISABLE (1 << 2)
#define CPUFLAG_ZERO              (1 << 1)
#define CPUFLAG_CARRY             (1 << 0)

// https://www.scs.stanford.edu/~dm/blog/va-opt.html
#define PARENS ()
#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__
#define FOR_EACH_HELPER(macro, param, a1, ...) macro(param, a1) __VA_OPT__(FOR_EACH_AGAIN PARENS (macro, param, __VA_ARGS__))
#define FOR_EACH(macro, param, ...) __VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, param, __VA_ARGS__)))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

// value types
#define CNST 0 // constant
#define ACCM 1 // accumulator
#define ADDR 2 // address
#define IMPL 3 // implicit

// helpers

#define _8 ({                                                \
    uint8_t val = ((uint8_t)handle->cpu.ram[handle->cpu.pc]); \
    handle->cpu.pc += 1;                                       \
    val;                                                        \
})

#define _16 ({                         \
    uint16_t val = M16(handle->cpu.pc); \
    handle->cpu.pc += 2;                 \
    val;                                  \
})

#define M16(addr) ({                                                                                \
    uint16_t _addr = addr;                                                                           \
    (uint16_t)(uint8_t)handle->cpu.ram[_addr] | ((uint16_t)(uint8_t)handle->cpu.ram[_addr + 1] << 8); \
})

#define A         handle->cpu.a
#define X         handle->cpu.x
#define Y         handle->cpu.y
#define F         handle->cpu.flags
#define PC        handle->cpu.pc
#define SP        handle->cpu.sp
#define RAM       handle->cpu.ram

// stack manip

#define PUSH(x) {                               \
    handle->cpu.ram[0x100 + handle->cpu.sp] = x; \
    handle->cpu.sp--;                             \
}

#define POP() ({                                              \
    int16_t val = handle->cpu.ram[0x100 + handle->cpu.sp + 1]; \
    handle->cpu.sp++;                                           \
    val;                                                         \
})

#define POP16() ({                                \
    int16_t val = M16(0x100 + handle->cpu.sp + 1); \
    handle->cpu.sp += 2;                            \
    val;                                             \
})

#define CALL(a, b) b(a)
#define s(x) ( int8_t)(x)
#define u(x) (uint8_t)(x)
#define instr(name, body, ...) void name(uint8_t valtype, int16_t value, NSFHandle* handle) body FOR_EACH(CALL, name, __VA_ARGS__)
#define imm(name) void name##_imm(NSFHandle* handle) { name(CNST,      _8,                 handle); } // immediate (constant)
#define acc(name) void name##_acc(NSFHandle* handle) { name(ACCM,       A,                 handle); } // accumulator
#define zpg(name) void name##_zpg(NSFHandle* handle) { name(ADDR,      _8,                 handle); } // zero page
#define zpx(name) void name##_zpx(NSFHandle* handle) { name(ADDR,      _8  + u(X),         handle); } // zero page x
#define zpy(name) void name##_zpy(NSFHandle* handle) { name(ADDR,      _8  + u(Y),         handle); } // zero page y
#define abs(name) void name##_abs(NSFHandle* handle) { name(ADDR,      _16,                handle); } // absolute
#define abx(name) void name##_abx(NSFHandle* handle) { name(ADDR,      _16 + u(X),         handle); } // absolute x
#define aby(name) void name##_aby(NSFHandle* handle) { name(ADDR,      _16 + u(Y),         handle); } // absolute y
#define rel(name) void name##_rel(NSFHandle* handle) { name(ADDR,   s( _8) + PC,           handle); } // relative
#define ind(name) void name##_ind(NSFHandle* handle) { name(ADDR, M16( _16       ),        handle); } // indirect
#define inx(name) void name##_inx(NSFHandle* handle) { name(ADDR, M16((_8  + u(X)) % 256), handle); } // indirect x
#define iny(name) void name##_iny(NSFHandle* handle) { name(ADDR, M16( _8) + u(Y),         handle); } // indirect y
#define imp(name) void name##_imp(NSFHandle* handle) { name(IMPL, 0,                       handle); } // implicit

#define GET_PTR_TO_VAL \
    int8_t* ptr = NULL; \
    if (valtype == ADDR) ptr = &RAM[(uint16_t)value]; \
    if (valtype == ACCM) ptr = &A;
#define GET_VAL_FROM_ADDR if (valtype == ADDR) value = RAM[(uint16_t)value];
#define FLAG(x) !!(F & (CPUFLAG_##x))
#define SET_NZ_FLAGS(val) \
    SET_IF(ZERO, (val) % 0x100 == 0); \
    SET_IF(NEGATIVE, (val) & 0x80); 
#define SET_IF(flag, cond) \
    if (cond) F |= CPUFLAG_##flag; \
    else F &= ~CPUFLAG_##flag

instr(ADC, { GET_VAL_FROM_ADDR
    uint16_t next = (uint8_t)A + (uint8_t)value + FLAG(CARRY);
    SET_IF(CARRY, next > 0xFF);
    SET_IF(OVERFLOW, ((~(A ^ (uint8_t)value)) & (A ^ (uint8_t)next) & 0x80));
    A = next;
    SET_NZ_FLAGS(A);
}, imm, zpg, zpx, abs, abx, aby, inx, iny, imp)

instr(AND, { GET_VAL_FROM_ADDR
    A &= value;
    SET_NZ_FLAGS(A);
}, imm, zpg, zpx, abs, abx, aby, inx, iny)

instr(ASL, { GET_PTR_TO_VAL
    SET_IF(CARRY, *ptr & 0x80);
    *ptr <<= 1;
    *ptr &= ~1;
    SET_NZ_FLAGS(*ptr);
}, acc, zpg, zpx, abs, abx)

instr(BCC, {
    if (!(F & CPUFLAG_CARRY)) PC = value; 
}, rel)

instr(BCS, {
    if (F & CPUFLAG_CARRY) PC = value; 
}, rel)

instr(BEQ, {
    if (F & CPUFLAG_ZERO) PC = value; 
}, rel)

instr(BIT, { GET_VAL_FROM_ADDR
    uint8_t result = value & A;
    SET_IF(OVERFLOW, result & 0x40);
    SET_NZ_FLAGS(result);
}, zpg, abs)

instr(BMI, {
    if (F & CPUFLAG_NEGATIVE) PC = value;
}, rel)

instr(BNE, {
    if (!(F & CPUFLAG_ZERO)) PC = value; 
}, rel)

instr(BPL, {
    if (!(F & CPUFLAG_NEGATIVE)) PC = value;
}, rel)

instr(BRK, {
    PUSH((PC >> 8) & 0xFF);
    PUSH( PC       & 0xFF);
    PUSH(F);
    PC = M16(0xFFFE);
    SET_IF(BREAK_CMD, true);
}, imp)

instr(BVC, {
    if (!(F & CPUFLAG_OVERFLOW)) PC = value;
}, rel)

instr(BVS, {
    if (F & CPUFLAG_OVERFLOW) PC = value;
}, rel)

instr(CLC, {
    SET_IF(CARRY, false);
}, imp)

instr(CLD, {
    SET_IF(DECIMAL, false);
}, imp)

instr(CLI, {
    SET_IF(INTERRUPT_DISABLE, false);
}, imp)

instr(CLV, {
    SET_IF(OVERFLOW, false);
}, imp)

instr(CMP, { GET_VAL_FROM_ADDR
    uint8_t temp = A - value;
    SET_IF(CARRY, (temp & 0x80) == 0);
    SET_NZ_FLAGS(temp);
}, imm, zpg, zpx, abs, abx, aby, inx, iny)

instr(CPX, { GET_VAL_FROM_ADDR
    uint8_t temp = X - value;
    SET_IF(CARRY, (temp & 0x80) == 0);
    SET_NZ_FLAGS(temp);
}, imm, zpg, abs)

instr(CPY, { GET_VAL_FROM_ADDR
    uint8_t temp = Y - value;
    SET_IF(CARRY, (temp & 0x80) == 0);
    SET_NZ_FLAGS(temp);
}, imm, zpg, abs)

instr(DEC, { GET_PTR_TO_VAL
    (*ptr)--;
    SET_NZ_FLAGS(*ptr);
}, zpg, zpx, abs, abx)

instr(DEX, {
    X--;
    SET_NZ_FLAGS(X);
}, imp)

instr(DEY, {
    Y--;
    SET_NZ_FLAGS(Y);
}, imp)

instr(EOR, { GET_VAL_FROM_ADDR
    A ^= value;
    SET_NZ_FLAGS(A);
}, imm, zpg, zpx, abs, abx, aby, inx, iny)

instr(INC, { GET_PTR_TO_VAL
    (*ptr)++;
    SET_NZ_FLAGS(*ptr);
}, zpg, zpx, abs, abx)

instr(INX, {
    X++;
    SET_NZ_FLAGS(X);
}, imp)

instr(INY, {
    Y++;
    SET_NZ_FLAGS(Y);
}, imp)

instr(JMP, {
    PC = value;
}, abs, ind)

instr(JSR, {
    PUSH((PC >> 8) & 0xFF);
    PUSH( PC       & 0xFF);
    PC = value;
}, abs)

instr(LDA, { GET_VAL_FROM_ADDR
    A = value;
    SET_NZ_FLAGS(A);
}, imm, zpg, zpx, abs, abx, aby, inx, iny)

instr(LDX, { GET_VAL_FROM_ADDR
    X = value;
    SET_NZ_FLAGS(X);
}, imm, zpg, zpy, abs, aby)

instr(LDY, { GET_VAL_FROM_ADDR
    Y = value;
    SET_NZ_FLAGS(Y);
}, imm, zpg, zpx, abs, abx)

instr(LSR, { GET_PTR_TO_VAL
    SET_IF(CARRY, *ptr & 1);
    *ptr >>= 1;
    *ptr &= ~0x80;
    SET_NZ_FLAGS(*ptr);
}, acc, zpg, zpx, abs, abx)

instr(NOP, {}, imp)

instr(ORA, { GET_VAL_FROM_ADDR
    A |= value;
    SET_NZ_FLAGS(A);
}, imm, zpg, zpx, abs, abx, aby, inx, iny)

instr(PHA, {
    PUSH(A);
}, imp)

instr(PHP, {
    PUSH(F);
}, imp)

instr(PLA, {
    A = POP();
}, imp)

instr(PLP, {
    F = POP();
}, imp)

instr(ROL, { GET_PTR_TO_VAL
    uint8_t old_bit = *ptr & 0x80;
    *ptr <<= 1;
    *ptr &= ~1;
    *ptr |= FLAG(CARRY);
    SET_IF(CARRY, old_bit);
    SET_NZ_FLAGS(*ptr);
}, acc, zpg, zpx, abs, abx)

instr(ROR, { GET_PTR_TO_VAL
    uint8_t old_bit = *ptr & 1;
    *ptr >>= 1;
    *ptr &= ~0x80;
    *ptr |= FLAG(CARRY) << 7;
    SET_IF(CARRY, old_bit);
    SET_NZ_FLAGS(*ptr);
}, acc, zpg, zpx, abs, abx)

instr(RTI, {
    F = POP();
    PC = POP16();
}, imp)

instr(RTS, {
    PC = POP16();
}, imp)

instr(SBC, { GET_VAL_FROM_ADDR
    int16_t next = A - value - (FLAG(CARRY) ^ 1);
    SET_IF(CARRY, (uint16_t)(next + (FLAG(CARRY))) < 0x100);
    SET_IF(OVERFLOW, ((A ^ value) & (A ^ next) & 0x80));
    SET_NZ_FLAGS(next);
    A = next;
}, imm, zpg, zpx, abs, abx, aby, inx, iny)

instr(SEC, {
    SET_IF(CARRY, true);
}, imp)

instr(SED, {
    SET_IF(DECIMAL, true);
}, imp)

instr(SEI, {
    SET_IF(INTERRUPT_DISABLE, true);
}, imp)

instr(STA, { GET_PTR_TO_VAL
    *ptr = A;
}, zpg, zpx, abs, abx, aby, inx, iny)

instr(STX, { GET_PTR_TO_VAL
    *ptr = X;
}, zpg, zpy, abs)

instr(STY, { GET_PTR_TO_VAL
    *ptr = Y;
}, zpg, zpx, abs)

instr(TAX, {
    X = A;
    SET_NZ_FLAGS(X);
}, imp)

instr(TAY, {
    Y = A;
    SET_NZ_FLAGS(Y);
}, imp)

instr(TSX, {
    X = SP;
    SET_NZ_FLAGS(X);
}, imp)

instr(TXA, {
    A = X;
    SET_NZ_FLAGS(A);
}, imp)

instr(TXS, {
    SP = X;
}, imp)

instr(TYA, {
    A = Y;
    SET_NZ_FLAGS(A);
}, imp)

#undef instr
#undef imm
#undef acc
#undef zpg
#undef zpx
#undef zpy
#undef abs
#undef abx
#undef aby
#undef rel
#undef ind
#undef inx
#undef iny
#undef imp

#define instr(name, ...) FOR_EACH(CALL, name, __VA_ARGS__)
#define imm(id) [id] = imm_
#define acc(id) [id] = acc_
#define zpg(id) [id] = zpg_
#define zpx(id) [id] = zpx_
#define zpy(id) [id] = zpy_
#define abs(id) [id] = abs_
#define abx(id) [id] = abx_
#define aby(id) [id] = aby_
#define rel(id) [id] = rel_
#define ind(id) [id] = ind_
#define inx(id) [id] = inx_
#define iny(id) [id] = iny_
#define imp(id) [id] = imp_
#define imm_(name) name##_imm,
#define acc_(name) name##_acc,
#define zpg_(name) name##_zpg,
#define zpx_(name) name##_zpx,
#define zpy_(name) name##_zpy,
#define abs_(name) name##_abs,
#define abx_(name) name##_abx,
#define aby_(name) name##_aby,
#define rel_(name) name##_rel,
#define ind_(name) name##_ind,
#define inx_(name) name##_inx,
#define iny_(name) name##_iny,
#define imp_(name) name##_imp,

void(*instr_table[256])(NSFHandle* handle) = {
    instr(ADC, imm(0x69), zpg(0x65), zpx(0x75), abs(0x6D), abx(0x7D), aby(0x79), inx(0x61), iny(0x71))
    instr(AND, imm(0x29), zpg(0x25), zpx(0x35), abs(0x2D), abx(0x3D), aby(0x39), inx(0x21), iny(0x31))
    instr(ASL, acc(0x0A), zpg(0x06), zpx(0x16), abs(0x0E), abx(0x1E))
    instr(BCC, rel(0x90))
    instr(BCS, rel(0xB0))
    instr(BEQ, rel(0xF0))
    instr(BIT, zpg(0x24), abs(0x2C))
    instr(BMI, rel(0x30))
    instr(BNE, rel(0xD0))
    instr(BPL, rel(0x10))
    instr(BRK, imp(0x00))
    instr(BVC, rel(0x50))
    instr(BVS, rel(0x70))
    instr(CLC, imp(0x18))
    instr(CLD, imp(0xD8))
    instr(CLI, imp(0x58))
    instr(CLV, imp(0xB8))
    instr(CMP, imm(0xC9), zpg(0xD5), zpx(0xC5), abs(0xCD), abx(0xDD), inx(0xC1), iny(0xD1))
    instr(CPX, imm(0xE0), zpg(0xE4), abs(0xEC))
    instr(CPY, imm(0xC0), zpg(0xC4), abs(0xCC), zpg(0xC6))
    instr(DEC, zpx(0xD6), abs(0xCE), abx(0xDE))
    instr(DEX, imp(0xCA))
    instr(DEY, imp(0x88))
    instr(EOR, imm(0x49), zpg(0x45), zpx(0x55), abs(0x4D), abx(0x5D), aby(0x59), inx(0x41), iny(0x51))
    instr(INC, zpg(0xE6), zpx(0xF6), abs(0xEE), abx(0xFE))
    instr(INX, imp(0xE8))
    instr(INY, imp(0xC8))
    instr(JMP, abs(0x4C), ind(0x6C))
    instr(JSR, abs(0x20))
    instr(LDA, imm(0xA9), zpg(0xA5), zpx(0xB5), abs(0xAD), abx(0xBD), aby(0xB9), inx(0xA1), iny(0xB1))
    instr(LDX, imm(0xA2), zpg(0xA6), zpy(0xB6), abs(0xAE), aby(0xBE))
    instr(LDY, imm(0xA0), zpg(0xA4), zpx(0xB4), abs(0xAC), abx(0xBC))
    instr(LSR, acc(0x4A), zpg(0x46), zpx(0x56), abs(0x4E), abx(0x5E))
    instr(NOP, imp(0xEA))
    instr(ORA, imm(0x09), zpg(0x05), zpx(0x15), abs(0x0D), abx(0x1D), aby(0x19), inx(0x01), iny(0x11))
    instr(PHA, imp(0x48))
    instr(PHP, imp(0x08))
    instr(PLA, imp(0x68))
    instr(PLP, imp(0x28))
    instr(ROL, acc(0x2A), zpg(0x26), zpx(0x36), abs(0x2E), abx(0x3E))
    instr(ROR, acc(0x6A), zpg(0x66), zpx(0x76), abs(0x6E), abx(0x7E))
    instr(RTI, imp(0x40))
    instr(RTS, imp(0x60))
    instr(SBC, imm(0xE9), zpg(0xE5), zpx(0xF5), abs(0xED), abx(0xFD), aby(0xF9), inx(0xE1), iny(0xF1))
    instr(SEC, imp(0x38))
    instr(SED, imp(0xF8))
    instr(SEI, imp(0x78))
    instr(STA, zpg(0x85), zpx(0x95), abs(0x8D), abx(0x9D), aby(0x99), inx(0x81), iny(0x91))
    instr(STX, zpg(0x86), zpy(0x96), abs(0x8E))
    instr(STY, zpg(0x84), zpx(0x94), abs(0x8C))
    instr(TAX, imp(0xAA))
    instr(TAY, imp(0xA8))
    instr(TSX, imp(0xBA))
    instr(TXA, imp(0x8A))
    instr(TXS, imp(0x9A))
    instr(TYA, imp(0x98))
};

void switch_banks(NSFHandle* handle) {
    if (handle->uses_bank_switching == FALSE) return;
    if (handle->uses_bank_switching == UNSET) {
        uint16_t total;
        for (int i = 0; i < 8; i++) {
            total += handle->cpu.ram[0x5FF8 + i];
            handle->cpu.banks[i] = 0xFF;
        }
        handle->uses_bank_switching = total ? TRUE : FALSE;
        if (total == 0) { // no bank switching
            memcpy(handle->cpu.ram + handle->load_addr, handle->program, handle->program_length);
            return;
        }
    }
    for (int i = 0; i < 8; i++) {
        if (handle->cpu.banks[i] == handle->cpu.ram[0x5FF8 + i]) continue;
        handle->cpu.banks[i] = handle->cpu.ram[0x5FF8 + i];
        int dst = i * 0x1000;
        int src = handle->cpu.banks[i] * 0x1000;
        int len = handle->program_length - src;
        if (len > 0x1000) len = 0x1000;
        memcpy(handle->cpu.ram + 0x8000 + dst, handle->program + src, len);
        memset(handle->cpu.ram + 0x8000 + dst + len, 0, 0x1000 - len);
    }
}

void init_memory(NSFHandle* handle) {
    memcpy(handle->cpu.ram + 0x5FF8, handle->bank_switch_values, 8);
    switch_banks(handle);
    memset(handle->cpu.ram + 0x0000, 0, 0x07FF);
    memset(handle->cpu.ram + 0x6000, 0, 0x7FFF);
}

void call_subroutine(NSFHandle* handle, uint16_t addr) {
    handle->cpu.pc = 0xFFFF;
    JSR(ADDR, addr, handle);
    while (handle->inited && handle->cpu.pc != 0xFFFF) {
        instr_table[_8](handle);
        switch_banks(handle);
    }
}

extern int usleep(__useconds_t __useconds); // unistd.h, including it conflicts with "brk"

int frame = 0;
void* cpu_update(void* handle_ptr) {
    NSFHandle* handle = handle_ptr;
    printf("[Song %d] init\n", handle->cur_song);
    call_subroutine(handle, handle->init_addr);
    while (handle->inited) {
        if (!handle->playing) {
            usleep(1000);
            continue;
        }
        printf("[Song %d] update %d\n", handle->cur_song, frame);
        handle->cpu.a = handle->cpu.x = handle->cpu.y = 0;
        call_subroutine(handle, handle->play_addr);
        usleep(handle->ntsc_play_speed);
    }
    return NULL;
}

void cpu_init(NSFHandle* handle) {
    //dbg_init();
    init_memory(handle);
    handle->cpu.a = handle->cur_song;
    handle->cpu.x = NTSC;
    handle->cpu.y = 0;
    handle->cpu.sp = 0xFF;
    handle->cpu.pc = 0xFFFC;
    handle->cpu.flags = CPUFLAG_RESERVED;
    pthread_create(&handle->cpu_thread_id, NULL, cpu_update, handle);
}

void cpu_dispose(NSFHandle* handle) {
    handle->inited = false;
    pthread_join(handle->cpu_thread_id, NULL);
}
