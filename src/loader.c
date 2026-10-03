#include <stdio.h>
#include <stdlib.h>
#include "loader.h"

char *loader_read_file(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        perror(path);
        return NULL;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(f);
        return NULL;
    }
    long len = ftell(f);
    if (len < 0 ) {
        perror("ftel");
        fclose(f);
        return NULL;
    }
    rewind(f);

    char *buf = malloc((size_t)len + 1);
    if (!buf) {
        fprintf( stderr, "Malloc gagal (%ld byte)\n", len);
        fclose(f);
        return NULL;
    }

    size_t got = fread(buf, 1, (size_t)len, f);
    fclose(f);
    if (got != (size_t)len) {
        fprintf(stderr, "fread tidak lengkap\n");
        free(buf);
        return NULL;
    }

    buf[len] = '\0';
    *size = (size_t)len;
    return buf;
}
