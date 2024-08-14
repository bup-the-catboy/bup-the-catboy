#ifndef LIBSERIAL_H
#define LIBSERIAL_H

#ifndef LIBSERIAL_STRMAXLEN
#define LIBSERIAL_STRMAXLEN 1024
#endif

#ifdef LIBSERIAL_SOURCE
#define HEADER(...)
#define SOURCE(...) __VA_ARGS__
#else
#define HEADER(...) __VA_ARGS__
#define SOURCE(...)
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define NO_INTELLISENSE

typedef char string[LIBSERIAL_STRMAXLEN];

#pragma pack(push,1)

#define OBJECT(name, type) typedef type LibSerialObj_##name;
#define STRUCT(definition) struct { definition }
#define INT8 int8_t
#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define FLOAT float
#define DOUBLE double
#define STRING string
#define ARRAY(type) type*
#define _(x) x;
#include LIBSERIAL_INCLUDE
#undef STRUCT
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef UINT8
#undef UINT16
#undef UINT32
#undef UINT64
#undef FLOAT
#undef DOUBLE
#undef STRING
#undef ARRAY
#undef _

#pragma pack(pop)

SOURCE(

enum __LibSerial_VarTypes {
    LibSerial_VarType_None,
    LibSerial_VarType_Struct,
    LibSerial_VarType_Array,
    LibSerial_VarType_Int8,
    LibSerial_VarType_Int16,
    LibSerial_VarType_Int32,
    LibSerial_VarType_Int64,
    LibSerial_VarType_UInt8,
    LibSerial_VarType_UInt16,
    LibSerial_VarType_UInt32,
    LibSerial_VarType_UInt64,
    LibSerial_VarType_Float,
    LibSerial_VarType_Double,
    LibSerial_VarType_String,
};

struct LibSerial_Object {
    enum __LibSerial_VarTypes type;
    struct LibSerial_Object* parent;
    struct LibSerial_Object* inner;
    struct LibSerial_Object* next;
};

)

#undef OBJECT
#define OBJECT(name, type) LibSerial_ObjType_##name,
enum LibSerial_ObjTypes {
#include LIBSERIAL_INCLUDE
    LibSerialNumObjects
};

#undef OBJECT
#ifdef LIBSERIAL_SOURCE
#define OBJECT(name, type) sizeof(LibSerialObj_##name),
#else
#define OBJECT(name, type)
#endif

SOURCE(struct LibSerial_Object* __libserialobjs[LibSerialNumObjects];)
SOURCE(int __libserialobjsiz[LibSerialNumObjects] = {)
#include LIBSERIAL_INCLUDE
SOURCE(};)

#define zalloc(size) memset(malloc(size), 0, size)

#define libserial_arrsize(arr) *((size_t*)(arr) - 1)
#define libserial_arrmake(arr, size) {                           \
    size_t* ptr = zalloc(sizeof(*(arr)) * size + sizeof(size_t)); \
    *ptr = size;                                                   \
    arr = (void*)(ptr + 1);                                         \
}

#define libserial_objtype(ptr) *((enum LibSerial_ObjTypes*)(ptr) - 1)
#define libserial_objmake(type) __libserial_objmake(LibSerial_ObjType_##type)
#define __libserial_objmake(type) ({                                                                 \
    enum LibSerial_ObjTypes* ptr = zalloc(__libserialobjsiz[type] + sizeof(enum LibSerial_ObjTypes)); \
    *ptr = (enum LibSerial_ObjTypes)(type);                                                            \
    (void*)(ptr + 1);                                                                                   \
})

#define READ(type, ptr) ({         \
    type val = *(type*)(ptr);       \
    ptr = (void*)((type*)(ptr) + 1); \
    val;                              \
})
#define WRITE(type, ptr, val) ({   \
    *(type*)(ptr) = val;            \
    ptr = (void*)((type*)(ptr) + 1); \
})

#include <stdio.h>

