#include "functions.h"
#include "game/data.h"
#include "game/savefile.h"

tile_collision(coin) {
    LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    savefile->coins++;
}

tile_collision(life_coin) {
    LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    savefile->lives++;
}