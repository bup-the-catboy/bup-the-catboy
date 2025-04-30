#include "functions.h"

#include "game/data.h"
#include "game/savefile.h"
#include "io/io.h"
#include "main.h"

#include <string.h>

void draw_coin(void* context, float dstX, float dstY, float dstW, float dstH, int srcX, int srcY, int srcW, int srcH, unsigned int color) {
    LE_Entity* entity = context;
    LE_DrawList* dl = LE_CreateDrawList();
    LE_DrawTileAt(get_tile_palette_by_id(generic)[TILE_DATA_coin], LE_TilemapGetTileset(LE_EntityGetTilemap(LE_EntityGetList(entity))), dstX, dstY, dstW / 16, dstH / 16, dl);
    LE_Render(dl, gfxcmd_process);
    LE_DestroyDrawList(dl);
}

entity_update(crate_loot) {
    if (entity_init(entity)) {
        entity->velY = -.4f;
    }
    LE_Direction dir;
    if (entity_collided(entity, &dir) && dir == LE_Direction_Up) {
        if (get(entity, "bounced", Bool, false) == false) {
            set(entity, "bounced", Bool, true);
            set(entity, "gravity", Float, 0.02);
            entity->velY = -.2f;
        }
    }
}

entity_texture(crate_coin) {
    *srcX = 0;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = 16;
    *h = 16;
    return gfxcmd_custom(draw_coin, entity);
}

entity_texture(crate_heart) {
    *srcX = 0;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = 16;
    *h = 16;
    return gfxcmd_texture("images/entities/heart.png");
}

entity_collision(crate_coin) {
    if (strcmp(get(collider, "tag", Ptr, ""), "player") != 0) return;
    LE_EntityList* list = LE_EntityGetList(collider);
    LE_CreateEntity(list, get_entity_builder_by_id(sparkles), entity->posX, entity->posY);
    for (int i = 0; i < 8; i++) {
        LE_CreateEntity(list, get_entity_builder_by_id(coin_particle), entity->posX, entity->posY - 0.5f);
    }
    savefile->coins++;
    LE_DeleteEntity(entity);
}

entity_collision(crate_heart) {
    if (strcmp(get(collider, "tag", Ptr, ""), "player") != 0) return;
    LE_EntityList* list = LE_EntityGetList(collider);
    // todo: some fancy effect or some shit idk
    savefile->hearts++;
    LE_DeleteEntity(entity);
}

entity_texture(crate_fragment) {
    float timer = get(entity, "timer", Float, 0);
    if (!(entity->flags & LE_EntityFlags_OnGround)) timer += delta_time;
    switch ((int)(timer / 5) % 4) {
        case 0:
            *w =  8;
            *h =  8;
            break;
        case 1:
            *w = -8;
            *h =  8;
            break;
        case 2:
            *w = -8;
            *h = -8;
            break;
        case 3:
            *w =  8;
            *h = -8;
            break;
    }
    *srcX = 0;
    *srcY = 0;
    *srcW = 8;
    *srcH = 8;
    set(entity, "timer", Float, timer);
    return gfxcmd_custom(entity_dither, dither_context(entity, gfxcmd_texture("images/entities/crate_fragment.png")));
}