#ifndef SMBR_AUDIO_WAV_H
#define SMBR_AUDIO_WAV_H

#include "audio.h"

struct Audio* audio_load_wav(unsigned char* bin, int len);

#endif