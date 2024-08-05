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