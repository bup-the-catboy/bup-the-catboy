#include "functions.h"

#include <lunarengine.h>
#include <math.h>

#include "game/overlay/transition.h"
#include "io/assets/assets.h"
#include "game/data.h"
#include "game/input.h"
#include "game/camera.h"
#include "game/level.h"
#include "game/overlay/hud.h"

#define arrsize(x) (sizeof(x) / sizeof(*(x)))

entity_texture(dead_player) {
    return GET_ASSET(struct GfxResource, "images/entities/player.png");
}

entity_texture(player) {
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "dead").asInt) {
        bool flip = entity->velY > 0;
        int sprite =
            fabsf(entity->velY) < 0.05 ? 8 :
            fabsf(entity->velY) < 0.15 ? 7 : 6;
        *srcX = sprite * 16;
        *srcY = 0;
        *srcW = 16;
        *srcH = 16;
        *w = 16;
        *h = 16 * (flip ? -1 : 1);
        return GET_ASSET(struct GfxResource, "images/entities/player.png");
    }
    int sprite = 0;
    LE_EntityProperty facing_left = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "facing_left");
    if (fabs(entity->velX) > 0) {
        if (entity->velX < 0) facing_left.asBool = true;
        if (entity->velX > 0) facing_left.asBool = false;
        sprite = (int)(entity->posX) % 2 + 1;
    }
    if (
        ( facing_left.asBool && is_button_down(BUTTON_MOVE_RIGHT)) ||
        (!facing_left.asBool && is_button_down(BUTTON_MOVE_LEFT))
    ) sprite = 5;
    if (entity->velY > 0) sprite = 4;
    if (entity->velY < 0) sprite = 3;
    LE_EntitySetProperty(entity, facing_left, "facing_left");
    *srcX = sprite * 16;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = facing_left.asBool ? -16 : 16;
    *h = 16;
    entity_apply_squish(entity, w, h);
    return GET_ASSET(struct GfxResource, "images/entities/player.png");
}

entity_update(player_spawner) {
    camera = camera_create();
    if (current_level->default_cambound >= 0 && current_level->default_cambound < current_level->num_cambounds) {
        camera_set_bounds(camera, current_level->cambounds[current_level->default_cambound]);
    }
    LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder_by_id(player), entity->posX, entity->posY);
    LE_DeleteEntity(entity);
}

entity_update(player) {
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "dead").asInt) {
        if (entity->velY >= 1) start_transition(reload_level, 60, LE_Direction_Up, cubic_in_out);
        return;
    }
    bool l = is_button_down(BUTTON_MOVE_LEFT);
    bool r = is_button_down(BUTTON_MOVE_RIGHT);
    if (entity->flags & LE_EntityFlags_OnGround) {
        if (is_button_pressed(BUTTON_MOVE_LEFT )) entity_spawn_dust(entity, false, true, 0.2f + entity->velX);
        if (is_button_pressed(BUTTON_MOVE_RIGHT)) entity_spawn_dust(entity, true, false, 0.2f - entity->velX);
    }
    if (l && !r) {
        entity->velX -= 0.02f * delta_time;
        if (entity->velX < -0.2f) entity->velX = -0.2f;
    }
    else if (!l && r) {
        entity->velX += 0.02f * delta_time;
        if (entity->velX > 0.2f) entity->velX = 0.2f;
    }
    else {
        float decel = (entity->flags & LE_EntityFlags_OnGround) ? 0.02f : 0.01f;
        if (entity->velX < 0) {
            entity->velX += decel * delta_time;
            if (entity->velX > 0) entity->velX = 0;
        }
        if (entity->velX > 0) {
            entity->velX -= decel * delta_time;
            if (entity->velX < 0) entity->velX = 0;
        }
    }
    if ((entity->flags & LE_EntityFlags_OnGround) && is_button_pressed(BUTTON_JUMP)) {
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = 1.5f }, "squish");
        entity->velY = -0.5f;
        if (!l && !r) entity->velX *= 0.6f;
        entity_spawn_dust(entity, true, true, 0.2f);
    }
    LE_EntityProperty peak_height = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = entity->posY }, "peak_height");
    LE_EntityProperty prev_in_air = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool  = false        }, "prev_in_air");
    if (prev_in_air.asBool && (entity->flags & LE_EntityFlags_OnGround) && entity->posY - peak_height.asFloat > 5) {
        entity_spawn_dust(entity, true, true, 0.2f);
    }
    if (is_button_pressed(BUTTON_MOUSE_LEFT)) {
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asInt = 1 }, "dead");
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = entity->posX }, "xpos");
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = entity->posY }, "ypos");
        entity->velX =  0.0f;
        entity->velY = -0.7f;
        entity->flags |= LE_EntityFlags_DisableCollision;
        return;
    }
    hud_update(entity);
    camera_set_focus(camera, entity->posX, 8);
    entity_fall_squish(entity, 10, .5f, .25f);
    entity_update_squish(entity, 5);
}
