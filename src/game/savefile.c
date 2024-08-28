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

struct SaveFile savefiles[NUM_SAVEFILES];
struct SaveFile* savefile = NULL;

enum FileSelectContext {
    Context_Select,
    Context_Erase,
    Context_CopyWhat,
    Context_CopyTo
};
enum FileSelectContext curr_context;
int copy_what = -1;

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

bool savefile_map_event(int index) {
    return (savefile->map_events[index / 8] >> (index % 8)) & 1;
}

void savefile_map_event_set(int index) {
    savefile->map_events[index / 8] |=  (1 << (index % 8));
}

void savefile_map_event_clear(int index) {
    savefile->map_events[index / 8] &= ~(1 << (index % 8));
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