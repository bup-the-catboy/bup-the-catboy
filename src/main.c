#include "assets/assets.h"
#include "assets/sound.h"

#include <SDL2/SDL.h>
#include <stdbool.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Window* window = SDL_CreateWindow("Super Mario Bros. Reimagined", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 768, 512, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool loop = true;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    audio_init();
    load_assets(renderer);
    while (loop) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) loop = false;
        }
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}