#include "camera.h"
#include "main.h"

#include <stdlib.h>
#include <string.h>

#define NUM_SCREENSHAKES 8
typedef struct {
    struct CameraBounds* bounds;
    float pos_x, pos_y;
    float foc_x, foc_y;
    float ssp_x, ssp_y; // position after screenshake is applied
    struct {
        int duration, start_duration;
        float x, y;
    } screenshakes[NUM_SCREENSHAKES];
} _Camera;

Camera* camera_create() {
    return memset(malloc(sizeof(_Camera)), 0, sizeof(_Camera));
}

void camera_set_bounds(Camera* camera, struct CameraBounds* bounds) {
    ((_Camera*)camera)->bounds = bounds;
}

void camera_set_focus(Camera* camera, float x, float y) {
    _Camera* cam = (_Camera*)camera;
    cam->foc_x = x;
    cam->foc_y = y;
}

void camera_get(Camera* camera, float* x, float* y) {
    _Camera* cam = (_Camera*)camera;
    if (x) *x = cam->ssp_x;
    if (y) *y = cam->ssp_y;
}

void camera_screenshake(Camera* camera, int duration, float x, float y) {
    _Camera* cam = (_Camera*)camera;
    for (int i = 0; i < NUM_SCREENSHAKES; i++) {
        if (cam->screenshakes[i].duration != 0) continue;
        cam->screenshakes[i].duration = cam->screenshakes[i].start_duration = duration;
        cam->screenshakes[i].x = x;
        cam->screenshakes[i].y = y;
        return;
    }
}

void camera_update(Camera* camera) {
    _Camera* cam = (_Camera*)camera;
    cam->pos_x += (cam->foc_x - cam->pos_x) / 10 * delta_time;
    cam->pos_y += (cam->foc_y - cam->pos_y) / 10 * delta_time;
    cam->ssp_x = cam->pos_x;
    cam->ssp_y = cam->pos_y;
    for (int i = 0; i < NUM_SCREENSHAKES; i++) {
        if (cam->screenshakes[i].duration == 0) continue;
        float intensity = cam->screenshakes[i].duration / (float)cam->screenshakes[i].start_duration;
        cam->ssp_x += (rand() % 2 - 1) * intensity * cam->screenshakes[i].x;
        cam->ssp_y += (rand() % 2 - 1) * intensity * cam->screenshakes[i].y;
        cam->screenshakes[i].duration--;
    }
}

void camera_snap(Camera* camera) {
    _Camera* cam = (_Camera*)camera;
    cam->pos_x = cam->foc_x;
    cam->pos_y = cam->foc_y;
}
