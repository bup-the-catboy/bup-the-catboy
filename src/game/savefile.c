#include "savefile.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#ifdef WINDOWS
#define BINARY "b"
#else
#define BINARY
#endif

#define SAVEFILE_FILENAME "btcb.sav"

struct SaveFile savefiles[NUM_SAVEFILES];
struct SaveFile* savefile = NULL;

void savefile_load() {
    FILE* f = fopen(SAVEFILE_FILENAME, "r" BINARY);
    if (!f) {
        for (int i = 0; i < NUM_SAVEFILES; i++) {
            savefile_erase(i);
        }
        savefile_save();
        return;
    }
    fread(savefiles, sizeof(savefiles), 1, f);
    fclose(f);
}

void savefile_select(int file) {
    savefile = &savefiles[file];
}

void savefile_erase(int file) {
    memset(&savefiles[file], 0, sizeof(struct SaveFile));
    savefiles[file].lives = 3;
}

void savefile_copy(int from, int to) {
    memcpy(&savefiles[to], &savefiles[from], sizeof(struct SaveFile));
}

void savefile_save() {
    FILE* f = fopen(SAVEFILE_FILENAME, "w" BINARY);
    fwrite(savefiles, sizeof(savefiles), 1, f);
    fclose(f);
}

struct SaveFile* savefile_get(int file) {
    return &savefiles[file];
}
