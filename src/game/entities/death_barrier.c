#include "functions.h"

entity_update(death_barrier) {
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player");
    LE_EntityProperty dead = LE_EntityGetPropertyOrDefault(player, (LE_EntityProperty){ .asInt = 0 }, "dead");
    if (!player) return;
    if (player->posY > entity->posY && dead.asInt == 0) dead.asInt = 1;
    LE_EntitySetProperty(player, dead, "dead");
}