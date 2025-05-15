#include "sfxr.h"
#include "rng.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct SFXRContext {
    int wave_type;
    float p_base_freq;
    float p_freq_limit;
    float p_freq_ramp;
    float p_freq_dramp;
    float p_duty;
    float p_duty_ramp;
    float p_vib_strength;
    float p_vib_speed;
    float p_vib_delay;
    float p_env_attack;
    float p_env_sustain;
    float p_env_decay;
    float p_env_punch;
    bool filter_on;
    float p_lpf_resonance;
    float p_lpf_freq;
    float p_lpf_ramp;
    float p_hpf_freq;
    float p_hpf_ramp;
    float p_pha_offset;
    float p_pha_ramp;
    float p_repeat_speed;
    float p_arp_speed;
    float p_arp_mod;
    float master_vol;
    float sound_vol;
    int phase;
    double fperiod;
    double fmaxperiod;
    double fslide;
    double fdslide;
    int period;
    float square_duty;
    float square_slide;
    int env_stage;
    int env_time;
    int env_length[3];
    float env_vol;
    float fphase;
    float fdphase;
    int iphase;
    float phaser_buffer[1024];
    int ipp;
    float noise_buffer[32];
    float fltp;
    float fltdp;
    float fltw;
    float fltw_d;
    float fltdmp;
    float fltphp;
    float flthp;
    float flthp_d;
    float vib_phase;
    float vib_speed;
    float vib_amp;
    int rep_time;
    int rep_limit;
    int arp_time;
    int arp_limit;
    double arp_mod;
};

void audio_sfxr_defaults(struct SFXRContext* context) {
    memset(context, 0, sizeof(struct SFXRContext));
    context->master_vol = 0.05f;
    context->sound_vol = 0.5f;
}

#pragma region sfxr_synth

// License for sfxr's source code, part of which is used here

/*
   Copyright (c) 2007 Tomas Pettersson <drpetter@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

*/

#define read(type) ({               \
    type val = *(type*)(data + ptr); \
    ptr += sizeof(type);              \
    val;                               \
})

float frnd(float x) {
    return random_float() * x;
}

void audio_sfxr_read(struct SFXRContext* context, unsigned char* data, int datalen) {
    size_t ptr = 0;
    context->sound_vol = 0.5f;
    int version = read(int);
    context->wave_type = read(int);
    if(version == 102) context->sound_vol = read(float);
    context->p_base_freq = read(float);
    context->p_freq_limit = read(float);
    context->p_freq_ramp = read(float);
    if(version >= 101) context->p_freq_dramp = read(float);
    context->p_duty = read(float);
    context->p_duty_ramp = read(float);
    context->p_vib_strength = read(float);
    context->p_vib_speed = read(float);
    context->p_vib_delay = read(float);
    context->p_env_attack = read(float);
    context->p_env_sustain = read(float);
    context->p_env_decay = read(float);
    context->p_env_punch = read(float);
    context->filter_on = read(bool);
    context->p_lpf_resonance = read(float);
    context->p_lpf_freq = read(float);
    context->p_lpf_ramp = read(float);
    context->p_hpf_freq = read(float);
    context->p_hpf_ramp = read(float);
    context->p_pha_offset = read(float);
    context->p_pha_ramp = read(float);
    context->p_repeat_speed = read(float);
    if(version >= 101) {
        context->p_arp_speed = read(float);
        context->p_arp_mod = read(float);
    }
}

