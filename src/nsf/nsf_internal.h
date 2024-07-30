#ifndef SMBR_NSF_INTERNAL_H
#define SMBR_NSF_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "cpu.h"

#define NTSC 0
#define PAL  1
#define MASK_NTSC (1 << NTSC)
#define MASK_PAL  (1 << PAL)

#define VRC6 0
#define VRC7 1
#define FDS  2
#define MMC5 3
#define N163 4
#define SS5B 5
#define VT02 6
#define MASK_VRC6 (1 << VRC6)
#define MASK_VRC7 (1 << VRC7)
#define MASK_FDS  (1 << FDS )
#define MASK_MMC5 (1 << MMC5)
#define MASK_N163 (1 << N163)
#define MASK_SS5B (1 << SS5B)
#define MASK_VT02 (1 << VT02)

#define FALSE 0
#define TRUE  1
#define UNSET 2

struct NSFHandle {
    int num_songs;
    int cur_song;
    bool playing, inited;
    uint8_t format_flags;
    uint8_t sound_chip_flags;
    uint16_t ntsc_play_speed, pal_play_speed;
    uint16_t load_addr, init_addr, play_addr;
    uint32_t program_length;
    char uses_bank_switching;
    unsigned char bank_switch_values[8];
    unsigned char* program;
    NES_CPU cpu;
    pthread_t cpu_thread_id;
};

#endif