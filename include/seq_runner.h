#ifndef SEQ_RUNNER_H
#define SEQ_RUNNER_H

#include <stddef.h>
#include "common.h"

void seq_run(const char *buf, size_t size, int workers, Result *out);

#endif