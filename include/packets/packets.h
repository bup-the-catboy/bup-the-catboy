#ifndef PacketsLib_H
#define PacketsLib_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef STR_BUFLEN
#define STR_BUFLEN 1024
#endif

#define COMBINE1(x, y) x##y
#define COMBINE(x, y) COMBINE1(x, y)
#define LN(x) COMBINE(x, __LINE__)

#ifdef __cplusplus
#define ARRFUNC auto LN(__arrfunc_) = [&](LN(__arrtype_)* packet)
#else
#define ARRFUNC void LN(__arrfunc_)(LN(__arrtype_)* packet)
#endif

#define NO_INTELLISENSE

#define zalloc(size) memset(malloc(size), 0, size)
#define deserialize_packet(type, buf) (Packet_##type*)deserialize_unk_packet(buf)
#define make_packet(type) ({ \
    Packet_##type* packet = (Packet_##type*)zalloc(sizeof(Packet_##type)); \
    packet->__type = PacketType_##type; \
    packet; \
})
#define make_array(arr, count) { \
    arr##_arrlen = count; \
    arr = (typeof(arr))zalloc(sizeof(*(arr)) * (count)); \
}
#define packet_type(x) *(int*)(x)

#define PACKET(name, definition) typedef struct { int __type; definition } Packet_##name;
#define INT8(id) char id;
#define INT16(id) short id;
#define INT32(id) int id;
#define INT64(id) long id;
#define FLOAT(id) float id;
#define DOUBLE(id) double id;
#define STRING(id) char id[STR_BUFLEN];
#define ARRAY(id, definition) int id##_arrlen; struct { definition }* id;
#define UNSIG unsigned
#include "packet_def.h"
#undef PACKET
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef FLOAT
#undef DOUBLE
#undef STRING
#undef ARRAY
#undef UNSIG

#define PACKET(name, definition) PacketType_##name,
enum PacketType {
#include "packet_def.h"
};
#undef PACKET

#define PACKET(name, definition) case PacketType_##name: { \
    Packet_##name* packet = (Packet_##name*)pkt; \
    definition \
    free(packet); \
} break;
#define INT8(id)
#define INT16(id)
#define INT32(id)
#define INT64(id)
#define FLOAT(id)
#define STRING(id)
#define ARRAY(id, definition) \
    typedef typeof(packet->id[0]) LN(__arrtype_); \
    ARRFUNC { \
        definition \
    }; \
    for(int LN(__iter_) = 0; \
        LN(__iter_) < packet->id##_arrlen; \
        LN(__iter_)++) LN(__arrfunc_)(&packet->id[LN(__iter_)]); \
    if (packet->id) free(packet->id);
#define UNSIG
static void free_packet(void* pkt) {
    int __type = packet_type(pkt);
    switch (__type) {
#include "packet_def.h"
    }
}
#undef PACKET
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef FLOAT
#undef DOUBLE
#undef STRING
#undef ARRAY

#define PUT(x) memcpy(buf + __iter, &(x), sizeof(x)); __iter += sizeof(x)
#define PACKET(name, definition) case PacketType_##name: { \
    Packet_##name* packet = (Packet_##name*)pkt; \
    int __iter = 0; \
    PUT(__type); \
    definition \
} break;
#define INT8(id) PUT(packet->id);
#define INT16(id) PUT(packet->id);
#define INT32(id) PUT(packet->id);
#define INT64(id) PUT(packet->id);
#define FLOAT(id) PUT(packet->id);
#define DOUBLE(id) PUT(packet->id);
#define STRING(id) memcpy(buf + __iter, packet->id, strlen(packet->id) + 1); __iter += strlen(packet->id) + 1;
#define ARRAY(id, definition) \
    PUT(packet->id##_arrlen); \
    typedef typeof(packet->id[0]) LN(__arrtype_); \
    ARRFUNC { \
        definition \
    }; \
    for(int LN(__iter_) = 0; \
        LN(__iter_) < packet->id##_arrlen; \
        LN(__iter_)++) LN(__arrfunc_)(&packet->id[LN(__iter_)]);

static void serialize_packet(void* pkt, char* buf) {
    int __type = packet_type(pkt);
    switch (__type) {
#include "packet_def.h"
    }
}

#undef PUT
#undef PACKET
#undef STRING
#define PUT(x) __iter += sizeof(x)
#define PACKET(name, definition) case PacketType_##name: { \
    Packet_##name* packet = (Packet_##name*)pkt; \
    int __iter = 0; \
    PUT(__type); \
    definition \
    return __iter; \
}
#define STRING(id) __iter += strlen(packet->id) + 1;
static int serialized_packet_size(void* pkt) {
    int __type = packet_type(pkt);
    switch (__type) {
#include "packet_def.h"
    }
    return 0;
}
#undef PACKET
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef FLOAT
#undef DOUBLE
#undef STRING
#undef ARRAY
#undef PUT

#define GET(x) memcpy(&(x), buf + __iter, sizeof(x)); __iter += sizeof(x)
#define PACKET(name, definition) case PacketType_##name: { Packet_##name* packet = make_packet(name); definition return packet; } break;
#define INT8(id) GET(packet->id);
#define INT16(id) GET(packet->id);
#define INT32(id) GET(packet->id);
#define INT64(id) GET(packet->id);
#define FLOAT(id) GET(packet->id);
#define DOUBLE(id) GET(packet->id);
#define STRING(id) strncpy(packet->id, buf + __iter, STR_BUFLEN); __iter += strlen(buf + __iter) + 1;
#define ARRAY(id, definition) \
    GET(packet->id##_arrlen); \
    make_array(packet->id, packet->id##_arrlen); \
    typedef typeof(packet->id[0]) LN(__arrtype_); \
    ARRFUNC { \
        definition \
    }; \
    for(int LN(__iter_) = 0; \
        LN(__iter_) < packet->id##_arrlen; \
        LN(__iter_)++) LN(__arrfunc_)(&packet->id[LN(__iter_)]);
static void* deserialize_unk_packet(char* buf) {
    int __iter = 0;
    int __type;
    GET(__type);
    switch (__type) {
#include "packet_def.h"
    }
    return NULL;
}
#undef PACKET
#undef INT8
#undef INT16
#undef INT32
#undef INT64
#undef FLOAT
#undef DOUBLE
#undef STRING
#undef ARRAY
#undef UNSIG
#undef GET

#undef COMBINE1
#undef COMBINE
#undef LN
#undef FUNCDEF
#undef NO_INTELLISENSE

#endif