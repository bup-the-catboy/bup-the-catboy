#ifndef SMBR_SOUND_H
#define SMBR_SOUND_H

#include <stdbool.h>
#include <SDL2/SDL_mixer.h>

struct Audio {
    float loop;
    bool looping;
    union {
        Mix_Music* music;
        Mix_Chunk* sound;
    };
};

void play_sound(struct Audio* sound);
void play_music(struct Audio* music);
void pause_music();
void resume_music();
void audio_init();

#endif