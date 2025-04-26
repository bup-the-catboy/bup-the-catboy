#include "functions.h"
#include "io/assets/assets.h"
#include "io/io.h"
#include "rng.h"

// haha funny robot game
// 🪙💥🔫

entity_texture(sparkles) {
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 4, 4, false, entity_get_anim_frame(entity));
    *w = 16;
    *h = 16;
    return GET_ASSET(struct GfxResource, "images/entities/sparkles.png");
}

entity_update(coin_particle) {
    if (entity_init(entity)) {
        entity->velY = random_range(-0.1, -0.3);
        entity->velX = random_range(-0.2,  0.2);
    }
}

entity_texture(coin_particle) {
    *srcX = 0;
    *srcY = 0;
    *srcW = 4;
    *srcH = 4;
    *w = 4;
    *h = 4;
    return gfxcmd_custom(entity_dither, dither_context(entity, GET_ASSET(struct GfxResource, "images/entities/coin_particle.png")));
}