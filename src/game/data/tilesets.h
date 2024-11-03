#ifndef NO_VSCODE
#include "defines.h"
#include "tiles.h"
#endif

TILESET(grass,
    PALETTE(generic)
    TEXTURE("images/tilesets/grass.png")
    SIZE(16, 16)
    TILES_IN_ROW(16)
)

TILESET(grass_bg,
    PALETTE(background)
    TEXTURE("images/tilesets/grass_bg.png")
    SIZE(512, 256)
    TILES_IN_ROW(2)
)