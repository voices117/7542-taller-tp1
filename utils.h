#ifndef __UTILS__
#define __UTILS__

#include <stdio.h>
#include <stdbool.h>

/* functions */
bool utils_strtoint(int *output, const char *input);
bool utils_write_hex_buf(FILE *f, const unsigned char *data, size_t size,
                         bool uppercase);

#endif
