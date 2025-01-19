#include "main.h"

#include "game/input.h"
#include "io/assets/assets.h"
#include "io/audio/audio.h"
#include "game/data.h"
#include "game/level.h"
#include "game/overlay/transition.h"
#include "game/savefile.h"
#include "game/overlay/menu.h"
#include "io/io.h"
#include "rng.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define STEPS_PER_SECOND 60
#define STEP_TIME (TICKS_PER_SEC / STEPS_PER_SECOND)

double internal_global_timer = 0;
uint64_t global_timer = 0;
uint64_t game_start_ticks = 0;
float delta_time = 0;
bool running = true;
pthread_t game_loop_thread;

int windoww, windowh;

void drawlist_renderer(void* resource, float dstx, float dsty, float dstw, float dsth, int srcx, int srcy, int srcw, int srch, unsigned int color) {
    struct GfxResource* res = resource;
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
    if (res) {
        switch (res->type) {
            case GfxResType_Texture:
                texX1 = (float)(srcx       ) / res->texture.width;
                texY1 = (float)(srcy       ) / res->texture.height;
                texX2 = (float)(srcx + srcw) / res->texture.width;
                texY2 = (float)(srcy + srch) / res->texture.height;
                graphics_select_texture(res);
                break;
            case GfxResType_Shader:
                graphics_select_shader(res);
                break;
        }
    }
    else graphics_select_texture(NULL);
    graphics_draw(dstX1, dstY1, dstX2, dstY2, texX1, texY1, texX2, texY2, color);
}

void init_game() {
    graphics_init("Bup the Catboy", WIDTH * 2, HEIGHT * 2);
    graphics_set_resolution(WIDTH, HEIGHT);
    controller_init();
    audio_init();
    load_assets();
    init_data();
    menu_init();
    savefile_load();
    savefile_select(0);
    load_level(GET_ASSET(struct Binary, "levels/1-1-entprop.lvl"));
    load_menu(title_screen);
}

void* game_loop(void* _) {
    while (running) {
        uint64_t num_ticks = ticks();
        if (game_start_ticks == 0) game_start_ticks = num_ticks;
        delta_time = (num_ticks - game_start_ticks) / TICKS_PER_SEC * STEPS_PER_SECOND;
        game_start_ticks = num_ticks;
        update_input();
        if (current_level != NULL) {
            update_transition();
            update_level(delta_time);
        }
        sync(game_start_ticks, STEP_TIME);
        internal_global_timer += delta_time;
        global_timer = internal_global_timer;
    }
    return NULL;
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], "--extract") == 0) {
            extract_assets();
            return 0;
        }
    }
    random_init();
    LE_DrawList* drawlist = LE_CreateDrawList();
    init_game();
    pthread_create(&game_loop_thread, NULL, game_loop, NULL);
    while (true) {
        if (requested_quit()) break;
        graphics_get_size(&windoww, &windowh);
        graphics_start_frame();
        render_level(drawlist, WIDTH, HEIGHT, min((ticks() - game_start_ticks) / STEP_TIME, 1));
        LE_DrawListAppend(drawlist, graphics_dummy_shader(), 0, 0, 0, 0, 0, 0, 0, 0);
        LE_Render(drawlist, drawlist_renderer);
        LE_ClearDrawList(drawlist);
        graphics_end_frame();
    }
    running = false;
    pthread_join(game_loop_thread, NULL);
    graphics_deinit();
    controller_deinit();
    audio_deinit();
    io_deinit();
    return 0;
}
