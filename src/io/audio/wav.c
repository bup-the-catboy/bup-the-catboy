#include "wav.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Context {
    short* samples;
    int length;
};

void validate_wav(unsigned char* data) {
    int format = 0, num_channels = 0, sample_rate = 0, bits_per_sample = 0;
    memcpy(&format, data + 20, 2);
    memcpy(&num_channels, data + 22, 2);
    memcpy(&sample_rate, data + 24, 4);
    memcpy(&bits_per_sample, data + 34, 2);
    if (format != 1 || num_channels != 2 || sample_rate != AUDIO_SAMPLE_RATE || bits_per_sample != 16) {
        printf("WAV file should be signed 16-bit stereo PCM with 44100 Hz sample rate\n");
    }
}

void* audio_wav_init(void* context) {
    int* instance = malloc(sizeof(int));
    *instance = 0;
    return instance;
}

void audio_wav_seek(void* context, void* instance, float sec) {
    *(int*)instance = sec / AUDIO_SAMPLE_RATE;
}

void audio_wav_speed(void* context, void* instance, float speed) {}

void audio_wav_free(void* context, void* instance) {
    free(instance);
}

bool audio_wav_play(void* context, void* instance, short* out, int samples) {
    struct Context* ctx = context;
    int* ptr = instance;
    bool done = false;
    for (int i = 0; i < samples; i++) {
        if (*ptr >= ctx->length) {
            out[i] = 0;
            done = true;
        }
        else out[i] = ctx->samples[*ptr];
        (*ptr)++;
    }
    return !done;
}

struct Audio* audio_load_wav(unsigned char* bin, int len) {
    validate_wav(bin);
    struct Audio* audio = malloc(sizeof(struct Audio));
    struct Context* context = malloc(sizeof(struct Context));
    memcpy(&context->length, bin + 40, 4);
    context->samples = (short*)bin + 44;
    context->length = (context->length - 44) / 2;
    audio->context = context;
    audio->init = audio_wav_init;
    audio->seek = audio_wav_seek;
    audio->speed = audio_wav_speed;
    audio->free = audio_wav_free;
    audio->play = audio_wav_play;
    return audio;
}