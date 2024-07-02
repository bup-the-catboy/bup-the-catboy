#include "input.h"

#include <SDL2/SDL.h>

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
#define MOUSEBTN(_)
#define KEYMAP(_) if (event.key.keysym.sym == _) SET_BUTTON(SDL_KEYUP);
#include "game/data/inputs.h"
#undef KEYMAP
#undef MOUSEBTN
        }
        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
#define KEYMAP(_)
#define MOUSEBTN(_) if (event.button.button == _) SET_BUTTON(SDL_MOUSEBUTTONUP);
#include "game/data/inputs.h"
#undef MOUSEBTN
#undef KEYMAP
        }
    }
    return exit;
}
