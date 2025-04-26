#include "functions.h"
#include "game/level.h"
#include "io/io.h"
#include "main.h"
#include "math_util.h"
#include "game/data.h"

#include <float.h>
#include <string.h>

bool entity_init(LE_Entity* entity) {
    if (exists(entity, "init")) return false;
    set(entity, "init", Bool, true);
    return true;
}

void entity_apply_squish(LE_Entity* entity, float* w, float* h) {
    float squish = get(entity, "squish", Float, 1);
    *w -= *w * (squish - 1);
    *h += *h * (squish - 1);
}

void entity_update_squish(LE_Entity* entity, float modifier) {
    float squish = get(entity, "squish", Float, 1);
    squish += (1 - squish) / modifier * delta_time;
    set(entity, "squish", Float, squish);
}

void entity_fall_squish(LE_Entity* entity, float max_distance, float max_squish, float offset) {
    float peak_height = get(entity, "peak_height", Float, entity->posY);
    bool  prev_in_air = get(entity, "prev_in_air", Bool,  false);
    if (entity->flags & LE_EntityFlags_OnGround) {
        if (prev_in_air) {
            prev_in_air = false;
            set(entity, "prev_in_air", Bool,  prev_in_air);
            float diff = entity->posY - peak_height;
            float squish = 1 - ((diff / max_distance) * max_squish + offset);
            set(entity, "squish", Float, squish);
        }
        peak_height = entity->posY;
    }
    else {
        if (peak_height > entity->posY) peak_height = entity->posY;
        prev_in_air = true;
    }
    set(entity, "peak_height", Float, peak_height);
    set(entity, "prev_in_air", Bool,  prev_in_air);
}

bool entity_can_jump(LE_Entity* entity) {
    float coyote = get(entity, "coyote", Float, 99);
    if (entity->flags & LE_EntityFlags_OnGround) coyote = 0;
    else coyote += delta_time;
    set(entity, "coyote", Float, coyote);
    return coyote < 5;
}

bool entity_jump_requested(LE_Entity* entity, bool jump_pressed) {
    float jump_timer = get(entity, "jump_timer", Float, 999);
    if (jump_pressed) jump_timer = 0;
    else jump_timer += delta_time;
    set(entity, "jump_timer", Float, jump_timer);
    return jump_timer < 5;
}

bool entity_flip_texture(LE_Entity* entity) {
    bool flip = get(entity, "facing_left", Bool, false);
    if (entity->velX < 0) flip = true;
    if (entity->velX > 0) flip = false;
    set(entity, "facing_left", Bool, flip);
    return flip;
}

void entity_animate(int* srcX, int* srcY, int* srcW, int* srcH, int width, int height, int delay, int frames, bool loop, int curr_frame) {
    bool reverse = curr_frame < 0;
    curr_frame = abs(curr_frame);
    int frame = ((loop ? curr_frame : (int)min(delay * frames - 1, curr_frame)) / delay) % frames;
    if (reverse) frame = frames - 1 - frame;
    *srcX = width * frame;
    *srcY = 0;
    *srcW = width;
    *srcH = height;
}

int entity_advance_anim_frame(LE_Entity* entity) {
    float anim_timer = get(entity, "anim_timer", Float, -1);
    anim_timer += delta_time;
    set(entity, "anim_timer", Float, anim_timer);
    return anim_timer;
}

int entity_get_anim_frame(LE_Entity* entity) {
    return get(entity, "anim_timer", Float, -1);
}

bool entity_collided(LE_Entity* entity, LE_Direction* dir) {
    LE_EntityProperty collision;
    if (!LE_EntityGetProperty(entity, &collision, "collision")) return false;
    delete(entity, "collision");
    *dir = collision.asInt;
    return true;
}

void entity_spawn_dust(LE_Entity* entity, bool left, bool right, float speed) {
    LE_EntityBuilder* builder = get_entity_builder_by_id(dust);
    LE_EntityList* list = LE_EntityGetList(entity);
    if (left ) LE_CreateEntity(list, builder, entity->posX, entity->posY)->velX = -speed;
    if (right) LE_CreateEntity(list, builder, entity->posX, entity->posY)->velX =  speed;
}

bool entity_should_squish(LE_Entity* entity, LE_Entity* collider) {
    float stomp_timer = get(entity, "stomp_timer", Float, 0);
    bool squish = false;
    if (stomp_timer != 0) stomp_timer -= delta_time;
    else if (collider->velY > 0.05) {
        stomp_timer = 5;
        squish = true;
    }
    set(entity, "stomp_timer", Float, stomp_timer);
    return squish || stomp_timer != 0;
}

struct DitherContext {
    LE_Entity* entity;
    void* gfxcmd;
};

void entity_dither(void* _context, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    struct DitherContext* context = _context;
    float timer = get(context->entity, "despawn_timer", Float, 0);
    int dither_amount = round(max(0, 16 - timer / 2));
    graphics_set_shader(GET_ASSET(struct GfxResource, "shaders/dither.glsl"));
    graphics_shader_set_int("u_dither_amount", dither_amount);
    graphics_shader_set_float("u_offset_x", remainder(-dstx, 1));
    graphics_shader_set_float("u_offset_y", remainder(-dsty, 1));
    gfxcmd_process(context->gfxcmd, dstx, dsty, dstw, dsth, srcx, srcy, srcw, srch, color);
    graphics_set_shader(graphics_dummy_shader());
    free(context);
}

void* dither_context(LE_Entity* entity, void* gfxcmd) {
    struct DitherContext* context = malloc(sizeof(struct DitherContext));
    context->entity = entity;
    context->gfxcmd = gfxcmd;
    return context;
}

LE_Entity* find_entity_with_tag(const char* tag) {
    LE_LayerList* layers = current_level->layers;
    LE_LayerListIter* i = LE_LayerListGetIter(layers);
    while (i) {
        LE_Layer* layer = LE_LayerListGet(i);
        if (LE_LayerGetType(layer) == LE_LayerType_Entity) {
            LE_EntityList* list = LE_LayerGetDataPointer(layer);
            LE_EntityListIter* j = LE_EntityListGetIter(list);
            while (j) {
                LE_Entity* entity = LE_EntityListGet(j);
                const char* entity_tag = get(entity, "tag", Ptr, "");
                if (strcmp(entity_tag, tag) == 0) return entity;
                j = LE_EntityListNext(j);
            }
        }
        i = LE_LayerListNext(i);
    }
    return NULL;
}

LE_Entity* find_nearest_entity_with_tag(LE_Entity* self, const char* tag, float* distance) {
    return find_nearest_entity_with_tag_at_position(LE_EntityGetList(self), self->posX, self->posY, tag, distance);
}

LE_Entity* find_nearest_entity_with_tag_at_position(LE_EntityList* list, float x, float y, const char* tag, float* distance) {
    LE_Entity* nearest = NULL;
    float nearest_dist = FLT_MAX;
    LE_EntityListIter* iter = LE_EntityListGetIter(list);
    while (iter) {
        LE_Entity* entity = LE_EntityListGet(iter);
        const char* entity_tag = get(entity, "tag", Ptr, "");
        if (strcmp(entity_tag, tag) == 0) {
            float dx = x - entity->posX;
            float dy = y - entity->posY;
            float dist = dx * dx + dy * dy;
            if (nearest_dist > dist) {
                nearest_dist = dist;
                nearest = entity;
            }
        }
        iter = LE_EntityListNext(iter);
    }
    if (distance) *distance = nearest_dist;
    return nearest;
}