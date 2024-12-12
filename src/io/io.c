#include "io.h"

#include <SDL2/SDL.h>

#include <time.h>

uint64_t ticks() {
    static uint64_t frequency = 0;
    if (frequency == 0) frequency = SDL_GetPerformanceFrequency();
    uint64_t counter = SDL_GetPerformanceCounter();
    return (counter * S_TO_NS) / frequency;
}

void sync(uint64_t start, int ms) {
    Uint64 end = ticks();
    Uint64 time = end - start;
    Sint64 wait_time = ms - time;
    if (wait_time <= 0) return;
    sleep_ns(wait_time);
}

void sleep_ns(uint64_t ns) {
    struct timespec ts = {
        .tv_sec  = ns / (uint64_t)S_TO_NS,
        .tv_nsec = ns % (uint64_t)S_TO_NS,
    };
    nanosleep(&ts, NULL);
}

void io_deinit() {
    SDL_Quit();
}

bool requested_quit() {
    return SDL_QuitRequested();
}