#include "main.h"

#include "assets/assets.h"
#include "audio/audio.h"
#include "game/data.h"
#include "game/level.h"
#include "game/input.h"
#include "game/network/packet.h"
#include "game/network/common.h"
#include "font/font.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define FPS 60

SDL_Window* window;
SDL_Renderer* renderer;

uint64_t global_timer = 0;

float scale = 1;
float translate_x = 0;
float translate_y = 0;

Uint64 frame_begin() {
    return SDL_GetTicks64();
}

void frame_end(Uint64 start_ticks, int fps) {
    Uint64 end_ticks = SDL_GetTicks64();
    Uint64 frame_time = end_ticks - start_ticks;
    Sint64 wait_time = (1000 / fps) - frame_time;
    if (wait_time <= 0) return;
    SDL_Delay(wait_time);
}

void drawlist_renderer(void* texture, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    SDL_FRect dst = { dstx * scale, dsty * scale, dstw * scale, dsth * scale };
    SDL_Rect  src = { srcx,         srcy,         srcw,         srch         };
    dst.x += translate_x;
    dst.y += translate_y;
    SDL_SetTextureColorMod(texture, color >> 24, color >> 16, color >> 8);
    SDL_SetTextureAlphaMod(texture, color >> 0);
    SDL_RenderCopyF(renderer, texture, &src, &dst);
}

void adjust_display(int width, int height, int* new_w, int* new_h) {
    float aspect_ratio = width / (float)height;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (w == 0) w = 1; // prevent division by 0
    if (h == 0) h = 1;
    float scr_aspect_ratio = w / (float)h;
    if (aspect_ratio > scr_aspect_ratio) {
        *new_w = w;
        *new_h = w / aspect_ratio;
         scale = w / (float)width;
    }
    else {
        *new_h = h;
        *new_w = h * aspect_ratio;
         scale = h / (float)height;
    }
    translate_x = (w - *new_w) / 2.f;
    translate_y = (h - *new_h) / 2.f;
}

int main(int argc, char** argv) {
    bool is_client = false;
    if (argc >= 2) {
        if (strcmp(argv[1], "--server") == 0) start_server();
        if (strcmp(argv[1], "--client") == 0) {
            is_client = true;
            start_client(argv[2]);
        }
    }
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    if (SDL_NumJoysticks() >= 1) joystick = SDL_JoystickOpen(0);
    window = SDL_CreateWindow("Bup the Catboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);
    audio_init();
    load_assets(renderer);
    init_data();
    libserial_init();
    if (!is_client) load_level(GET_ASSET(struct Binary, "levels/test2.lvl"));
    LE_DrawList* drawlist = LE_CreateDrawList();
    SDL_RenderSetIntegerScale(renderer, true);
    while (true) {
        if (handle_sdl_events()) break;
        Uint64 frame = frame_begin();
        int frame_w, frame_h;
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        adjust_display(WIDTH, HEIGHT, &frame_w, &frame_h);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        SDL_RenderClear(renderer);
        if (current_level != NULL) {
            update_level();
            LE_Draw(current_level->layers, WIDTH, HEIGHT, drawlist);
            LE_Render(drawlist, drawlist_renderer);
            LE_ClearDrawList(drawlist);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect rect = { 0, 0, translate_x, height };
        SDL_RenderFillRect(renderer, &rect);
        rect.x = 0; rect.y = 0; rect.w = width, rect.h = translate_y;
        SDL_RenderFillRect(renderer, &rect);
        rect.x = width - translate_x; rect.y = 0; rect.w = translate_x, rect.h = height;
        SDL_RenderFillRect(renderer, &rect);
        rect.x = 0; rect.y = height - translate_y; rect.w = width, rect.h = translate_y;
        SDL_RenderFillRect(renderer, &rect);
        SDL_RenderPresent(renderer);
        process_packets();
        frame_end(frame, FPS);
        global_timer++;
    }
    disconnect();
    audio_deinit();
    if (joystick) SDL_JoystickClose(joystick);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}