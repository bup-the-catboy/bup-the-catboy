#ifndef NO_VSCODE
#include "defines.h"
#endif

ENTITY(player_spawner,
    UPDATE(player_spawner_update)
    SIZE(0.75f, 0.75f)
    LVLEDIT_TEXTURE("images/entities/player.png")
    LVLEDIT_CROP(0, 0, 16, 16)
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

ENTITY(squashed_mouse,
    UPDATE(gravity_update)
    UPDATE(squashed_mouse_update)
    UPDATE(despawn_update)
    TEXTURE(squashed_mouse_texture)
    SIZE(0.75f, 0.25f)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    DEFAULT_PROPERTY("despawn_timer", Float, 60)
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
    LVLEDIT_CROP(0, 0, 16, 16)
)

ENTITY(turtle_shell_fragment,
    UPDATE(gravity_update)
    UPDATE(despawn_update)
    UPDATE(friction_update)
    TEXTURE(turtle_shell_fragment_texture)
    SIZE(0.5f, 0.5f)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    DEFAULT_PROPERTY("friction", Float, 0.01)
    DEFAULT_PROPERTY("despawn_timer", Float, 300)
    LVLEDIT_HIDE()
)

ENTITY(turtle_shell,
    UPDATE(turtle_shell_update)
    UPDATE(gravity_update)
    UPDATE(walk_update)
    UPDATE(animable_update)
    TEXTURE(turtle_shell_texture)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("walk_speed", Float, 0.2)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_HIDE()
)

ENTITY(turtle,
    UPDATE(gravity_update)
    UPDATE(walk_update)
    UPDATE(animable_update)
    COLLISION(squash_collision)
    TEXTURE(turtle_texture)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("walk_speed", Float, 0.04)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    DEFAULT_PROPERTY("squashed", Int, ENTITY_BUILDER_turtle_shell)
    LVLEDIT_TEXTURE("images/entities/turtle.png")
    LVLEDIT_CROP(0, 0, 16, 16)
)

ENTITY(dust,
    UPDATE(dust_update)
    TEXTURE(dust_texture)
    SIZE(0.5f, 0.5f)
    LVLEDIT_HIDE()
)

ENTITY(shader_controller,
    TEXTURE(shader_controller_texture)
    DEFAULT_PROPERTY("shader", Ptr, (void*)"")
    LVLEDIT_TEXTURE("images/lvledit/shader.png")
    LVLEDIT_CROP(0, 0, 16, 16)
)