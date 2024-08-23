#ifndef BTCB_SAVEFILE_H
#define BTCB_SAVEFILE_H

#include <stdint.h>
typedef          _BitInt(256) int256_t;
typedef unsigned _BitInt(256) uint256_t;

#define LEVEL_FLAG_CATCOIN1   (1 << 0)
#define LEVEL_FLAG_CATCOIN2   (1 << 1)
#define LEVEL_FLAG_CATCOIN3   (1 << 2)
#define LEVEL_FLAG_EXIT_UP    (1 << 3)
#define LEVEL_FLAG_EXIT_LEFT  (1 << 4)
#define LEVEL_FLAG_EXIT_DOWN  (1 << 5)
#define LEVEL_FLAG_EXIT_RIGHT (1 << 6)

#define NUM_SAVEFILES 4

struct SaveFile {
    uint8_t levels_completed;
    uint8_t coins;
    uint8_t lives;
    uint8_t map_x;
    uint8_t map_y;
    uint8_t map_id;
    uint8_t padding[10];
    uint8_t level_flags[256];
    uint256_t map_events;
};

extern struct SaveFile* savefile;

void savefile_load();
void savefile_select(int file);
void savefile_erase(int file);
void savefile_copy(int from, int to);
void savefile_save();
struct SaveFile* savefile_get(int file);

#endif