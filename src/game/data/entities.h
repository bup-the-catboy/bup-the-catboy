#ifndef NO_VSCODE
#include "defines.h"
#endif

ENTITY(player_spawner,
    UPDATE(player_spawner_update)
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("tag", Ptr, (void*)"player")
    LVLEDIT_TEXTURE("images/entities/player.png")
    LVLEDIT_CROP(0, 0, 16, 16)
)

ENTITY(player,
    UPDATE(gravity_update)
    UPDATE(player_update)
    TEXTURE(player_texture)
    SIZE(0.75f, 0.75f)
    PRIORITY(10)
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
    DEFAULT_PROPERTY("has_vision", Int, 1)
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
    DEFAULT_PROPERTY("has_vision", Int, 1)
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
    SIZE(0.75f, 0.75f)
    DEFAULT_PROPERTY("shader", Ptr, (void*)"")
    LVLEDIT_TEXTURE("images/lvledit/shader.png")
    LVLEDIT_CROP(0, 0, 16, 16)
)

ENTITY(death_barrier,
    UPDATE(death_barrier_update)
    SIZE(0.75f, 0.75f)
    LVLEDIT_TEXTURE("images/lvledit/deathbarrier.png")
    LVLEDIT_CROP(0, 0, 16, 16)
)

ENTITY(crate_fragment,
    UPDATE(despawn_update)
    UPDATE(gravity_update)
    UPDATE(friction_update)
    TEXTURE(crate_fragment_texture)
    SIZE(0.5f, 0.5f)
    DEFAULT_PROPERTY("despawn_timer", Float, 300)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    DEFAULT_PROPERTY("friction", Float, 100)
    LVLEDIT_HIDE()
)

ENTITY(sparkles,
    UPDATE(animable_update)
    UPDATE(despawn_update)
    TEXTURE(sparkles_texture)
    DEFAULT_PROPERTY("despawn_timer", Float, 16)
    LVLEDIT_HIDE()
)

ENTITY(coin_particle,
    UPDATE(despawn_update)
    UPDATE(gravity_update)
    UPDATE(coin_particle_update)
    TEXTURE(coin_particle_texture)
    FLAGS(LE_EntityFlags_DisableCollision)
    DEFAULT_PROPERTY("despawn_timer", Float, 300)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_HIDE()
)

ENTITY(trail_spawner,
    UPDATE(trail_spawner_update)
    UPDATE(gravity_update)
    UPDATE(despawn_update)
    FLAGS(LE_EntityFlags_DisableCollision)
    DEFAULT_PROPERTY("despawn_timer", Float, 60)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_HIDE()
)

ENTITY(trail,
    UPDATE(animable_update)
    UPDATE(despawn_update)
    TEXTURE(trail_texture)
    FLAGS(LE_EntityFlags_DisableCollision)
    DEFAULT_PROPERTY("despawn_timer", Float, 40)
    LVLEDIT_HIDE()
)

ENTITY(level_finish,
    UPDATE(level_finish_update)
    TEXTURE(level_finish_texture)
    LVLEDIT_HIDE()
)

ENTITY(worldmap_player,
    UPDATE(worldmap_player_update)
    TEXTURE(worldmap_player_texture)
    SIZE(0.5f, 0.5f)
    FLAGS(LE_EntityFlags_DisableCollision)
    DEFAULT_PROPERTY("tag", Ptr, (void*)"player")
    LVLEDIT_TEXTURE("images/entities/player.png")
    LVLEDIT_CROP(0, 0, 16, 16)
    LVLEDIT_PROPERTIES(
        int(curr_node, 0)
    )
)

ENTITY(worldmap_node,
    SIZE(0.5f, 0.5f)
    FLAGS(LE_EntityFlags_DisableCollision)
    LVLEDIT_TEXTURE("images/lvledit/worldmap_node.png")
    LVLEDIT_CROP(0, 0, 8, 8)
    LVLEDIT_PROPERTIES(
        int(id, 0)
        int(up_node, -1)
        int(left_node, -1)
        int(down_node, -1)
        int(right_node, -1)
        int(level_id, -1)
        int(requires_level, -1)
        bool(secret_path, false)
    )
)

ENTITY(notified,
    UPDATE(despawn_update)
    UPDATE(notified_update)
    TEXTURE(notified_texture)
    FLAGS(LE_EntityFlags_DisableCollision)
    DEFAULT_PROPERTY("despawn_timer", Float, 60)
    LVLEDIT_HIDE()
)

ENTITY(crate_loot,
    SIZE(1.f, 1.f)
    FLAGS(LE_EntityFlags_DisableCollision)
    DEFAULT_PROPERTY("is_warp", Bool, false)
    DEFAULT_PROPERTY("entity_or_warp_id", Int, -1)
    DEFAULT_PROPERTY("tag", Ptr, (void*)"crate_loot")
    LVLEDIT_TEXTURE("images/lvledit/crate_loot.png")
    LVLEDIT_CROP(0, 0, 16, 16)
    LVLEDIT_PROPERTIES(
        int(entity_or_warp_id, -1)
        bool(is_warp, false)
    )
)

ENTITY(crate_coin,
    UPDATE(gravity_update)
    UPDATE(crate_coin_update)
    COLLISION(crate_coin_collision)
    TEXTURE(crate_coin_texture)
    SIZE(.75f, .75f)
    DEFAULT_PROPERTY("gravity", Float, 0.03)
    LVLEDIT_HIDE()
)