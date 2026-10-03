#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"

typedef struct {uint64_t state;} Rng;

static void rng_seed(Rng * r, uint64_t seed) {
    r->state = seed ? seed : 88172645463325252ULL;
}

static uint32_t rng_next(Rng * r) {
    uint64_t x = r->state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    r->state = x;
    return (uint32_t)(x >> 32);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Pemakaian : %s <output.log> [jumlah baris] [seed]\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    long n = (argc > 2) ? atol(argv[2]) : DATA_BASE;
    uint64_t seed = (argc > 3) ? strtoull(argv[3], NULL, 10) : SEED_NIM ;
    if (n <= 0) {
        fprintf(stderr, "Jumlah baris harus > 0\n");
        return 1;
    }

    FILE *f = fopen(path, "w");
    if (!f) {
        perror("fopen");
        return 1;
    }

    static const char *METHODS[] = {"GET", "POST", "PUT", "DELETE"};
    static const char *PATHS[] = {"/index", "/login", "/api/users", "/api/orders", "/chart", "/search", "/admin"};

    static const int CODE_INFO[] = {200, 301};
    static const int CODE_WARN[] = {403, 404, 301};
    static const int CODE_ERROR[] = {500, 503, 404};

    Rng rng;
    rng_seed(&rng, seed);

    for (long i = 0; i < n; i++) {
        uint32_t roll = rng_next(&rng) % 100;
        const char *level;
        int code;
        if (roll < 70) {
            level = "INFO";
            code = CODE_INFO[rng_next(&rng) % 2];
        } else if (roll < 90) {
            level = "WARNING";
            code = CODE_WARN[rng_next(&rng) % 3];
        } else {
            level = "ERROR";
            code = CODE_ERROR[rng_next(&rng) % 3];
        }

        int octet = (rng_next(&rng) % 100 < 40) ? 1 + rng_next(&rng) % 10 : 1 + rng_next(&rng) % 200;

        long sec = i % 86400;

        fprintf(f, "2026-10-02 %02ld:%02ld:%02ld [%s] 10.0.0.%d %s %s %d\n",
        sec / 3600, (sec / 60) % 60, sec % 60, level, octet,
        METHODS[rng_next(&rng) % 4], PATHS[rng_next(&rng) % 7], code);
    }

    fclose(f);
    printf("Selesai : %ld baris --> %s (seed %llu)\n", n, path, (unsigned long long)seed);

    return 0;
}