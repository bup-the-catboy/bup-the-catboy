#include <lunarengine.h>

#define tile_texture(name) int tiletex_##name(LE_TileData* tiledata)
#define tile_collision(name) void tilecol_##name(LE_TileData* tile, LE_Tilemap* tilemap, LE_Entity* entity, int tileX, int tileY, LE_Direction direction)

tile_collision(crate);
tile_collision(brick);
tile_collision(coin);
tile_collision(life_coin);
