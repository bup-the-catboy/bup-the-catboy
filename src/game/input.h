#ifndef SMBR_INPUT_H
#define SMBR_INPUT_H

#include <stdbool.h>
#include <SDL2/SDL.h>

#ifndef NO_VSCODE
#define NO_VSCODE
#endif

#define INPUT(_) _ = (1 << (__COUNTER__)),
#define MOUSEBTN(_)
#define KEYMAP(_)
enum ButtonIDs {
#include "game/data/inputs.h"
};
#undef INPUT
#undef MOUSEBTN
#undef KEYMAP

bool is_button_down(int key);
bool is_button_up(int key);
bool is_button_pressed(int key);
bool is_button_released(int key);
void get_mouse_position(int* x, int* y);
bool handle_sdl_events();

#endif