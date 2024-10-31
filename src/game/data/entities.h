#ifndef NO_VSCODE
#include "defines.h"
#endif

ENTITY(player_spawner,
    UPDATE(player_spawner_update)
    LVLEDIT_TEXTURE("images/entities/player.png")
)

ENTITY(player,
    UPDATE(gravity_update)
    UPDATE(player_update)
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_HIDE()
)

ENTITY(network_player,
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
    LVLEDIT_HIDE()
)

ENTITY(mouse,
    UPDATE(gravity_update)
    UPDATE(walk_update)
    TEXTURE(mouse_texture)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("walk_speed", Float, 0.04)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_TEXTURE("images/entities/mouse.png")
)
