#include "functions.h"

#include "game/data.h"

entity_update(death_barrier) {
    float distance;
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player", &distance);
    int powerup_state = get(player, "powerup_state", Int, POWERUP_base);
    if (!player) return;
    if (distance < 16 && powerup_state != POWERUP_death) powerup_state = POWERUP_death;
    set(player, "powerup_state", Int, powerup_state);
}