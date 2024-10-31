#ifndef ENDIANNESS_H
#define ENDIANNESS_H

#define FLIP_ENDIAN(x) ({                \
    union {                               \
        char arr[sizeof(x)];               \
        typeof(x) val;                      \
    } in, out;                               \
    in.val = (x);                             \
    for (int i = 0; i < sizeof(x); i++) {      \
        out.arr[sizeof(x) - 1 - i] = in.arr[i]; \
    }                                            \
    out.val;                                      \
})

#if defined(__ORDER_LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN__)
#define BE(x) FLIP_ENDIAN(x)
#define LE(x) (x)
#else
#define BE(x) (x)
#define LE(x) FLIP_ENDIAN(x)
#endif

#endif
