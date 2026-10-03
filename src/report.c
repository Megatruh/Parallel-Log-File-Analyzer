#include <stdio.h>
#include <string.h>
#include "config.h"
#include "report.h"

static const int STATUS_CODES[N_STATUS] = {200, 301, 403, 404, 500, 503};
static const char *LEVEL_NAMES[N_LEVEL] = {"INFO", "WARNING", "ERROR"};

static void garis(char c) {
    for (int i = 0; i < 60; i++)
    {
        putchar(c);
    }
    putchar('\n');
}

void report_banner(const char *subjudul) {
    garis('=');
    printf("%s\n", JUDUL);
    if (subjudul && subjudul[0]) {
        printf("%s\n", subjudul);
    }
    printf("By %s (%s)\n", NAMA, NIM);
    garis('=');
}

void report_result(const Result *r) {
    printf("Total baris : %ld\n", r->lines);
    for (int i = 0; i < N_LEVEL; i++)
    {
        printf(" %-8s : %ld\n", LEVEL_NAMES[i], r->level[i]);
    }
    for (int i = 0; i < N_STATUS; i++)
    {
        printf(" status %d : %ld\n", STATUS_CODES[i], r->status[i]);
    }

    int used[N_IP_BUCKET];
    memset(used, 0, sizeof used);
    garis('-');
    printf("TOP 3 IP\t:\n");
    for (int k = 0; k < 3; k++)
    {
        int best = -1;
        for (int l = 0; l < N_IP_BUCKET; l++) {
            if (!used[l] && (best < 0 || r->ip[l] > r->ip[best])) {
                best = l;
            }
        }
        used[best] = 1;
        printf("10.0.0.%d (%ld)\n", best, r->ip[best]);
    }
    printf("\nChecksum\t: %016llx\n", (unsigned long long)r->fp);
}

void report_timing(const char *mode, int worker, long lines, double t_read, double t_proc, double t_baseline){
    garis('-');
    printf("Mode\t\t: %s\n", mode);

    if (strcmp(mode, "thread") == 0 )
    {
        printf("Jumlah thread\t: %d\n",worker);
    } else if (strcmp(mode, "proc") == 0)
    {
        printf("Jumlah process\t: %d\n",worker);
    } else {
        printf("Jumlah worker\t: %d\n",worker);
    }  

    printf("Waktu Baca file\t: %.3f detik\n", t_read);
    printf("Waktu Proses\t: %.3f detik\n", t_proc);
    printf("Waktu Total\t: %.3f detik\n", t_read + t_proc);
    if(t_proc > 0) {
        printf("Throughput\t: %.0f baris/detik\n", (double)lines / t_proc);
        // printf("Throughput\t: %.0f baris/detik\n");
    }
    if (t_baseline > 0 && t_proc > 0) {
        double speedup = t_baseline / t_proc;
        printf("Speedup\t: %.2fx\n", speedup);
        printf("Efisiensi\t: %.1f %%\n", 100.0 * speedup / worker);
    }
    garis('=');
}

void report_csv(const char *mode, int worker, const Result *r, double t_read, double t_proc) {
    printf("%s,%d,%ld,%.4f,%.4f,%016llx\n", mode, worker, r->lines, t_read, t_proc, (unsigned long long)r->fp);
}

