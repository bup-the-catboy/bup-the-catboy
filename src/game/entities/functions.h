#include <lunarengine.h>

#define entity_update(name) void name##_update(LE_Entity* entity)
#define entity_texture(name) void* name##_texture(LE_Entity* entity, int* w, int* h, int* srcX, int* srcY, int* srcW, int* srcH)
#define entity_collision(name) void name##_collision(LE_Entity* entity, LE_Entity* collider)

entity_update(player);
entity_update(network_player);
entity_texture(player);
entity_texture(network_player);