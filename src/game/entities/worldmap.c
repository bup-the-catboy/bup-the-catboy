#include "functions.h"

#include <lunarengine.h>

#include "game/input.h"
#include "game/overlay/transition.h"
#include "game/level.h"
#include "game/savefile.h"
#include "io/io.h"
#include "math_util.h"

static int activated_level = 0;
static void start_level() {
    load_level_by_id(activated_level);
}

static bool can_go(LE_Entity* entity) {
    int  level  = get(entity, "requires_level", Int, -1);
    bool secret = get(entity, "secret_path", Bool, false);
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
        if (get(entity, "id", Int, id) == id) return entity;
        iter = LE_EntityListNext(iter);
    }
    return NULL;
}

static int continue_to(LE_Entity* entity, int prev, int curr) {
    LE_Entity* curr_node = get_worldmap_node(entity, curr);
    int up    = get(curr_node,    "up_node", Int, -1);
    int left  = get(curr_node,  "left_node", Int, -1);
    int down  = get(curr_node,  "down_node", Int, -1);
    int right = get(curr_node, "right_node", Int, -1);
    int level = get(curr_node, "level_id"  , Int, -1);
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
    int curr_node_id = get(entity, "curr_node", Int, 0);
    int next_node_id = get(entity, "next_node", Int, curr_node_id);
    LE_Entity* curr_node = get_worldmap_node(entity, curr_node_id);
    LE_Entity* next_node = get_worldmap_node(entity, next_node_id);
    bool flipped = get(entity, "flipped", Bool, false);
    bool walking = curr_node_id != next_node_id;
    if (curr_node->posX < next_node->posX) flipped = false;
    if (curr_node->posX > next_node->posX) flipped = true;
    *srcX = (walking
        ? walk_anim_table[(global_timer / 10) % 2]
        : idle_anim_table[(global_timer / 10) % 6]
    ) * 16;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = flipped ? -16 : 16;
    *h = 16;
    return gfxcmd_texture("images/entities/player.png");
}

entity_update(worldmap_player) {
    int curr_node_id = get(entity, "curr_node", Int, 0);
    int next_node_id = get(entity, "next_node", Int, curr_node_id);
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
            if (curr_node_id == next_node_id) {
                savefile->map_id = curr_level_id - LVLID_WORLDMAP;
                savefile->map_node = curr_node_id;
                savefile_save();
            }
        }
    }
    else {
        int up    = get(curr_node,    "up_node", Int, 0);
        int left  = get(curr_node,  "left_node", Int, 0);
        int down  = get(curr_node,  "down_node", Int, 0);
        int right = get(curr_node, "right_node", Int, 0);
        int level = get(curr_node, "level_id"  , Int, 0);
        if (is_button_pressed(BUTTON_JUMP) && level != -1) {
            activated_level = level;
            start_transition(start_level, 60, LE_Direction_Up, cubic_in_out);
        }
        if (is_button_pressed(BUTTON_MOVE_UP))    next_node_id = up;
        if (is_button_pressed(BUTTON_MOVE_LEFT))  next_node_id = left;
        if (is_button_pressed(BUTTON_MOVE_DOWN))  next_node_id = down;
        if (is_button_pressed(BUTTON_MOVE_RIGHT)) next_node_id = right;
        if (next_node_id == -1) next_node_id = curr_node_id;
    }
    set(entity, "curr_node", Int, curr_node_id);
    set(entity, "next_node", Int, next_node_id);
}
