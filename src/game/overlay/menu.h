#ifndef BTCB_OVERLAY_MENU_H
#define BTCB_OVERLAY_MENU_H

#include <stdbool.h>
#include <lunarengine.h>

#ifndef NO_VSCODE
#define NO_VSCODE
#endif
#define MENU(name, content) MENU_##name,
enum MenuIDs {
    MENU_none = -1,
#include "game/data/menus.h"
    MENU_COUNT
};
#undef MENU

#define push_menu(id) _push_menu(MENU_##id)
#define load_menu(id) _load_menu(MENU_##id)
void _push_menu(int index);
void _load_menu(int index);
void pop_menu();
void pop_menu_multi(int count);
void menu_init();
bool menu_visible();
bool render_menu(LE_DrawList* drawlist);

#endif