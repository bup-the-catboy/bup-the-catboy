#include "functions.h"
#include "io/io.h"
#include "main.h"

entity_texture(mouse) {
    bool do_flip = entity_flip_texture(entity);
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 10, 2, true, global_timer);
    *w = 16 * (do_flip ? -1 : 1);
    *h = 16;
    return gfxcmd_texture("images/entities/mouse.png");
}

entity_texture(squashed_mouse) {
    *srcX = 32;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = 16;
    *h = 16;
    entity_apply_squish(entity, w, h);
    return gfxcmd_custom(entity_dither, dither_context(entity, gfxcmd_texture( "images/entities/mouse.png")));
}

entity_update(squashed_mouse) {
    if (entity_init(entity)) set(entity, "squish", Float, .5f);
    entity_update_squish(entity, 5);
}