SOURCE(

void __freeobj(struct LibSerial_Object* obj) {
    if (obj->inner) __freeobj(obj->inner);
    if (obj->next) __freeobj(obj->next);
    free(obj);
}

size_t __objlen(struct LibSerial_Object* obj) {
    size_t len = 0;
    struct LibSerial_Object* curr = obj;
    while (curr) {
        switch (curr->type) {
            case LibSerial_VarType_Struct:
                len += __objlen(curr->inner);
                break;
            case LibSerial_VarType_Int8:
            case LibSerial_VarType_UInt8:
                len += 1;
                break;
            case LibSerial_VarType_Int16:
            case LibSerial_VarType_UInt16:
                len += 2;
                break;
            case LibSerial_VarType_Int32:
            case LibSerial_VarType_UInt32:
            case LibSerial_VarType_Float:
                len += 4;
                break;
            case LibSerial_VarType_Array:
            case LibSerial_VarType_Int64:
            case LibSerial_VarType_UInt64:
            case LibSerial_VarType_Double:
                len += 8;
                break;
            case LibSerial_VarType_String:
                len += LIBSERIAL_STRMAXLEN;
                break;
            default: break;
        }
        curr = curr->next;
    }
    return len;
}

void __deserialize(void** out, unsigned char** in, struct LibSerial_Object* obj) {
    struct LibSerial_Object* curr = obj;
    uint32_t arrlen;
    void *arr, *_arr;
    while (curr) {
        switch (curr->type) {
            case LibSerial_VarType_Struct:
                __deserialize(out, in, curr->inner);
                break;
            case LibSerial_VarType_Array:
                arrlen = READ(uint32_t, *in);
                arr = zalloc(arrlen * __objlen(curr->inner) + sizeof(size_t));
                WRITE(size_t, arr, arrlen);
                WRITE(void*, *out, arr);
                _arr = arr;
                for (uint32_t i = 0; i < arrlen; i++) {
                    __deserialize(&_arr, in, curr->inner);
                }
                break;
            case LibSerial_VarType_Int8:
            case LibSerial_VarType_UInt8:
                WRITE(int8_t, *out, READ(int8_t, *in));
                break;
            case LibSerial_VarType_Int16:
            case LibSerial_VarType_UInt16:
                WRITE(int16_t, *out, READ(int16_t, *in));
                break;
            case LibSerial_VarType_Int32:
            case LibSerial_VarType_UInt32:
            case LibSerial_VarType_Float:
                WRITE(int32_t, *out, READ(int32_t, *in));
                break;
            case LibSerial_VarType_Int64:
            case LibSerial_VarType_UInt64:
            case LibSerial_VarType_Double:
                WRITE(int64_t, *out, READ(int64_t, *in));
                break;
            case LibSerial_VarType_String:
                strcpy((char*)*out, (char*)*in);
                *in += strlen((char*)*in) + 1;
                *out = (char*)*out + LIBSERIAL_STRMAXLEN;
                break;
            default: break;
        }
        curr = curr->next;
    }
}

void __serialize(void** in, unsigned char** out, struct LibSerial_Object* obj) {
    struct LibSerial_Object* curr = obj;
    uint32_t arrlen;
    void* arr;
    while (curr) {
        switch (curr->type) {
            case LibSerial_VarType_Struct:
                __serialize(in, out, curr->inner);
                break;
            case LibSerial_VarType_Array:
                arr = READ(void*, *in);
                arrlen = libserial_arrsize(arr);
                WRITE(uint32_t, *out, arrlen);
                for (uint32_t i = 0; i < arrlen; i++) {
                    __serialize(&arr, out, curr->inner);
                }
                break;
            case LibSerial_VarType_Int8:
            case LibSerial_VarType_UInt8:
                WRITE(int8_t, *out, READ(int8_t, *in));
                break;
            case LibSerial_VarType_Int16:
            case LibSerial_VarType_UInt16:
                WRITE(int16_t, *out, READ(int16_t, *in));
                break;
            case LibSerial_VarType_Int32:
            case LibSerial_VarType_UInt32:
            case LibSerial_VarType_Float:
                WRITE(int32_t, *out, READ(int32_t, *in));
                break;
            case LibSerial_VarType_Int64:
            case LibSerial_VarType_UInt64:
            case LibSerial_VarType_Double:
                WRITE(int64_t, *out, READ(int64_t, *in));
                break;
            case LibSerial_VarType_String:
                strcpy((char*)*out, (char*)*in);
                *out += strlen((char*)*out) + 1;
                *in = (char*)*in + LIBSERIAL_STRMAXLEN;
                break;
            default: break;
        }
        curr = curr->next;
    }
}

size_t __calcsiz(void** data, struct LibSerial_Object* obj) {
    size_t size = 0;
    struct LibSerial_Object* curr = obj;
    uint32_t arrlen;
    void* arr;
    while (curr) {
        switch (curr->type) {
            case LibSerial_VarType_Struct:
                size += __calcsiz(data, curr->inner);
                break;
            case LibSerial_VarType_Array:
                size += 4;
                arr = READ(void*, *data);
                arrlen = libserial_arrsize(arr);
                for (uint32_t i = 0; i < arrlen; i++) {
                    size += __calcsiz(&arr, curr->inner);
                }
                break;
            case LibSerial_VarType_Int8:
            case LibSerial_VarType_UInt8:
                size += 1;
                READ(int8_t, *data);
                break;
            case LibSerial_VarType_Int16:
            case LibSerial_VarType_UInt16:
                size += 2;
                READ(int16_t, *data);
                break;
            case LibSerial_VarType_Int32:
            case LibSerial_VarType_UInt32:
            case LibSerial_VarType_Float:
                size += 4;
                READ(int32_t, *data);
                break;
            case LibSerial_VarType_Int64:
            case LibSerial_VarType_UInt64:
            case LibSerial_VarType_Double:
                size += 8;
                READ(int64_t, *data);
                break;
            case LibSerial_VarType_String:
                size += strlen((char*)*data) + 1;
                *data = (char*)*data + LIBSERIAL_STRMAXLEN;
                break;
            default: break;
        }
        curr = curr->next;
    }
    return size;
}

void __free(void** data, struct LibSerial_Object* obj) {
    struct LibSerial_Object* curr = obj;
    uint32_t arrlen;
    void *arr, *_arr;
    while (curr) {
        switch (curr->type) {
            case LibSerial_VarType_Struct:
                __free(data, curr->inner);
                break;
            case LibSerial_VarType_Array:
                arr = _arr = READ(void*, *data);
                arrlen = libserial_arrsize(arr);
                for (uint32_t i = 0; i < arrlen; i++) {
                    __free(&arr, curr->inner);
                }
                free((size_t*)_arr - 1);
                break;
            case LibSerial_VarType_Int8:
            case LibSerial_VarType_UInt8:
                READ(int8_t, *data);
                break;
            case LibSerial_VarType_Int16:
            case LibSerial_VarType_UInt16:
                READ(int16_t, *data);
                break;
            case LibSerial_VarType_Int32:
            case LibSerial_VarType_UInt32:
            case LibSerial_VarType_Float:
                READ(int32_t, *data);
                break;
            case LibSerial_VarType_Int64:
            case LibSerial_VarType_UInt64:
            case LibSerial_VarType_Double:
                READ(int64_t, *data);
                break;
            case LibSerial_VarType_String:
                *data = (char*)*data + LIBSERIAL_STRMAXLEN;
                break;
            default: break;
        }
            
        curr = curr->next;
    }
}

)

