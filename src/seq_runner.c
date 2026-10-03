#include <stdio.h>
#include "seq_runner.h"
#include "analyzer.h"

void seq_run(const char *buf, size_t size, int workers, Result *out) {
    // fprintf(stderr, "DEBUG seq_run size=%zu workers=%d\n", size, workers);
    (void)workers;
    analyzer_process_range(buf, 0, size, out);
    // fprintf(stderr, "DEBUG seq_run selesai, out->lines=%ld\n", out->lines);
}