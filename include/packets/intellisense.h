// include this in packet_def.h so that intellisense doesnt cry

#ifndef NO_INTELLISENSE

#define STR_BUFLEN 1
#define PACKET(name, definition) typedef struct { definition } Packet_##name;
#define INT8(id) char id;
#define INT16(id) short id;
#define INT32(id) int id;
#define INT64(id) long id;
#define FLOAT(id) float id;
#define DOUBLE(id) double id;
#define STRING(id) char id[STR_BUFLEN];
#define ARRAY(id, definition) int __arrlen_##id; struct { definition }* id;
#define UNSIG unsigned

#endif