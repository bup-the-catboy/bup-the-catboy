#include "defines.h"

ENTITY(player,
    UPDATE(player_update)
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
)

ENTITY(network_player,
    UPDATE(network_player_update)
    TEXTURE(network_player_texture)
    SIZE(0.75f, 0.75f)
)
