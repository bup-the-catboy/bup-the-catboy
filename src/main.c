#include "assets/assets.h"
#include "assets/sound.h"
#include "game/data.h"
#include "game/level.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define FPS 60

Uint64 start_ticks;
SDL_Window* window;
SDL_Renderer* renderer;

void frame_begin() {
    start_ticks = SDL_GetTicks64();
}

void frame_end() {
    Uint64 end_ticks = SDL_GetTicks64();
    Uint64 frame_time = end_ticks - start_ticks;
    Sint64 wait_time = (1000 / FPS) - frame_time;
    if (wait_time <= 0) return;
    SDL_Delay(wait_time);
}

void drawlist_renderer(void* texture, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch) {
    SDL_FRect dst = { dstx, dsty, dstw, dsth };
    SDL_Rect  src = { srcx, srcy, srcw, srch };
    SDL_RenderCopyF(renderer, texture, &src, &dst);
}

int main() {
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    window = SDL_CreateWindow("Super Mario Bros. Reimagined", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 384, 256, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool loop = true;
    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    audio_init();
    load_assets(renderer);
    init_data();
    load_level(GET_ASSET(struct Binary, "levels/test.lvl"));
    LE_DrawList* drawlist = LE_CreateDrawList();
    while (loop) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) loop = false;
        }
        frame_begin();
        SDL_RenderClear(renderer);
        if (current_level != NULL) {
            update_level();
            LE_Draw(current_level->layers, 384, 256, drawlist);
            LE_Render(drawlist, drawlist_renderer);
            LE_ClearDrawList(drawlist);
        }
        SDL_RenderPresent(renderer);
        frame_end();
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}