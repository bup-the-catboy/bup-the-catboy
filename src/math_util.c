#include "math_util.h"

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