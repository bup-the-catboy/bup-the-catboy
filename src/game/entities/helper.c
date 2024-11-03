#include "functions.h"
#include "main.h"

bool entity_init(LE_Entity* entity) {
    if (LE_EntityGetProperty(entity, NULL, "init")) return false;
    LE_EntitySetProperty(entity, (LE_EntityProperty){ .asBool = true }, "init");
    return true;
}

void entity_apply_squish(LE_Entity* entity, float* w, float* h) {
    LE_EntityProperty squish = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 1 }, "squish");
    *w -= *w * (squish.asFloat - 1);
    *h += *h * (squish.asFloat - 1);
}

void entity_update_squish(LE_Entity* entity, float modifier) {
    LE_EntityProperty squish = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 1 }, "squish");
    squish.asFloat += (1 - squish.asFloat) / modifier;
    LE_EntitySetProperty(entity, squish, "squish");
}

void entity_fall_squish(LE_Entity* entity, float max_distance, float max_squish, float offset) {
    LE_EntityProperty peak_height = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = entity->posY }, "peak_height");
    LE_EntityProperty prev_in_air = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool  = false        }, "prev_in_air");
    if (entity->flags & LE_EntityFlags_OnGround) {
        if (prev_in_air.asBool) {
            prev_in_air.asBool = false;
            LE_EntitySetProperty(entity, prev_in_air, "prev_in_air");
            float diff = entity->posY - peak_height.asFloat;
            float squish = 1 - ((diff / max_distance) * max_squish + offset);
            LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = squish }, "squish");
        }
        peak_height.asFloat = entity->posY;
    }
    else {
        if (peak_height.asFloat > entity->posY) peak_height.asFloat = entity->posY;
        prev_in_air.asBool = true;
    }
    LE_EntitySetProperty(entity, peak_height, "peak_height");
    LE_EntitySetProperty(entity, prev_in_air, "prev_in_air");
}

bool entity_can_jump(LE_Entity* entity) {
    LE_EntityProperty coyote = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 999 }, "coyote");
    if (entity->flags & LE_EntityFlags_OnGround) coyote.asInt = 0;
    else coyote.asInt++;
    LE_EntitySetProperty(entity, coyote, "coyote");
    return coyote.asInt < 3;
}

bool entity_jump_requested(LE_Entity* entity, bool jump_pressed) {
    LE_EntityProperty jump_timer = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 999 }, "jump_timer");
    if (jump_pressed) jump_timer.asInt = 0;
    else jump_timer.asInt++;
    LE_EntitySetProperty(entity, jump_timer, "jump_timer");
    return jump_timer.asInt < 5;
}

bool entity_flip_texture(LE_Entity* entity) {
    LE_EntityProperty flip = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool  = false }, "facing_left");
    if (entity->velX < 0) flip.asBool = true;
    if (entity->velX > 0) flip.asBool = false;
    LE_EntitySetProperty(entity, flip, "facing_left");
    return flip.asBool;
}

void entity_animate(int* srcX, int* srcY, int* srcW, int* srcH, int width, int height, int delay, int frames) {
    int frame = (global_timer / delay) % frames;
    *srcX = width * frame;
    *srcY = 0;
    *srcW = width;
    *srcH = height;
}

bool entity_collided(LE_Entity* entity, enum LE_Direction* dir) {
    LE_EntityProperty collision;
    if (!LE_EntityGetProperty(entity, &collision, "collision")) return false;
    LE_EntityDelProperty(entity, "collision");
    *dir = collision.asInt;
    return true;
}