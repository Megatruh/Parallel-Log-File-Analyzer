#ifndef CHUNKER_H
#define CHUNKER_H

#include <stddef.h>

size_t chunker_boundary(const char *buf, size_t size, int i, int n);

#endif