#include "data.h"

#include "game/entities/functions.h"
#include "game/tiles/functions.h"

#include <string.h>

LE_EntityBuilder* entity_builders[256];
LE_TileData* tile_data[256];
LE_Tileset* tilesets[256];

#define NO_VSCODE

#define ENUM(_)
#define ENTITY(id, data) entity_builders[ENTITY_BUILDER_##id] = ({ \
    LE_EntityBuilder* builder = LE_CreateEntityBuilder();           \
    data                                                             \
    builder;                                                          \
});
#define TILE(id, data) tile_data[TILE_DATA_##id] = ({ \
    LE_TileData* tile = LE_CreateTileData();           \
    data                                                \
    tile;                                                \
});
#define TILESET(id, data) tile_data[TILESET_##id] = ({ \
    LE_Tileset* tileset = LE_CreateTileset();           \
    data                                                 \
    for (int i = 0; i < 256; i++) {                       \
        LE_TilesetAddTile(tileset, tile_data[i]);          \
    }                                                       \
    tileset;                                                 \
});

void init_data() {
    memset(entity_builders, 0, sizeof(entity_builders));
    memset(tile_data, 0, sizeof(tile_data));
    memset(tilesets, 0, sizeof(tilesets));

#define UPDATE(   func) LE_EntityBuilderAddUpdateCallback   (builder, func );
#define COLLISION(func) LE_EntityBuilderAddCollisionCallback(builder, func );
#define TEXTURE(  func) LE_EntityBuilderAddTextureCallback  (builder, func );
#define FLAGS(   flags) LE_EntityBuilderSetFlags            (builder, flags);
#define SIZE(     w, h) LE_EntityBuilderSetHitboxSize       (builder, w, h );
#include "game/data/entities.h"
#undef UPDATE
#undef COLLISION
#undef TEXTURE
#undef FLAGS
#undef SIZE

#define COLLISION(func) LE_TileAddCollisionCallback(tile, func);
#define TEXTURE(  func) LE_TileAddTextureCallback  (tile, func);
#define SOLID(        ) LE_TileSetSolid            (tile, true);
#include "game/data/tiles.h"
#undef COLLISION
#undef TEXTURE
#undef SOLID

#define TEXTURE(     tex ) LE_TilesetSetTexture   (tileset, tex );
#define SIZE(        w, h) LE_TilesetSetTileSize  (tileset, w, h);
#define TILES_IN_ROW(x   ) LE_TilesetSetTilesInRow(tileset, x   );
#include "game/data/tilesets.h"
#undef TEXTURE
#undef SIZE
#undef TILES_IN_ROW
}

LE_EntityBuilder* __get_entity_builder(enum EntityBuilderIDs id) {
    return entity_builders[id];
}

LE_TileData* __get_tile_data(enum TileDataIDs id) {
    return tile_data[id];
}

LE_Tileset* __get_tileset(enum TilesetIDs id) {
    return tilesets[id];
}
