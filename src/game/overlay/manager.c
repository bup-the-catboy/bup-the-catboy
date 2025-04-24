#include "manager.h"

#include "menu.h"
#include "hud.h"
#include "transition.h"

void layer_overlay(LE_DrawList *drawlist, void* param, float camx, float camy, float scx, float scy) {
    if (!render_menu(drawlist))
        render_hud(drawlist);
    render_transition(drawlist);
}