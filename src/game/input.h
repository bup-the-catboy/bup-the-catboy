#ifndef BTCB_INPUT_H
#define BTCB_INPUT_H

#define LIBSERIAL_INCLUDE "game/network/packets.h"
#include <libserial.h>

#include <stdbool.h>
#include <SDL2/SDL.h>

#ifndef NO_VSCODE
#define NO_VSCODE
#endif

#define INPUT(_) _ = (1 << (__COUNTER__)),
#define MOUSEBTN(_)
#define KEYMAP(_)
#define CONTROLLER(_)
#define JOYSTICK(_)
enum ButtonIDs {
#include "game/data/inputs.h"
    NUM_INPUTS = __COUNTER__
};
#undef INPUT
#undef MOUSEBTN
#undef KEYMAP
#undef CONTROLLER
#undef JOYSTICK

extern SDL_Joystick* joystick;

bool is_button_down(int id, int key);
bool is_button_up(int id, int key);
bool is_button_pressed(int id, int key);
bool is_button_released(int id, int key);
void get_mouse_position(int id, int* x, int* y);
bool handle_sdl_events(int id);
void get_input_from_packet(LibSerialObj_Input* input);

#endif