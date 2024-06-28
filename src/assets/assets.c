#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <foreach.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "assets.h"
#include "sound.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct Asset {
    char name[PATH_MAX];
    void* data;
    struct Asset* next;
};

struct Asset* asset_list;

unsigned char asset_data[] = {
#ifdef NO_VSCODE // so it doesnt destroy the intellisense
#include "asset_data.h"
#endif
};

void get_extension(char* out, const char* in) {
    size_t len = strlen(in);
    for (size_t i = len - 1; i >= 0; i--) {
        if (in[i] == '.' || in[i] == '/') {
            strcpy(out, in + i + 1);
            return;
        }
    }
    strcpy(out, in);
}

struct MemoryStream {
    void* data;
    size_t size;
    size_t ptr;
};

size_t memstream_read(void* out, size_t size, size_t n, void* in) {
    struct MemoryStream* memstream = in;
    size_t bytes = size * n;
    size_t remaining = memstream->size - memstream->ptr;
    if (remaining < bytes) bytes = remaining;
    memcpy(out, memstream->data + memstream->ptr, bytes);
    memstream->ptr += bytes;
    return bytes;
}

int memstream_seek(void* data, long offset, int pos) {
    struct MemoryStream* memstream = data;
    size_t newpos = pos;
    switch (pos) {
        case SEEK_SET:
            newpos = 0;
            break;
        case SEEK_CUR:
            newpos = memstream->ptr;
            break;
        case SEEK_END:
            newpos = memstream->size;
            break;
    }
    newpos += offset;
    if (newpos > memstream->size) return -1;
    memstream->ptr = newpos;
    return 0;
}

int memstream_close(void* data) {
    return 0;
}

long memstream_tell(void* data) {
    return ((struct MemoryStream*)data)->ptr;
}

bool starts_with(const char* a, const char* b) {
    for (int i = 0; i < strlen(b); i++) {
        if (a[i] != b[i]) return false;
    }
    return true;
}

#define MK_COND(comp) || strcmp(ext, #comp) == 0 
#define EXT(...) else if (false FOR_EACH(MK_COND, __VA_ARGS__))
#define _ if (false) (void)0;
void load_assets(SDL_Renderer* renderer) {
    asset_list = malloc(sizeof(struct Asset));
    asset_list->name[0] = 0;
    asset_list->data = NULL;
    asset_list->next = NULL;
    struct Asset* curr = asset_list;
    int ptr = 0;
    char buf[PATH_MAX];
    while (true) {
        memset(buf, 0, PATH_MAX);
        struct Asset* asset = malloc(sizeof(struct Asset));
        strncpy(buf, (char*)asset_data + ptr, PATH_MAX);
        if (!buf[0]) break;
        ptr += strlen(buf) + 1;
        int datasize = *(int*)(asset_data + ptr);
        ptr += 4;
        void* data = malloc(datasize);
        memcpy(data, asset_data + ptr, datasize);
        ptr += datasize;
        memcpy(asset->name, buf, PATH_MAX);
        char ext[PATH_MAX];
        curr->next = asset;
        curr = asset;
        get_extension(ext, buf);
        _ EXT(png) {
            int w, h;
            unsigned char* image = stbi_load_from_memory(data, datasize, &w, &h, NULL, STBI_rgb_alpha);
            SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(image, w, h, 32, 4 * w, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            asset->data = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
            stbi_image_free(image);
            free(data);
        }
        EXT(wav) {
            struct Audio* audio = malloc(sizeof(struct Audio));
            SDL_RWops* src = SDL_RWFromMem(data, datasize);
            audio->sound = Mix_LoadWAV_RW(src, true);
            audio->looping = false;
            asset->data = audio;
            free(data);
        }
        EXT(ogg) {
            struct Audio* audio = malloc(sizeof(struct Audio));
            SDL_RWops* src = SDL_RWFromMem(data, datasize);
            audio->music = Mix_LoadMUS_RW(src, true);
            OggVorbis_File f;
            struct MemoryStream stream = (struct MemoryStream){ .data = data, .size = datasize, .ptr = 0 };
            ov_open_callbacks(&stream, &f, NULL, 0, (ov_callbacks){
                memstream_read,
                memstream_seek,
                memstream_close,
                memstream_tell
            });
            vorbis_comment* comments = ov_comment(&f, -1);
            audio->looping = false;
            if (comments) {
                for (int i = 0; i < comments->comments; i++) {
                    if (starts_with(comments->user_comments[i], "_SMBR_LOOP=")) {
                        audio->looping = true;
                        audio->loop = atof(comments->user_comments[i] + strlen("_SMBR_LOOP="));
                    }
                }
            }
            ov_clear(&f);
            free(data);
            asset->data = audio;
        }
        else {
            struct Binary* bin = malloc(sizeof(struct Binary));
            bin->length = datasize;
            bin->ptr = data;
            asset->data = bin;
        }
    }
}

void* get_asset(const char* name) {
    struct Asset* curr = asset_list->next;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr->data;
        curr = curr->next;
    }
    return NULL;
}