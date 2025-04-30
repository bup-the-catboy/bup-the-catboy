#include "functions.h"
#include "io/io.h"
#include "main.h"

entity_update(dust) {
    if (entity_init(entity)) set(entity, "initial_speed", Float, entity->velX);
    float initial_speed = get(entity, "initial_speed", Float, entity->velX);
    entity->velX -= initial_speed / 16 * delta_time;
    int frame = entity_advance_anim_frame(entity);
    if (frame >= 8 * 2) LE_DeleteEntity(entity);
}

entity_texture(dust) {
    entity_animate(srcX, srcY, srcW, srcH, 8, 8, 2, 8, false, entity_get_anim_frame(entity));
    *w = 8;
    *h = 8;
    return gfxcmd_texture("images/entities/dust.png");
}