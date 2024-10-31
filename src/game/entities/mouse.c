#include "functions.h"
#include "assets/assets.h"

entity_texture(mouse) {
    bool do_flip = entity_flip_texture(entity);
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 10, 2);
    *w = 16 * (do_flip ? -1 : 1);
    *h = 16;
    return GET_ASSET(struct Texture, "images/entities/mouse.png");
}