#ifndef BTCB_CAMERA_H
#define BTCB_CAMERA_H

#include <stdbool.h>

struct CameraBounds {
    int x, y;
    bool freed;
    struct CameraBounds* up;
    struct CameraBounds* left;
    struct CameraBounds* down;
    struct CameraBounds* right;
};

void camera_set_bounds(struct CameraBounds* bounds);
void camera_set_focus(float x, float y);
void camera_get(float* x, float* y);
void camera_screenshake(int duration, float x, float y);
void camera_update();
void camera_snap();

#endif