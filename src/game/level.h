#ifndef BTCB_LEVEL_H
#define BTCB_LEVEL_H

#include <lunarengine.h>

#include "io/assets/assets.h"
#include "game/camera.h"
#include "main.h"

#define PAUSE_FLAG_NO_UPDATE_CAMERA   (1<<0)
#define PAUSE_FLAG_NO_UPDATE_ENTITIES (1<<1)
#define PAUSE_FLAG_USER_PAUSED        (1<<2)
#define UNPAUSED 0

#define LVLID_TITLE 100
#define LVLID_WORLDMAP 101

enum EntityPropertyType {
    EntityPropertyType_Int,
    EntityPropertyType_Bool,
    EntityPropertyType_Float,
    EntityPropertyType_String,
};

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
void load_level_by_id(int id);
void load_world_map_with_node(int id, int node);
void change_level_music(int track);
void change_level_theme(int theme);
void activate_warp_no_transition(struct Warp* warp);
void activate_warp(struct Warp* warp, LE_Direction direction);
void reload_level();
void set_pause_state(int pause_state);
void update_level(float delta_time);
void render_level(LE_DrawList* drawlist, int width, int height, float interpolation);
void post_update(void(*func)(void*), void* user_data);
void post_update_timer(void(*func)(void*), void* user_data, int timer);
uint32_t get_unique_entity_id();

#endif