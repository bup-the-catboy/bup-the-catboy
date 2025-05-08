#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <endianness.h>

#ifdef WINDOWS
#define BINARY "b"
#else
#define BINARY
#endif

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

void write_file(FILE* f, const char* filename) {
    struct stat s;
    stat(filename, &s);
    if (S_ISDIR(s.st_mode)) {
        DIR* d = opendir(filename);
        struct dirent* entry;
        while ((entry = readdir(d))) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "%s/%s", filename, entry->d_name);
            write_file(f, path);
        }
        closedir(d);
    }
    else {
        FILE* file = fopen(filename, "r" BINARY);
        uint32_t filesize = s.st_size;
        uint32_t filesize_le = LE(filesize);
        const char* name = filename + 2;
        fwrite(name, strlen(name) + 1, 1, f);
        fwrite(&filesize_le, sizeof(uint32_t), 1, f);
        char* data = malloc(filesize);
        fread(data, filesize, 1, file);
        fclose(file);
        fwrite(data, filesize, 1, f);
        free(data);
    }
}

int main() {
    FILE* f = fopen("assets.bin", "w+" BINARY);
    chdir("assets");
    write_file(f, ".");
    uint8_t zero = 0;
    fwrite(&zero, 1, 1, f);
    fclose(f);
    return 0;
}
