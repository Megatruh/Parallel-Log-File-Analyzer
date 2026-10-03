#include "chunker.h"

size_t chunker_boundary(const char *buf, size_t size, int i, int n) {
    if (i <= 0) return 0;
    if (i >= n) return size;

    size_t p = size * (size_t)i / (size_t)n;
    if (p == 0) return 0;
    while (p < size && buf[p-1] != '\n')
    {
        p++;
    }
    return p;
}