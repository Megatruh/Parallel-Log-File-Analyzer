#include <string.h>
#include "analyzer.h"
#include "parser.h"

void analyzer_init(Result *r) {
    memset(r, 0, sizeof *r);
}

void analyzer_add(Result *r, const Parsed *p) {
    r->lines++;
    r->level[p->level]++;
    if (p->status >= 0)
    {
        r->status[p->status]++;
    }
    r->ip[p->octet]++;
    r->fp ^= p->fp;
}

void analyzer_process_range(const char *buf, size_t start, size_t end, Result *r) {
    size_t pos = start;
    while (pos < end)
    {
        const char *nl = memchr(buf + pos, '\n', end - pos );
        size_t len = nl ? (size_t)(nl - (buf + pos)) : end - pos;

        Parsed p;
        if (len > 0 && parser_parse_line(buf + pos, len, &p ))
        {
            analyzer_add(r, &p);
        }
        pos += len + 1;
    }
}

void analyzer_merge(Result *dst, const Result *src) {
    dst->lines += src->lines;
    for (int i = 0; i< N_LEVEL;i++) dst->level[i] += src->level[i];
    for (int i = 0; i< N_STATUS;i++) dst->status[i] += src->status[i];
    for (int i = 0; i< N_IP_BUCKET;i++) dst->ip[i] += src->ip[i];
    dst->fp ^= src->fp;
}

int analyzer_equal(const Result *a, const Result *b) {
    return a->lines == b->lines && a->fp == b->fp &&
        memcmp(a->level, b->level, sizeof a->level) == 0 &&
        memcmp(a->status, b->status, sizeof a->status) == 0 &&
        memcmp(a->ip, b->ip, sizeof a->ip) == 0;
}
