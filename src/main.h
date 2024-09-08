#ifndef BTCB_MAIN_H
#define BTCB_MAIN_H

#include <stdint.h>
#include <stdbool.h>

#include <lunarengine.h>

#define WIDTH  384
#define HEIGHT 256
#define MAX_PLAYERS 16

extern uint64_t global_timer;
extern LE_DrawList* client_drawlist;
extern int client_player_id;
extern bool client;

#endif