#include <stdlib.h>

#include "linked_list.h"
#include "lunarengine.h"

typedef DEFINE_LIST(TileTextureCallback) TexCallbackList;
typedef DEFINE_LIST(TileCollisionCallback) CollCallbackList;

typedef struct {
    TexCallbackList* textureCallbacks;
    CollCallbackList* collisionCallbacks;
    bool solid;
} _LE_TileData;

typedef DEFINE_LIST(_LE_TileData) _LE_TileList;

typedef struct {
    void* texture;
    int tilesInRow;
    int tileWidth;
    int tileHeight;
    _LE_TileList* tiles;
} _LE_Tileset;

typedef struct {
    int width, height;
    int* data;
    _LE_Tileset* tileset;
} _LE_Tilemap;

LE_TileData* LE_CreateTileData() {
    _LE_TileData* data = malloc(sizeof(_LE_TileData));
    data->textureCallbacks = LE_LL_Create();
    data->collisionCallbacks = LE_LL_Create();
    data->solid = false;
    return (LE_TileData*)data;
}

void LE_TileAddTextureCallback(LE_TileData* tile, TileTextureCallback tex) {
    LE_LL_Add(((_LE_TileData*)tile)->textureCallbacks, tex);
}

void LE_TileAddCollisionCallback(LE_TileData* tile, TileCollisionCallback coll) {
    LE_LL_Add(((_LE_TileData*)tile)->collisionCallbacks, coll);
}

void LE_TileCollisionEvent(LE_TileData* tile, LE_Tilemap* tilemap, LE_Entity* entity, int tileX, int tileY, enum LE_Direction direction) {
    CollCallbackList* curr = ((_LE_TileData*)tile)->collisionCallbacks->next;
    while (curr) {
        ((TileCollisionCallback)curr->value)(tile, tilemap, entity, tileX, tileY, direction);
        curr = curr->next;
    }
}

void LE_TileSetSolid(LE_TileData* tile, bool solid) {
    ((_LE_TileData*)tile)->solid = solid;
}

void LE_DrawTileAt(LE_TileData* tile, LE_Tileset* tileset, float x, float y, float scaleW, float scaleH, LE_DrawList* dl) {
    _LE_TileData* t = (_LE_TileData*)tile;
    TexCallbackList* tex = t->textureCallbacks;
    int texture = -1;
    while (tex->next) {
        tex = tex->next;
        texture = ((TileTextureCallback)tex->value)((LE_TileData*)t);
        if (texture != -1) break;
    }
    if (texture == -1) return;
    _LE_Tileset* ts = (_LE_Tileset*)tileset;
    int tx = texture % ts->tilesInRow;
    int ty = texture / ts->tilesInRow;
    LE_DrawListAppend(dl, ts->texture,
        x, y, ts->tileWidth * scaleW, ts->tileHeight * scaleH,
        tx * ts->tileWidth, ty * ts->tileHeight, ts->tileWidth, ts->tileHeight
    );
}

bool LE_TileIsSolid(LE_TileData* tile) {
    return ((_LE_TileData*)tile)->solid;
}

void LE_DestroyTileData(LE_TileData* tile) {
    _LE_TileData* td = (_LE_TileData*)tile;
    LE_LL_Free(td->collisionCallbacks);
    LE_LL_Free(td->textureCallbacks);
    free(tile);
}

LE_Tileset* LE_CreateTileset() {
    _LE_Tileset* tileset = malloc(sizeof(_LE_Tileset));
    tileset->texture = NULL;
    tileset->tilesInRow = 0;
    tileset->tileWidth = 0;
    tileset->tileHeight = 0;
    tileset->tiles = LE_LL_Create();
    return (LE_Tileset*)tileset;
}

void LE_TilesetSetTexture(LE_Tileset* tileset, void* texture) {
    ((_LE_Tileset*)tileset)->texture = texture;
}

void LE_TilesetSetTileSize(LE_Tileset* tileset, int width, int height) {
    ((_LE_Tileset*)tileset)->tileWidth  = width;
    ((_LE_Tileset*)tileset)->tileHeight = height;
}

void LE_TilesetSetTilesInRow(LE_Tileset* tileset, int tilesInRow) {
    ((_LE_Tileset*)tileset)->tilesInRow = tilesInRow;
}