void audio_sfxr_reset(struct SFXRContext* context, bool restart) {
    context->fperiod = 100.0 / (context->p_base_freq * context->p_base_freq + 0.001);
    context->period = (int)context->fperiod;
    context->fmaxperiod = 100.0 / (context->p_freq_limit * context->p_freq_limit + 0.001);
    context->fslide = 1.0 - pow((double)context->p_freq_ramp, 3.0) * 0.01;
    context->fdslide = -pow((double)context->p_freq_dramp, 3.0) * 0.000001;
    context->square_duty = 0.5f - context->p_duty * 0.5f;
    context->square_slide = -context->p_duty_ramp * 0.00005f;
    if (context->p_arp_mod >= 0.0f) context->arp_mod = 1.0 - pow((double)context->p_arp_mod, 2.0) * 0.9;
    else context->arp_mod = 1.0 + pow((double)context->p_arp_mod, 2.0) * 10.0;
    context->arp_time = 0;
    context->arp_limit = (int)(pow(1.0f - context->p_arp_speed, 2.0f) * 20000 + 32);
    if (context->p_arp_speed == 1.0f) context->arp_limit = 0;
    if (!restart) {
        context->phase = 0;
        context->fltp = 0.0f;
        context->fltdp = 0.0f;
        context->fltw = pow(context->p_lpf_freq, 3.0f) * 0.1f;
        context->fltw_d = 1.0f + context->p_lpf_ramp * 0.0001f;
        context->fltdmp = 5.0f / (1.0f + pow(context->p_lpf_resonance, 2.0f) * 20.0f) * (0.01f + context->fltw);
        if (context->fltdmp > 0.8f) context->fltdmp = 0.8f;
        context->fltphp = 0.0f;
        context->flthp = pow(context->p_hpf_freq, 2.0f) * 0.1f;
        context->flthp_d = 1.0 + context->p_hpf_ramp * 0.0003f;
        context->vib_phase = 0.0f;
        context->vib_speed = pow(context->p_vib_speed, 2.0f) * 0.01f;
        context->vib_amp = context->p_vib_strength * 0.5f;
        context->env_vol = 0.0f;
        context->env_stage = 0;
        context->env_time = 0;
        context->env_length[0] = (int)(context->p_env_attack * context->p_env_attack * 100000.0f);
        context->env_length[1] = (int)(context->p_env_sustain * context->p_env_sustain * 100000.0f);
        context->env_length[2] = (int)(context->p_env_decay * context->p_env_decay * 100000.0f);
        context->fphase = pow(context->p_pha_offset, 2.0f) * 1020.0f;
        if (context->p_pha_offset < 0.0f) context->fphase = -context->fphase;
        context->fdphase = pow(context->p_pha_ramp, 2.0f) * 1.0f;
        if (context->p_pha_ramp < 0.0f) context->fdphase = -context->fdphase;
        context->iphase = abs((int)context->fphase);
        context->ipp = 0;
        for (int i = 0; i < 1024; i++) context->phaser_buffer[i] = 0.0f;
        for (int i = 0; i < 32;   i++) context-> noise_buffer[i] = frnd(2.0f) - 1.0f;
        context->rep_time = 0;
        context->rep_limit = (int)(pow(1.0f - context->p_repeat_speed, 2.0f) * 20000 + 32);
        if (context->p_repeat_speed == 0.0f) context->rep_limit=0;
    }
}

