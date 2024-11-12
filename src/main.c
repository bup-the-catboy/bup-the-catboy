#include "main.h"

#include "game/input.h"
#include "io/assets/assets.h"
#include "io/audio/audio.h"
#include "game/data.h"
#include "game/level.h"
#include "game/network/packet.h"
#include "game/network/common.h"
#include "game/overlay/transition.h"
#include "game/savefile.h"
#include "game/overlay/menu.h"
#include "io/io.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define FPS 60

uint64_t global_timer = 0;

int windoww, windowh;
float viewx, viewy, vieww, viewh;
LE_DrawList* client_drawlist = NULL;
int client_player_id = 0;
bool client = false;

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
        texX1 = (float)(srcx       ) / tex->width;
        texY1 = (float)(srcy       ) / tex->height;
        texX2 = (float)(srcx + srcw) / tex->width;
        texY2 = (float)(srcy + srch) / tex->height;
    }
    graphics_select_texture(texture);
    graphics_draw(texX1, texY1, texX2, texY2, dstX1, dstY1, dstX2, dstY2, color);
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
    graphics_update_viewport(viewx, viewy, vieww, viewh);
}

void render_border(float x1, float y1, float x2, float y2) {
    graphics_select_texture(NULL);
    graphics_draw(x1, y1, x2, y2, 0, 0, 0, 0, 0x000000FF);
}

void init_game() {
    graphics_init("Bup the Catboy", WIDTH * 2, HEIGHT * 2);
    controller_init();
    audio_init();
    load_assets();
    init_data();
    menu_init();
    libserial_init();
    savefile_load();
    savefile_select(0);
    if (!client) {
        load_level(GET_ASSET(struct Binary, "levels/1-1.lvl"));
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
    LE_DrawList* drawlist = LE_CreateDrawList();
    init_game();
    while (true) {
        if (requested_quit()) break;
        update_input(client_player_id);
        graphics_get_size(&windoww, &windowh);
        update_viewport();
        graphics_start_frame();
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
        graphics_end_frame(FPS);
        global_timer++;
    }
    disconnect();
    graphics_deinit();
    controller_deinit();
    audio_deinit();
    io_deinit();
    return 0;
}