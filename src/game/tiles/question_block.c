#include "functions.h"
#include "game/data.h"

tile_collision(question_block) {
    if (direction == LE_Direction_Down) {
        LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_empty_block);
    }
}