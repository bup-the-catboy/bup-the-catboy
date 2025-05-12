#include "functions.h"
#include "game/camera.h"
#include "game/data.h"
#include "game/level.h"
#include "game/savefile.h"
#include "io/io.h"
#include "rng.h"
#include <string.h>

// haha funny robot game
// 🪙💥🔫

entity_texture(sparkles) {
    entity_animate(srcX, srcY, srcW, srcH, 16, 16, 4, 4, false, entity_get_anim_frame(entity));
    *w = 16;
    *h = 16;
    return gfxcmd_texture("images/entities/sparkles.png");
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
    return gfxcmd_texture("images/entities/coin_particle.png");
}

static void draw_cat_coin(void* entity, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    bool collected = savefile->level_flags[curr_level_id] & (1 << get(entity, "id", Int, 0));
    LE_DrawList* dl = LE_CreateDrawList();
    LE_Tileset* tileset = LE_TilemapGetTileset(LE_EntityGetTilemap(LE_EntityGetList(entity)));
    LE_TileData** data = get_tile_palette_by_id(generic);
    LE_DrawTileAt(data[collected * 4 + TILE_DATA_catcoin_top_left],     tileset, dstx +  0, dsty +  0, 1, 1, dl);
    LE_DrawTileAt(data[collected * 4 + TILE_DATA_catcoin_top_right],    tileset, dstx + 16, dsty +  0, 1, 1, dl);
    LE_DrawTileAt(data[collected * 4 + TILE_DATA_catcoin_bottom_left],  tileset, dstx +  0, dsty + 16, 1, 1, dl);
    LE_DrawTileAt(data[collected * 4 + TILE_DATA_catcoin_bottom_right], tileset, dstx + 16, dsty + 16, 1, 1, dl);
    LE_Render(dl, gfxcmd_process);
    LE_DestroyDrawList(dl);
}

entity_texture(cat_coin) {
    *w = 32;
    *h = 32;
    return gfxcmd_custom(draw_cat_coin, entity);
}

entity_collision(cat_coin) {
    if (strcmp(get(collider, "tag", Ptr, ""), "player") != 0) return;
    int mask = 1 << get(entity, "id", Int, 0);
    if (!(savefile->level_flags[curr_level_id] & mask)) camera_screenshake(camera, 30, .5f, .5f);
    LE_EntityList* list = LE_EntityGetList(find_entity_with_tag("player"));
    LE_DeleteEntity(entity);
    LE_CreateEntity(list, get_entity_builder_by_id(sparkles), entity->posX, entity->posY - 0.5f);
    for (int i = 0; i < 16; i++) {
        LE_CreateEntity(list, get_entity_builder_by_id(coin_particle), entity->posX, entity->posY - 0.5f);
    }
    savefile->level_flags[curr_level_id] |= mask;
}