#include "functions.h"
#include "game/data.h"
#include "main.h"

#include <string.h>

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
    entity->velY += LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "gravity").asFloat * delta_time;
}

entity_update(animable) {
    entity_advance_anim_frame(entity);
}

entity_update(despawn) {
    LE_EntityProperty timer;
    LE_EntityGetProperty(entity, &timer, "despawn_timer");
    timer.asFloat -= delta_time;
    if (timer.asFloat <= 0) LE_DeleteEntity(entity);
    LE_EntitySetProperty(entity, timer, "despawn_timer");
}

entity_update(friction) {
    float friction = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "friction").asFloat;
    if (!(entity->flags & LE_EntityFlags_OnGround)) return;
    if (entity->velX > 0) {
        entity->velX -= friction * delta_time;
        if (entity->velX < 0) entity->velX = 0;
    }
    if (entity->velX < 0) {
        entity->velX += friction * delta_time;
        if (entity->velX > 0) entity->velX = 0;
    }
}

entity_collision(squash) {
    const char* tag = LE_EntityGetPropertyOrDefault(collider, (LE_EntityProperty){ .asPtr = (void*)"" }, "tag").asPtr;
    if (strcmp(tag, "player") != 0) return;
    if (!(collider->posY < entity->posY - entity->height / 2)) return;
    collider->velY = -0.2f;
    enum EntityBuilderIDs builder = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "squashed").asInt;
    LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder(builder), entity->posX, entity->posY);
    LE_DeleteEntity(entity);
}