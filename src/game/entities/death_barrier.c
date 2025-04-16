#include "functions.h"

#include "game/data.h"

entity_update(death_barrier) {
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player", NULL);
    LE_EntityProperty powerup_state = LE_EntityGetPropertyOrDefault(player, (LE_EntityProperty){ .asInt = POWERUP_base }, "powerup_state");
    if (!player) return;
    if (player->posY > entity->posY && powerup_state.asInt != POWERUP_death) powerup_state.asInt = hurt;
    LE_EntitySetProperty(player, powerup_state, "powerup_state");
}