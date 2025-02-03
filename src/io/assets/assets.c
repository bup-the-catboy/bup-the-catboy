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
#include "io/io.h"
#include "io/audio/nsf.h"
#include "io/audio/wav.h"
#include "binary_reader.h"

#ifdef WINDOWS
#define BINARY "b"
#else
#define BINARY
#endif

#ifndef PATH_MAX
#define PATH_MAX 256
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
        struct Asset* asset = calloc(sizeof(struct Asset), 1);
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
                struct GfxResource* texture = graphics_load_texture(data, datasize);
                free(data);
                asset->data = texture;
            }
            EXT(wav) {
                asset->data = audio_load_wav(data, datasize);
            }
            EXT(nsf) {
                asset->data = audio_load_nsf(data, datasize);
            }
            EXT(glsl) {
                char* shader = malloc(datasize + 1);
                memcpy(shader, data, datasize);
                shader[datasize] = 0;
                asset->data = graphics_load_shader(shader);
                free(shader);
                free(data);
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
