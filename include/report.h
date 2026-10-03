#ifndef REPORT_H
#define REPORT_H

#include "common.h"

void report_banner(const char *subjudul);

void report_result(const Result *r);

void report_timing(const char *mode, int worker, long lines, double t_read, double t_proc, double t_baseline);

void report_csv(const char *mode, int worker, const Result *r, double t_read, double t_proc);


#endif