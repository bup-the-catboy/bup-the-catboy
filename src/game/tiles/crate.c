#include "functions.h"
#include "game/entities/functions.h"

#include "game/data.h"
#include "game/level.h"
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
        LE_Entity* loot = find_nearest_entity_with_tag_at_position(LE_EntityGetList(entity), tileX + .5f, tileY + 1, "crate_loot", NULL);
        if (!loot) return;
        int id = get(loot, "entity_or_warp_id", Int, -1);
        if (get(loot, "is_warp", Bool, false)) activate_warp(&current_level->warps[id], LE_Direction_Up);
        else {
            LE_Entity* loot = LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder(id), tileX + .5f, tileY + 1);
            set(loot, "from_crate", Bool, true);
        }
    }
}