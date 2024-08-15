#include "level.h"

#include <stdlib.h>
#include <lunarengine.h>

#include "assets/binary_reader.h"
#include "audio/nsf.h"
#include "audio/audio.h"
#include "camera.h"
#include "data.h"
#include "input.h"

struct AudioInstance* music_instance;
struct Level* current_level = NULL;
uint64_t global_timer = 0;
uint32_t unique_entity_id = 0;

void change_level_music(int track) {
    struct Audio* nsf = GET_ASSET(struct Audio, "audio/music.nsf");
    if (music_instance) {
        audio_stop(music_instance);
    }
    audio_nsf_select_track(nsf, track);
    music_instance = audio_play(nsf);
}

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

uint32_t get_unique_entity_id() {
    return unique_entity_id++;
}

struct Level* parse_level(unsigned char* data, unsigned int* ptheme, unsigned int* pmusic, unsigned int* pcambound, int datalen) {
    struct Level* level = malloc(sizeof(struct Level));
    struct BinaryStream* stream = binary_stream_create(data);

    level->raw = data;
    level->raw_length = datalen;

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
    level->layers = LE_CreateLayerList();
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
        int scale_x = 1, scale_y = 1;
        switch (type) {
        case LE_LayerType_Tilemap: {
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
            LE_Tileset* tileset = get_tileset(theme);
            LE_TilemapSetTileset(tilemap, tileset);
            LE_TilesetGetTileSize(tileset, &scale_x, &scale_y);
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
                // theres a super low chance that entities will have the same unique id but SHHHHH
                LE_EntitySetProperty(entity, (LE_EntityProperty){ .asInt = get_unique_entity_id() }, "unique_id");
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
            layer = LE_AddEntityLayer(level->layers, el);
        } break; }
        layer->scrollSpeedX = smx;
        layer->scrollSpeedY = smy;
        layer->scrollOffsetX = sox;
        layer->scrollOffsetY = soy;
        layer->scaleW = scx * scale_x;
        layer->scaleH = scy * scale_y;
        stream = binary_stream_close(stream);
        stream = binary_stream_close(stream);
    }
    stream = binary_stream_close(stream);

    LE_LayerListIter* iter = LE_LayerListGetIter(level->layers);
    while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        enum LE_LayerType type = LE_LayerGetType(layer);
        if (type == LE_LayerType_Entity) {
            LE_EntityList* entitylist = LE_LayerGetDataPointer(layer);
            void* tilemap_index = LE_EntityGetTilemap(entitylist);
            LE_Layer* tilemap = LE_LayerGetByIndex(level->layers, (int)(uintptr_t)tilemap_index);
            LE_EntityAssignTilemap(entitylist, LE_LayerGetDataPointer(tilemap));
            layer->scaleW *= tilemap->scaleW;
            layer->scaleH *= tilemap->scaleH;
        }
        iter = LE_LayerListNext(iter);
    }

    if (ptheme) *ptheme = theme;
    if (pmusic) *pmusic = music;
    if (pcambound) *pcambound = cambound;

    return level;
}

void load_level(struct Binary* binary) {
    load_level_impl(binary->ptr, binary->length);
}

void load_level_impl(unsigned char* data, int datalen) {
    if (current_level) destroy_level(current_level);

    unique_entity_id = 0;

    unsigned int theme, music, cambound;
    current_level = parse_level(data, &theme, &music, &cambound, datalen);

    LE_EntityList* list = LE_LayerGetDataPointer(LE_LayerGetByIndex(current_level->layers, 1));
    LE_Entity*  player = LE_CreateEntity(list, get_entity_builder_by_id(        player), 1.5, 14);
    LE_Entity* nplayer = LE_CreateEntity(list, get_entity_builder_by_id(network_player), 1.5, 14);
    LE_EntitySetProperty( player, (LE_EntityProperty){ .asInt = get_unique_entity_id() }, "unique_id");
    LE_EntitySetProperty(nplayer, (LE_EntityProperty){ .asInt = get_unique_entity_id() }, "unique_id");

    if (cambound >= 0 && cambound < current_level->num_cambounds) camera_set_bounds(current_level->cambounds[cambound]);
    change_level_music(music);

    camera_set_focus(12, 8);
    camera_snap();
}

void reload_level() {
    unsigned char* leveldata = malloc(current_level->raw_length);
    memcpy(leveldata, current_level->raw, current_level->raw_length);
    load_level_impl(leveldata, current_level->raw_length);
    free(leveldata);
}

void update_level() {
    global_timer++;
    LE_LayerListIter* iter = LE_LayerListGetIter(current_level->layers);
    while (iter) {
        LE_Layer* layer = LE_LayerListGet(iter);
        enum LE_LayerType type = LE_LayerGetType(layer);
        if (type == LE_LayerType_Entity) LE_UpdateEntities(LE_LayerGetDataPointer(layer));
        iter = LE_LayerListNext(iter);
    }
    float x, y;
    camera_update();
    camera_get(&x, &y);
    LE_ScrollCamera(current_level->layers, x, y);
}
