#ifndef BTCB_SAVEFILE_H
#define BTCB_SAVEFILE_H

#include <stdint.h>
#include <stdbool.h>

#define LEVEL_FLAG_CATCOIN1   (1 << 0)
#define LEVEL_FLAG_CATCOIN2   (1 << 1)
#define LEVEL_FLAG_CATCOIN3   (1 << 2)
#define LEVEL_FLAG_EXIT_UP    (1 << 3)
#define LEVEL_FLAG_EXIT_LEFT  (1 << 4)
#define LEVEL_FLAG_EXIT_DOWN  (1 << 5)
#define LEVEL_FLAG_EXIT_RIGHT (1 << 6)

#define NUM_SAVEFILES 4

typedef uint8_t uint256_t[32];

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

bool savefile_map_event(int index);
void savefile_map_event_set(int index);
void savefile_map_event_clear(int index);

void menubtn_file_select(int selected_index);
void menubtn_file_copy(int selected_index);
void menubtn_file_erase(int selected_index);
void menubtn_file_cancel(int selected_index);

#endif