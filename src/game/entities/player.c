#include "font/font.h"
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
#include "rng.h"

#define arrsize(x) (sizeof(x) / sizeof(*(x)))

static void draw_dead_player(void* param, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    LE_Entity* entity = param;
    float timer = get(entity, "dead_timer", Float, 0);
    float shake_intensity = max(0, (30 - timer) / 30) * 8;
    float x = random_range(-shake_intensity, shake_intensity);
    float y = random_range(-shake_intensity, shake_intensity);
    graphics_set_shader(GET_ASSET(struct GfxResource, "shaders/iris.glsl"));
    graphics_shader_set_float("u_xpos", 0);
    graphics_shader_set_float("u_ypos", 0);
    graphics_shader_set_float("u_radius", 0);
    gfxcmd_process(GET_ASSET(struct GfxResource, "images/entities/player.png"), dstx + x, dsty + y, dstw, dsth, srcx, srcy, srcw, srch, color);
    graphics_set_shader(graphics_dummy_shader());
    if (timer >= 60) {
        shake_intensity = max(0, (30 - (timer - 60)) / 30) * 8;
        x = random_range(-shake_intensity, shake_intensity);
        y = random_range(-shake_intensity, shake_intensity);
        float width, height;
        const char* msg = "${^200}DEFEAT";
        LE_DrawList* dl = LE_CreateDrawList();
        text_size(&width, &height, msg);
        render_text(dl, (WIDTH - width) / 2 + x, 64 + y, msg);
        LE_Render(dl, gfxcmd_process);
        LE_DestroyDrawList(dl);
    }
}

static void draw_player(void* param, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    LE_Entity* entity = param;
    bool is_hidden = get(entity, "hidden", Bool, false);
    if (is_hidden) graphics_set_shader(GET_ASSET(struct GfxResource, "shaders/noise.glsl"));
    gfxcmd_process(GET_ASSET(struct GfxResource, "images/entities/player.png"), dstx, dsty, dstw, dsth, srcx, srcy, srcw, srch, color);
    graphics_set_shader(graphics_dummy_shader());
}

static int idle_anim_table[] = { 0, 1, 2, 3, 2, 1 };

entity_texture(player) {
    bool disable_input = get(entity, "disable_input", Bool, false);
    if (get(entity, "powerup_state", Int, POWERUP_base) == POWERUP_death) {
        *srcX = 9 * 16;
        *srcY = 0;
        *srcW = 16;
        *srcH = 16;
        *w = 16 * (get(entity, "death_left", Bool, false) ? 1 : -1);
        *h = 16;
        return gfxcmd_custom(draw_dead_player, entity);
    }
    int sprite = idle_anim_table[(global_timer / 10) % 6];
    bool pouncing    = get(entity, "pouncing",    Bool, false);
    bool facing_left = get(entity, "facing_left", Bool, false);
    if (fabs(entity->velX) > 0) {
        if (entity->velX < 0) facing_left = true;
        if (entity->velX > 0) facing_left = false;
        sprite = (int)(entity->posX) % 2 + 4;
    }
    if (
        ( facing_left && is_button_down(BUTTON_MOVE_RIGHT) && !disable_input) ||
        (!facing_left && is_button_down(BUTTON_MOVE_LEFT)  && !disable_input)
    ) sprite = 8;
    if (entity->velY > 0) sprite = 7;
    if (entity->velY < 0) sprite = 6;
    if (pouncing)         sprite = 12;
    set(entity, "facing_left", Bool, facing_left);
    *srcX = sprite * 16;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = facing_left ? -16 : 16;
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
    int id = get(entity, "powerup_state", Int, POWERUP_base);
    if (id == -1) {
        struct Powerup* parent = get_powerup(id)->parent;
        if (parent == NULL) parent = get_powerup_by_id(death);
        id = (int)((uintptr_t)parent - (uintptr_t)get_powerup(0)) / sizeof(struct Powerup);
        set(entity, "powerup_state", Int, id);
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
        set(entity, "xpos", Float, xpos);
        set(entity, "ypos", Float, ypos);
        set(entity, "gravity", Float, 0.001);
        entity->velX =  0.05f * (get(entity, "death_left", Bool, false) ? -1 : 1);
        entity->velY = -0.05f;
        entity->flags |= LE_EntityFlags_DisableCollision;
        return true;
    }
    float dead_timer = get(entity, "dead_timer", Float, 0);
    dead_timer += delta_time;
    set(entity, "dead_timer", Float, dead_timer);
    if (dead_timer >= 120) start_transition(reload_level, 60, LE_Direction_Up, cubic_in_out);
    return true;
}

powerup(base) {
    bool disable_input = get(entity, "disable_input", Bool, false);
    bool pouncing = get(entity, "pouncing", Bool, false);
    float pounce_timer = get(entity, "pounce_timer", Float, 0);
    if (!pouncing) {
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
        if (get(entity, "jumping", Bool, false)) {
            float height = get(entity, "jumping_from", Float, entity->posY) - entity->posY;
            set(entity, "coyote", Float, 999);
            if (height > 3 || !(is_button_down(BUTTON_JUMP) || disable_input)) set(entity, "jumping", Bool, false);
            else entity->velY = -0.3f;
        }
        if (entity_jump_requested(entity, is_button_pressed(BUTTON_JUMP) && !disable_input) & entity_can_jump(entity)) {
            set(entity, "squish", Float, 1.5f);
            set(entity, "coyote", Float, 999);
            set(entity, "jumping", Bool, true);
            set(entity, "jumping_from", Float, entity->posY);
            if (!l && !r) entity->velX *= 0.6f;
            entity_spawn_dust(entity, true, true, 0.2f);
        }
        if (is_button_pressed(BUTTON_POUNCE) && !disable_input) {
            set(entity, "squish", Float, 1.75f);
            bool facing_left = get(entity, "facing_left", Bool, false);
            pouncing = true;
            pounce_timer = 5;
            entity->velX +=  0.3f * (facing_left ? -1 : 1);
            entity->velY  = -0.2f;
            if (entity->velX >  0.35f) entity->velX =  0.35f;
            if (entity->velX < -0.35f) entity->velX = -0.35f;
            entity_spawn_dust(entity, true, true, 0.2f);
            camera_screenshake(camera, 10, 0.5, 0.5);
        }
    }
    else pounce_timer -= delta_time;
    float peak_height = get(entity, "peak_height", Float, entity->posY);
    bool prev_in_air = get(entity, "prev_in_air", Bool, false);
    if (prev_in_air && (entity->flags & LE_EntityFlags_OnGround) && entity->posY - peak_height > 5) {
        entity_spawn_dust(entity, true, true, 0.2f);
    }
    if ((entity->flags & LE_EntityFlags_OnGround) && pounce_timer <= 0) pouncing = false;
    hud_update(entity);
    if (!disable_input) camera_set_focus(camera, entity->posX, 8);
    entity_fall_squish(entity, 10, .5f, .25f);
    entity_update_squish(entity, 5);
    set(entity, "pouncing", Bool, pouncing);
    set(entity, "pounce_timer", Float, pounce_timer);
    return false;
}

powerup(test) {
    bool hidden = get(entity, "hidden", Bool, false);
    set(entity, "prev_hidden", Bool, hidden);
    hidden = is_button_down(BUTTON_MOVE_DOWN);
    set(entity, "hidden", Bool, hidden);
    return true;
}
