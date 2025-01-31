#include "functions.h"

#include "game/data.h"

#include <string.h>

tile_collision(crate) {
    char* tag = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asPtr = "" }, "tag").asPtr;
    bool pouncing = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "pouncing").asBool;
    if (strcmp(tag, "player") == 0 && pouncing) {
        LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    }
}