#include "functions.h"
#include "game/data.h"

tile_collision(brick) {
    if (direction == LE_Direction_Down) {
        LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    }
}