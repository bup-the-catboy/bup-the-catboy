#include "level.h"

#include <stdlib.h>
#include <lunarengine.h>
#include <string.h>

#include "io/assets/binary_reader.h"
#include "io/audio/nsf.h"
#include "io/audio/audio.h"
#include "camera.h"
#include "data.h"
#include "overlay/manager.h"
#include "overlay/transition.h"
#include "game/entities/functions.h"
#include "threadlock.h"

struct AudioInstance* music_instance;
struct Level* current_level = NULL;
struct Warp* current_warp;
uint8_t curr_level_id = 0;
uint32_t unique_entity_id = 2;
Camera* camera;
int curr_audio_track = -1;
int curr_theme = -1;
int pause_state = UNPAUSED;

struct PostUpdate {
    void(*func)(void*);
    void *user_data;
    int timer;
    struct PostUpdate* next;
};

struct PostUpdate *post_update_list, *post_update_list_head;

int find_tileset_id(LE_Tileset* tileset) {
    for (int i = 0; i < TILESET_TYPE_COUNT; i++) {
        if (get_theme(curr_theme)[i] == tileset) return i;
    }
    return -1;
}

void change_level_music(int track) {
    if (curr_audio_track == track) return;
    curr_audio_track = track;
    struct Audio* nsf = GET_ASSET(struct Audio, "audio/music.nsf");
    if (music_instance) audio_stop(music_instance);
    audio_nsf_select_track(nsf, track);
    music_instance = audio_play(nsf);
}

void change_level_theme(int theme) {
    if (curr_theme == theme) return;
    LE_LayerListIter* iter = LE_LayerListGetIter(current_level->layers);
    while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        if (LE_LayerGetType(layer) == LE_LayerType_Tilemap) {
            LE_Tilemap* tilemap = LE_LayerGetDataPointer(layer);
            LE_Tileset* tileset = LE_TilemapGetTileset(tilemap);
            LE_TilemapSetTileset(tilemap, get_theme(theme)[find_tileset_id(tileset)]);
        }
        iter = LE_LayerListNext(iter);
    }
    curr_theme = theme;
}

void destroy_level(struct Level* level) {
    for (int i = 0; i < level->num_cambounds; i++) {
        free(level->cambounds[i]->poly);
        free(level->cambounds[i]);
    }
    free(level->cambounds);
    free(level->warps);
    LE_LayerListIter* iter = LE_LayerListGetIter(level->layers);
    while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        switch (LE_LayerGetType(layer)) {
            case LE_LayerType_Entity:
                LE_DestroyEntityList(LE_LayerGetDataPointer(layer));
                break;
            case LE_LayerType_Tilemap:
                LE_DestroyTilemap(LE_LayerGetDataPointer(layer));
                break;
            default:
                break;
        }
        iter = LE_LayerListNext(iter);
    }
    LE_DestroyLayerList(level->layers);
    free(level->raw);
    free(level);
    free(camera);
    camera = NULL;
}

uint32_t get_unique_entity_id() {
    return unique_entity_id++;
}

struct Level* parse_level(unsigned char* data, int datalen) {
    struct Level* level = malloc(sizeof(struct Level));
    struct BinaryStream* stream = binary_stream_create(data);

    level->raw = malloc(datalen);
    level->raw_length = datalen;
    memcpy(level->raw, data, datalen);

    stream = binary_stream_goto(stream);
    BINARY_STREAM_READ(stream, level->default_theme);
    BINARY_STREAM_READ(stream, level->default_music);
    BINARY_STREAM_READ(stream, level->default_cambound);
    stream = binary_stream_close(stream);

    stream = binary_stream_goto(stream);
    BINARY_STREAM_READ(stream, level->num_cambounds);
    level->cambounds = malloc(sizeof(CameraBounds*) * level->num_cambounds);
    for (unsigned int i = 0; i < level->num_cambounds; i++) {
        stream = binary_stream_goto(stream);
        CameraBounds* bounds = malloc(sizeof(CameraBounds));
        BINARY_STREAM_READ(stream, bounds->num_vert);
        bounds->poly = malloc(sizeof(Point) * bounds->num_vert);
        for (unsigned int j = 0; j < bounds->num_vert; j++) {
            BINARY_STREAM_READ(stream, bounds->poly[j].x);
            BINARY_STREAM_READ(stream, bounds->poly[j].y);
        }
        level->cambounds[i] = bounds;
        stream = binary_stream_close(stream);
    }
    stream = binary_stream_close(stream);

