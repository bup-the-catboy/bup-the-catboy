#include "audio.h"

#include <stdlib.h>
#include <pthread.h>

#include <SDL2/SDL.h>

#define _(x) ((struct _AudioInstance*)x)
struct _AudioInstance {
    struct Audio* meta;
    void* data;
    bool playing;
    bool oneshot;
    bool do_free;
};

struct InstanceList {
    struct _AudioInstance* instance;
    struct InstanceList* next;
    struct InstanceList* prev;
};

struct InstanceList* instances;

bool audio_do_loop = true;
pthread_t audio_thread_id;
void* audio_thread(void* param);

struct InstanceList* make_list_entry(struct InstanceList* prev) {
    struct InstanceList* instance = malloc(sizeof(struct InstanceList));
    instance->instance = NULL;
    instance->next = NULL;
    instance->prev = prev;
    return instance;
}

void audio_init() {
    SDL_Init(SDL_INIT_AUDIO);
    instances = make_list_entry(NULL);
    pthread_create(&audio_thread_id, NULL, audio_thread, NULL);
}

void audio_stop(struct AudioInstance* instance) {
    _(instance)->do_free = true;
}

void audio_pause(struct AudioInstance* instance) {
    _(instance)->playing = false;
}

void audio_resume(struct AudioInstance* instance) {
    _(instance)->playing = true;
}

void audio_seek(struct AudioInstance* instance, float sec) {
    _(instance)->meta->seek(_(instance)->meta->context, _(instance)->data, sec);
}

void audio_speed(struct AudioInstance* instance, float speed) {
    _(instance)->meta->speed(_(instance)->meta->context, _(instance)->data, speed);
}

struct AudioInstance* audio_play(struct Audio* audio) {
    struct _AudioInstance* instance = malloc(sizeof(struct _AudioInstance));
    instance->meta = audio;
    instance->data = instance->meta->init(instance->meta->context);
    instance->oneshot = false;
    instance->do_free = false;
    instance->playing = true;
    struct InstanceList* list = instances;
    while (list->instance) {
        list = list->next;
    }
    list->instance = instance;
    list->next = make_list_entry(list);
    return (struct AudioInstance*)instance;
}

void audio_play_oneshot(struct Audio* audio) {
    struct AudioInstance* instance = audio_play(audio);
    _(instance)->oneshot = true;
}

void audio_update(short* out, int num_samples) {
    int mixed[num_samples];
    short samples[num_samples];
    struct InstanceList* list = instances;
    memset(mixed, 0, sizeof(mixed));
    while (list->instance) {
        if ((list->instance->oneshot && !list->instance->playing) || list->instance->do_free) {
            struct InstanceList* next = list->next;
            list->instance->meta->free(list->instance->meta->context, list->instance->data);
            if (list->next) list->next->prev = list->prev;
            if (list->prev) list->prev->next = list->next;
            else instances = next;
            free(list->instance);
            free(list);
            list = next;
            continue;
        }
        if (!list->instance->playing) continue;
        if (!list->instance->meta->play(list->instance->meta->context, list->instance->data, samples, num_samples)) {
            list->instance->playing = false;
        }
        for (int i = 0; i < num_samples; i++) {
            mixed[i] += samples[i];
        }
        list = list->next;
    }
    for (int i = 0; i < num_samples; i++) {
        if (mixed[i] > INT16_MAX) mixed[i] = INT16_MAX;
        if (mixed[i] < INT16_MIN) mixed[i] = INT16_MIN;
        out[i] = mixed[i];
    }
}

void audio_deinit() {
    audio_do_loop = false;
    pthread_join(audio_thread_id, NULL);
}

void* audio_thread(void* param) {
    SDL_AudioSpec spec;
    spec.freq = AUDIO_SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 1024;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
    short buffer[2048];
    while (audio_do_loop) {
        audio_update(buffer, 2048);
        while (SDL_GetQueuedAudioSize(1) > sizeof(buffer)) {
            SDL_Delay(10);
        }
        SDL_QueueAudio(1, buffer, sizeof(buffer));
    }
    struct InstanceList* list = instances;
    while (list->instance) {
        struct InstanceList* next = list->next;
        free(list->instance);
        free(list);
        list = next;
    }
    instances = list;
    instances->prev = NULL;
    instances->next = NULL;
    SDL_CloseAudio();
    return NULL;
}
