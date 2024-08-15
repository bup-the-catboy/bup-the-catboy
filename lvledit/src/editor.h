#ifndef BTCB_LVLEDIT_EDITOR_H
#define BTCB_LVLEDIT_EDITOR_H

#include <SDL2/SDL.h>

bool should_quit();
void editor_process_event(SDL_Event* event);
void editor_run(SDL_Renderer* renderer);

#endif