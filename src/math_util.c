#include "math_util.h"

#ifdef WINDOWS
#undef min
#undef max
#endif

#define _( ...)
#define __(...) __VA_ARGS__

#define _DOUBLE(x)  (,) double x      __
#define _FLOAT(x)   (,) float x       __
#define _LDOUBLE(x) (,) long double x __

#define MATHFUNC(name, body, ...) \
    double      name   (EXPAND(_ FOR_EACH(_DOUBLE,  __VA_ARGS__)())) { typedef double type;      (void)((type)0); body } \
    float       name##f(EXPAND(_ FOR_EACH(_FLOAT,   __VA_ARGS__)())) { typedef float type;       (void)((type)0); body } \
    long double name##l(EXPAND(_ FOR_EACH(_LDOUBLE, __VA_ARGS__)())) { typedef long double type; (void)((type)0); body }

MATHFUNC(clamp, {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}, x, min, max)

MATHFUNC(wrap, {
    type range = max - min;
    while (x < min) x += range;
    while (x > max) x -= range;
    return x;
}, x, min, max)

MATHFUNC(min, {
    return a < b ? a : b;
}, a, b)

MATHFUNC(max, {
    return a > b ? a : b;
}, a, b)

MATHFUNC(lerp, {
    return (t - f) * x + f;
}, x, f, t)

MATHFUNC(map, {
    return (fmax - fmin) / (x - fmin) * (tmax - tmin) + tmin;
}, fmin, fmax, tmin, tmax, x)

MATHFUNC(sin_range, {
    return (sin(x * 2 * M_PI) + 1) / 2 * (max - min) + min;
}, x, min, max)

#define IN_OUT(func) { return in_out(x, func##_in, func##_out); }
#define EXPO_IN(val) { return expo_in(x, val); }
#define EXPO_OUT(val) { return expo_out(x, val); }

float in_out(float x, Easing in, Easing out) {
    if (x < 0.5f) return in(x * 2) / 2;
    return (out(x * 2 - 1) + 1) / 2;
}

float expo_in(float x, float e) {
    return pow(x, e);
}

float expo_out(float x, float e) {
    return 1 - pow(1 - x, e);
}

float linear(float x) {
    return x;
}

float sin_in(float x) {
    return 1 - cos((x * M_PI) / 2);
}

float sin_out(float x) {
    return sin((x * M_PI) / 2);
}

float elastic_in(float x) {
    float c1 = 1.70158f;
    float c2 = c1 + 1;
    return c2 * x * x * x - c1 * x * x;
}

float elastic_out(float x) {
    float c1 = 1.70158f;
    float c2 = c1 + 1;
    return 1 + c2 * pow(x - 1, 3) + c1 * pow(x - 1, 2);
}

float quad_in(float x) EXPO_IN(2)
float quad_out(float x) EXPO_OUT(2)
float cubic_in(float x) EXPO_IN(3)
float cubic_out(float x) EXPO_OUT(3)
float sin_in_out(float x) IN_OUT(sin)
float quad_in_out(float x) IN_OUT(quad)
float cubic_in_out(float x) IN_OUT(cubic)
float elastic_in_out(float x) IN_OUT(elastic)

bool rect_contains_point(float x, float y, float rx, float ry, float rw, float rh) {
    return x >= rx && y >= ry && x <= rx + rw && y <= ry + rh;
}

bool rect_intersects_rect(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h) {
    float r1t = fminf(r1y, r1y + r1h); float r2t = fminf(r2y, r2y + r2h);
    float r1b = fmaxf(r1y, r1y + r1h); float r2b = fmaxf(r2y, r2y + r2h);
    float r1l = fminf(r1x, r1x + r1w); float r2l = fminf(r2x, r2x + r2w);
    float r1r = fmaxf(r1x, r1x + r1w); float r2r = fmaxf(r2x, r2x + r2w);
    return r2l < r1r && r2r > r1l && r2t < r1b && r2b > r1t;
}

bool rect_contains_rect(float r1x, float r1y, float r1w, float r1h, float r2x, float r2y, float r2w, float r2h)  {
    float r1t = fminf(r1y, r1y + r1h); float r2t = fminf(r2y, r2y + r2h);
    float r1b = fmaxf(r1y, r1y + r1h); float r2b = fmaxf(r2y, r2y + r2h);
    float r1l = fminf(r1x, r1x + r1w); float r2l = fminf(r2x, r2x + r2w);
    float r1r = fmaxf(r1x, r1x + r1w); float r2r = fmaxf(r2x, r2x + r2w);
    return r1l >= r2l && r1r <= r2r && r1t >= r2t && r1b <= r2b;
}

void clamp_rect(float cx, float cy, float cw, float ch, float* rx, float* ry, float rw, float rh) {
    if (rw < cw) {
        if (*rx < cx) *rx = cx;
        else if (*rx + rw > cx + cw) *rx = cx + cw - rw;
    }
    else *rx = cx + (cw - rw) / 2;

    if (rh < ch) {
        if (*ry < cy) *ry = cy;
        else if (*ry + rh > cy + ch) *ry = cy + ch - rh;
    }
    else *ry = cy + (ch - rh) / 2;
}
