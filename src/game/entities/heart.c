#include "functions.h"

#include "io/io.h"

entity_texture(broken_heart) {
    int sprite = get(entity, "sprite", Int, 0);
    *srcX = sprite * 16;
    *srcY = 0;
    *srcW = 16;
    *srcH = 16;
    *w = 16;
    *h = 16;
    return gfxcmd_texture("images/entities/broken_heart.png");
}