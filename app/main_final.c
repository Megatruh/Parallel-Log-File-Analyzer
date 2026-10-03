#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "timer.h"
#include "loader.h"
#include "analyzer.h"
#include "report.h"
#include "seq_runner.h"

typedef void (*runner_fn)(const char *buf, size_t size, int workers, Result *out);

static void usage(const char *prog) {
    fprintf(stderr, "Pemakaian : %s <file.log> <seq|thread|proc> <jumlah_worker> [-q]\n", prog);
}

int main(int argc, char **argv) {
    if (argc < 4 ) {
        usage(argv[0]);
        return 1;
    }

    const char *path = argv[1];
    const char *mode = argv[2];
    int workers = atoi(argv[3]);
    int quiet = (argc > 4 && strcmp(argv[4], "-q")==0);

    if(workers < 1 || workers > MAX_WORKER) {
        fprintf(stderr, "Jumah_worker harus 1..%d\n", MAX_WORKER);
        return 1;
    }
    
    runner_fn run = NULL;
    if(strcmp(mode, "seq") == 0) {
        run = seq_run;
        workers = 1;
    }

    if(!run) {
        fprintf(stderr, "mode tidka dikenal : %s\n", mode);
        usage(argv[0]);
        return 1;
    }

    if(!quiet) {
        report_banner("MODE FINAL");
    }

    double t0= timer_now();
    size_t size = 0;
    char *buf = loader_read_file(path, &size);
    if(!buf) return 1;
    double t_read = timer_elapsed(t0);
    
    Result res;
    analyzer_init(&res);
    t0 = timer_now();
    // fprintf(stderr, "DEBUG size=%zu buf=%p\n", size, (void *)buf);
    run(buf, size, workers, &res);
    double t_proc = timer_elapsed(t0);

    if(quiet) {
        report_csv(mode, workers, &res, t_read, t_proc);
    } else {
        report_result(&res);
        report_timing(mode, workers, res.lines, t_read, t_proc, 0);
    }

    free(buf);
    return 0;
}