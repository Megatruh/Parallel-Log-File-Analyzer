#ifndef THREAD_RUNNER_H
#define THREAD_RUNNER_H

#include <stddef.h>
#include "common.h"

void thread_run(const char *buf, size_t size, int workers, Result *out);

#endif 