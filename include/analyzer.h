#ifndef ANALYZER_H
#define ANALYZER_H

#include <stddef.h>
#include "common.h"

void analyzer_init(Result *r);

void analyzer_add(Result *r, const Parsed *p);

void analyzer_process_range(const char *buf, size_t start, size_t end, Result *r);

void analyzer_merge(Result *dst, const Result *src);

int analyzer_equal(const Result *a, const Result *b);

#endif