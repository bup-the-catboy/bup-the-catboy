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

typedef struct {} Camera;

Camera* camera_create();
void camera_set_bounds(Camera* camera, struct CameraBounds* bounds);
void camera_set_focus(Camera* camera, float x, float y);
void camera_get(Camera* camera, float* x, float* y);
void camera_screenshake(Camera* camera, int duration, float x, float y);
void camera_update(Camera* camera);
void camera_snap(Camera* camera);

#endif