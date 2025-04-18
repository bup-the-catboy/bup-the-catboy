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
#include "main.h"

#define arrsize(x) (sizeof(x) / sizeof(*(x)))

static void draw_iris(void* param, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    LE_Entity* entity = param;
    float timer  = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "dead_timer").asFloat;
    float xpos   = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "xpos").asFloat;
    float ypos   = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "ypos").asFloat;
    float radius = quad_out(1 - timer / 60.f) * max(WIDTH, HEIGHT);
    float clr    = quad_out(1 - timer / 20.f);
    if (clr < 0) clr = 0;
    graphics_set_shader(GET_ASSET(struct GfxResource, "shaders/iris.glsl"));
    graphics_shader_set_float("u_xpos", xpos);
    graphics_shader_set_float("u_ypos", ypos);
    graphics_shader_set_float("u_radius", radius);
    graphics_shader_set_float("u_bgcol_r", 1);
    graphics_shader_set_float("u_bgcol_g", clr);
    graphics_shader_set_float("u_bgcol_b", clr);
    graphics_shader_set_float("u_bgcol_a", 1);
    graphics_set_shader(graphics_dummy_shader());
}

static void draw_player(void* param, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    LE_Entity* entity = param;
    bool is_hidden = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "hidden").asBool;
    if (is_hidden) graphics_set_shader(GET_ASSET(struct GfxResource, "shaders/noise.glsl"));
    gfxcmd_process(GET_ASSET(struct GfxResource, "images/entities/player.png"), dstx, dsty, dstw, dsth, srcx, srcy, srcw, srch, color);
    graphics_set_shader(graphics_dummy_shader());
}

static int idle_anim_table[] = { 0, 1, 2, 3, 2, 1 };

entity_texture(player) {
    bool disable_input = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "disable_input").asBool;
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = POWERUP_base }, "powerup_state").asInt == POWERUP_death) {
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
        ( facing_left.asBool && is_button_down(BUTTON_MOVE_RIGHT) && !disable_input) ||
        (!facing_left.asBool && is_button_down(BUTTON_MOVE_LEFT)  && !disable_input)
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
    return gfxcmd_custom(draw_player, entity);
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
    int id = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = POWERUP_base }, "powerup_state").asInt;
    if (id == -1) {
        struct Powerup* parent = get_powerup(id)->parent;
        if (parent == NULL) parent = get_powerup_by_id(death);
        id = (int)((uintptr_t)parent - (uintptr_t)get_powerup(0)) / sizeof(struct Powerup);
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asInt = id }, "powerup_state");
    }
    struct Powerup* powerup = get_powerup(id);
    while (powerup) {
        if (!powerup->callback(entity)) break;
        powerup = powerup->parent;
    }
}

powerup(death) {
    if (!(entity->flags & LE_EntityFlags_DisableCollision)) {
        float xpos, ypos;
        set_pause_state(PAUSE_FLAG_NO_UPDATE_CAMERA);
        LE_EntityLastDrawnPos(entity, &xpos, &ypos);
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = xpos }, "xpos");
        LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = ypos }, "ypos");
        entity->velX =  0.0f;
        entity->velY = -0.7f;
        entity->flags |= LE_EntityFlags_DisableCollision;
        return true;
    }
    LE_EntitySetProperty(entity,
        (LE_EntityProperty){
            .asFloat = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){
                .asFloat = 0
            }, "dead_timer").asFloat + delta_time
        },
        "dead_timer"
    );
    if (entity->velY >= 1) start_transition(reload_level, 60, LE_Direction_Up, cubic_in_out);
    return true;
}

powerup(base) {
    LE_EntityProperty hidden = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "hidden");
    LE_EntitySetProperty(entity, hidden, "prev_hidden");
    hidden.asBool = is_button_down(BUTTON_MOVE_DOWN);
    LE_EntitySetProperty(entity, hidden, "hidden");
    bool disable_input = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "disable_input").asBool;
    LE_EntityProperty pouncing = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "pouncing");
    LE_EntityProperty pounce_timer = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "pounce_timer");
    if (!pouncing.asBool) {
        bool l = is_button_down(BUTTON_MOVE_LEFT) && !disable_input;
        bool r = is_button_down(BUTTON_MOVE_RIGHT) && !disable_input;
        if (entity->flags & LE_EntityFlags_OnGround) {
            if (is_button_pressed(BUTTON_MOVE_LEFT ) && !disable_input) entity_spawn_dust(entity, false, true, 0.2f + entity->velX);
            if (is_button_pressed(BUTTON_MOVE_RIGHT) && !disable_input) entity_spawn_dust(entity, true, false, 0.2f - entity->velX);
        }
        if (l && !r && entity->velX >= -0.2f) entity->velX -= 0.02f * delta_time;
        else if (!l && r && entity->velX <= 0.2f) entity->velX += 0.02f * delta_time;
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
        if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "jumping").asBool) {
            float height = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = entity->posY }, "jumping_from").asFloat - entity->posY;
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = 999 }, "coyote");
            if (height > 3 || !(is_button_down(BUTTON_JUMP) || disable_input)) LE_EntitySetProperty(entity, (LE_EntityProperty){ .asBool = false }, "jumping");
            else entity->velY = -0.3f;
        }
        if (entity_jump_requested(entity, is_button_pressed(BUTTON_JUMP) && !disable_input) & entity_can_jump(entity)) {
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = 1.5f }, "squish");
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = 999  }, "coyote");
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asBool = true  }, "jumping");
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = entity->posY  }, "jumping_from");
            if (!l && !r) entity->velX *= 0.6f;
            entity_spawn_dust(entity, true, true, 0.2f);
        }
        if (is_button_pressed(BUTTON_POUNCE) && !disable_input) {
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = 1.75f }, "squish");
            LE_EntityProperty facing_left = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "facing_left");
            pouncing.asBool = true;
            pounce_timer.asFloat = 5;
            entity->velX +=  0.3f * (facing_left.asBool ? -1 : 1);
            entity->velY  = -0.2f;
            if (entity->velX >  0.35f) entity->velX =  0.35f;
            if (entity->velX < -0.35f) entity->velX = -0.35f;
            entity_spawn_dust(entity, true, true, 0.2f);
            camera_screenshake(camera, 10, 0.5, 0.5);
        }
    }
    else pounce_timer.asFloat -= delta_time;
    LE_EntityProperty peak_height = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = entity->posY }, "peak_height");
    LE_EntityProperty prev_in_air = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool  = false        }, "prev_in_air");
    if (prev_in_air.asBool && (entity->flags & LE_EntityFlags_OnGround) && entity->posY - peak_height.asFloat > 5) {
        entity_spawn_dust(entity, true, true, 0.2f);
    }
    if ((entity->flags & LE_EntityFlags_OnGround) && pounce_timer.asFloat <= 0) pouncing.asBool = false;
    hud_update(entity);
    if (!disable_input) camera_set_focus(camera, entity->posX, 8);
    entity_fall_squish(entity, 10, .5f, .25f);
    entity_update_squish(entity, 5);
    LE_EntitySetProperty(entity, pouncing, "pouncing");
    LE_EntitySetProperty(entity, pounce_timer, "pounce_timer");
    return false;
}

powerup(test) {
    LE_EntityProperty hidden = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "hidden");
    LE_EntitySetProperty(entity, hidden, "prev_hidden");
    hidden.asBool = is_button_down(BUTTON_MOVE_DOWN);
    LE_EntitySetProperty(entity, hidden, "hidden");
    return true;
}
