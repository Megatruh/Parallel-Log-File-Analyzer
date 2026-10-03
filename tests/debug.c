#include <stdio.h>
#include <string.h>
#include "loader.h"
#include "parser.h"

int main(void) {
    size_t size = 0;
    char *buf = loader_read_file("/tmp/e.log", &size);
    printf("loader: buf=%p size=%zu\n", (void *)buf, size);

    const char *l = "2026-10-02 00:00:00 [INFO] 10.0.0.30 GET /index 301";
    Parsed p;
    int ok = parser_parse_line(l, strlen(l), &p);
    printf("parser: ok=%d level=%d status=%d octet=%d\n", ok, p.level, p.status, p.octet);
    return 0;
}