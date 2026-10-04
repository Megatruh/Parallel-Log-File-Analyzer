#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "timer.h"
#include "loader.h"
#include "analyzer.h"
#include "report.h"
#include "seq_runner.h"
#include "thread_runner.h"
#include "proc_runner.h"

typedef void (*runner_fn)(const char *buf, size_t size, int workers, Result *out);

static void usage(const char *prog) {
    fprintf(stderr, "Pemakaian: %s <file.log> <seq|thread|proc> <jumlah_worker> [-q] [-b]\n"
                    "  -q  mode senyap (satu baris CSV)\n"
                    "  -b  jalankan baseline sequential dulu: hitung speedup, efisiensi, dan verifikasi hasil\n",
            prog);
}

int main(int argc, char **argv) {
    if (argc < 4 ) {
        usage(argv[0]);
        return 1;
    }

    const char *path = argv[1];
    const char *mode = argv[2];
    int workers = atoi(argv[3]);
    int quiet = 0, baseline = 0;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-q") == 0)      quiet = 1;
        else if (strcmp(argv[i], "-b") == 0) baseline = 1;
        else { usage(argv[0]); return 1; }
    }

    if(workers < 1 || workers > MAX_WORKER) {
        fprintf(stderr, "Jumah_worker harus 1..%d\n", MAX_WORKER);
        return 1;
    }
    
    runner_fn run = NULL;
    if (strcmp(mode, "seq") == 0){ run = seq_run; workers = 1; }
    else if (strcmp(mode, "thread") == 0) run = thread_run;
    else if (strcmp(mode, "proc") == 0) run = proc_run;
    else { fprintf(stderr, "mode tidak dikenal: %s\n", mode); usage(argv[0]); return 1; }


    if(!quiet) {
        report_banner("MODE FINAL");
    }

    double t0= timer_now();
    size_t size = 0;
    char *buf = loader_read_file(path, &size);
    if(!buf) return 1;
    double t_read = timer_elapsed(t0);
    
    int pakai_baseline = baseline && run != seq_run;
    Result base;
    double t_base = 0;
    if (pakai_baseline) {
        analyzer_init(&base);
        t0 = timer_now();
        seq_run(buf, size, 1, &base);
        t_base = timer_elapsed(t0);
    }

    Result res;
    analyzer_init(&res);
    t0 = timer_now();
    run(buf, size, workers, &res);
    double t_proc = timer_elapsed(t0);

    if (quiet) {
        report_csv(mode, workers, & res, t_read, t_proc);
    } else {
        report_result(&res);
        if (pakai_baseline)
            printf("Verifikasi      : %s\n", analyzer_equal(&res, &base)
                   ? "SAMA dengan hasil sequential" : "BERBEDA dari sequential (ada bug!)");
        report_timing(mode, workers, res.lines, t_read, t_proc, t_base);
    }

    free(buf);
    return 0;
}