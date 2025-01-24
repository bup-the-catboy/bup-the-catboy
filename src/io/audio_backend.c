#include <SDL2/SDL.h>

#include "io/audio/audio.h"

void audio_backend_open() {
    SDL_Init(SDL_INIT_AUDIO);
    SDL_AudioSpec spec;
    spec.freq = AUDIO_SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 2;
    spec.samples = 1024;
    spec.callback = NULL;
    SDL_OpenAudio(&spec, NULL);
    SDL_PauseAudio(0);
}

void audio_backend_queue(short* samples, int num_samples) {
    while (SDL_GetQueuedAudioSize(1) > sizeof(short) * num_samples) {
        SDL_Delay(1);
    }
    SDL_QueueAudio(1, samples, sizeof(short) * num_samples);
    SDL_Delay(1);
}

void audio_backend_close() {
    SDL_CloseAudio();
}
