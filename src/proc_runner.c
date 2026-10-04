#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc_runner.h"
#include "analyzer.h"
#include "chunker.h"

static void write_all(int fd, const void *data, size_t n) {
    const char *p = data;
    while (n > 0) {
        ssize_t w = write(fd, p, n);
        if (w <= 0) _exit(1);
        p += w;
        n -= (size_t)w;
    }
}

static int read_all(int fd, void *data, size_t n) {
    char *p = data;
    size_t got = 0;
    while (got < n) {
        ssize_t r = read(fd, p + got, n - got);
        if (r <= 0) break;
        got += (size_t)r;
    }
    return got == n;
}

void proc_run(const char *buf, size_t size, int workers, Result *out) {
    int (*fd)[2] = malloc((size_t)workers * sizeof *fd);
    pid_t *pid = malloc((size_t)workers * sizeof *pid);
    if (!fd || !pid) { fprintf(stderr, "alokasi memori gagal\n"); exit(1); }

    fflush(stdout);                       

    for (int i = 0; i < workers; i++) {
        if (pipe(fd[i]) != 0) { perror("pipe"); exit(1); }
        pid[i] = fork();
        if (pid[i] < 0) { perror("fork"); exit(1); }

        if (pid[i] == 0) {                                   
            close(fd[i][0]);
            Result r;
            analyzer_init(&r);
            analyzer_process_range(
                buf, 
                chunker_boundary(buf, size, i, workers),
                chunker_boundary(buf, size, i + 1, workers), 
                &r);
            write_all(fd[i][1], &r, sizeof r);
            close(fd[i][1]);
            _exit(0);                                        
        }
        close(fd[i][1]);                                     
    }

    for (int i = 0; i < workers; i++) {                      
        Result r;
        if (!read_all(fd[i][0], &r, sizeof r)) {
            fprintf(stderr, "gagal membaca hasil dari proses %d\n", i);
            exit(1);
        }
        close(fd[i][0]);
        int status = 0;
        waitpid(pid[i], &status, 0);
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "proses anak %d berakhir tidak normal\n", i);
            exit(1);
        }
        analyzer_merge(out, &r);
    }

    free(fd);
    free(pid);
}