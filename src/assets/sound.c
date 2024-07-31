#include <SDL2/SDL_mixer.h>

#include "sound.h"

bool paused = false;
struct Audio* currmusic = NULL;

void music_finished() {
    if (!currmusic->looping) return;
    Mix_PlayMusic(currmusic->music, 0);
    Mix_SetMusicPosition(currmusic->loop);
}

void play_sound(struct Audio* sound) {
    Mix_PlayChannel(-1, sound->sound, 0);
}

void play_music(struct Audio* music) {
    paused = false;
    currmusic = music;
    Mix_PlayMusic(music->music, 0);
}

void pause_music() {
    if (!currmusic || paused) return;
    Mix_PauseMusic();
}

void resume_sound() {
    if (!currmusic || !paused) return;
    Mix_ResumeMusic();
}

void audio_init() {
    Mix_Init(MIX_INIT_OGG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 512);
    Mix_HookMusicFinished(music_finished);
}