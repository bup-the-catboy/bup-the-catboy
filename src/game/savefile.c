#include "savefile.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "game/overlay/menu.h"

#ifdef WINDOWS
#define BINARY "b"
#else
#define BINARY
#endif

#define SAVEFILE_FILENAME "btcb.sav"

struct {
    uint8_t selected_savefile;
    struct SaveFile savefiles[NUM_SAVEFILES];
} savedata;
struct SaveFile* savefile = NULL;

enum {
    Context_Select,
    Context_Erase,
    Context_CopyWhat,
    Context_CopyTo
} curr_context;
int copy_what = -1;

void savefile_load() {
    FILE* f = fopen(SAVEFILE_FILENAME, "r" BINARY);
    if (!f) {
        savedata.selected_savefile = 0;
        for (int i = 0; i < NUM_SAVEFILES; i++) {
            savefile_erase(i);
        }
        savefile_save();
    }
    else {
        fread(&savedata, sizeof(savedata), 1, f);
        fclose(f);
    }
    savefile_select(savedata.selected_savefile);
}

void savefile_select(int file) {
    savedata.selected_savefile = file;
    savefile = &savedata.savefiles[file];
}

void savefile_erase(int file) {
    memset(&savedata.savefiles[file], 0, sizeof(struct SaveFile));
}

void savefile_copy(int from, int to) {
    memcpy(&savedata.savefiles[to], &savedata.savefiles[from], sizeof(struct SaveFile));
}

void savefile_save() {
    FILE* f = fopen(SAVEFILE_FILENAME, "w" BINARY);
    fwrite(&savedata, sizeof(savedata), 1, f);
    fclose(f);
}

struct SaveFile* savefile_get(int file) {
    return &savedata.savefiles[file];
}

void menubtn_file_select(int selected_index) {
    int file = selected_index - 1;
    switch (curr_context) {
        case Context_Select:
            savefile_select(file);
            pop_menu();
            break;
        case Context_CopyWhat:
            curr_context = Context_CopyTo;
            copy_what = file;
            push_menu(file_to);
            break;
        case Context_CopyTo:
            savefile_copy(copy_what, file);
            pop_menu_multi(2);
            break;
        case Context_Erase:
            savefile_erase(file);
            pop_menu();
            break;
    }
}

void menubtn_file_copy(int selected_index) {
    curr_context = Context_CopyWhat;
    push_menu(file_what);
}

void menubtn_file_erase(int selected_index) {
    curr_context = Context_Erase;
    push_menu(file_what);
}

void menubtn_file_cancel(int selected_index) {
    if (curr_context == Context_CopyTo) pop_menu_multi(2);
    else pop_menu();
    curr_context = Context_Select;
}