#include "functions.h"
#include "assets/assets.h"

entity_texture(mouse) {
    bool do_flip = entity_flip_texture(entity);
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 10, 2);
    *w = 16 * (do_flip ? -1 : 1);
    *h = 16;
    return GET_ASSET(struct Texture, "images/entities/mouse.png");
}

entity_texture(squashed_mouse) {
    *srcX = 32;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = 16;
    *h = 16;
    return GET_ASSET(struct Texture, "images/entities/mouse.png");
}

entity_update(squashed_mouse) {
    LE_EntityProperty timer = (LE_EntityProperty){ .asInt = 60 };
    LE_EntityGetProperty(entity, &timer, "timer");
    timer.asInt--;
    if (timer.asInt == 0) LE_DeleteEntity(entity);
    LE_EntitySetProperty(entity, timer, "timer");
}