#include "functions.h"
#include "game/data.h"
#include "main.h"

#include <string.h>

entity_update(walk) {
    float distance = 0;
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player", &distance);
    if (player && distance > 256) return;
    float walk_speed = get(entity, "walk_speed", Float, 0);
    float walk_timer = get(entity, "walk_timer", Float, 0);
    LE_Direction dir;
    if (entity->velX == 0) entity->velX = -walk_speed;
    if (walk_timer > 0) {
        walk_timer -= delta_time;
        if (walk_timer <= 0) {
            if (entity->posX < player->posX) entity->velX =  walk_speed;
            if (player->posX < entity->posX) entity->velX = -walk_speed;
            walk_timer = 0;
        }
    }
    if (entity_collided(entity, &dir)) {
        if (dir == LE_Direction_Left ) entity->velX = -walk_speed;
        if (dir == LE_Direction_Right) entity->velX =  walk_speed;
    }
    if (get(entity, "has_vision", Bool, false)) {
        bool      hidden = get(player,      "hidden", Bool, false);
        bool prev_hidden = get(player, "prev_hidden", Bool, false);
        if (prev_hidden && !hidden) {
            if (entity->posX < player->posX) entity->velX =  0.001f;
            if (player->posX < entity->posX) entity->velX = -0.001f;
            walk_timer = 30;
            LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder_by_id(notified), entity->posX, entity->posY - 1.25f);
            set(entity, "facing_left", Bool, player->posX < entity->posX);
        }
    }
    set(entity, "walk_timer", Float, walk_timer);
}

entity_update(gravity) {
    entity->velY += get(entity, "gravity", Float, 0) * delta_time;
}

entity_update(animable) {
    entity_advance_anim_frame(entity);
}

entity_update(despawn) {
    float timer = get(entity, "despawn_timer", Float, 0);
    timer -= delta_time;
    if (timer <= 0) LE_DeleteEntity(entity);
    set(entity, "despawn_timer", Float, timer);
}

entity_update(friction) {
    float friction = get(entity, "friction", Float, 0);
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
    const char* tag = get(collider, "tag", Ptr, "");
    if (strcmp(tag, "turtle_shell") == 0) {
        LE_DeleteEntity(entity);
        return;
    }
    if (strcmp(tag, "player") != 0) return;
    if (!entity_should_squish(entity, collider)) {
        if (get(collider, "hidden", Bool, false)) return;
        set(collider, "powerup_state", Int, hurt);
        set(collider, "death_left", Bool, collider->posX < entity->posX);
        return;
    }
    collider->velY = -0.2f;
    enum EntityBuilderIDs builder = get(entity, "squashed", Int, 0);
    LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder(builder), entity->posX, entity->posY);
    LE_DeleteEntity(entity);
}