    stream = binary_stream_goto(stream);
    BINARY_STREAM_READ(stream, level->num_warps);
    level->warps = malloc(sizeof(struct Warp) * level->num_warps);
    for (int i = 0; i < level->num_warps; i++) {
        stream = binary_stream_goto(stream);
        BINARY_STREAM_READ(stream, level->warps[i].next_theme);
        BINARY_STREAM_READ(stream, level->warps[i].next_music);
        BINARY_STREAM_READ(stream, level->warps[i].next_cambound);
        BINARY_STREAM_READ(stream, level->warps[i].next_level);
        BINARY_STREAM_READ(stream, level->warps[i].next_layer);
        BINARY_STREAM_READ(stream, level->warps[i].next_pos_x);
        BINARY_STREAM_READ(stream, level->warps[i].next_pos_y);
        stream = binary_stream_close(stream);
    }
    stream = binary_stream_close(stream);

    stream = binary_stream_goto(stream);
    unsigned int num_layers;
    BINARY_STREAM_READ(stream, num_layers);
    level->layers = LE_CreateLayerList();
    LE_AddCustomLayer(level->layers, layer_overlay, NULL);
    for (int i = 0; i < num_layers; i++) {
        stream = binary_stream_goto(stream);
        unsigned int type;
        float smx, smy, sox, soy, scx, scy;
        BINARY_STREAM_READ(stream, type);
        BINARY_STREAM_READ(stream, smx);
        BINARY_STREAM_READ(stream, smy);
        BINARY_STREAM_READ(stream, sox);
        BINARY_STREAM_READ(stream, soy);
        BINARY_STREAM_READ(stream, scx);
        BINARY_STREAM_READ(stream, scy);
        LE_Layer* layer;
        stream = binary_stream_goto(stream);
        switch (type) {
        case LE_LayerType_Tilemap: {
            int tileset;
            unsigned int w, h;
            BINARY_STREAM_READ(stream, tileset);
            BINARY_STREAM_READ(stream, w);
            BINARY_STREAM_READ(stream, h);
            LE_Tilemap* tilemap = LE_CreateTilemap(w, h);
            for (unsigned int y = 0; y < h; y++) {
                for (unsigned int x = 0; x < w; x++) {
                    unsigned char tile;
                    BINARY_STREAM_READ(stream, tile);
                    LE_TilemapSetTile(tilemap, x, y, tile);
                }
            }
            LE_TilemapSetTileset(tilemap, get_theme(level->default_theme)[tileset]);
            layer = LE_AddTilemapLayer(level->layers, tilemap);
        } break;
        case LE_LayerType_Entity: {
            LE_EntityList* el = LE_CreateEntityList();
            unsigned int tilemap, num_entities;
            BINARY_STREAM_READ(stream, tilemap);
            BINARY_STREAM_READ(stream, num_entities);
            LE_EntityAssignTilemap(el, (LE_Tilemap*)(uintptr_t)tilemap);
            for (unsigned int j = 0; j < num_entities; j++) {
                stream = binary_stream_goto(stream);
                unsigned char entityID;
                BINARY_STREAM_READ(stream, entityID);
                binary_stream_skip(stream, 3);
                float x, y;
                BINARY_STREAM_READ(stream, x);
                BINARY_STREAM_READ(stream, y);
                LE_Entity* entity = LE_CreateEntity(el, get_entity_builder(entityID), x, y);
                unsigned int num_properties;
                BINARY_STREAM_READ(stream, num_properties);
                for (unsigned int i = 0; i < num_properties; i++) {
                    stream = binary_stream_goto(stream);
                    char name[256];
                    uint8_t type;
                    binary_stream_read_string(stream, name, 256);
                    BINARY_STREAM_READ(stream, type);
                    LE_EntityProperty property;
                    switch (type) {
                        case EntityPropertyType_Int:   BINARY_STREAM_READ(stream, property.asInt);   break;
                        case EntityPropertyType_Bool:  BINARY_STREAM_READ(stream, property.asBool);  break;
                        case EntityPropertyType_Float: BINARY_STREAM_READ(stream, property.asFloat); break;
                        case EntityPropertyType_String:
                            property.asPtr = (void*)(stream->data + stream->ptr);
                            stream->ptr += strlen(property.asPtr) + 1;
                            break;
                    }
                    LE_EntitySetProperty(entity, property, name);
                    stream = binary_stream_close(stream);
                }
                stream = binary_stream_close(stream);
            }
            layer = LE_AddEntityLayer(level->layers, el);
        } break; }
        layer->scrollSpeedX = smx * 16;
        layer->scrollSpeedY = smy * 16;
        layer->scrollOffsetX = sox;
        layer->scrollOffsetY = soy;
        layer->scaleW = scx;
        layer->scaleH = scy;
        stream = binary_stream_close(stream);
        stream = binary_stream_close(stream);
    }
    stream = binary_stream_close(stream);

