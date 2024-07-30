#ifndef SMBR_NSF_CPU_H
#define SMBR_NSF_CPU_H

#include <stdint.h>

#include "nsf.h"

typedef struct {
    int8_t* ram;
    int8_t a, x, y;
    uint8_t sp;
    uint8_t flags;
    uint16_t pc;
    uint8_t banks[8];
} NES_CPU;

void cpu_init(NSFHandle* handle);
void cpu_dispose(NSFHandle* handle);

#endif