#include "manager.h"

#include "menu.h"
#include "hud.h"

void layer_overlay(LE_DrawList *drawlist, float camx, float camy, float scx, float scy) {
    if (render_menu(drawlist)) return;
    render_hud(drawlist);
}