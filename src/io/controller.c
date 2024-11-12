#include "io.h"

#include <SDL2/SDL.h>

SDL_Joystick* joystick;

void controller_init() {
    SDL_Init(SDL_INIT_JOYSTICK);
}

int controller_count() {
    return SDL_NumJoysticks();
}

void controller_select(int index) {
    if (joystick) SDL_JoystickClose(joystick);
    joystick = SDL_JoystickOpen(index);
}

const char* controller_name(int index) {
    return SDL_JoystickNameForIndex(index);
}

const char* controller_current_name() {
    return SDL_JoystickName(joystick);
}

bool controller_key_down(int keycode) {
    return SDL_GetKeyboardState(NULL)[keycode];
}

bool controller_button_down(int button) {
    if (!joystick) return false;
    return SDL_JoystickGetButton(joystick, button);
}

bool controller_mouse_down(int button) {
    return SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(button);
}

void controller_mouse_pos(int* x, int* y) {
    SDL_GetMouseState(x, y);
}

float controller_get_axis(int index) {
    if (!joystick) return 0;
    SDL_PumpEvents();
    return SDL_JoystickGetAxis(joystick, index);
}

int controller_num_axes() {
    if (!joystick) return 0;
    return SDL_JoystickNumAxes(joystick);
}

int controller_num_buttons() {
    if (!joystick) return 0;
    return SDL_JoystickNumButtons(joystick);
}

void controller_deinit() {
    if (joystick) SDL_JoystickClose(joystick);
}