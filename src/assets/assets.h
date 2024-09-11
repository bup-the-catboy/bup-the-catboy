#ifndef BTCB_ASSETS_H
#define BTCB_ASSETS_H

#include <SDL2/SDL.h>

#define GET_ASSET(type, name) ((typeof(type)*)get_asset(name))

struct Binary {
    unsigned char* ptr;
    unsigned int length;
};

void load_assets(SDL_Renderer* renderer);
void* get_asset(const char* name);
const char* get_asset_name(void* asset);
void extract_assets();

#endif