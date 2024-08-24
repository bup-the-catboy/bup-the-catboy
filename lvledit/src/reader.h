#ifndef BTCB_LVLEDIT_READER_H
#define BTCB_LVLEDIT_READER_H

#include <vector>
#include <stdlib.h>
#include <string.h>

#define curr_ptr ptr[0]
#define push_ptr(x) ptr.insert(stream->ptr.begin(), x)
#define pop_ptr() ptr.erase(stream->ptr.begin())

struct ReadStream {
    unsigned char* data;
    std::vector<int> ptr;
};

static ReadStream* reader_create(unsigned char* ptr) {
    ReadStream* stream = new ReadStream();
    stream->data = ptr;
    stream->ptr = {0};
    return stream;
}

template<typename T> static T reader_read(ReadStream* stream) {
    T val;
    reader_read_ptr(stream, &val, sizeof(T));
    return val;
}

static void reader_goto(ReadStream* stream) {
    stream->push_ptr(reader_read<int>(stream));
}

static void reader_pop_block(ReadStream* stream) {
    stream->pop_ptr();
}

static void reader_read_ptr(ReadStream* stream, void* out, int size) {
    memcpy(out, stream->data + stream->curr_ptr, size);
    stream->curr_ptr += size;
}

static void reader_skip(ReadStream* stream, int bytes) {
    stream->curr_ptr += bytes;
}

static void reader_close(ReadStream* stream) {
    delete stream;
}

#endif