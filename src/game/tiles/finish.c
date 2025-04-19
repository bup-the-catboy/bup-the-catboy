#include "functions.h"

#include "game/entities/functions.h"
#include "game/level.h"
#include "game/data.h"
#include <stdlib.h>

struct FinishLevelData {
    float pos_x, pos_y, vel_x, vel_y;
};

void finish_level_anim(void* _) {
    struct FinishLevelData* data = _;
    camera = camera_create();
    camera_set_focus(camera, 12, 8);
    camera_snap(camera);
    LE_Entity* player = find_entity_with_tag("player");
    player->posX = data->pos_x + 12;
    player->posY = data->pos_y + 8;
    player->velX = data->vel_x;
    player->velY = data->vel_y;
    set(player, "disable_input", Bool, true);
    LE_CreateEntity(LE_EntityGetList(player), get_entity_builder_by_id(level_finish), 0, 0);
    for (int i = 0; i < 8; i++) {
        LE_CreateEntity(LE_EntityGetList(player), get_entity_builder_by_id(trail_spawner), player->posX, player->posY);
    }
    free(data);
}

void finish_level() {
    float cam_x, cam_y;
    camera_get(camera, &cam_x, &cam_y);
    LE_Entity* player = find_entity_with_tag("player");
    struct FinishLevelData* data = malloc(sizeof(struct FinishLevelData));
    data->pos_x = player->posX - cam_x;
    data->pos_y = player->posY - cam_y;
    data->vel_x = player->velX;
    data->vel_y = player->velY;
    load_level(GET_ASSET(struct Binary, "levels/levelclear.lvl"));
    post_update(finish_level_anim, data);
}

tile_collision(finish) {
    post_update(finish_level, NULL);
}