#ifndef LUNAR_ENGINE_H
#define LUNAR_ENGINE_H

#include <stdbool.h>
#include <stdio.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

typedef struct {} LE_Tileset;
typedef struct {} LE_Tilemap;
typedef struct {} LE_TileData;
typedef struct {} LE_DrawList;
typedef struct {} LE_EntityBuilder;
typedef struct {} LE_LayerList;
typedef struct {} LE_EntityList;
typedef struct {} LE_EntityListIter;
typedef struct {} LE_LayerListIter;

enum LE_EntityFlags {
    LE_EntityFlags_SolidHitbox      = 1 << 0,
    LE_EntityFlags_ShouldDelete     = 1 << 1,
    LE_EntityFlags_DisableCollision = 1 << 2,
    LE_EntityFlags_OnGround         = 1 << 3,
};

enum LE_Direction {
    LE_Direction_Up,
    LE_Direction_Left,
    LE_Direction_Down,
    LE_Direction_Right
};

enum LE_LayerType {
    LE_LayerType_Tilemap,
    LE_LayerType_Entity,
    LE_LayerType_Custom
};

typedef union LE_EntityProperty {
    int asInt;
    bool asBool;
    float asFloat;
    void* asPtr;
} LE_EntityProperty;

typedef struct {
    float posX, posY;
    float velX, velY;
    float width, height;
    enum LE_EntityFlags flags;
} LE_Entity;

typedef struct {
    float scrollOffsetX, scrollSpeedX;
    float scrollOffsetY, scrollSpeedY;
    float scaleW, scaleH;
} LE_Layer;

typedef void(*DrawListRenderer)(void* texture, 
    float dstX, float dstY, float dstW, float dstH,
    int   srcX, int   srcY, int   srcW, int   srcH,
    unsigned int color
);
typedef void*(*EntityTextureCallback)(
    LE_Entity* entity, float* width, float* height,
    int* srcX, int* srcY, int* srcW, int* srcH
);
typedef void(*EntityUpdateCallback)(LE_Entity* entity);
typedef void(*EntityCollisionCallback)(LE_Entity* entity, LE_Entity* collider);
typedef int(*TileTextureCallback)(LE_TileData* tile);
typedef void(*TileCollisionCallback)(
    LE_TileData* tile, LE_Tilemap* tilemap, LE_Entity* entity,
    int tileX, int tileY,
    enum LE_Direction direction
);
typedef void(*CustomLayer)(
    LE_DrawList* dl,
    float scrollOffsetX, float scrollOffsetY,
    float scaleX, float scaleH
);

LE_LayerList* LE_CreateLayerList();
LE_Layer* LE_AddTilemapLayer(LE_LayerList* layers, LE_Tilemap* tilemap);
LE_Layer* LE_AddEntityLayer(LE_LayerList* layers, LE_EntityList* entities);
LE_Layer* LE_AddCustomLayer(LE_LayerList* layers, CustomLayer callback);
LE_Layer* LE_LayerGetByIndex(LE_LayerList* layers, int index);
void LE_MoveLayer(LE_Layer* layer, int index);
int  LE_IndexOfLayer(LE_Layer* layer);
void LE_ScrollCamera(LE_LayerList* layers, float camX, float camY);
void LE_GetCameraPos(LE_LayerList* layers, float* camX, float* camY);
enum LE_LayerType LE_LayerGetType(LE_Layer* layer);
void* LE_LayerGetDataPointer(LE_Layer* layer);
void LE_Draw(LE_LayerList* layers, int screenW, int screenH, LE_DrawList* dl);
void LE_DrawSingleLayer(LE_Layer* layer, int screenW, int screenH, LE_DrawList* dl);
void LE_DestroyLayer(LE_Layer* layer);
void LE_DestroyLayerList(LE_LayerList* layers);
int  LE_NumLayers(LE_LayerList* layers);
LE_LayerListIter* LE_LayerListGetIter(LE_LayerList* list);
LE_LayerListIter* LE_LayerListNext(LE_LayerListIter* iter);
LE_LayerListIter* LE_LayerListPrev(LE_LayerListIter* iter);
LE_Layer* LE_LayerListGet(LE_LayerListIter* iter);

LE_DrawList* LE_CreateDrawList();
void LE_Render(LE_DrawList* dl, DrawListRenderer renderer);
void LE_DrawListAppend(LE_DrawList* dl, void* texture, float dstX, float dstY, float dstW, float dstH, int srcX, int srcY, int srcW, int srcH);
void LE_DrawSetColor(LE_DrawList* dl, unsigned int rgba);
void LE_ClearDrawList(LE_DrawList* dl);
void LE_DestroyDrawList(LE_DrawList* list);
int LE_DrawListSize(LE_DrawList* list);
unsigned int LE_DrawGetColor(LE_DrawList* dl);