bool audio_sfxr_synth(int length, float* buffer, struct SFXRContext* context) {
    bool playing_sample = true;
    for (int i = 0; i < length; i++) {
        if (!playing_sample) break;
        context->rep_time++;
        if (context->rep_limit != 0 && context->rep_time >= context->rep_limit) {
            context->rep_time = 0;
            audio_sfxr_reset(context, true);
        }
        context->arp_time++;
        if (context->arp_limit != 0 && context->arp_time >= context->arp_limit) {
            context->arp_limit = 0;
            context->fperiod *= context->arp_mod;
        }
        context->fslide += context->fdslide;
        context->fperiod *= context->fslide;
        if (context->fperiod > context->fmaxperiod) {
            context->fperiod = context->fmaxperiod;
            if (context->p_freq_limit > 0.0f) playing_sample = false;
        }
        float rfperiod = context->fperiod;
        if (context->vib_amp > 0.0f) {
            context->vib_phase += context->vib_speed;
            rfperiod = context->fperiod * (1.0 + sin(context->vib_phase) * context->vib_amp);
        }
        context->period = (int)rfperiod;
        if (context->period < 8) context->period = 8;
        context->square_duty += context->square_slide;
        if (context->square_duty < 0.0f) context->square_duty = 0.0f;
        if (context->square_duty > 0.5f) context->square_duty = 0.5f;
        context->env_time++;
        if (context->env_time > context->env_length[context->env_stage]) {
            context->env_time = 0;
            context->env_stage++;
            if (context->env_stage == 3) playing_sample = false;
        }
        if (context->env_stage == 0) context->env_vol = (float)context->env_time / context->env_length[0];
        if (context->env_stage == 1) context->env_vol = 1.0f + pow(1.0f - (float)context->env_time / context->env_length[1], 1.0f) * 2.0f * context->p_env_punch;
        if (context->env_stage == 2) context->env_vol = 1.0f - (float)context->env_time / context->env_length[2];
        context->fphase += context->fdphase;
        context->iphase = abs((int)context->fphase);
        if (context->iphase > 1023) context->iphase = 1023;
        if (context->flthp_d != 0.0f) {
            context->flthp *= context->flthp_d;
            if (context->flthp < 0.00001f) context->flthp = 0.00001f;
            if (context->flthp > 0.1f) context->flthp = 0.1f;
        }
        float ssample = 0.0f;
        for (int si = 0; si < 8; si++) {
            float sample = 0.0f;
            context->phase++;
            if (context->phase >= context->period) {
                context->phase %= context->period;
                if (context->wave_type == 3) {
                    for (int i = 0; i < 32; i++) context->noise_buffer[i] = frnd(2.0f) - 1.0f;
                }
            }
            float fp = (float)context->phase / context->period;
            switch (context->wave_type) {
                case 0:
                    if (fp < context->square_duty) sample = 0.5f;
                    else sample = -0.5f;
                    break;
                case 1:
                    sample = 1.0f - fp * 2;
                    break;
                case 2:
                    sample = (float)sin(fp * 2 * M_PI);
                    break;
                case 3:
                    sample = context->noise_buffer[context->phase * 32 / context->period];
                    break;
            }
            float pp = context->fltp;
            context->fltw *= context->fltw_d;
            if (context->fltw < 0.0f) context->fltw = 0.0f;
            if (context->fltw > 0.1f) context->fltw = 0.1f;
            if (context->p_lpf_freq != 1.0f) {
                context->fltdp += (sample - context->fltp) * context->fltw;
                context->fltdp -= context->fltdp * context->fltdmp;
            }
            else {
                context->fltp = sample;
                context->fltdp = 0.0f;
            }
            context->fltp += context->fltdp;
            context->fltphp += context->fltp - pp;
            context->fltphp -= context->fltphp * context->flthp;
            sample = context->fltphp;
            context->phaser_buffer[context->ipp & 1023] = sample;
            sample += context->phaser_buffer[(context->ipp - context->iphase + 1024) & 1023];
            context->ipp = (context->ipp + 1) & 1023;
            ssample += sample * context->env_vol;
        }
        ssample = ssample / 8 * context->master_vol;
        ssample *= 2.0f * context->sound_vol;
        if (buffer != NULL) {
            if (ssample > 1.0f) ssample = 1.0f;
            if (ssample < -1.0f) ssample = -1.0f;
            *buffer++ = ssample;
        }
    }
    return playing_sample;
}

#pragma endregion

void* audio_sfxr_init(void* context) {
    struct SFXRContext* instance = malloc(sizeof(struct SFXRContext));
    memcpy(instance, context, sizeof(struct SFXRContext));
    return instance;
}

void audio_sfxr_seek(void* context, void* instance, float sec) {}
void audio_sfxr_speed(void* context, void* instance, float speed) {}

void audio_sfxr_free(void* context, void* instance) {
    free(instance);
}

bool audio_sfxr_play(void* context, void* instance, short* out, int samples) {
    float s[samples / 2];
    memset(s, 0, sizeof(float) * samples / 2);
    bool playing = audio_sfxr_synth(samples / 2, s, (struct SFXRContext*)instance);
    for (int i = 0; i < samples / 2; i++) {
        out[i * 2] = out[i * 2 + 1] = s[i] * 65536;
    }
    return playing;
}

struct Audio* audio_load_sfxr(unsigned char* bin, int len) {
    struct Audio* audio = malloc(sizeof(struct Audio));
    struct SFXRContext* context = malloc(sizeof(struct SFXRContext));
    audio_sfxr_defaults(context);
    audio_sfxr_read(context, bin, len);
    audio_sfxr_reset(context, false);
    audio->context = context;
    audio->init = audio_sfxr_init;
    audio->seek = audio_sfxr_seek;
    audio->speed = audio_sfxr_speed;
    audio->free = audio_sfxr_free;
    audio->play = audio_sfxr_play;
    return audio;
}