#undef OBJECT

#ifdef LIBSERIAL_SOURCE
#define OBJECT(name, type) {                                                   \
    struct LibSerial_Object* current = zalloc(sizeof(struct LibSerial_Object)); \
    __libserialobjs[LibSerial_ObjType_##name] = current;                         \
    type                                                                          \
}
#else
#define OBJECT(name, type)
#endif
#define _(x)

#define NODE(id, ...) {                                                        \
    if (current->type != LibSerial_VarType_None) {                              \
        struct LibSerial_Object* next = zalloc(sizeof(struct LibSerial_Object)); \
        current->next = next;                                                     \
        current = next;                                                            \
    }                                                                               \
    current->type = id;                                                              \
    __VA_ARGS__                                                                       \
}

#define CONTAINER(definition)                                                \
    struct LibSerial_Object* child = zalloc(sizeof(struct LibSerial_Object)); \
    child->parent = current;                                                   \
    current->inner = child;                                                     \
    current = child;                                                             \
    definition                                                                    \
    current = child->parent;

#define STRUCT(definition) NODE(LibSerial_VarType_Struct, CONTAINER(definition))
#define ARRAY( type      ) NODE(LibSerial_VarType_Array,  CONTAINER(type      ))
#define INT8               NODE(LibSerial_VarType_Int8                         )
#define INT16              NODE(LibSerial_VarType_Int16                        )
#define INT32              NODE(LibSerial_VarType_Int32                        )
#define INT64              NODE(LibSerial_VarType_Int64                        )
#define UINT8              NODE(LibSerial_VarType_UInt8                        )
#define UINT16             NODE(LibSerial_VarType_UInt16                       )
#define UINT32             NODE(LibSerial_VarType_UInt32                       )
#define UINT64             NODE(LibSerial_VarType_UInt64                       )
#define FLOAT              NODE(LibSerial_VarType_Float                        )
#define DOUBLE             NODE(LibSerial_VarType_Double                       )
#define STRING             NODE(LibSerial_VarType_String                       )

