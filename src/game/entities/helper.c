#include "functions.h"

void entity_apply_squish(LE_Entity* entity, float* w, float* h) {
    LE_EntityProperty squish;
    if (!LE_EntityGetProperty(entity, &squish, "squish")) squish.asFloat = 1;
    *w -= *w * (squish.asFloat - 1);
    *h += *h * (squish.asFloat - 1);
}

void entity_update_squish(LE_Entity* entity, float modifier) {
    LE_EntityProperty squish;
    if (!LE_EntityGetProperty(entity, &squish, "squish")) squish.asFloat = 1;
    squish.asFloat += (1 - squish.asFloat) / modifier;
    LE_EntitySetProperty(entity, squish, "squish");
}

void entity_fall_squish(LE_Entity* entity, float max_distance, float max_squish, float offset) {
    LE_EntityProperty peak_height;
    LE_EntityProperty prev_in_air;
    if (!LE_EntityGetProperty(entity, &peak_height, "peak_height")) peak_height.asFloat = entity->posY;
    if (!LE_EntityGetProperty(entity, &prev_in_air, "prev_in_air")) prev_in_air.asBool = false;
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