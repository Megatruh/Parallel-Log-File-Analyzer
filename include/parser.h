#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include "common.h"

int parser_parse_line(const char *line, size_t len, Parsed *out);

#endif