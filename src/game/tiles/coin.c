#include "functions.h"
#include "game/data.h"

tile_collision(coin) {
        LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
}
