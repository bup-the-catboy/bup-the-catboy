#include "io.h"

#include <SDL2/SDL.h>

void io_deinit() {
    SDL_Quit();
}

bool requested_quit() {
    return SDL_QuitRequested();
}