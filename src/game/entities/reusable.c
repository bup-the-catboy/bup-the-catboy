#include "functions.h"

entity_update(walk) {
    LE_EntityProperty walk_speed = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "walk_speed");
    enum LE_Direction dir;
    if (entity->velX == 0) entity->velX = -walk_speed.asFloat;
    if (entity_collided(entity, &dir)) {
        if (dir == LE_Direction_Left ) entity->velX = -walk_speed.asFloat;
        if (dir == LE_Direction_Right) entity->velX =  walk_speed.asFloat;
    }
}

entity_update(gravity) {
    entity->velY += LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "gravity").asFloat;
}