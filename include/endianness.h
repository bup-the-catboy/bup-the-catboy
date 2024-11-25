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

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#ifndef IS_LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN
#endif
#define BE(x) FLIP_ENDIAN(x)
#define LE(x) (x)
#else
#ifndef IS_BIG_ENDIAN
#define IS_BIG_ENDIAN
#endif
#define BE(x) (x)
#define LE(x) FLIP_ENDIAN(x)
#endif

#endif
