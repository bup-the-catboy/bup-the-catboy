#include <lunarengine.h>

bool entity_init(LE_Entity* entity);
void entity_apply_squish(LE_Entity* entity, float* w, float* h);
void entity_update_squish(LE_Entity* entity, float modifier);
void entity_fall_squish(LE_Entity* entity, float max_distance, float max_squish, float offset);
bool entity_can_jump(LE_Entity* entity);
bool entity_jump_requested(LE_Entity* entity, bool jump_pressed);
bool entity_flip_texture(LE_Entity* entity);
void entity_animate(int* srcX, int* srcY, int* srcW, int* srcH, int width, int height, int delay, int frames);
bool entity_collided(LE_Entity* entity, enum LE_Direction* dir);

#define entity_update(name) void name##_update(LE_Entity* entity)
#define entity_texture(name) void* name##_texture(LE_Entity* entity, float* w, float* h, int* srcX, int* srcY, int* srcW, int* srcH)
#define entity_collision(name) void name##_collision(LE_Entity* entity, LE_Entity* collider)

entity_update(player_spawner);
entity_update(player);
entity_update(gravity);
entity_update(walk);
entity_texture(player);
entity_texture(mouse);