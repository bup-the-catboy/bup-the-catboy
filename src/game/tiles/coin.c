#include "functions.h"
#include "game/data.h"
#include "game/savefile.h"

#include <string.h>

tile_collision(coin) {
    if (strcmp(LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asPtr = "" }, "tag").asPtr, "player") != 0) return;
    LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    savefile->coins++;
}

tile_collision(life_coin) {
    LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    savefile->lives++;
}