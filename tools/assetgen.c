#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

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
        FILE* file = fopen(filename, "r");
        uint32_t filesize = s.st_size;
        const char* name = filename + 2;
        fwrite(name, strlen(name) + 1, 1, f);
        fwrite(&filesize, sizeof(uint32_t), 1, f);
        char* data = malloc(filesize);
        fread(data, filesize, 1, file);
        fclose(file);
        fwrite(data, filesize, 1, f);
        free(data);
    }
}

int main() {
    FILE* f = tmpfile();
    chdir("assets");
    write_file(f, ".");
    uint8_t zero = 0;
    fwrite(&zero, 1, 1, f);
    chdir("..");
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    FILE* out = fopen("src/assets/asset_data.h", "w");
    for (size_t i = 0; i < size; i++) {
        uint8_t byte;
        fread(&byte, 1, 1, f);
        fprintf(out, "0x%02x,", byte);
    }
    fclose(f);
    fclose(out);
}
