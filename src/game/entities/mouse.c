#include "functions.h"
#include "io/assets/assets.h"
#include "main.h"

entity_texture(mouse) {
    bool do_flip = entity_flip_texture(entity);
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 10, 2, true, global_timer);
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
    entity_apply_squish(entity, w, h);
    return GET_ASSET(struct Texture, "images/entities/mouse.png");
}

entity_update(squashed_mouse) {
    if (entity_init(entity)) LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = .5f }, "squish");
    entity_update_squish(entity, 5);
}