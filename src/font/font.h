#ifndef BTCB_FONT_H
#define BTCB_FONT_H

#include <lunarengine.h>
#include <stdarg.h>

#define CHAR_COIN1 "\x80"
#define CHAR_COIN2 "\x81"
#define CHAR_COIN3 "\x82"
#define CHAR_COIN4 "\x83"
#define CHAR_LIVES "\x7F"
#define CHAR_HEART "\xA4"
#define CHAR_COINS (CHAR_COIN1 CHAR_COIN2 CHAR_COIN3 CHAR_COIN4)

#define CHAR_CATCOIN_TLO "\x84"
#define CHAR_CATCOIN_TRO "\x85"
#define CHAR_CATCOIN_BLO "\x86"
#define CHAR_CATCOIN_BRO "\x87"
#define CHAR_CATCOIN_TLF "\x88"
#define CHAR_CATCOIN_TRF "\x89"
#define CHAR_CATCOIN_BLF "\x8A"
#define CHAR_CATCOIN_BRF "\x8B"

#define CHAR_CATCOIN_TL 0
#define CHAR_CATCOIN_TR 1
#define CHAR_CATCOIN_BL 2
#define CHAR_CATCOIN_BR 3

#define CHAR_CATCOIN_O (CHAR_CATCOIN_TLO CHAR_CATCOIN_TRO CHAR_CATCOIN_BLO CHAR_CATCOIN_BRO)
#define CHAR_CATCOIN_F (CHAR_CATCOIN_TLF CHAR_CATCOIN_TRF CHAR_CATCOIN_BLF CHAR_CATCOIN_BRF)

void render_text (LE_DrawList* dl, float x, float y, const char* fmt, ...);
void render_textv(LE_DrawList* dl, float x, float y, const char* fmt, va_list args);
void text_size (float* w, float* h, const char* fmt, ...);
void text_sizev(float* w, float* h, const char* fmt, va_list args);

#endif