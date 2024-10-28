#ifndef BTCB_AUDIO_H
#define BTCB_AUDIO_H

#define AUDIO_SAMPLE_RATE 48000

#include <stdbool.h>

struct AudioInstance {};

struct Audio {
    void*  context;
    void*(*init  )(void* context);
    void (*seek  )(void* context, void* instance, float sec);
    void (*speed )(void* context, void* instance, float speed);
    void (*free  )(void* context, void* instance);
    bool (*play  )(void* context, void* instance, short* out, int samples);
};

void audio_init();
void audio_stop(struct AudioInstance* instance);
void audio_pause(struct AudioInstance* instance);
void audio_resume(struct AudioInstance* instance);
void audio_seek(struct AudioInstance* instance, float sec);
void audio_speed(struct AudioInstance* instance, float speed);
void audio_play_oneshot(struct Audio* audio); // holy fucking shit niko oneshot 🥞🐱
struct AudioInstance* audio_play(struct Audio* audio);
void audio_update(short* out, int samples);
void audio_deinit();

#endif