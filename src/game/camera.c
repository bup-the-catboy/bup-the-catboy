#include "camera.h"
#include "main.h"

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "math_util.h"

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

static float distance(Point a, Point b) {
    return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

static Point line_project(Point p, Point a, Point b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float apx = p.x - a.x;
    float apy = p.y - a.y;
    float ab_ab =  dx * dx +  dy * dy;
    float ap_ab = apx * dx + apy * dy;
    if (ap_ab == 0) return a;
    else {
        float t = ap_ab / ab_ab;
        if (t < 0) return a;
        if (t > 1) return b;
        return (Point){ a.x + t * dx, a.y + t * dy };
    }
}

static bool is_in_polygon(Point point, Point* polygon, int num_verts) {
    int count = 0;
    for (int i = 0; i < num_verts; i++) {
        Point a = polygon[i];
        Point b = polygon[(i + 1) % num_verts];
        if ((point.y > min(a.y, b.y)) && (point.y <= max(a.y, b.y)) && (point.x <= max(a.x, b.x))) {
            float xIntersect = (point.y - a.y) * (b.x - a.x) / (b.y - a.y) + a.x;
            if (a.x == b.x || point.x <= xIntersect) count++;
        }
    }
    return count % 2 == 1;
}

Camera* camera_create() {
    return memset(malloc(sizeof(_Camera)), 0, sizeof(_Camera));
}

void camera_set_bounds(Camera* camera, CameraBounds* bounds) {
    _Camera* cam = (_Camera*)camera;
    cam->bounds = bounds;
    camera_set_focus(camera, cam->foc_x, cam->foc_y);
}

void camera_set_focus(Camera* camera, float x, float y) {
    _Camera* cam = (_Camera*)camera;
    Point target = (Point){ x, y };
    if (cam->bounds && !is_in_polygon(target, cam->bounds->poly, cam->bounds->num_vert)) {
        Point proj;
        float dist = FLT_MAX;
        for (int i = 0; i < cam->bounds->num_vert; i++) {
            Point a = cam->bounds->poly[i];
            Point b = cam->bounds->poly[(i + 1) % cam->bounds->num_vert];
            Point curr_proj = line_project(target, a, b);
            float curr_dist = distance(target, curr_proj);
            if (dist > curr_dist) {
                dist = curr_dist;
                proj = curr_proj;
            }
        }
        target = proj;
    }
    cam->foc_x = target.x;
    cam->foc_y = target.y;
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
