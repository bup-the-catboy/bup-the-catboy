#include "camera.h"

#include <stdlib.h>

struct CameraBounds* cambounds = NULL;
float campos_x = 0;
float campos_y = 0;
float camfoc_x = 0;
float camfoc_y = 0;
float camssp_x = 0; // position after screen shake is applied
float camssp_y = 0;

struct Screenshake {
    int duration, start_duration;
    float x, y;
};

#define NUM_SCREENSHAKES 8
struct Screenshake screenshakes[NUM_SCREENSHAKES];

void camera_set_bounds(struct CameraBounds* bounds) {
    cambounds = bounds;
}

void camera_set_focus(float x, float y) {
    camfoc_x = x;
    camfoc_y = y;
}

void camera_get(float* x, float* y) {
    if (x) *x = camssp_x;
    if (y) *y = camssp_y;
}

void camera_screenshake(int duration, float x, float y) {
    for (int i = 0; i < NUM_SCREENSHAKES; i++) {
        if (screenshakes[i].duration != 0) continue;
        screenshakes[i].duration = screenshakes[i].start_duration = duration;
        screenshakes[i].x = x;
        screenshakes[i].y = y;
        return;
    }
}

void camera_update() {
    campos_x += (camfoc_x - campos_x) / 10;
    campos_y += (camfoc_y - campos_y) / 10;
    camssp_x = campos_x;
    camssp_y = campos_y;
    for (int i = 0; i < NUM_SCREENSHAKES; i++) {
        if (screenshakes[i].duration == 0) continue;
        float intensity = screenshakes[i].duration / (float)screenshakes[i].start_duration;
        camssp_x += (rand() % 2 - 1) * intensity * screenshakes[i].x;
        camssp_y += (rand() % 2 - 1) * intensity * screenshakes[i].y;
        screenshakes[i].duration--;
    }
}

void camera_snap() {
    campos_x = camfoc_x;
    campos_y = camfoc_y;
}
