#include "nsf.h"

#include <gme.h>

#include <stdlib.h>

struct Template {
    unsigned char* data;
    int len;
    int track;
};

void* audio_nsf_init(void* context) {
    struct Template* template = context;
    Music_Emu* emu;
    gme_open_data(template->data, template->len, &emu, AUDIO_SAMPLE_RATE);
    gme_start_track(emu, template->track);
    return emu;
}

void audio_nsf_seek(void* context, void* instance, float sec) {
    gme_seek(instance, sec * 1000);
}

void audio_nsf_speed(void* context, void* instance, float speed) {
    gme_set_tempo(instance, speed);
}

void audio_nsf_free(void* context, void* instance) {
    gme_delete(instance);
}

bool audio_nsf_play(void* context, void* instance, short* out, int samples) {
    gme_play(instance, samples, out);
    return true;
}

void audio_nsf_select_track(struct Audio* nsf, int track) {
    ((struct Template*)nsf->context)->track = track;
}

struct Audio* audio_load_nsf(unsigned char* bin, int len) {
    struct Audio* audio = malloc(sizeof(struct Audio));
    struct Template* template = malloc(sizeof(struct Template));
    template->data = bin;
    template->len = len;
    audio->context = template;
    audio->init = audio_nsf_init;
    audio->seek = audio_nsf_seek;
    audio->speed = audio_nsf_speed;
    audio->free = audio_nsf_free;
    audio->play = audio_nsf_play;
    return audio;
}