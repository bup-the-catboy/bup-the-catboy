#ifndef BTCB_MAIN_H
#define BTCB_MAIN_H

#include <stdint.h>
#include <stdbool.h>

#include <lunarengine.h>

#define WIDTH  384
#define HEIGHT 256

extern uint64_t global_timer;
extern float delta_time;
extern LE_DrawList* drawlist;
extern float render_interpolation;

#endif