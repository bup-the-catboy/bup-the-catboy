#include "binary_reader.h"

#include <string.h>
#include <stdlib.h>

struct BinaryStream* binary_stream_create(const unsigned char* binary) {
    struct BinaryStream* stream = malloc(sizeof(struct BinaryStream));
    stream->ptr = 0;
    stream->data = binary;
    stream->parent = NULL;
    return stream;
}

struct BinaryStream* binary_stream_goto(struct BinaryStream* stream) {
    unsigned int offset;
    binary_stream_read(stream, &offset, sizeof(offset));
    if (offset == 0) return stream;
    struct BinaryStream* offsetted = binary_stream_create(stream->data);
    offsetted->ptr = offset;
    offsetted->parent = stream;
    return offsetted;
}

struct BinaryStream* binary_stream_close(struct BinaryStream* stream) {
    struct BinaryStream* parent = stream->parent;
    free(stream);
    return parent;
}

void binary_stream_read(struct BinaryStream* stream, void* dest, size_t size) {
    memcpy(dest, stream->data + stream->ptr, size);
    stream->ptr += size;
}

void binary_stream_read_string(struct BinaryStream* stream, char* dest, size_t buflen) {
    strncpy(dest, (char*)(stream->data + stream->ptr), buflen - 1);
    stream->ptr += strlen(dest) + 1;
}

void binary_stream_skip(struct BinaryStream* stream, int amount) {
    stream->ptr += amount;
}
