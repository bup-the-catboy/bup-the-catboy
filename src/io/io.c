#include "io.h"

#include <SDL2/SDL.h>

uint64_t ticks() {
    return SDL_GetTicks64();
}

void sync(uint64_t start, int ms) {
    Uint64 end = SDL_GetTicks64();
    Uint64 time = end - start;
    Sint64 wait_time = ms - time;
    if (wait_time <= 0) return;
    SDL_Delay(wait_time);
}

void io_deinit() {
    SDL_Quit();
}

bool requested_quit() {
    return SDL_QuitRequested();
}