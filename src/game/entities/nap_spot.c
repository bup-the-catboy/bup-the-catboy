#include "font/font.h"
#include "functions.h"
#include "game/overlay/transition.h"
#include "game/savefile.h"
#include "io/io.h"
#include "game/level.h"
#include "game/data.h"
#include "rng.h"
#include <stdlib.h>

static void load_world_map() {
    load_level_by_id(LVLID_WORLDMAP + savefile->map_id);
}

static void start_world_map_transition() {
    start_transition(load_world_map, 60, LE_Direction_Up, cubic_in_out);
}

entity_update(nap_spot) {
    float distance = 0;
    LE_Entity* player = find_nearest_entity_with_tag(entity, "player", &distance);
    if (distance > .5f || get(player, "sleeping", Bool, false)) return;
    post_update_timer(start_world_map_transition, NULL, 180);
    player->posX = entity->posX;
    player->posY = entity->posY;
    player->velX = player->velY = 0;
    player->width = player->height = 0;
    set(player, "sleeping", Bool, true);
    set(player, "gravity", Float, 0);
    LE_CreateEntity(LE_EntityGetList(player), get_entity_builder_by_id(level_finish), 0, 0);
    savefile->level_flags[0] |= get(entity, "is_secret", Bool, false) ? LEVEL_FLAG_SECRET_EXIT : LEVEL_FLAG_COMPLETED;
    savefile_save();
}

entity_texture(nap_spot) {
    if (!get(entity, "display_nap_spot", Bool, true)) return NULL;
    *srcX = 0;
    *srcY = 0;
    *srcW = *w = 32;
    *srcH = *h = 32;
    return gfxcmd_texture("images/entities/napspot.png");
}

entity_texture(level_finish) {
    float timer = get(entity, "timer", Float, 0);
    if (timer < 0) return NULL;
    float shake = max(0, (30 - timer) / 30) * 16;
    float text_w, text_h;
    const char* text = "${^200}LEVEL CLEAR";
    text_size(&text_w, &text_h, text);
    render_text(drawlist, (WIDTH - text_w) / 2 + random_range(-shake, shake), 64 + random_range(-shake, shake), text);
    return NULL;
}
