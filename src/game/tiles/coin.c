#include "functions.h"
#include "game/data.h"
#include "game/savefile.h"
#include "game/entities/functions.h"

#include <string.h>

tile_collision(coin) {
    if (strcmp(get(entity, "tag", Ptr, ""), "player") != 0) return;
    LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    LE_EntityList* list = LE_EntityGetList(find_entity_with_tag("player"));
    LE_CreateEntity(list, get_entity_builder_by_id(sparkles), tileX + 0.5, tileY + 1);
    for (int i = 0; i < 8; i++) {
        LE_CreateEntity(list, get_entity_builder_by_id(coin_particle), tileX + 0.5, tileY + 0.5);
    }
    savefile->coins++;
}

tile_collision(life_coin) {
    LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
    savefile->lives++;
}