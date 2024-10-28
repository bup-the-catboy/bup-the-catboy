#ifndef NO_VSCODE
#include "defines.h"
#endif

ENTITY(player_spawner,
    UPDATE(player_spawner_update)
    LVLEDIT_TEXTURE("images/entities/player.png")
)

ENTITY(player,
    UPDATE(player_update)
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
    LVLEDIT_HIDE()
)

ENTITY(network_player,
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
    LVLEDIT_HIDE()
)
