#include <stdio.h>
#include <stdlib.h>
#include "loader.h"
#include "chunker.h"
#include "analyzer.h"

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "Pemakaian: %s <file.log>\n", argv[0]); return 1; }

    size_t size;
    char *buf = loader_read_file(argv[1], &size);
    if (!buf) return 1;

    Result ref;                                  /* referensi: proses sekali jalan */
    analyzer_init(&ref);
    analyzer_process_range(buf, 0, size, &ref);
    printf("referensi: %ld baris\n", ref.lines);

    int ns[] = {1, 2, 3, 4, 7, 8, 64, 1000};
    int semua_ok = 1;
    for (size_t k = 0; k < sizeof ns / sizeof ns[0]; k++) {
        int n = ns[k];
        Result total;
        analyzer_init(&total);
        for (int i = 0; i < n; i++) {
            Result part;
            analyzer_init(&part);
            analyzer_process_range(buf, chunker_boundary(buf, size, i, n),
                                        chunker_boundary(buf, size, i + 1, n), &part);
            analyzer_merge(&total, &part);
        }
        int sama = analyzer_equal(&total, &ref);
        semua_ok &= sama;
        printf("n=%-4d sama=%d\n", n, sama);
    }
    puts(semua_ok ? "SEMUA OK" : "ADA YANG BEDA");
    free(buf);
    return 0;
}