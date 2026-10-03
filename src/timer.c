#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "timer.h"

double timer_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

double timer_elapsed(double start) {
    return timer_now() - start;
}

/*
cek hasil kodingan : printf '#include <stdio.h>\n#include <unistd.h>\n#include "timer.h"\nint main(void){double t=timer_now();sleep(1);printf("%%.3f\\n",timer_elapsed(t));return 0;}\n' | gcc -Iinclude -Wall -Wextra -x c - src/timer.c -o /tmp/t && /tmp/t
*/