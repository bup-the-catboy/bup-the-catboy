#ifndef BTCB_CAMERA_H
#define BTCB_CAMERA_H

#include <stdbool.h>

typedef struct {} Camera;

typedef struct {
    float x, y;
} Point;

typedef struct {
    Point* poly;
    int num_vert;
} CameraBounds;

Camera* camera_create();
void camera_set_bounds(Camera* camera, CameraBounds* bounds);
void camera_set_focus(Camera* camera, float x, float y);
void camera_get(Camera* camera, float* x, float* y);
void camera_screenshake(Camera* camera, int duration, float x, float y);
void camera_update(Camera* camera);
void camera_snap(Camera* camera);

#endif