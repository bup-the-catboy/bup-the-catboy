#include "functions.h"

#include "main.h"
#include "io/assets/assets.h"

entity_texture(crate_fragment) {
    LE_EntityProperty timer = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "timer");
    if (!(entity->flags & LE_EntityFlags_OnGround)) timer.asFloat += delta_time;
    switch ((int)(timer.asFloat / 5) % 4) {
        case 0:
            *w =  8;
            *h =  8;
            break;
        case 1:
            *w = -8;
            *h =  8;
            break;
        case 2:
            *w = -8;
            *h = -8;
            break;
        case 3:
            *w =  8;
            *h = -8;
            break;
    }
    *srcX = 0;
    *srcY = 0;
    *srcW = 8;
    *srcH = 8;
    LE_EntitySetProperty(entity, timer, "timer");
    return GET_ASSET(struct GfxResource, "images/entities/crate_fragment.png");
}