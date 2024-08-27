#ifndef BTCB_HUD_H
#define BTCB_HUD_H

#include <lunarengine.h>

void hud_update(LE_Entity* entity);
void render_hud(LE_DrawList* drawlist);
void show_hud_element(int id);

#endif