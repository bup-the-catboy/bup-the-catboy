#ifndef BTCB_DATA_H
#define BTCB_DATA_H

#include <lunarengine.h>

#define get_entity_builder_by_id(id) get_entity_builder(ENTITY_BUILDER_##id)
#define get_tile_palette_by_id(  id) get_tile_palette  (  TILE_PALETTE_##id)
#define get_tileset_by_id(       id) get_tileset       (       TILESET_##id)
#define get_theme_by_id(         id) get_theme         (         THEME_##id)
#define get_powerup_by_id(       id) get_powerup       (       POWERUP_##id)
#define _tile(                   id)                   [     TILE_DATA_##id]

#define POWERUP(     id, _1, _2) ,       POWERUP_##id
#define TILESET(     id, _1)     ,       TILESET_##id
#define TILE(        id, _1)     ,     TILE_DATA_##id
#define THEME(       id, _1)     ,         THEME_##id
#define ENTITY(      id, _1)     ,ENTITY_BUILDER_##id
#define TILE_PALETTE(id, _1)     ,__TILE_PALETTE_##id = -1 _1
#define TILESET_TYPE(id    )    TILESET_TYPE_##id,

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

enum ThemeIDs  {
    __THEME = -1
#include "game/data/themes.h"
};

enum TilesetTypes {
#include "game/data/tileset_types.h"
    TILESET_TYPE_COUNT
};

#undef TILE_PALETTE
#define TILE_PALETTE(id, _1) ,TILE_PALETTE_##id

enum TilePaletteIDs {
    __TILE_PALETTE = -1
#include "game/data/tiles.h"
};

enum PowerupIDs {
    hurt = -1
#include "game/data/powerup.h"
};

struct Powerup {
    struct Powerup* parent;
    bool(*callback)(LE_Entity*);
    int color;
};

void init_data();

LE_EntityBuilder* get_entity_builder(enum EntityBuilderIDs id);
LE_TileData**     get_tile_palette  (enum   TilePaletteIDs id);
LE_Tileset*       get_tileset       (enum       TilesetIDs id);
LE_Tileset**      get_theme         (enum         ThemeIDs id);
struct Powerup*   get_powerup       (enum       PowerupIDs id);

#undef ENTITY
#undef TILESET
#undef TILE
#undef THEME
#undef TILESET_TYPE
#undef TILE_PALETTE
#undef POWERUP
#undef ENUM
#undef NO_VSCODE

#endif