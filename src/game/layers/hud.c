#include "font/font.h"
#include "main.h"

void layer_hud(LE_DrawList* drawlist, float camx, float camy, float scx, float scy) {
    render_text(drawlist, 8, 8, CHAR_LIVES "*03");
    render_text(drawlist, 8, 20, "%c*00", CHAR_COINS[(global_timer / 10) % 4]);
    render_text(drawlist, WIDTH - 66,  8, CHAR_CATCOIN_TLF CHAR_CATCOIN_TRF "${_4}" CHAR_CATCOIN_TLF CHAR_CATCOIN_TRF "${_4}" CHAR_CATCOIN_TLO CHAR_CATCOIN_TRO);
    render_text(drawlist, WIDTH - 66, 16, CHAR_CATCOIN_BLF CHAR_CATCOIN_BRF "${_4}" CHAR_CATCOIN_BLF CHAR_CATCOIN_BRF "${_4}" CHAR_CATCOIN_BLO CHAR_CATCOIN_BRO);
}
