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
#include "bupscript/bupscript.h"

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define FPS 60

SDL_Window* window;
SDL_GLContext gl_context;

uint64_t global_timer = 0;

int windoww, windowh;
float viewx, viewy, vieww, viewh;
LE_DrawList* client_drawlist = NULL;
int client_player_id = 0;
bool client = false;

Uint64 frame_begin() {
    glClearColor(.5f, .5f, .5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return SDL_GetTicks64();
}

void frame_end(Uint64 start_ticks, int fps) {
    glFlush();
    SDL_GL_SwapWindow(window);
    Uint64 end_ticks = SDL_GetTicks64();
    Uint64 frame_time = end_ticks - start_ticks;
    Sint64 wait_time = (1000 / fps) - frame_time;
    if (wait_time <= 0) return;
    SDL_Delay(wait_time);
}

void drawlist_renderer(void* texture, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    struct Texture* tex = texture;
    float texX1 = 0, texX2 = 0, texY1 = 0, texY2 = 0;
    float dstX1 = dstx;
    float dstY1 = dsty;
    float dstX2 = dstx + dstw;
    float dstY2 = dsty + dsth;
    if (dstX2 < dstX1) {
        dstX1 -= dstw;
        dstX2 -= dstw;
    }
    if (dstY2 < dstY1) {
        dstY1 -= dsth;
        dstY2 -= dsth;
    }
    if (texture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex->gl_texture);
        texX1 = (float)(srcx       ) / tex->width;
        texY1 = (float)(srcy       ) / tex->height;
        texX2 = (float)(srcx + srcw) / tex->width;
        texY2 = (float)(srcy + srch) / tex->height;
    }
    else glDisable(GL_TEXTURE_2D);
    glColor4ub(color >> 24, color >> 16, color >> 8, color >> 0);
    glBegin(GL_QUADS);
    glTexCoord2f(texX1, texY1);
    glVertex2f(dstX1, dstY1);
    glTexCoord2f(texX2, texY1);
    glVertex2f(dstX2, dstY1);
    glTexCoord2f(texX2, texY2);
    glVertex2f(dstX2, dstY2);
    glTexCoord2f(texX1, texY2);
    glVertex2f(dstX1, dstY2);
    glEnd();
}

void update_viewport() {
    if (windoww == 0) windoww = 1; // prevent division by 0
    if (windowh == 0) windowh = 1;
    float vieww = WIDTH, viewh = HEIGHT;
    float aspect_ratio = WIDTH / (float)HEIGHT;
    float scr_aspect_ratio = windoww / (float)windowh;
    if (aspect_ratio > scr_aspect_ratio) viewh = windowh / (windoww / vieww);
    else vieww = windoww / (windowh / viewh);
    viewx = (vieww - WIDTH)  / 2;
    viewy = (viewh - HEIGHT) / 2;
    glViewport(0, 0, windoww, windowh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-viewx, vieww - viewx, viewh - viewy, -viewy, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void render_border(float x1, float y1, float x2, float y2) {
    glDisable(GL_TEXTURE_2D);
    glColor4ub(0, 0, 0, 255);
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void init_game() {
    audio_init();
    load_assets();
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
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    window = SDL_CreateWindow("Bup the Catboy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * 2, HEIGHT * 2, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    gl_context = SDL_GL_CreateContext(window);
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);
    LE_DrawList* drawlist = LE_CreateDrawList();
    init_game();
    while (true) {
        if (handle_sdl_events(client_player_id)) break;
        SDL_GetWindowSize(window, &windoww, &windowh);
        update_viewport();
        Uint64 frame = frame_begin();
        if (client && client_drawlist && LE_DrawListSize(client_drawlist)) {
            LE_Render(client_drawlist, drawlist_renderer);
            LE_DestroyDrawList(client_drawlist);
            client_drawlist = NULL;
        }
        else if (current_level != NULL) {
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
            LE_Render(drawlist, drawlist_renderer);
            LE_ClearDrawList(drawlist);
        }
        render_border(-viewx, -viewy, WIDTH + viewx, 0);
        render_border(-viewx, -viewy, 0, HEIGHT + viewy);
        render_border(-viewx, HEIGHT, WIDTH + viewx, HEIGHT + viewy);
        render_border(WIDTH, -viewy, WIDTH + viewx, HEIGHT + viewy);
        process_packets();
        frame_end(frame, FPS);
        global_timer++;
    }
    disconnect();
    audio_deinit();
    if (joystick) SDL_JoystickClose(joystick);
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}