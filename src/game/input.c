#include "input.h"

#include "io/io.h"
#include "main.h"

unsigned int      button_down;
unsigned int prev_button_down;
int mouse_x, mouse_y;

bool is_button_down(int key) {
    return !!(button_down & key);
}

bool is_button_up(int key) {
    return  !(button_down & key);
}

bool is_button_pressed(int key) {
    return !!((button_down & ~prev_button_down) & key);
}

bool is_button_released(int key) {
    return !!((prev_button_down & ~button_down) & key);
}

void get_mouse_position(int* x, int* y) {
    if (x) *x = mouse_x;
    if (y) *y = mouse_y;
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

void update_input() {
    int curr;
    prev_button_down = button_down;
    button_down = 0;

#define INPUT(_) curr = _;
#define KEYMAP(_) if (controller_key_down(_)) button_down |= curr;
#define MOUSEBTN(_)
#define CONTROLLER(_)
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK

#define KEYMAP(_)
#define MOUSEBTN(_) if (controller_mouse_down(_)) button_down |= curr;
#define CONTROLLER(_)
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK

#define KEYMAP(_)
#define MOUSEBTN(_)
#define CONTROLLER(_) if (controller_button_down(_)) button_down |= curr;
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
        button_down |= curr;
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK
}
