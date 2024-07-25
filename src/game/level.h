#ifndef SMBR_LEVEL_H
#define SMBR_LEVEL_H

#include <lunarengine.h>

#include "assets/assets.h"

struct Warp {
    int next_theme;
    int next_music;
    int next_cambound;
    int next_level;
    int next_layer;
    float next_pos_x;
    float next_pos_y;  
};

struct Level {
    int num_cambounds;
    int num_warps;
    struct CameraBounds** cambounds;
    struct Warp* warps;
    LE_LayerList* layers;
    unsigned char* raw;
};

extern struct Level* current_level;
extern uint64_t global_timer;

void load_level(struct Binary* data);
void load_level_impl(const unsigned char* data);
struct Level* get_current_level();
void update_level();

#endif