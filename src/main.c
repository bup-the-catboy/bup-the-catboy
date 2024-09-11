#include "main.h"

#include "assets/assets.h"
#include "audio/audio.h"
#include "game/camera.h"
#include "game/data.h"
#include "game/level.h"
#include "game/input.h"
#include "game/network/packet.h"
#include "game/network/common.h"
#include "game/overlay/transition.h"
#include "game/savefile.h"
#include "game/overlay/menu.h"
#include "font/font.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_blendmode.h>
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
LE_DrawList* client_drawlist = NULL;
int client_player_id = 0;
bool client = false;

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
    bool flipx = dstw < 0;
    bool flipy = dsth < 0;
    if (flipx) dstw *= -1;
    if (flipy) dsth *= -1;
    SDL_FRect dst = { dstx * scale, dsty * scale, dstw * scale, dsth * scale };
    SDL_Rect  src = { srcx,         srcy,         srcw,         srch         };
    dst.x += translate_x;
    dst.y += translate_y;
    if (texture) {
        SDL_SetTextureColorMod(texture, color >> 24, color >> 16, color >> 8);
        SDL_SetTextureAlphaMod(texture, color >> 0);
        SDL_RenderCopyExF(renderer, texture, &src, &dst, 0, NULL, flipy * 2 + flipx);
    }
    else {
        SDL_SetRenderDrawColor(renderer, color >> 24, color >> 16, color >> 8, color >> 0);
        SDL_RenderFillRectF(renderer, &dst);
    }
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

void init_game() {
    audio_init();
    load_assets(renderer);
    init_data();
    menu_init();
    libserial_init();
    savefile_load();
    savefile_select(0);
    if (!client) {
        load_level(GET_ASSET(struct Binary, "levels/test2.lvl"));
        create_player(current_level->default_cambound);
        load_menu(title_screen);
    }
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], "--server") == 0) start_server();
        if (strcmp(argv[1], "--client") == 0) {
            client = true;
            start_client(argv[2]);
        }
        if (strcmp(argv[1], "--extract") == 0) {
            extract_assets();
            return 0;
        }
    }
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    if (SDL_NumJoysticks() >= 1) joystick = SDL_JoystickOpen(0);
    window = SDL_CreateWindow("Bup the Catboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);
    LE_DrawList* drawlist = LE_CreateDrawList();
    init_game();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    while (true) {
        if (handle_sdl_events(client_player_id)) break;
        Uint64 frame = frame_begin();
        int frame_w, frame_h;
        int width, height;
        SDL_GetWindowSize(window, &width, &height);
        adjust_display(WIDTH, HEIGHT, &frame_w, &frame_h);
        SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
        if (client && client_drawlist && LE_DrawListSize(client_drawlist)) {
            SDL_RenderClear(renderer);
            LE_Render(client_drawlist, drawlist_renderer);
            LE_DestroyDrawList(client_drawlist);
            client_drawlist = NULL;
        }
        else if (current_level != NULL) {
            SDL_RenderClear(renderer);
            update_transition();
            update_level();
            render_level(players[0].camera, drawlist, WIDTH, HEIGHT);
            for (int i = 1; i < MAX_PLAYERS; i++) {
                if (!players[i].camera) continue;
                LE_DrawList* dl = LE_CreateDrawList();
                render_level(players[i].camera, dl, WIDTH, HEIGHT);
                send_packet_to(i, packet_rendered_screen(dl));
                LE_DestroyDrawList(dl);
            }
            render_text(drawlist, 8, 8, "true...");
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