#ifndef PROC_RUNNER_H
#define PROC_RUNNER_H

#include <stddef.h>
#include "common.h"

void proc_run(const char *buf, size_t size, int workers, Result *out);

#endif 
