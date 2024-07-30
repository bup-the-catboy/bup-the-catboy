#include "nsf.h"
#include "nsf_internal.h"

#include "cpu.h"

#include "assets/binary_reader.h"

NSFHandle* nsf_load(const unsigned char* buf, unsigned int len) {
    NSFHandle* handle = malloc(sizeof(NSFHandle));
    struct BinaryStream* stream = binary_stream_create(buf);
    binary_stream_skip(stream, 6);
    binary_stream_read(stream, &handle->num_songs, 1);
    binary_stream_read(stream, &handle->cur_song, 1);
    binary_stream_read(stream, &handle->load_addr, 2);
    binary_stream_read(stream, &handle->init_addr, 2);
    binary_stream_read(stream, &handle->play_addr, 2);
    binary_stream_skip(stream, 32 * 3);
    binary_stream_read(stream, &handle->ntsc_play_speed, 2);
    binary_stream_read(stream, &handle->bank_switch_values, 8);
    binary_stream_read(stream, &handle->pal_play_speed, 2);
    binary_stream_read(stream, &handle->format_flags, 1);
    binary_stream_read(stream, &handle->sound_chip_flags, 1);
    binary_stream_skip(stream, 1);
    binary_stream_read(stream, &handle->program_length, 3);
    handle->program = malloc(handle->program_length);
    handle->cpu.ram = malloc(0x10000);
    handle->playing = handle->inited = false;
    handle->uses_bank_switching = UNSET;
    handle->cur_song--;
    if (handle->program_length == 0) handle->program_length = len - stream->ptr;
    binary_stream_read(stream, handle->program, handle->program_length);
    return handle;
}

void nsf_select(NSFHandle* handle, int song) {
    if (song < 0 || song >= handle->num_songs) return;
    handle->cur_song = song;
}

void nsf_play(NSFHandle* handle) {
    handle->playing = handle->inited = true;
    cpu_init(handle);
}

bool nsf_is_playing(NSFHandle* handle) {
    return handle->playing;
}

void nsf_stop(NSFHandle* handle) {
    if (!handle->inited) return;
    handle->playing = false;
}

void nsf_resume(NSFHandle* handle) {
    if (!handle->inited) return;
    handle->playing = true;
}

void nsf_dispose(NSFHandle* handle) {
    cpu_dispose(handle);
    free(handle->program);
    free(handle->cpu.ram);
    free(handle);
}
