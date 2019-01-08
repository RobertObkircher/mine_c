#include "files.h"
#include "allocorexit.h"

#include <log.h>

char *read_file_contents(char *path) {
    FILE *file = fopen(path, "r");
    if (!file || fseek(file, 0, SEEK_END)) {
        log_error("Couldn't read file %s", path);
        return NULL;
    }

    size_t length = (size_t) ftell(file);
    rewind(file);

    char *buffer = malloc_or_exit(length + 1);
    size_t read = fread(buffer, 1, length, file);
    buffer[read] = 0;

    if(read == 0) {
        log_warn("file was empty: %s", path);
    }

    fclose(file);

    return buffer;
}
