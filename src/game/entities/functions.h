#include <lunarengine.h>

void entity_apply_squish(LE_Entity* entity, float* w, float* h);
void entity_update_squish(LE_Entity* entity, float modifier);
void entity_fall_squish(LE_Entity* entity, float max_distance, float max_squish, float offset);
bool entity_can_jump(LE_Entity* entity);
bool entity_jump_requested(LE_Entity* entity, bool jump_pressed);

#define entity_update(name) void name##_update(LE_Entity* entity)
#define entity_texture(name) void* name##_texture(LE_Entity* entity, float* w, float* h, int* srcX, int* srcY, int* srcW, int* srcH)
#define entity_collision(name) void name##_collision(LE_Entity* entity, LE_Entity* collider)

entity_update(player_spawner);
entity_update(player);
entity_texture(player);