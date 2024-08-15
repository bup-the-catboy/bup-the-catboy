#ifndef BTCB_LVLEDIT_WRITER_H
#define BTCB_LVLEDIT_WRITER_H

typedef struct WriteStream WriteStream;

WriteStream* writer_create(int initial_block_size);
void writer_make_offset(WriteStream* stream, int block_size);
void writer_make_null_offset(WriteStream* stream);
void writer_pop_block(WriteStream* stream);
void writer_write_ptr(WriteStream* stream, void* ptr, int size);
char* writer_close(WriteStream* stream, int* size);
template<typename T> void writer_write(WriteStream* stream, T value);

#endif