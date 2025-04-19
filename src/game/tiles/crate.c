#include "functions.h"
#include "game/entities/functions.h"

#include "game/data.h"
#include "rng.h"

#include <string.h>

tile_collision(crate) {
    char* tag = get(entity, "tag", Ptr, "");
    bool pouncing = get(entity, "pouncing", Bool, false);
    if (strcmp(tag, "player") == 0 && pouncing) {
        LE_TilemapSetTile(tilemap, tileX, tileY, TILE_DATA_air);
        for (int i = 0; i < 8; i++) {
            LE_Entity* fragment = LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder_by_id(crate_fragment), tileX, tileY + 1);
            fragment->velX = (random_float() * 2 - 1) * 0.2;
            fragment->velY = random_range(-0.4, -0.2);
        }
    }
}