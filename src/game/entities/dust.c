#include "functions.h"
#include "assets/assets.h"

entity_update(dust) {
    LE_EntityProperty initial_speed = (LE_EntityProperty){ .asFloat = entity->velX };
    if (entity_init(entity)) LE_EntitySetProperty(entity, initial_speed, "initial_speed");
    LE_EntityGetProperty(entity, &initial_speed, "intial_speed");
    entity->velX -= initial_speed.asFloat / 8;
    if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0 }, "anim_timer").asInt == 8 * 2) LE_DeleteEntity(entity);
}

entity_texture(dust) {
    entity_animate(srcX, srcY, srcW, srcH, 8, 8, 2, 8, false, entity_get_anim_frame(entity));
    *w = 8;
    *h = 8;
    return GET_ASSET(struct Texture, "images/entities/dust.png");
}