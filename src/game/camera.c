#include "camera.h"
#include "main.h"
#include "math_util.h"
#include "rng.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>

#define NUM_SCREENSHAKES 8
typedef struct {
    CameraBounds* bounds;
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

void camera_set_bounds(Camera* camera, CameraBounds* bounds) {
    _Camera* cam = (_Camera*)camera;
    cam->bounds = bounds;
    camera_set_focus(camera, cam->foc_x, cam->foc_y);
}

void camera_set_focus(Camera* camera, float foc_x, float foc_y) {
    _Camera* cam = (_Camera*)camera;
    if (cam->bounds) {
        float x = foc_x - 12;
        float y = foc_y - 8;
        float best_dist = INFINITY;
        float best_x, best_y;
        bool skip = false;
        for (int i = 0; i < cam->bounds->num_rects && !skip; i++) {
            Rectangle* rect = &cam->bounds->rects[i];
            if (rect_contains_rect(rect->x, rect->y, rect->w, rect->h, x, y, 24, 16)) skip = true;
            if (!rect_intersects_rect(rect->x, rect->y, rect->w, rect->h, x, y, 24, 16)) continue;
            float new_x = x;
            float new_y = y;
            clamp_rect(rect->x, rect->y, rect->w, rect->h, &new_x, &new_y, 24, 16);
            float dist = (x - new_x) * (x - new_x) + (y - new_y) * (y - new_y);
            if (best_dist > dist) {
                best_dist = dist;
                best_x = new_x;
                best_y = new_y;
            }
        }
        if (!skip) x = best_x, y = best_y;
        foc_x = x + 12;
        foc_y = y + 8;
    }
    cam->foc_x = foc_x;
    cam->foc_y = foc_y;
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
        cam->ssp_x += random_range(-1, 1) * intensity * cam->screenshakes[i].x;
        cam->ssp_y += random_range(-1, 1) * intensity * cam->screenshakes[i].y;
        cam->screenshakes[i].duration--;
    }
}

void camera_snap(Camera* camera) {
    _Camera* cam = (_Camera*)camera;
    cam->pos_x = cam->foc_x;
    cam->pos_y = cam->foc_y;
}
