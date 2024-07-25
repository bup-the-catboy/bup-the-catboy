#ifndef SMBR_BINARY_READER_H
#define SMBR_BINARY_READER_H

#include "assets/assets.h"

struct BinaryStream {
    int ptr;
    const unsigned char* data;
    struct BinaryStream* parent;
};

#define BINARY_STREAM_READ(stream, var) binary_stream_read(stream, &var, sizeof(var))
#define BINARY_STREAM_GOTO(stream) ({                       \
    struct BinaryStream* child = binary_stream_goto(stream); \
    ((child) == (stream)) ? NULL : (child);                   \
})

struct BinaryStream* binary_stream_create(const unsigned char* data);
struct BinaryStream* binary_stream_goto(struct BinaryStream* stream);
struct BinaryStream* binary_stream_close(struct BinaryStream* stream);
void binary_stream_read(struct BinaryStream* stream, void* dest, size_t size);
void binary_stream_read_string(struct BinaryStream*, char* dest, size_t buflen);
void binary_stream_skip(struct BinaryStream* stream, int amount);

#endif