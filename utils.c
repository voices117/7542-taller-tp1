#include "utils.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/**
 * Converts a sequence of numeric digits into an integer value.
 *
 * \param output The integer read from the input string (in case of success).
 * \param input The string containing the value to read.
 * \return false if it couldn't read an integer from the string.
 */
bool utils_strtoint(int *output, const char *input) {
  errno = 0;
  long int value = strtol(input, NULL, 10);
  if (errno != 0 || value > INT_MAX) {
    return false;
  }

  *output = value;
  return true;
}

/**
 * Writes a buffer encoded in hexadecimal to a file.
 *
 * \param f Output file.
 * \param data Buffer with the data to write.
 * \param size Size of the input buffer.
 * \param uppercase If true, the hexadecimal characters are uppercase.
 * \return false in case the write fails.
 */
bool utils_write_hex_buf(FILE *f, const unsigned char *data, size_t size,
                         bool uppercase) {
  /* selects the appropriate fromat */
  const char *fmt = uppercase ? "%02X" : "%02x";

  /* writes each byte encoded */
  for (size_t i = 0; i < size; i++) {
    if (fprintf(f, fmt, data[i]) != 2) {
      return false;
    }
  }
  return true;
}