    LE_LayerListIter* iter = LE_LayerListGetIter(level->layers);
    while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        LE_LayerType type = LE_LayerGetType(layer);
        if (type == LE_LayerType_Entity) {
            LE_EntityList* entitylist = LE_LayerGetDataPointer(layer);
            int tilemap_index = (int)(uintptr_t)LE_EntityGetTilemap(entitylist);
            if (tilemap_index >= 0) {
                LE_Layer* tilemap = LE_LayerGetByIndex(level->layers, tilemap_index + 1);
                LE_EntityAssignTilemap(entitylist, LE_LayerGetDataPointer(tilemap));
                layer->scaleW *= tilemap->scaleW;
                layer->scaleH *= tilemap->scaleH;
            }
            else LE_EntityAssignTilemap(entitylist, NULL);
        }
        iter = LE_LayerListNext(iter);
    }
    return level;
}

void do_warp() {
    if (curr_level_id != current_warp->next_level) load_level_by_id(current_warp->next_level);
    change_level_music(current_warp->next_music);
    change_level_theme(current_warp->next_theme);
    camera_set_bounds(camera, current_level->cambounds[current_warp->next_cambound]);
    LE_Entity* player = find_entity_with_tag("player");
    player->posX = current_warp->next_pos_x;
    player->posY = current_warp->next_pos_y;
    LE_EntityChangeLists(player, LE_LayerGetDataPointer(LE_LayerGetByIndex(current_level->layers, current_warp->next_layer + 1)));
}

void activate_warp_no_transition(struct Warp* warp) {
    current_warp = warp;
    do_warp();
}

void activate_warp(struct Warp* warp, LE_Direction direction) {
    if (is_transition_active()) return;
    current_warp = warp;
    start_transition(do_warp, 60, direction, quad_in_out);
}

void load_level(struct Binary* binary) {
    load_level_impl(binary->ptr, binary->length);
}

void load_level_impl(unsigned char* data, int datalen) {
    threadlock_mutex_lock(THREADLOCK_LEVEL_UPDATE);
    if (current_level) destroy_level(current_level);
    unique_entity_id = 0;
    current_level = parse_level(data, datalen);
    curr_theme = current_level->default_theme;
    change_level_music(current_level->default_music);
    threadlock_mutex_unlock(THREADLOCK_LEVEL_UPDATE);
}

void load_level_by_id(int id) {
    curr_level_id = id;
    char level_name[32];
    snprintf(level_name, 31, "levels/level%d.lvl", curr_level_id);
    load_level(GET_ASSET(struct Binary, level_name));
}

void reload_level() {
    pause_state = UNPAUSED;
    unsigned char* leveldata = malloc(current_level->raw_length);
    memcpy(leveldata, current_level->raw, current_level->raw_length);
    load_level_impl(leveldata, current_level->raw_length);
    free(leveldata);
}

void set_pause_state(int state) {
    pause_state = state;
}

void init_post_update_list() {
    post_update_list = post_update_list_head = calloc(sizeof(struct PostUpdate), 1);
}

void process_post_update() {
    struct PostUpdate* curr = post_update_list;
    init_post_update_list();
    while (curr) {
        if (curr->timer > 0) post_update_timer(curr->func, curr->user_data, curr->timer - 1);
        else if (curr->func) curr->func(curr->user_data);
        struct PostUpdate* next = curr->next;
        free(curr);
        curr = next;
    }
}

void post_update(void(*func)(void*), void* user_data) {
    post_update_timer(func, user_data, 0);
}

void post_update_timer(void(*func)(void*), void* user_data, int timer) {
    post_update_list_head->func = func;
    post_update_list_head->user_data = user_data;
    post_update_list_head->timer = timer;
    post_update_list_head->next = calloc(sizeof(struct PostUpdate), 1);
    post_update_list_head = post_update_list_head->next;
}

void update_level(float delta_time) {
    threadlock_mutex_lock(THREADLOCK_LEVEL_UPDATE);
    if (!post_update_list) init_post_update_list();
    LE_UpdateLayerList(current_level->layers);
    LE_LayerListIter* iter = LE_LayerListGetIter(current_level->layers);
    if (!(pause_state & PAUSE_FLAG_NO_UPDATE_ENTITIES)) while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        LE_LayerType type = LE_LayerGetType(layer);
        if (type == LE_LayerType_Entity) LE_UpdateEntities(LE_LayerGetDataPointer(layer), delta_time);
        iter = LE_LayerListNext(iter);
    }
    if (!(pause_state & PAUSE_FLAG_NO_UPDATE_CAMERA)) {
        if (camera) {
            float x, y;
            camera_update(camera);
            camera_get(camera, &x, &y);
            LE_ScrollCamera(current_level->layers, x, y);
        }
        else LE_ScrollCamera(current_level->layers, 12, 8);
    }
    threadlock_mutex_unlock(THREADLOCK_LEVEL_UPDATE);
    process_post_update();
}

void render_level(LE_DrawList* drawlist, int width, int height, float interpolation) {
    threadlock_mutex_lock(THREADLOCK_LEVEL_UPDATE);
    LE_Draw(current_level->layers, width, height, interpolation, drawlist);
    threadlock_mutex_unlock(THREADLOCK_LEVEL_UPDATE);
}
