#include "functions.h"

#include "io/assets/assets.h"
#include "io/io.h"

entity_update(notified) {
    if (entity_init(entity)) set(entity, "squish", Float, 1.75f);
    entity_update_squish(entity, 5);
}

entity_texture(notified) {
    *srcX = 0;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = 16;
    *h = 16;
    entity_apply_squish(entity, w, h);
    return gfxcmd_custom(entity_dither, dither_context(entity, GET_ASSET(struct GfxResource, "images/entities/notified.png")));
}
