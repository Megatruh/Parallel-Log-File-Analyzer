#include <stdio.h>
#include "seq_runner.h"
#include "analyzer.h"

void seq_run(const char *buf, size_t size, int workers, Result *out) {
    (void)workers;
    analyzer_process_range(buf, 0, size, out);
}