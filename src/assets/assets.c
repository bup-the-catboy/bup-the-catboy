#include <GL/gl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <foreach.h>
#include <sys/stat.h>

#ifdef LINUX
#include <linux/limits.h>
#endif

#include "assets.h"
#include "audio/audio.h"
#include "audio/nsf.h"
#include "audio/wav.h"
#include "audio/sfxr.h"
#include "binary_reader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#ifdef WINDOWS
#define BINARY "b"
#else
#define BINARY
#endif

struct Asset {
    char name[PATH_MAX];
    void* data;
    struct Asset* next;
};

struct Asset* asset_list;
bool force_binary = false;

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
void load_assets() {
    asset_list = malloc(sizeof(struct Asset));
    asset_list->name[0] = 0;
    asset_list->data = NULL;
    asset_list->next = NULL;
    struct Asset* curr = asset_list;
    char buf[PATH_MAX];
    struct BinaryStream* stream = binary_stream_create(asset_data);
    while (true) {
        memset(buf, 0, PATH_MAX);
        struct Asset* asset = malloc(sizeof(struct Asset));
        binary_stream_read_string(stream, buf, PATH_MAX);
        if (!buf[0]) break;
        int datasize;
        BINARY_STREAM_READ(stream, datasize);
        void* data = malloc(datasize);
        binary_stream_read(stream, data, datasize);
        memcpy(asset->name, buf, PATH_MAX);
        char ext[PATH_MAX];
        curr->next = asset;
        curr = asset;
        get_extension(ext, buf);
        bool binary_fallback = false;
        if (!force_binary) {
            _ EXT(png) {
                struct Texture* texture = malloc(sizeof(struct Texture));
                unsigned char* image = stbi_load_from_memory(data, datasize, &texture->width, &texture->height, NULL, STBI_rgb_alpha);
                glGenTextures(1, &texture->gl_texture);
                glBindTexture(GL_TEXTURE_2D, texture->gl_texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
                glBindTexture(GL_TEXTURE_2D, 0);
                stbi_image_free(image);
                free(data);
                asset->data = texture;
            }
            EXT(wav) {
                asset->data = audio_load_wav(data, datasize);
            }
            EXT(nsf) {
                asset->data = audio_load_nsf(data, datasize);
            }
            EXT(sfxr) {
                asset->data = audio_load_sfxr(data, datasize);
            }
            else binary_fallback = true;
        }
        else binary_fallback = true;
        if (binary_fallback) {
            struct Binary* bin = malloc(sizeof(struct Binary));
            bin->length = datasize;
            bin->ptr = data;
            asset->data = bin;
        }
    }
    binary_stream_close(stream);
}

void* get_asset(const char* name) {
    struct Asset* curr = asset_list->next;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr->data;
        curr = curr->next;
    }
    printf("asset %s not found\n", name);
    return NULL;
}

const char* get_asset_name(void* asset) {
    struct Asset* curr = asset_list->next;
    while (curr) {
        if (curr->data == asset) return curr->name;
        curr = curr->next;
    }
    printf("asset %p not found\n", asset);
    return NULL;
}

#ifdef WINDOWS
#define mkdir(name, mode) mkdir(name)
#endif

void mkdir_recursive(const char* path, mode_t mode) {
    char* dup = strdup(path);
    for (int i = 0; i < strlen(dup); i++) {
        if (dup[i] == '/') {
            dup[i] = 0;
            mkdir(dup, mode);
            dup[i] = '/';
        }
    }
    mkdir(dup, mode);
    free(dup);
}

void extract_assets() {
    force_binary = true;
    load_assets();
    struct Asset* curr = asset_list->next;
    while (curr) {
        printf("extracting %s\n", curr->name);
        char path[PATH_MAX + 10];
        snprintf(path, PATH_MAX + 10, "assets/%s", curr->name);
        char* folder = strdup(path);
        for (int i = strlen(folder) - 1; i >= 0; i--) {
            if (folder[i] == '/') {
                folder[i] = 0;
                break;
            }
        }
        mkdir_recursive(folder, 0777);
        free(folder);
        struct Binary* data = (struct Binary*)curr->data;
        FILE* f = fopen(path, "w" BINARY);
        fwrite(data->ptr, data->length, 1, f);
        fclose(f);
        curr = curr->next;
    }
}
