#ifndef SMBR_DATA_H
#define SMBR_DATA_H

#include <lunarengine.h>

#define get_entity_builder(id) num_get_entity_builder(ENTITY_BUILDER_##id)
#define get_tile_data(     id) num_get_tile_data     (     TILE_DATA_##id)
#define get_tileset(       id) num_get_tileset       (       TILESET_##id)

#define ENTITY( id, _1) ,ENTITY_BUILDER_##id
#define TILESET(id, _1) ,       TILESET_##id
#define TILE(   id, _1) ,     TILE_DATA_##id

#define ENUM(value) = value

#define NO_VSCODE

enum EntityBuilderIDs {
    __ENTITY_BUILDER = -1
#include "game/data/entities.h"
};

enum TileDataIDs {
    __TILE_DATA = -1
#include "game/data/tiles.h"
};

enum TilesetIDs {
    __TILESET = -1
#include "game/data/tilesets.h"
};

void init_data();
LE_EntityBuilder* num_get_entity_builder(int id);
LE_TileData*      num_get_tile_data     (int id);
LE_Tileset*       num_get_tileset       (int id);

#undef ENTITY
#undef TILESET
#undef TILE
#undef ENUM
#undef NO_VSCODE

#endif