void libserial_init() SOURCE({
    struct LibSerial_Object* current;
    (void)current;
)
#include LIBSERIAL_INCLUDE
SOURCE(}) HEADER(;)

#undef NODE
#undef CONTAINER
#undef OBJECT
#undef STRUCT
#undef UNION
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef UINT8
#undef UINT16
#undef UINT32
#undef UINT64
#undef FLOAT
#undef DOUBLE
#undef STRING
#undef ARRAY
#undef _

#ifdef LIBSERIAL_SOURCE
#define OBJECT(name, type) __freeobj(__libserialobjs[LibSerial_ObjType_##name]);
#else
#define OBJECT(name, type)
#endif

void libserial_deinit() SOURCE({)
#include LIBSERIAL_INCLUDE
SOURCE(}) HEADER(;)

#undef OBJECT

void* libserial_deserialize(unsigned char* in) SOURCE({
    uint32_t type = READ(uint32_t, in);
    void* obj = __libserial_objmake(type);
    void* _obj = obj;
    __deserialize(&_obj, &in, __libserialobjs[type]);
    return obj;
}) HEADER(;)

unsigned char* libserial_serialize(void* obj, size_t* outsize) SOURCE({
    uint32_t type = libserial_objtype(obj);
    void* _obj = obj;
    struct LibSerial_Object* object_meta = __libserialobjs[type];
    size_t size = __calcsiz(&_obj, object_meta) + 4;
    unsigned char* _data;
    unsigned char* data = _data = zalloc(size);
    WRITE(uint32_t, _data, type);
    __serialize(&obj, &_data, object_meta);
    if (outsize) *outsize = size;
    return data;
}) HEADER(;)

void libserial_objfree(void* obj) SOURCE({
    uint32_t type = libserial_objtype(obj);
    void* _obj = obj;
    __free(&_obj, __libserialobjs[type]);
    free((enum LibSerial_ObjTypes*)obj - 1);
}) HEADER(;)

#undef READ
#undef WRITE
#undef OBJECT
#undef HEADER
#undef SOURCE

#endif
