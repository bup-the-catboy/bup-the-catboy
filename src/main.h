#ifndef BTCB_MAIN_H
#define BTCB_MAIN_H

#include <stdint.h>
#include <stdbool.h>

#include <lunarengine.h>

#define WIDTH  384
#define HEIGHT 256

extern uint64_t global_timer;
extern float delta_time;
extern float render_interpolation;
extern LE_DrawList* drawlist;

void drawlist_append(void* cmd);
void drawlist_append_rect(void* cmd,
    float dstX, float dstY, float dstW, float dstH,
    int   srcX, int   srcY, int   srcW, int   srcH
);

#endif