#ifndef BTCB_LEVEL_H
#define BTCB_LEVEL_H

#include <lunarengine.h>

#include "io/assets/assets.h"
#include "game/camera.h"
#include "main.h"

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
    CameraBounds** cambounds;
    struct Warp* warps;
    LE_LayerList* layers;
    unsigned char* raw;
    int raw_length;
    unsigned int default_cambound, default_music, default_theme;
};

extern struct Level* current_level;
extern uint8_t curr_level_id;
extern Camera* camera;

void load_level(struct Binary* data);
void load_level_impl(unsigned char* data, int datalen);
void change_level_music(int track);
void reload_level();
void update_level(float delta_time);
void render_level(LE_DrawList* drawlist, int width, int height, float interpolation);
uint32_t get_unique_entity_id();

#endif