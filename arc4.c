#include "arc4.h"
#include <string.h>

#if SHOW_DEBUG
#include <stdio.h>
#endif

/**
 * Swaps 2 variables.
 *
 * \param a Parameter to swap.
 * \param b Parameter to swap.
 */
static void _swap(unsigned char *a, unsigned char *b) {
  unsigned char tmp = *a;
  *a = *b;
  *b = tmp;
}

/**
 * Encrypts/decrypts a chunk of given data (in place).
 *
 * \param arc4 ARC4 structure.
 * \param data Buffer with the data to convert. The output is this same buffer.
 * \param data_size Size of the input/output buffer.
 */
static void _apply_key_stream(arc4_t *arc4, unsigned char *data,
                              size_t data_size) {
  for (size_t i = 0; i < data_size; i++) {
    /* PRGA */
    arc4->i = (arc4->i + 1) % ARC4_STATE_SIZE;
    arc4->j = (arc4->j + arc4->state[arc4->i]) % ARC4_STATE_SIZE;
    _swap(arc4->state + arc4->i, arc4->state + arc4->j);

    size_t sum = arc4->state[arc4->i] + arc4->state[arc4->j];
    size_t k = arc4->state[sum % ARC4_STATE_SIZE];
    data[i] = k ^ data[i];

#if SHOW_DEBUG
    fprintf(stderr, "%02X", (unsigned int)k);
#endif
  }
}

/**
 * Initializes the ARC4 structure and the internal state of the algorithm.
 *
 * \param arc4 The structure to initialize.
 * \param key The ARC4 key.
 * \param in_cb Callback that will feed the ARC4 input data.
 * \param in_cb_ctx User defined context to pass to `in_cb` in each call.
 * \param out_cb Callback that will handle the output stream.
 * \param out_cb_ctx User defined context to pass to `out_cb` in each call.
 * \return false in case of an error.
 */
bool arc4_create(arc4_t *arc4, const char *key, arc4_in_cb_t in_cb,
                 void *in_cb_ctx, arc4_out_cb_t out_cb, void *out_cb_ctx) {
  /* IO callbacks are mandatory */
  if (out_cb == NULL || in_cb == NULL) {
    return false;
  }

  /* initializes the internal fields */
  arc4->out_cb = out_cb;
  arc4->out_cb_ctx = out_cb_ctx;
  arc4->in_cb = in_cb;
  arc4->in_cb_ctx = in_cb_ctx;
  arc4->i = 0;
  arc4->j = 0;

  /* initializes the internal state */
  for (size_t i = 0; i < ARC4_STATE_SIZE; i++) {
    arc4->state[i] = i;
  }

  size_t j = 0;
  size_t key_len = strlen(key);
  for (size_t i = 0; i < ARC4_STATE_SIZE; i++) {
    j = (j + arc4->state[i] + key[i % key_len]) % ARC4_STATE_SIZE;
    _swap(arc4->state + i, arc4->state + j);
  }

  return true;
}

/**
 * Encrypts/decrypts a stream of data read from the input callback and sends it
 * to the output callback.
 *
 * \param arc4 An initialized ARC4 structure.
 * \return false if the output callback failed to send the data.
 */
bool arc4_start(arc4_t *arc4) {
  unsigned char data[ARC4_INTERNAL_BUFFER_SIZE];
  ssize_t bytes_read =
      arc4->in_cb(arc4->in_cb_ctx, data, ARC4_INTERNAL_BUFFER_SIZE);

  while (bytes_read > 0) {
    /* encrypts/decrypts and sends the data */
    _apply_key_stream(arc4, data, bytes_read);
    if (!arc4->out_cb(arc4->out_cb_ctx, data, bytes_read)) {
      return false;
    }

    /* gets the next chunk of data */
    bytes_read = arc4->in_cb(arc4->in_cb_ctx, data, ARC4_INTERNAL_BUFFER_SIZE);
  }

  /* a negative value received from the callback means a read error */
  return (bytes_read == 0);
}

/**
 * Releases all allocated resources.
 *
 * \param arc4 An initialized ARC4 structure.
 */
void arc4_destroy(arc4_t *arc4) { /* nothing to do */
}
