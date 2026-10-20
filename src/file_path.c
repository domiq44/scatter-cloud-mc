#include "file_path.h"
#include <string.h>

static void copy_truncated(const char *source, size_t source_size, char *out, size_t out_size) {
    size_t copy_size = source_size < out_size - 1 ? source_size : out_size - 1;
    memcpy(out, source, copy_size);
    out[copy_size] = '\0';
}

void replace_extension(const char *filename, const char *old_ext, const char *new_ext, char *out,
                       size_t out_size) {
    if (!out || out_size == 0)
        return;

    size_t len = strlen(filename);
    size_t old_len = strlen(old_ext);
    size_t new_len = strlen(new_ext);

    if (len >= old_len && strcmp(filename + len - old_len, old_ext) == 0) {
        size_t base_len = len - old_len;
        if (base_len > out_size - 1 || new_len > out_size - 1 - base_len) {
            copy_truncated(filename, len, out, out_size);
            return;
        }
        for (size_t i = 0; i < base_len; i++)
            out[i] = filename[i];
        for (size_t i = 0; i <= new_len; i++)
            out[base_len + i] = new_ext[i];
    } else {
        if (len > out_size - 1 || new_len > out_size - 1 - len) {
            copy_truncated(filename, len, out, out_size);
            return;
        }
        for (size_t i = 0; i < len; i++)
            out[i] = filename[i];
        for (size_t i = 0; i <= new_len; i++)
            out[len + i] = new_ext[i];
    }
}
