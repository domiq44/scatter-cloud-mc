#include "test_file_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int create_temporary_file(char *path, size_t path_size, const char *contents) {
    int path_length = snprintf(path, path_size, "/tmp/solar_duration_test_XXXXXX");
    if (path_length < 0 || (size_t)path_length >= path_size) {
        return 0;
    }

    int descriptor = mkstemp(path);
    if (descriptor < 0) {
        return 0;
    }

    FILE *file = fdopen(descriptor, "w");
    if (!file) {
        close(descriptor);
        unlink(path);
        return 0;
    }

    int success = fputs(contents, file) != EOF;
    if (fclose(file) != 0) {
        success = 0;
    }
    if (!success) {
        unlink(path);
    }
    return success;
}

void remove_temporary_file(const char *path) {
    if (path) {
        unlink(path);
    }
}