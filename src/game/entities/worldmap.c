#include "functions.h"

#include <lunarengine.h>

#include "game/input.h"
#include "game/overlay/transition.h"
#include "game/level.h"
#include "game/savefile.h"
#include "math_util.h"

static int activated_level = 0;
static void start_level() {
    char level[32];
    snprintf(level, 32, "levels/level%d.lvl", activated_level);
    load_level(GET_ASSET(struct Binary, level));
}

static bool can_go(LE_Entity* entity) {
    int  level  = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt =  -1 }, "requires_level").asInt;
    bool secret = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "secret_path").asBool;
    if (level == -1) return true;
    return savefile->level_flags[level] & (secret ? LEVEL_FLAG_SECRET_EXIT : LEVEL_FLAG_COMPLETED);
}

static float approach_point(float* curr_x, float* curr_y, float target_x, float target_y, float step) {
    float dx = target_x - *curr_x;
    float dy = target_y - *curr_y;
    float dist = sqrtf(dx * dx + dy * dy);
    float angle = atan2f(dy, dx);
    if (step > dist) step = dist;
    *curr_x += cosf(angle) * step;
    *curr_y += sinf(angle) * step;
    return step;
}

static LE_Entity* get_worldmap_node(LE_Entity* entity, int id) {
    LE_EntityList* list = LE_EntityGetList(entity);
    LE_EntityListIter* iter = LE_EntityListGetIter(list);
    while (iter) {
        LE_Entity* entity = LE_EntityListGet(iter);
        if (LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = -1 }, "id").asInt == id) return entity;
        iter = LE_EntityListNext(iter);
    }
    return NULL;
}

static int continue_to(LE_Entity* entity, int prev, int curr) {
    LE_Entity* curr_node = get_worldmap_node(entity, curr);
    int up    = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 },    "up_node").asInt;
    int left  = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 },  "left_node").asInt;
    int down  = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 },  "down_node").asInt;
    int right = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 }, "right_node").asInt;
    int level = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 }, "level_id"  ).asInt;
    if (level != -1) return curr;
    int next = curr;
    if (up == prev) {
        if (left != -1 && down == -1 && right == -1) next = left;
        if (left == -1 && down != -1 && right == -1) next = down;
        if (left == -1 && down == -1 && right != -1) next = right;
    }
    if (left == prev) {
        if (up != -1 && down == -1 && right == -1) next = up;
        if (up == -1 && down != -1 && right == -1) next = down;
        if (up == -1 && down == -1 && right != -1) next = right;
    }
    if (down == prev) {
        if (up != -1 && left == -1 && right == -1) next = up;
        if (up == -1 && left != -1 && right == -1) next = left;
        if (up == -1 && left == -1 && right != -1) next = right;
    }
    if (right == prev) {
        if (up != -1 && left == -1 && down == -1) next = up;
        if (up == -1 && left != -1 && down == -1) next = left;
        if (up == -1 && left == -1 && down != -1) next = down;
    }
    LE_Entity* next_node = get_worldmap_node(entity, next);
    return can_go(next_node) ? next : curr;
}

static int idle_anim_table[] = { 0, 1, 2, 3, 2, 1 };
static int walk_anim_table[] = { 4, 5 };

entity_texture(worldmap_player) {
    float prev_pos_x = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = entity->posX }, "prev_pos").asFloat;
    bool flipped = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asBool = false }, "flipped").asBool;
    bool walking = entity->posX == prev_pos_x;
    if (entity->posX < prev_pos_x) flipped = true;
    if (entity->posX > prev_pos_x) flipped = false;
    *srcX = (walking
        ? walk_anim_table[(global_timer / 10) % 2]
        : idle_anim_table[(global_timer / 10) % 6]
    ) * 16;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = flipped ? -16 : 16;
    *h = 16;
    LE_EntitySetProperty(entity, (LE_EntityProperty){ .asFloat = entity->posX }, "prev_pos");
    return GET_ASSET(struct GfxResource, "images/entities/player.png");
}

entity_update(worldmap_player) {
    int curr_node_id = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = 0            }, "curr_node").asInt;
    int next_node_id = LE_EntityGetPropertyOrDefault(entity, (LE_EntityProperty){ .asInt = curr_node_id }, "next_node").asInt;
    LE_Entity* curr_node = get_worldmap_node(entity, curr_node_id);
    LE_Entity* next_node = get_worldmap_node(entity, next_node_id);
    if (!curr_node) return;
    if (entity_init(entity)) {
        entity->posX = curr_node->posX;
        entity->posY = curr_node->posY;
        camera = camera_create();
        camera_set_focus(camera, 12, 8);
        camera_snap(camera);
    }
    if (next_node && can_go(next_node) && curr_node_id != next_node_id) {
        float desired_step = .2f;
        float step = approach_point(&entity->posX, &entity->posY, next_node->posX, next_node->posY, desired_step);
        if (desired_step != step) {
            int prev_node_id = curr_node_id;
            curr_node_id = next_node_id;
            next_node_id = continue_to(entity, prev_node_id, curr_node_id);
        }
    }
    else {
        int up    = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 },    "up_node").asInt;
        int left  = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 },  "left_node").asInt;
        int down  = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 },  "down_node").asInt;
        int right = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 }, "right_node").asInt;
        int level = LE_EntityGetPropertyOrDefault(curr_node, (LE_EntityProperty){ .asInt = 0 }, "level_id"  ).asInt;
        if (is_button_pressed(BUTTON_JUMP) && level != 0) {
            activated_level = level;
            start_transition(start_level, 60, LE_Direction_Up, cubic_in_out);
        }
        if (is_button_pressed(BUTTON_MOVE_UP))    next_node_id = up;
        if (is_button_pressed(BUTTON_MOVE_LEFT))  next_node_id = left;
        if (is_button_pressed(BUTTON_MOVE_DOWN))  next_node_id = down;
        if (is_button_pressed(BUTTON_MOVE_RIGHT)) next_node_id = right;
    }
    LE_EntitySetProperty(entity, (LE_EntityProperty){ .asInt = curr_node_id }, "curr_node");
    LE_EntitySetProperty(entity, (LE_EntityProperty){ .asInt = next_node_id }, "next_node");
}
