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
#include "io/io.h"

#define arrsize(x) (sizeof(x) / sizeof(*(x)))

static void draw_iris(void* param) {
    LE_Entity* entity = param;
    float timer = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "dead_timer").asFloat;
    float xpos  = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "xpos").asFloat;
    float ypos  = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "ypos").asFloat;
    float radius = quad_out(1 - timer / 30.f) * max(WIDTH, HEIGHT);
    float color  = quad_out(1 - timer / 20.f);
    graphics_select_shader(GET_ASSET(struct GfxResource, "shaders/iris.glsl"));
    graphics_shader_set_float("u_xpos", xpos);
    graphics_shader_set_float("u_ypos", ypos);
    graphics_shader_set_float("u_radius", radius);
    graphics_shader_set_float("u_bgcol_r", 1);
    graphics_shader_set_float("u_bgcol_g", color);
    graphics_shader_set_float("u_bgcol_b", color);
    graphics_shader_set_float("u_bgcol_a", 1);
    graphics_apply_shader();
}

static int idle_anim_table[] = { 0, 1, 2, 3, 2, 1 };

entity_texture(player) {
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "dead").asInt) {
        bool flip = entity->velY > 0;
        int sprite =
            fabsf(entity->velY) < 0.05 ? 11 :
            fabsf(entity->velY) < 0.15 ? 10 : 9;
        *srcX = sprite * 16;
        *srcY = 0;
        *srcW = 16;
        *srcH = 16;
        *w = 16;
        *h = 16 * (flip ? -1 : 1);
        drawlist_append(gfxcmd_custom(draw_iris, entity));
        return GET_ASSET(struct GfxResource, "images/entities/player.png");
    }
    int sprite = idle_anim_table[(global_timer / 10) % 6];
    LE_EntityProperty pouncing = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "pouncing");
    LE_EntityProperty facing_left = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "facing_left");
    if (fabs(entity->velX) > 0) {
        if (entity->velX < 0) facing_left.asBool = true;
        if (entity->velX > 0) facing_left.asBool = false;
        sprite = (int)(entity->posX) % 2 + 4;
    }
    if (
        ( facing_left.asBool && is_button_down(BUTTON_MOVE_RIGHT)) ||
        (!facing_left.asBool && is_button_down(BUTTON_MOVE_LEFT))
    ) sprite = 8;
    if (entity->velY > 0) sprite = 7;
    if (entity->velY < 0) sprite = 6;
    if (pouncing.asBool) sprite = 12;
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
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "dead").asInt == 2) {
        LE_EntitySetProperty(entity,
            (LE_EntityProperty){
                .asFloat = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){
                    .asFloat = 0
                }, "dead_timer").asFloat + delta_time
            },
            "dead_timer"
        );
        if (entity->velY >= 1) start_transition(reload_level, 60, LE_Direction_Up, cubic_in_out);
        return;
    }
    LE_EntityProperty pouncing = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "pouncing");
    LE_EntityProperty pounce_timer = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "pounce_timer");
    if (!pouncing.asBool) {
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
        if (is_button_pressed(BUTTON_POUNCE)) {
            LE_EntityProperty facing_left = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "facing_left");
            pouncing.asBool = true;
            pounce_timer.asFloat = 5;
            entity->velX +=  0.3f * (facing_left.asBool ? -1 : 1);
            entity->velY  = -0.2f;
            if (entity->velX >  0.35f) entity->velX =  0.35f;
            if (entity->velX < -0.35f) entity->velX = -0.35f;
            entity_spawn_dust(entity, true, true, 0.2f);
        }
    }
    else pounce_timer.asFloat -= delta_time;
    LE_EntityProperty peak_height = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = entity->posY }, "peak_height");
    LE_EntityProperty prev_in_air = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool  = false        }, "prev_in_air");
    if (prev_in_air.asBool && (entity->flags & LE_EntityFlags_OnGround) && entity->posY - peak_height.asFloat > 5) {
        entity_spawn_dust(entity, true, true, 0.2f);
    }
    if ((entity->flags & LE_EntityFlags_OnGround) && pounce_timer.asFloat <= 0) pouncing.asBool = false;
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "dead").asInt == 1) {
        float xpos, ypos;
        set_pause_state(PAUSE_FLAG_NO_UPDATE_CAMERA);
        LE_EntityLastDrawnPos(entity, &xpos, &ypos);
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asInt = 2 }, "dead");
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = xpos }, "xpos");
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = ypos }, "ypos");
        entity->velX =  0.0f;
        entity->velY = -0.7f;
        entity->flags |= LE_EntityFlags_DisableCollision;
        return;
    }
    hud_update(entity);
    camera_set_focus(camera, entity->posX, 8);
    entity_fall_squish(entity, 10, .5f, .25f);
    entity_update_squish(entity, 5);
    LE_EntitySetProperty(entity, pouncing, "pouncing");
    LE_EntitySetProperty(entity, pounce_timer, "pounce_timer");
}
