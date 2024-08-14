#ifndef NO_INTELLISENSE

#include <stdint.h>

typedef char string[1024];

#define OBJECT(name, type) typedef type LibSerialObj_##name;
#define STRUCT(definition) struct { definition }
#define INT8 int8_t
#define INT16 int16_t
#define INT32 int32_t
#define INT64 int64_t
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define FLOAT float
#define DOUBLE double
#define STRING string
#define ARRAY(type) type*
#define _(x) x;

#endif