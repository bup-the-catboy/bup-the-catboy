#ifndef SMBR_AUDIO_NSF_H
#define SMBR_AUDIO_NSF_H

#include "audio.h"

void audio_nsf_select_track(struct Audio* nsf, int track);
struct Audio* audio_load_nsf(unsigned char* bin, int len);

#endif