LE_EntityBuilder* LE_CreateEntityBuilder();
void LE_EntityBuilderAddTextureCallback(LE_EntityBuilder* builder, EntityTextureCallback callback);
void LE_EntityBuilderAddUpdateCallback(LE_EntityBuilder* builder, EntityUpdateCallback callback);
void LE_EntityBuilderAddCollisionCallback(LE_EntityBuilder* builder, EntityCollisionCallback callback);
void LE_EntityBuilderSetHitboxSize(LE_EntityBuilder* builder, float width, float height);
void LE_EntityBuilderSetFlags(LE_EntityBuilder* builder, enum LE_EntityFlags flags);
void LE_EntityBuilderAppendFlags(LE_EntityBuilder* builder, enum LE_EntityFlags flags);
void LE_EntityBuilderClearFlags(LE_EntityBuilder* builder, enum LE_EntityFlags flags);
void LE_DestroyEntityBuilder(LE_EntityBuilder* builder);

LE_EntityList* LE_CreateEntityList();
LE_Entity* LE_CreateEntity(LE_EntityList* list, LE_EntityBuilder* builder, float x, float y);
LE_Entity* LE_EntityGetPlatform(LE_Entity* entity);
void LE_EntityAssignTilemap(LE_EntityList* list, LE_Tilemap* tilemap);
void LE_EntitySetProperty(LE_Entity* entity, LE_EntityProperty property, const char* name);
void LE_EntityDelProperty(LE_Entity* entity, const char* name);
bool LE_EntityGetProperty(LE_Entity* entity, LE_EntityProperty* property, const char* name);
int  LE_EntityNumProperties(LE_Entity* entity);
const char* LE_EntityGetPropertyKey(LE_Entity* entity, int index);
void LE_EntityChangeLists(LE_Entity* entity, LE_EntityList* destlist);
void LE_EntityCollision(LE_Entity* entity, LE_Entity* collider);
void LE_UpdateEntities(LE_EntityList* list);
void LE_UpdateEntity(LE_Entity* entity);
void LE_DrawEntity(LE_Entity* entity, float x, float y, float scaleW, float scaleH, LE_DrawList* dl);
void LE_DeleteEntity(LE_Entity* entity);
void LE_DestroyEntityList(LE_EntityList* list);
int  LE_NumEntities(LE_EntityList* list);
LE_EntityList* LE_EntityGetList(LE_Entity* entity);
LE_Tilemap* LE_EntityGetTilemap(LE_EntityList* list);
LE_EntityListIter* LE_EntityListGetIter(LE_EntityList* list);
LE_EntityListIter* LE_EntityListNext(LE_EntityListIter* iter);
LE_EntityListIter* LE_EntityListPrev(LE_EntityListIter* iter);
LE_Entity* LE_EntityListGet(LE_EntityListIter* iter);

LE_TileData* LE_CreateTileData();
void LE_TileAddTextureCallback(LE_TileData* tile, TileTextureCallback callback);
void LE_TileAddCollisionCallback(LE_TileData* tile, TileCollisionCallback callback);
void LE_TileCollisionEvent(LE_TileData* tile, LE_Tilemap* tilemap, LE_Entity* entity, int tileX, int tileY, enum LE_Direction direction);
void LE_TileSetSolid(LE_TileData* tile, bool solid);
void LE_DrawTileAt(LE_TileData* tile, LE_Tileset* tileset, float x, float y, float scaleW, float scaleH, LE_DrawList* dl);
bool LE_TileIsSolid(LE_TileData* tile);
void LE_DestroyTileData(LE_TileData* tile);

LE_Tileset* LE_CreateTileset();
void LE_TilesetSetTexture(LE_Tileset* tileset, void* texture);
void LE_TilesetSetTileSize(LE_Tileset* tileset, int width, int height);
void LE_TilesetSetTilesInRow(LE_Tileset* tileset, int tilesInRow);
void LE_TilesetGetTileSize(LE_Tileset* tileset, int* width, int* height);
void LE_TilesetAddTile(LE_Tileset* tileset, LE_TileData* tile);
LE_TileData* LE_TilesetGetData(LE_Tileset* tileset, int tileIndex);
void LE_DestroyTileset(LE_Tileset* tileset);

LE_Tilemap* LE_CreateTilemap(int width, int height);
void LE_TilemapSetTileset(LE_Tilemap* tilemap, LE_Tileset* tileset);
void LE_TilemapSetTile(LE_Tilemap* tilemap, int x, int y, int tile);
void LE_TilemapGetSize(LE_Tilemap* tilemap, int* width, int* height);
int  LE_TilemapGetTile(LE_Tilemap* tilemap, int x, int y);
LE_TileData* LE_TilemapGetTileData(LE_Tilemap* tilemap, int x, int y);
LE_Tileset * LE_TilemapGetTileset (LE_Tilemap* tilemap);
void LE_TilemapSetRepeating(LE_Tilemap* tilemap, bool repeating);
void LE_DrawWholeTilemap(LE_Tilemap* tilemap, float x, float y, float scaleW, float scaleH, LE_DrawList* dl);
void LE_DrawPartialTilemap(LE_Tilemap* tilemap, float x, float y, int fromX, int fromY, int toX, int toY, float scaleW, float scaleH, LE_DrawList* dl);
void LE_DestroyTilemap(LE_Tilemap* tilemap);

#endif