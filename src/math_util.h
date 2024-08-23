#ifndef BTCB_MATH_UTIL_H
#define BTCB_MATH_UTIL_H

#include <foreach.h>
#include <math.h>
#include <stdlib.h>

#define _( ...)
#define __(...) __VA_ARGS__

#define _DOUBLE(x)  (,) double x      __
#define _FLOAT(x)   (,) float x       __
#define _LDOUBLE(x) (,) long double x __

#define MATHFUNC(name, ...) \
    double      name   (EXPAND(_ FOR_EACH(_DOUBLE,  __VA_ARGS__)())); \
    float       name##f(EXPAND(_ FOR_EACH(_FLOAT,   __VA_ARGS__)())); \
    long double name##l(EXPAND(_ FOR_EACH(_LDOUBLE, __VA_ARGS__)()));

MATHFUNC(clamp, x, min, max)
MATHFUNC(wrap,  x, min, max)
MATHFUNC(min, a, b)
MATHFUNC(max, a, b)

#undef _
#undef __
#undef _DOUBLE
#undef _FLOAT
#undef _LDOUBLE
#undef MATHFUNC

#endif