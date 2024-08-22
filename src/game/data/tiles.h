#include "defines.h"

TILE(air,
    LVLEDIT_HIDE()
)

TILE(ground,
    SIMPLE_STATIONARY_TEXTURE(1)
    SOLID()
)

TILE(question_block,
    SIMPLE_ANIMATED_TEXTURE(8, 10, 7, 11, 12, 13, 7, 8, 9, 10)
    COLLISION(tilecol_question_block)
    SOLID()
)

TILE(brick,
    SIMPLE_ANIMATED_TEXTURE(8, 10, 2, 2, 2, 2, 3, 4, 5, 6)
    COLLISION(tilecol_brick)
    SOLID()
)

TILE(empty_block,
    SIMPLE_STATIONARY_TEXTURE(14)
    SOLID()
)

TILE(coin,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 16, 17, 18, 19)
    COLLISION(tilecol_coin)
)

TILE(block,
    SIMPLE_STATIONARY_TEXTURE(15)
    SOLID()
)

TILE(vert_pipe_top_left,
    SIMPLE_STATIONARY_TEXTURE(32)
    SOLID()
)

TILE(vert_pipe_top_right,
    SIMPLE_STATIONARY_TEXTURE(33)
    SOLID()
)

TILE(vert_pipe_left,
    SIMPLE_STATIONARY_TEXTURE(48)
    SOLID()
)

TILE(vert_pipe_right,
    SIMPLE_STATIONARY_TEXTURE(49)
    SOLID()
)

TILE(vert_pipe_bottom_left,
    SIMPLE_STATIONARY_TEXTURE(64)
    SOLID()
)

TILE(vert_pipe_bottom_right,
    SIMPLE_STATIONARY_TEXTURE(65)
    SOLID()
)

TILE(hor_pipe_top_left,
    SIMPLE_STATIONARY_TEXTURE(34)
    SOLID()
)

TILE(hor_pipe_top_right,
    SIMPLE_STATIONARY_TEXTURE(36)
    SOLID()
)

TILE(hor_pipe_top,
    SIMPLE_STATIONARY_TEXTURE(35)
    SOLID()
)

TILE(hor_pipe_bottom,
    SIMPLE_STATIONARY_TEXTURE(51)
    SOLID()
)

TILE(hor_pipe_bottom_left,
    SIMPLE_STATIONARY_TEXTURE(50)
    SOLID()
)

TILE(hor_pipe_bottom_right,
    SIMPLE_STATIONARY_TEXTURE(52)
    SOLID()
)

TILE(vert_pipe_conn_top_left,
    SIMPLE_STATIONARY_TEXTURE(66)
    SOLID()
)

TILE(vert_pipe_conn_top_right,
    SIMPLE_STATIONARY_TEXTURE(67)
    SOLID()
)

TILE(vert_pipe_conn_bottom_left,
    SIMPLE_STATIONARY_TEXTURE(82)
    SOLID()
)

TILE(vert_pipe_conn_bottom_right,
    SIMPLE_STATIONARY_TEXTURE(83)
    SOLID()
)

TILE(hor_pipe_conn_top_left,
    SIMPLE_STATIONARY_TEXTURE(68)
    SOLID()
)

TILE(hor_pipe_conn_top_right,
    SIMPLE_STATIONARY_TEXTURE(69) // nice
    SOLID()
)

TILE(hor_pipe_conn_bottom_left,
    SIMPLE_STATIONARY_TEXTURE(84)
    SOLID()
)

TILE(hor_pipe_conn_bottom_right,
    SIMPLE_STATIONARY_TEXTURE(85)
    SOLID()
)

TILE(catcoin_top_left,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 24, 26, 28, 30)
)

TILE(catcoin_top_right,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 25, 27, 29, 31)
)

TILE(catcoin_bottom_left,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 40, 42, 44, 46)
)

TILE(catcoin_bottom_right,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 41, 43, 45, 47)
)

TILE(catcoin_top_left_outline,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 56, 58, 60, 62)
)

TILE(catcoin_top_right_outline,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 57, 59, 61, 63)
)

TILE(catcoin_bottom_left_outline,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 72, 74, 76, 78)
)

TILE(catcoin_bottom_right_outline,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 73, 75, 77, 79)
)
