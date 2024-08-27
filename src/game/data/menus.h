#include "defines.h"

MENU(title_screen,
    IMAGE("images/logo.png", 0, 0, 90, 49, WIDTH / 2 - 90, 24, 180, 98)
    POSITION(0 POS_CENTER, 48 POS_BOTTOM)
    ITEM("Start Game", BUTTON(menubtn_start))
    ITEM("Select File", BUTTON(menubtn_select_file))
    ITEM("Settings", BUTTON(menubtn_settings))
    ITEM("Quit", BUTTON(menubtn_quit))
)

MENU(file_select,
    POSITION(0 POS_CENTER, 0 POS_CENTER)
    ITEM("File Select", SEPARATOR())
    DYNAMIC(menudyn_file_buttons)
    ITEM("Copy", BUTTON(menubtn_file_copy))
    ITEM("Erase", BUTTON(menubtn_file_erase))
    ITEM("Back", BUTTON(menubtn_back))
)

MENU(file_what,
    POSITION(0 POS_CENTER, 0 POS_CENTER)
    ITEM("What?", SEPARATOR())
    DYNAMIC(menudyn_file_buttons)
    ITEM("Cancel", BUTTON(menubtn_file_cancel))
)

MENU(file_to,
    POSITION(0 POS_CENTER, 0 POS_CENTER)
    ITEM("To?", SEPARATOR())
    DYNAMIC(menudyn_file_buttons)
    ITEM("Cancel", BUTTON(menubtn_file_cancel))
)

MENU(settings,
    POSITION(0 POS_CENTER, 0 POS_CENTER)
    ITEM("Settings", SEPARATOR())
    ITEM("not done pls lemme cook :pray:", SEPARATOR())
    ITEM("Back", BUTTON(menubtn_back))
)
