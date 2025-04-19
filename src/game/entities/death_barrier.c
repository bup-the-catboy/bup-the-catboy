#include "functions.h"

#include "game/data.h"

entity_update(death_barrier) {
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player", NULL);
    int powerup_state = get(player, "powerup_state", Int, POWERUP_base);
    if (!player) return;
    if (player->posY > entity->posY && powerup_state != POWERUP_death) powerup_state = hurt;
    set(player, "powerup_state", Int, powerup_state);
}