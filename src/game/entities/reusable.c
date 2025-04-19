#include "functions.h"
#include "game/data.h"
#include "main.h"

#include <string.h>

entity_update(walk) {
    float distance = 0;
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player", &distance);
    if (player && distance > 256) return;
    LE_EntityProperty walk_speed = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "walk_speed");
    LE_EntityProperty walk_timer = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "walk_timer");
    LE_Direction dir;
    if (entity->velX == 0) entity->velX = -walk_speed.asFloat;
    if (walk_timer.asFloat > 0) {
        walk_timer.asFloat -= delta_time;
        if (walk_timer.asFloat <= 0) {
            if (entity->posX < player->posX) entity->velX =  walk_speed.asFloat;
            if (player->posX < entity->posX) entity->velX = -walk_speed.asFloat;
            walk_timer.asFloat = 0;
        }
    }
    if (entity_collided(entity, &dir)) {
        if (dir == LE_Direction_Left ) entity->velX = -walk_speed.asFloat;
        if (dir == LE_Direction_Right) entity->velX =  walk_speed.asFloat;
    }
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "has_vision").asBool) {
        bool      hidden = LE_EntityGetPropertyOrDefault(player, (LE_EntityProperty){ .asBool = false },      "hidden").asBool;
        bool prev_hidden = LE_EntityGetPropertyOrDefault(player, (LE_EntityProperty){ .asBool = false }, "prev_hidden").asBool;
        if (prev_hidden && !hidden) {
            if (entity->posX < player->posX) entity->velX =  0.001f;
            if (player->posX < entity->posX) entity->velX = -0.001f;
            walk_timer.asFloat = 30;
            LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder_by_id(notified), entity->posX, entity->posY - 1.25f);
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asBool = player->posX < entity->posX }, "facing_left");
        }
    }
    LE_EntitySetProperty(entity, walk_timer, "walk_timer");
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
    if (strcmp(tag, "turtle_shell") == 0) {
        LE_DeleteEntity(entity);
        return;
    }
    if (strcmp(tag, "player") != 0) return;
    if (!entity_should_squish(entity, collider)) {
        if (LE_EntityGetPropertyOrDefault(collider, (LE_EntityProperty){ .asBool = false }, "hidden").asBool) return;
        LE_EntitySetProperty(collider, (LE_EntityProperty){ .asInt = hurt }, "powerup_state");
        return;
    }
    collider->velY = -0.2f;
    enum EntityBuilderIDs builder = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "squashed").asInt;
    LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder(builder), entity->posX, entity->posY);
    LE_DeleteEntity(entity);
}