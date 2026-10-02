#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "config.h"

typedef struct {uint64_t state;} Rng;

static void rng_seed(Rng * r, unit64_t seed) {
    r->state = seed ? seed : 88172645463325252ULL;
}

static unint32_t rng_next(Rng * r) {
    uint64_t x = r->state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    r->state = x;
    return (uint32_t)(x >> 32);
}

int main(int argc, char **argv) {
    /* 
    kita lanjut nanti di https://claude.ai/chat/63931a00-70d9-4b9a-a274-b5f58aa34fda
    */
}