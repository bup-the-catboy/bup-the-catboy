#include <lunarengine.h>

bool entity_init(LE_Entity* entity);
void entity_apply_squish(LE_Entity* entity, float* w, float* h);
void entity_update_squish(LE_Entity* entity, float modifier);
void entity_fall_squish(LE_Entity* entity, float max_distance, float max_squish, float offset);
bool entity_can_jump(LE_Entity* entity);
bool entity_jump_requested(LE_Entity* entity, bool jump_pressed);
bool entity_flip_texture(LE_Entity* entity);
void entity_animate(int* srcX, int* srcY, int* srcW, int* srcH, int width, int height, int delay, int frames, bool loop, int curr_frame);
int  entity_advance_anim_frame(LE_Entity* entity);
int  entity_get_anim_frame(LE_Entity* entity);
bool entity_collided(LE_Entity* entity, LE_Direction* dir);
void entity_spawn_dust(LE_Entity* entity, bool left, bool right, float speed);
bool entity_should_squish(LE_Entity* entity, LE_Entity* collider);

void  entity_dither(void* context, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color);
void  entity_turtle_shelled(void* _context, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color);
void* custom_cmd_context(LE_Entity* entity, void* gfxcmd);

LE_Entity* find_entity_with_tag(const char* tag);
LE_Entity* find_nearest_entity_with_tag(LE_Entity* self, const char* tag, float* distance);
LE_Entity* find_nearest_entity_with_tag_at_position(LE_EntityList* list, float x, float y, const char* tag, float* distance);

#define entity_update(name) void name##_update(LE_Entity* entity)
#define entity_texture(name) void* name##_texture(LE_Entity* entity, float* w, float* h, int* srcX, int* srcY, int* srcW, int* srcH)
#define entity_collision(name) void name##_collision(LE_Entity* entity, LE_Entity* collider)
#define powerup(name) bool powerup_##name##_update(LE_Entity* entity)

#define get(ent, name, type, default) LE_EntityGetPropertyOrDefault(ent, (LE_EntityProperty){ .as##type = default }, name).as##type
#define set(ent, name, type, value)   LE_EntitySetProperty         (ent, (LE_EntityProperty){ .as##type = value   }, name)
#define delete(ent, name) LE_EntityDelProperty(ent,       name)
#define exists(ent, name) LE_EntityGetProperty(ent, NULL, name)

entity_update(player_spawner);
entity_update(player);
entity_update(dead_player);
entity_update(walk);
entity_update(squashed_mouse);
entity_update(turtle_shell);
entity_update(turtle_shelled);
entity_update(dust);
entity_update(gravity);
entity_update(animable);
entity_update(despawn);
entity_update(friction);
entity_update(death_barrier);
entity_update(coin_particle);
entity_update(trail_spawner);
entity_update(timer);
entity_update(worldmap_player);
entity_update(notified);
entity_update(crate_loot);
entity_update(nap_spot);
entity_collision(squash);
entity_collision(crate_coin);
entity_collision(crate_heart);
entity_collision(crate_powerup);
entity_texture(player);
entity_texture(dead_player);
entity_texture(mouse);
entity_texture(squashed_mouse);
entity_texture(turtle);
entity_texture(turtle_shell);
entity_texture(turtle_shell_fragment);
entity_texture(dust);
entity_texture(crate_fragment);
entity_texture(shader_controller);
entity_texture(sparkles);
entity_texture(coin_particle);
entity_texture(trail);
entity_texture(level_finish);
entity_texture(worldmap_player);
entity_texture(notified);
entity_texture(crate_coin);
entity_texture(crate_heart);
entity_texture(crate_powerup);
entity_texture(broken_heart);
entity_texture(nap_spot);

powerup(death);
powerup(base);
powerup(test);