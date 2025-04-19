#include "functions.h"

#include "io/assets/assets.h"

entity_update(notified) {
    if (entity_init(entity)) LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = 1.75f }, "squish");
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
    return GET_ASSET(struct GfxResource, "images/entities/notified.png");
}
