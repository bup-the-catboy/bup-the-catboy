#include "lunarengine.h"

#define EXPAND(x) x
#define _CONCAT(a, b) a##b
#define CONCAT( a, b) _CONCAT(a, b)

#define RUN(a, b) EXPAND(CONCAT(a, b))

#define CENTERX(entity) 0
#define CENTERY(entity) entity->height / 2
#define COORDX x
#define COORDY y
#define SIZEX width
#define SIZEY height
#define DIR_ULX LE_Direction_Left
#define DIR_ULY LE_Direction_Up
#define DIR_DRX LE_Direction_Right
#define DIR_DRY LE_Direction_Down
#define CORRECT_TILE_ULX x - entity->width / 2
#define CORRECT_TILE_ULY y
#define CORRECT_TILE_DRX x + 1 + entity->width / 2
#define CORRECT_TILE_DRY y + 1 + entity->height
#define CORRECT_ENTITY_ULX curr->posX - curr->width / 2 - entity->width / 2
#define CORRECT_ENTITY_ULY curr->posY - curr->height / 2
#define CORRECT_ENTITY_DRX curr->posX + curr->width / 2 + entity->width / 2
#define CORRECT_ENTITY_DRY curr->posY + entity->height
#define IS_XX true
#define IS_XY false
#define IS_YX false
#define IS_YY true

bool LE_RectIntersectsRect(
    float x1a, float y1a, float x2a, float y2a,
    float x1b, float y1b, float x2b, float y2b
) {
    return x2a > x1b && x2b > x1a && y2a > y1b && y2b > y1a;
}

void LE_EntitySetPlatform(LE_Entity* entity, LE_Entity* platform);

#define COLLISION(AXIS)                                                                                                       \
void CONCAT(LE_RunCollision, AXIS)(LE_Entity* entity) {                                                                       \
    if (entity->flags & LE_EntityFlags_DisableCollision) return;                                                              \
    LE_Tilemap* tilemap = LE_EntityGetTilemap(LE_EntityGetList(entity));                                                      \
    int w = 0;                                                                                                                \
    int h = 0;                                                                                                                \
    if (tilemap) LE_TilemapGetSize(tilemap, &w, &h);                                                                          \
    float fx = entity->posX - entity->width / 2;                                                                              \
    float fy = entity->posY - entity->height;                                                                                 \
    float tx = entity->posX + entity->width / 2;                                                                              \
    float ty = entity->posY;                                                                                                  \
    int tfx = fx - 1;                                                                                                         \
    int tfy = fy - 1;                                                                                                         \
    int ttx = tx + 1;                                                                                                         \
    int tty = ty + 1;                                                                                                         \
    bool collided = false;                                                                                                    \
    if (RUN(IS_Y, AXIS)) entity->flags &= ~LE_EntityFlags_OnGround;                                                           \
    for (int y = tfy; y <= tty; y++) {                                                                                        \
        for (int x = tfx; x <= ttx; x++) {                                                                                    \
            if (x < 0 || y < 0 || x >= w || y >= h) continue;                                                                 \
            LE_TileData* tile = LE_TilemapGetTileData(tilemap, x, y);                                                         \
            bool solid = LE_TileIsSolid(tile);                                                                                \
            bool side = RUN(entity->vel, AXIS) < 0;                                                                           \
            if (!LE_RectIntersectsRect(fx, fy, tx, ty, x, y, x + 1, y + 1)) continue;                                         \
            if (side) {                                                                                                       \
                LE_TileCollisionEvent(tile, tilemap, entity, x, y, RUN(DIR_DR, AXIS));                                        \
                if (solid) RUN(entity->pos, AXIS) = RUN(CORRECT_TILE_DR, AXIS);                                               \
            }                                                                                                                 \
            else {                                                                                                            \
                LE_TileCollisionEvent(tile, tilemap, entity, x, y, RUN(DIR_UL, AXIS));                                        \
                if (solid) {                                                                                                  \
                    RUN(entity->pos, AXIS) = RUN(CORRECT_TILE_UL, AXIS);                                                      \
                    if (RUN(IS_Y, AXIS)) entity->flags |= LE_EntityFlags_OnGround;                                            \
                }                                                                                                             \
            }                                                                                                                 \
            if (solid) collided = true;                                                                                       \
        }                                                                                                                     \
    }                                                                                                                         \
    LE_EntitySetPlatform(entity, NULL);                                                                                       \
    LE_EntityListIter* iter = LE_EntityListGetIter(LE_EntityGetList(entity));                                                 \
    while (iter) {                                                                                                            \
        LE_Entity* curr = LE_EntityListGet(iter);                                                                             \
        if (curr == entity) {                                                                                                 \
            iter = LE_EntityListNext(iter);                                                                                   \
            continue;                                                                                                         \
        }                                                                                                                     \
        if (!LE_RectIntersectsRect(                                                                                           \
            entity->posX - entity->width / 2, entity->posY - entity->height, entity->posX + entity->width / 2, entity->posY,  \
                curr->posX -   curr->width / 2,   curr->posY -   curr->height,   curr->posX +   curr->width / 2,   curr->posY \
        )) {                                                                                                                  \
            iter = LE_EntityListNext(iter);                                                                                   \
            continue;                                                                                                         \
        }                                                                                                                     \
        bool solid = curr->flags & LE_EntityFlags_SolidHitbox && LE_EntityGetPlatform(curr) != entity;                        \
        bool side = RUN(entity->vel, AXIS) < 0;                                                                               \
        LE_EntityCollision(curr, entity);                                                                                     \
        if (solid) {                                                                                                          \
            if (side) {                                                                                                       \
                RUN(entity->pos, AXIS) = RUN(CORRECT_ENTITY_DR, AXIS);                                                        \
                if (RUN(IS_Y, AXIS)) {                                                                                        \
                    LE_EntitySetPlatform(entity, curr);                                                                       \
                    entity->flags |= LE_EntityFlags_OnGround;                                                                 \
                }                                                                                                             \
            }                                                                                                                 \
            else RUN(entity->pos, AXIS) = RUN(CORRECT_ENTITY_UL, AXIS);                                                       \
            collided = true;                                                                                                  \
        }                                                                                                                     \
        iter = LE_EntityListNext(iter);                                                                                       \
    }                                                                                                                         \
    if (collided) RUN(entity->vel, AXIS) = 0;                                                                                 \
}

COLLISION(X)
COLLISION(Y)