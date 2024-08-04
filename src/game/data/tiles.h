#include "defines.h"

TILE(air,
    LVLEDIT_HIDE()
)

TILE(ground,
    SIMPLE_STATIONARY_TEXTURE(1)
    SOLID()
)

TILE(dirt,
    SIMPLE_STATIONARY_TEXTURE(2)
    SOLID()
)

TILE(coin,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 28, 13, 29, 14)
)

TILE(question_block,
    SIMPLE_ANIMATED_TEXTURE(4, 10, 19, 20, 21, 22)
    SOLID()
)

TILE(brick,
    SIMPLE_ANIMATED_TEXTURE(8, 10, 6, 6, 6, 6, 6, 3, 4, 5)
    SOLID()
)