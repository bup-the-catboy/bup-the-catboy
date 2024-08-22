#ifndef BTCB_FONT_H
#define BTCB_FONT_H

#include <lunarengine.h>

#define COIN1 "\x80"
#define COIN2 "\x81"
#define COIN3 "\x82"
#define COIN4 "\x83"
#define LIVES "\x84"
#define COINS (COIN1 COIN2 COIN3 COIN4)

void render_text(LE_DrawList* dl, float x, float y, const char* fmt, ...);

#endif