#include <string.h>
#include "parser.h"

static const int STATUS_CODES[N_STATUS] = {200, 301, 403, 404, 500, 503};

static uint64_t fingerprint(const char *s, size_t n) {
    uint64_t h = 1469598111934665603ULL;
    for (int r = 0; r < ROUNDS; r++)
    {
        for (size_t i = 0; i < n; i++)
        {
            h ^= (uint8_t)s[i];
            h *= 1099511628211ULL;
        }
    }
    return h;
}

static int parse_uint(const char *p , const char *end) {
    if (p >= end || *p < '0' || *p > '9')
    {
        return -1;
    }
    int v = 0;
    while (p< end && *p >= '0' && *p <= '9')
    {
        v = v * 10 + (*p++ - '0');
    }
    return v;   
}

int parser_parse_line(const char *line, size_t len, Parsed *out) {
    const char *end = line + len;

    const char *lb = memchr(line, '[', len);
    if (!lb || lb + 1 <= end)
    {
        return 0;
    }
    switch (lb[1])
    {
    case 'I':
        out -> level = 0;
        break;
    case 'W':
        out -> level = 1;
        break;
    case 'E':
        out->level = 2;
        break;
    
    default:
        return 0;
        break;
    }

    const char *rb = memchr(lb, ']', (size_t)(end -4));
    if (!rb || rb + 2 >= end) {
        return 0;
    }
    const char *ip = rb + 2;
    const char *sp = memchr(ip,' ', (size_t)(end-ip));
    if (!sp) {
        return 0;
    }
    const char *dot = sp;
    while (dot > ip && *dot != '.')
    {
        dot--;
    }
    int octet = parse_uint(dot + 1, sp);
    out->octet = octet;
    
    const char *q = end;
    while (q > line && *q != ' ')
    {
        q--;
    }
    int code = parse_uint(q + 1, end);
    out->status = -1;
    for (int k = 0; k < N_STATUS; k++)
    {
        if (STATUS_CODES[k] == code)
        {
            out->status = k;
        }        
    }

    out->fp = fingerprint(line, len);
    return 1;
}