#ifndef BTCB_LVLEDIT_WRITER_H
#define BTCB_LVLEDIT_WRITER_H

#include <vector>
#include <stdlib.h>
#include <string.h>

#define curr_ptr ptr[0]
#define push_ptr(x) ptr.insert(stream->ptr.begin(), x)
#define pop_ptr() ptr.erase(stream->ptr.begin())

struct WriteStream {
    char* data;
    int total_size;
    std::vector<int> ptr;
};

template<typename T> static void writer_write(WriteStream* stream, T value) {
    writer_write_ptr(stream, &value, sizeof(T));
}

static WriteStream* writer_create(int initial_block_size) {
    WriteStream* stream = new WriteStream();
    stream->data = (char*)malloc(initial_block_size);
    stream->total_size = initial_block_size;
    stream->ptr = {0};
    return stream;
}

static void writer_make_offset(WriteStream* stream, int block_size) {
    writer_write(stream, stream->total_size);
    stream->push_ptr(stream->total_size);
    stream->total_size += block_size;
    stream->data = (char*)realloc(stream->data, stream->total_size);
}

static void writer_make_null_offset(WriteStream* stream) {
    writer_write(stream, 0);
}

static void writer_pop_block(WriteStream* stream) {
    stream->pop_ptr();
}

static void writer_write_ptr(WriteStream* stream, void* ptr, int size) {
    memcpy(stream->data + stream->curr_ptr, ptr, size);
    stream->curr_ptr += size;
}

static void writer_skip(WriteStream* stream, int bytes) {
    stream->curr_ptr += bytes;
}

static char* writer_close(WriteStream* stream, int* size) {
    char* data = stream->data;
    if (size) *size = stream->total_size;
    delete stream;
    return data;
}

#endif