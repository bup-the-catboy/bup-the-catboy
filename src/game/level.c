#include "level.h"

#include <stdlib.h>
#include <lunarengine.h>

#include "assets/binary_reader.h"
#include "assets/sound.h"
#include "camera.h"
#include "data.h"

#define NO_VSCODE
#define MUSIC(_1, _2) _2,
const char* music_table[] = {
#include "game/data/music.h"
};

struct Level* current_level = NULL;

void destroy_level(struct Level* level) {
    for (int i = 0; i < level->num_cambounds; i++) {
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
    free(level);
}

void load_level(struct Binary* binary) {
    load_level_impl(binary->ptr);
}

void load_level_impl(const unsigned char* data) {
    if (current_level) destroy_level(current_level);
    struct Level* level = malloc(sizeof(struct Level));
    struct BinaryStream* stream = binary_stream_create(data);

    stream = binary_stream_goto(stream);
    unsigned int theme, music, cambound;
    BINARY_STREAM_READ(stream, theme);
    BINARY_STREAM_READ(stream, music);
    BINARY_STREAM_READ(stream, cambound);
    stream = binary_stream_close(stream);

    stream = binary_stream_goto(stream);
    BINARY_STREAM_READ(stream, level->num_cambounds);
    level->cambounds = malloc(sizeof(struct CameraBounds*) * level->num_cambounds);
    for (unsigned int i = 0; i < level->num_cambounds; i++) {
        stream = binary_stream_goto(stream);
        unsigned int num_nodes, num_edges;
        BINARY_STREAM_READ(stream, num_nodes);
        BINARY_STREAM_READ(stream, num_edges);
        struct CameraBounds* nodes = malloc(sizeof(struct CameraBounds) * num_nodes);
        for (unsigned int j = 0; j < num_nodes; j++) {
            BINARY_STREAM_READ(stream, nodes[j].x);
            BINARY_STREAM_READ(stream, nodes[j].y);
        }
        for (unsigned int j = 0; j < num_edges; j++) {
            unsigned int a, b;
            BINARY_STREAM_READ(stream, a);
            BINARY_STREAM_READ(stream, b);
            if (nodes[a].x == nodes[b].x && nodes[a].y == nodes[b].y - 1) {
                nodes[a].up = &nodes[b];
                nodes[b].down = &nodes[a];
            }
            if (nodes[a].x == nodes[b].x - 1 && nodes[a].y == nodes[b].y) {
                nodes[a].left = &nodes[b];
                nodes[b].right = &nodes[a];
            }
            if (nodes[a].x == nodes[b].x && nodes[a].y == nodes[b].y + 1) {
                nodes[a].down = &nodes[b];
                nodes[b].up = &nodes[a];
            }
            if (nodes[a].x == nodes[b].x + 1 && nodes[a].y == nodes[b].y) {
                nodes[a].right = &nodes[b];
                nodes[b].left = &nodes[a];
            }
        }
        level->cambounds[i] = nodes;
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
    LE_LayerList* layers = LE_CreateLayerList();
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
        case 0: { // tilemap
            unsigned int w, h;
            BINARY_STREAM_READ(stream, w);
            BINARY_STREAM_READ(stream, h);
            w *= 24; h *= 16;
            LE_Tilemap* tilemap = LE_CreateTilemap(w, h);
            for (unsigned int y = 0; y < h; y++) {
                for (unsigned int x = 0; x < w; x++) {
                    unsigned char tile;
                    BINARY_STREAM_READ(stream, tile);
                    LE_TilemapSetTile(tilemap, x, y, tile);
                }
            }
            LE_TilemapSetTileset(tilemap, num_get_tileset(theme));
            layer = LE_AddTilemapLayer(layers, tilemap);
        } break;
        case 1: { // entities
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
                LE_Entity* entity = LE_CreateEntity(el, num_get_entity_builder(entityID), x, y);
                unsigned int num_properties;
                BINARY_STREAM_READ(stream, num_properties);
                for (unsigned int i = 0; i < num_properties; i++) {
                    char name[256];
                    binary_stream_read_string(stream, name, 256);
                    unsigned int value;
                    BINARY_STREAM_READ(stream, value);
                    LE_EntityProperty property;
                    property.asInt = value;
                    LE_EntitySetProperty(entity, property, name);
                }
                stream = binary_stream_close(stream);
            }
            layer = LE_AddEntityLayer(layers, el);
        } break; }
        layer->scrollSpeedX = smx;
        layer->scrollSpeedY = smy;
        layer->scrollOffsetX = sox;
        layer->scrollOffsetY = soy;
        layer->scaleW = scx;
        layer->scaleH = scy;
        stream = binary_stream_close(stream);
        stream = binary_stream_close(stream);
    }
    LE_LayerListIter* iter = LE_LayerListGetIter(layers);
    while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        enum LE_LayerType type = LE_LayerGetType(layer);
        iter = LE_LayerListNext(iter);
    }
    stream = binary_stream_close(stream);

    camera_set_bounds(level->cambounds[cambound]);
    play_music(GET_ASSET(struct Audio, music_table[music]));

    current_level = level;
}
