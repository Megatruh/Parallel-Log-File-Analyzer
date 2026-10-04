#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "thread_runner.h"
#include "analyzer.h"
#include "chunker.h"


typedef struct {
    const char *buf;
    size_t      start;
    size_t      end;
    Result      res;
} Task;

static void *worker(void *arg) {
    Task *t = arg;
    analyzer_process_range(t->buf, t->start, t->end, &t->res); 
    return NULL;
}

void thread_run(const char *buf, size_t size, int workers, Result *out) {
    Task *tasks = aligned_alloc(64, (size_t)workers * sizeof(Task));
    pthread_t *th = malloc((size_t)workers * sizeof *th);
    if (!tasks || !th) { fprintf(stderr, "alokasi memori gagal\n"); exit(1); }

    for (int i = 0; i < workers; i++) {
        tasks[i].buf   = buf;
        tasks[i].start = chunker_boundary(buf, size, i, workers);
        tasks[i].end   = chunker_boundary(buf, size, i + 1, workers);
        analyzer_init(&tasks[i].res);
        if (pthread_create(&th[i], NULL, worker, &tasks[i]) != 0) {
            fprintf(stderr, "pthread_create gagal (thread %d)\n", i);
            exit(1);
        }
    }

    for (int i = 0; i < workers; i++) {
        pthread_join(th[i], NULL);
        analyzer_merge(out, &tasks[i].res);
    }

    free(th);
    free(tasks);
}