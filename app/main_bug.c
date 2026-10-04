#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "config.h"
#include "timer.h"
#include "loader.h"
#include "chunker.h"
#include "analyzer.h"
#include "report.h"

/* Data yang dipakai bersama oleh semua thread. */
static const char *g_buf;
static size_t      g_size;
static int         g_threads;
static Result      g_shared;          /* BUG: satu Result untuk SEMUA thread, tanpa kunci */

static void *worker(void *arg) {
    int id = (int)(intptr_t)arg;
    size_t start = chunker_boundary(g_buf, g_size, id, g_threads);
    size_t end   = chunker_boundary(g_buf, g_size, id + 1, g_threads);

    /* BUG: semua thread menaikkan penghitung di g_shared yang sama.
     * lines++ / level[i]++ bukan operasi atomik (baca -> tambah -> tulis),
     * sehingga pembaruan thread lain bisa tertimpa dan hilang. */
    analyzer_process_range(g_buf, start, end, &g_shared);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Pemakaian: %s <file.log> <jumlah_thread> [jumlah_percobaan]\n", argv[0]);
        return 1;
    }
    g_threads = atoi(argv[2]);
    int trials = (argc > 3) ? atoi(argv[3]) : 5;
    if (g_threads < 1 || g_threads > MAX_WORKER || trials < 1) {
        fprintf(stderr, "thread harus 1..%d dan percobaan >= 1\n", MAX_WORKER);
        return 1;
    }

    report_banner("MODE BUG: RACE CONDITION");

    size_t size = 0;
    char *buf = loader_read_file(argv[1], &size);
    if (!buf) return 1;
    g_buf = buf;
    g_size = size;

    Result ref;                                   /* kebenaran: hasil sekuensial */
    analyzer_init(&ref);
    analyzer_process_range(buf, 0, size, &ref);
    printf("Jumlah thread   : %d\n", g_threads);
    printf("Baris seharusnya: %ld\n", ref.lines);
    printf("------------------------------------------------------------\n");

    pthread_t th[MAX_WORKER];
    int salah = 0;
    for (int t = 1; t <= trials; t++) {
        analyzer_init(&g_shared);

        double t0 = timer_now();
        for (int i = 0; i < g_threads; i++)
            pthread_create(&th[i], NULL, worker, (void *)(intptr_t)i);
        for (int i = 0; i < g_threads; i++)
            pthread_join(th[i], NULL);
        double dt = timer_elapsed(t0);

        int sama = analyzer_equal(&g_shared, &ref);
        if (!sama) salah++;
        printf("Percobaan %d: %ld baris (hilang %ld), checksum %s, %.3f detik -> %s\n",
               t, g_shared.lines, ref.lines - g_shared.lines,
               (g_shared.fp == ref.fp) ? "sama" : "BEDA", dt,
               sama ? "benar" : "SALAH");
    }

    printf("------------------------------------------------------------\n");
    printf("Hasil salah: %d dari %d percobaan\n", salah, trials);
    puts(salah ? "KESIMPULAN: RACE CONDITION TERDETEKSI (hasil tidak konsisten)"
               : "KESIMPULAN: race belum muncul; naikkan thread/data atau turunkan ROUNDS");

    free(buf);
    return 0;
}