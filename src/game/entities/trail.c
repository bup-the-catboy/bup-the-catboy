#include "functions.h"

#include <lunarengine.h>
#include "math_util.h"

#include "game/data.h"
#include "io/assets/assets.h"
#include "rng.h"

entity_update(trail_spawner) {
    if (entity_init(entity)) {
        entity->velY = random_range(-0.1, -0.3);
        entity->velX = random_range(-0.2,  0.2);
    }
    float prevPosX = get(entity, "prev_pos_x", Float, entity->posX);
    float prevPosY = get(entity, "prev_pos_y", Float, entity->posY);

    int num_particles = sqrtf((entity->posX - prevPosX) * (entity->posX - prevPosX) + (entity->posY - prevPosY) * (entity->posY - prevPosY)) * 8;
    for (int i = 0; i < num_particles; i++) {
        float t = (float)i / num_particles;
        float x = lerp(t, prevPosX, entity->posX);
        float y = lerp(t, prevPosY, entity->posY);
        LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder_by_id(trail), x, y);
    }

    set(entity, "prev_pos_x", Float, entity->posX);
    set(entity, "prev_pos_y", Float, entity->posY);
}

entity_texture(trail) {
    entity_animate(srcX, srcY, srcW, srcH, 8, 8, 5, 8, false, entity_get_anim_frame(entity));
    *w = 8;
    *h = 8;
    return GET_ASSET(struct GfxResource, "images/entities/trail.png");
}