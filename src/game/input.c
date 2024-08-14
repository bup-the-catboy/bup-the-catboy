#include "input.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

unsigned int      button_down;
unsigned int prev_button_down;
int mouse_x, mouse_y;

SDL_Joystick* joystick = NULL;

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

bool handle_sdl_events() {
    bool exit = false;
    SDL_Event event;
    int curr;
    prev_button_down = button_down;
#define SET_BUTTON(is_released) button_down = event.type == is_released ? (button_down & ~curr) : (button_down | curr)
#define INPUT(_) curr = _;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) exit = true;
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
#define KEYMAP(_) if (event.key.keysym.sym == _) SET_BUTTON(SDL_KEYUP);
#define MOUSEBTN(_)
#define CONTROLLER(_)
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK
        }
        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
#define KEYMAP(_)
#define MOUSEBTN(_) if (event.button.button == _) SET_BUTTON(SDL_MOUSEBUTTONUP);
#define CONTROLLER(_)
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK
        }
        if (event.type == SDL_JOYBUTTONDOWN || event.type == SDL_JOYBUTTONUP) {
#define KEYMAP(_)
#define MOUSEBTN(_)
#define CONTROLLER(_) if (event.cbutton.button == _) SET_BUTTON(SDL_JOYBUTTONUP);
#define JOYSTICK(_)
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK
        }
        if (event.type == SDL_JOYAXISMOTION) {
            bool held;
#define KEYMAP(_)
#define MOUSEBTN(_)
#define CONTROLLER(_)
#define JOYSTICK(_)                                                \
    held = true;                                                    \
    if (abs(event.caxis.value) <= DEADZONE) held = false;            \
    if (event.caxis.axis == (_) / 2) {                                \
        if (!(                                                         \
            (event.caxis.value < 0 && (_) % 2 == 0) ||                  \
            (event.caxis.value > 0 && (_) % 2 == 1)                      \
        )) held = false;                                                  \
        button_down = held ? (button_down | curr) : (button_down & ~curr); \
    }
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
#undef CONTROLLER
#undef JOYSTICK
        }
    }
    return exit;
}
