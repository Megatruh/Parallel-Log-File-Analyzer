#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include "config.h"

// Hasil analisisi satu worker (atau hasil akhir setelah merge)
typedef struct __attribute__((aligned(64))) {
    long lines;
    long level[N_LEVEL];
    long status[N_STATUS];
    long ip[N_IP_BUCKET];
    uint64_t fp;
} Result;

_Static_assert(sizeof(Result) % 64 == 0 , "Result harus kelipatan 64 byte");

// Hasil parsing SATU baris log
typedef struct {
    int level;
    int status;
    int octet;
    uint64_t fp;
} Parsed;

#endif
/*cara cek 
printf '#include <stdio.h>\n#include "common.h"\nint main(void){printf("%%zu\\n", sizeof(Result));return 0;}\n' | gcc -Iinclude -Wall -Wextra -x c - -o /tmp/t && /tmp/t
*/