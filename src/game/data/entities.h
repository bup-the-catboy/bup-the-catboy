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
    DEFAULT_PROPERTY("tag", Ptr, (void*)"player")
    LVLEDIT_HIDE()
)

ENTITY(network_player,
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("tag", Ptr, (void*)"player")
    LVLEDIT_HIDE()
)

ENTITY(squashed_mouse,
    UPDATE(gravity_update)
    UPDATE(squashed_mouse_update)
    TEXTURE(squashed_mouse_texture)
    SIZE(0.75f, 0.25f)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_HIDE()
)

ENTITY(mouse,
    UPDATE(gravity_update)
    UPDATE(walk_update)
    UPDATE(animable_update)
    COLLISION(squash_collision)
    TEXTURE(mouse_texture)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("walk_speed", Float, 0.04)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    DEFAULT_PROPERTY("squashed", Int, ENTITY_BUILDER_squashed_mouse)
    LVLEDIT_TEXTURE("images/entities/mouse.png")
)

ENTITY(dust,
    UPDATE(dust_update)
    TEXTURE(dust_texture)
    SIZE(0.5f, 0.5f)
    LVLEDIT_HIDE()
)
