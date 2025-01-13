#include "functions.h"
#include "game/data.h"
#include "io/assets/assets.h"
#include "main.h"
#include "rng.h"

entity_texture(turtle) {
    bool do_flip = entity_flip_texture(entity);
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 10, 2, true, global_timer);
    *w = 16 * (do_flip ? -1 : 1);
    *h = 16;
    return GET_ASSET(struct GfxResource, "images/entities/turtle.png");
}

entity_texture(turtle_shell) {
    bool do_flip = entity_flip_texture(entity);
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 5, 3, true, do_flip ? -global_timer : global_timer);
    *srcX += 48;
    *w = 16;
    *h = 16;
    return GET_ASSET(struct GfxResource, "images/entities/turtle.png");
}

entity_texture(turtle_shell_fragment) {
    *srcX = 0;
    *srcY = 0;
    *srcW = 8;
    *srcH = 8;
    *w = 8;
    *h = 8;
    return GET_ASSET(struct GfxResource, "images/entities/turtle_shell_fragment.png");
}

entity_update(turtle_shell) {
    enum LE_Direction dir;
    if (entity_init(entity)) {
        LE_EntityProperty walk_speed = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asFloat = 0 }, "walk_speed");
        LE_Entity* player = find_nearest_entity_with_tag(entity, "player");
        if (player->posX < entity->posX) entity->velX =  walk_speed.asFloat;
        else                             entity->velX = -walk_speed.asFloat;
    }
    if (entity_collided(entity, &dir)) {
        if (dir == LE_Direction_Left || dir == LE_Direction_Right) {
            float x = entity->posX;
            float y = entity->posY;
            for (int i = 0; i < 4; i++) {
                LE_Entity* fragment = LE_CreateEntity(LE_EntityGetList(entity), get_entity_builder_by_id(turtle_shell_fragment), x, y);
                fragment->velX = random_float() * (dir == LE_Direction_Left ? -0.2 : 0.2);
                fragment->velY = random_range(-0.4, -0.2);
            }
            LE_DeleteEntity(entity);
        }
    }
}

