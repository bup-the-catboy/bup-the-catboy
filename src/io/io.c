#include "io.h"

#include <SDL2/SDL.h>

#include <time.h>

uint64_t ticks() {
    return SDL_GetTicks64();
}

void sync(uint64_t start, int ms) {
    Uint64 end = ticks();
    Uint64 time = end - start;
    Sint64 wait_time = ms - time;
    if (wait_time <= 0) return;
    sleep_ms(wait_time);
}

void sleep_ms(uint64_t ms) {
    SDL_Delay(ms);
}

void io_deinit() {
    SDL_Quit();
}

bool requested_quit() {
    return SDL_QuitRequested();
}