void LE_TilesetGetTileSize(LE_Tileset* tileset, int* width, int* height) {
    if (width)  *width  = ((_LE_Tileset*)tileset)->tileWidth;
    if (height) *height = ((_LE_Tileset*)tileset)->tileHeight;
}

void LE_TilesetAddTile(LE_Tileset* tileset, LE_TileData* tile) {
    LE_LL_Add(((_LE_Tileset*)tileset)->tiles, tile);
}

LE_TileData* LE_TilesetGetData(LE_Tileset* tileset, int tileIndex) {
    return (LE_TileData*)LE_LL_Get(((_LE_Tileset*)tileset)->tiles, tileIndex);
}

void LE_DestroyTileset(LE_Tileset* tileset) {
    LE_LL_Free(((_LE_Tileset*)tileset)->tiles);
    free(tileset);
}

LE_Tilemap* LE_CreateTilemap(int width, int height) {
    _LE_Tilemap* tilemap = malloc(sizeof(_LE_Tilemap));
    tilemap->width = width;
    tilemap->height = height;
    tilemap->data = malloc(sizeof(int) * width * height);
    tilemap->tileset = NULL;
    return (LE_Tilemap*)tilemap;
}

void LE_TilemapSetTileset(LE_Tilemap* tilemap, LE_Tileset* tileset) {
    ((_LE_Tilemap*)tilemap)->tileset = (_LE_Tileset*)tileset;
}

void LE_TilemapSetTile(LE_Tilemap* tilemap, int x, int y, int tile) {
    _LE_Tilemap* t = (_LE_Tilemap*)tilemap;
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) return;
    t->data[y * t->width + x] = tile;
}

int LE_TilemapGetTile(LE_Tilemap* tilemap, int x, int y) {
    _LE_Tilemap* t = (_LE_Tilemap*)tilemap;
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) return 0;
    return t->data[y * t->width + x];
}

void LE_TilemapGetSize(LE_Tilemap* tilemap, int* width, int* height) {
    _LE_Tilemap* t = (_LE_Tilemap*)tilemap;
    if (width)  *width  = t->width;
    if (height) *height = t->height;
}

LE_TileData* LE_TilemapGetTileData(LE_Tilemap* tilemap, int x, int y) {
    _LE_Tilemap* t = (_LE_Tilemap*)tilemap;
    if (x < 0 || y < 0 || x >= t->width || y >= t->height) return NULL;
    return LE_TilesetGetData((LE_Tileset*)t->tileset, t->data[y * t->width + x]);
}

LE_Tileset* LE_TilemapGetTileset (LE_Tilemap* tilemap) {
    return (LE_Tileset*)((_LE_Tilemap*)tilemap)->tileset;
}

void LE_DrawWholeTilemap(LE_Tilemap* tilemap, float x, float y, float scaleW, float scaleH, LE_DrawList* dl) {
    _LE_Tilemap* t = (_LE_Tilemap*)tilemap;
    LE_DrawPartialTilemap(tilemap, x, y, 0, 0, t->width - 1, t->height - 1, scaleW, scaleH, dl);
}

void LE_DrawPartialTilemap(LE_Tilemap* tilemap, float x, float y, int fromX, int fromY, int toX, int toY, float scaleW, float scaleH, LE_DrawList* dl) {
    int w, h;
    LE_TilemapGetSize(tilemap, &w, &h);
    _LE_Tileset* tileset = ((_LE_Tilemap*)tilemap)->tileset;
    if (!tileset) return;
    for (int Y = fromY; Y <= toY; Y++) {
        for (int X = fromX; X <= toX; X++) {
            if (X < 0 || Y < 0 || X >= w || Y >= h) continue;
            LE_DrawTileAt(LE_TilemapGetTileData(tilemap, X, Y), (LE_Tileset*)tileset, (x + X) * scaleW * tileset->tileWidth, (y + Y) * scaleH * tileset->tileHeight, scaleW, scaleH, dl);
        }
    }
}

void LE_DestroyTilemap(LE_Tilemap* tilemap) {
    free(((_LE_Tilemap*)tilemap)->tileset);
    free(tilemap);
}
