#ifndef BTCB_DATA_H
#define BTCB_DATA_H

#include <lunarengine.h>

#define get_entity_builder_by_id(id) get_entity_builder(ENTITY_BUILDER_##id)
#define get_tile_data_by_id(     id) get_tile_data     (     TILE_DATA_##id)
#define get_tileset_by_id(       id) get_tileset       (       TILESET_##id)

#define ENTITY( id, _1) ,ENTITY_BUILDER_##id
#define TILESET(id, _1) ,       TILESET_##id
#define TILE(   id, _1) ,     TILE_DATA_##id

#define ENUM(value) = value

#ifndef NO_VSCODE
#define NO_VSCODE
#endif

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

LE_EntityBuilder* get_entity_builder(enum EntityBuilderIDs id);
LE_TileData*      get_tile_data     (enum      TileDataIDs id);
LE_Tileset*       get_tileset       (enum       TilesetIDs id);

#undef ENTITY
#undef TILESET
#undef TILE
#undef ENUM
#undef NO_VSCODE

#endif