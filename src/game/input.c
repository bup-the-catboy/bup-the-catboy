#include "input.h"

#include "game/network/common.h"
#include "io/io.h"
#include "game/network/packet.h"
#include "main.h"

struct {
    unsigned int      button_down;
    unsigned int prev_button_down;
    int mouse_x, mouse_y;
} inputs[MAX_PLAYERS];

bool is_button_down(int id, int key) {
    return !!(inputs[id].button_down & key);
}

bool is_button_up(int id, int key) {
    return  !(inputs[id].button_down & key);
}

bool is_button_pressed(int id, int key) {
    return !!((inputs[id].button_down & ~inputs[id].prev_button_down) & key);
}

bool is_button_released(int id, int key) {
    return !!((inputs[id].prev_button_down & ~inputs[id].button_down) & key);
}

void get_mouse_position(int id, int* x, int* y) {
    if (x) *x = inputs[id].mouse_x;
    if (y) *y = inputs[id].mouse_y;
}

#define A       1
#define B       0
#define X       3
#define Y       2
#define L       4
#define R       5
#define ZL      6
#define ZR      7
#define PLUS    9
#define MINUS   8
#define D_UP    11
#define D_LEFT  13
#define D_DOWN  12
#define D_RIGHT 14

#define LEFT  *4+0
#define RIGHT *4+1
#define UP    *4+2
#define DOWN  *4+3

#define DEADZONE 16384

void get_input_from_packet(LibSerialObj_Input* input) {
    int id = input->player;
    inputs[id].prev_button_down = inputs[id].button_down;
    inputs[id].button_down = input->input;
}

void update_input(int id) {
    int curr;
    inputs[id].prev_button_down = inputs[id].button_down;
    inputs[id].button_down = 0;

#define INPUT(_) curr = _;
#define KEYMAP(_) if (controller_key_down(_)) inputs[id].button_down |= curr;
#define MOUSEBTN(_)
#define CONTROLLER(_)
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK

#define KEYMAP(_)
#define MOUSEBTN(_) if (controller_mouse_down(_)) inputs[id].button_down |= curr;
#define CONTROLLER(_)
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK

#define KEYMAP(_)
#define MOUSEBTN(_)
#define CONTROLLER(_) if (controller_button_down(_)) inputs[id].button_down |= curr;
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK

#define KEYMAP(_)
#define MOUSEBTN(_)
#define CONTROLLER(_)
#define JOYSTICK(_) if ( \
    (_ % 2 == 0 && controller_get_axis(_ / 2) < -DEADZONE) || \
    (_ % 2 == 1 && controller_get_axis(_ / 2) >  DEADZONE))    \
        inputs[id].button_down |= curr;
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK
    if (id != 0) send_packet(packet_input(id));
}
