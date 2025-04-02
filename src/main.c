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

#define STEPS_PER_SECOND 30
#define STEP_TIME (TICKS_PER_SEC / STEPS_PER_SECOND)
#define TIME_SCALE 2

double internal_global_timer = 0;
uint64_t global_timer = 0;
uint64_t game_start_ticks = 0;
float delta_time = 0;
bool running = true;
pthread_t game_loop_thread;
LE_DrawList* drawlist;
float render_interpolation = 0;

int windoww, windowh;

void drawlist_append(void* cmd) {
    drawlist_append_rect(cmd, 0, 0, 0, 0, 0, 0, 0, 0);
}

void drawlist_append_rect(void* cmd,
    float dstX, float dstY, float dstW, float dstH,
    int   srcX, int   srcY, int   srcW, int   srcH
) {
    LE_DrawListAppend(drawlist, cmd, dstX, dstY, dstW, dstH, srcX, srcY, srcW, srcH);
}

void init_game() {
    random_init();
    graphics_init("Bup the Catboy", WIDTH * 2, HEIGHT * 2);
    graphics_set_resolution(WIDTH, HEIGHT);
    controller_init();
    audio_init();
    load_assets();
    init_data();
    menu_init();
    savefile_load();
    savefile_select(0);
    load_level(GET_ASSET(struct Binary, "levels/demo.lvl"));
    load_menu(title_screen);
}

void* game_loop(void* _) {
    while (running) {
        uint64_t num_ticks = ticks();
        if (game_start_ticks == 0) game_start_ticks = num_ticks;
        delta_time = (num_ticks - game_start_ticks) / TICKS_PER_SEC * STEPS_PER_SECOND * TIME_SCALE;
        game_start_ticks = num_ticks;
        update_input();
        if (current_level != NULL) {
            update_transition();
            update_level(delta_time);
        }
        sync(game_start_ticks, STEP_TIME / TIME_SCALE);
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
    drawlist = LE_CreateDrawList();
    init_game();
    pthread_create(&game_loop_thread, NULL, game_loop, NULL);
    while (true) {
        if (requested_quit()) break;
        graphics_get_size(&windoww, &windowh);
        graphics_start_frame();
        render_interpolation = min((ticks() - game_start_ticks) / STEP_TIME * TIME_SCALE, 1);
        render_level(drawlist, WIDTH, HEIGHT, render_interpolation);
        drawlist_append(graphics_dummy_shader());
        LE_Render(drawlist, gfxcmd_process);
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
