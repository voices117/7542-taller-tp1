#ifndef __ARC4_H__
#define __ARC4_H__

#include <stdbool.h>
#include <unistd.h>
#include <stddef.h>

/** Constants */

/** ARC4 state array size. */
#define ARC4_STATE_SIZE 256

/** ARC4 internal buffer size. */
#define ARC4_INTERNAL_BUFFER_SIZE 1024

/** If not 0, prints the key-stream through STDERR */
#define SHOW_DEBUG 1

/** Types */

/**
 * Callback that handles the output of the ARC4 stream.
 *
 * \param ctx User defined context.
 * \param data Buffer of data.
 * \param size Bytes in data buffer.
 * \return false in case of an error.
 */
typedef bool (*arc4_out_cb_t)(void *ctx, const void *data, size_t size);

/**
 * Callback that the ARC4 type uses to get it's input.
 *
 * \param ctx User defined context.
 * \param data Buffer of data.
 * \param size Maximum number of bytes to write into the data buffer.
 * \return A positive number is the bytes written into the buffer.
 *         A negative number means that no data could be read because of an
 *         error.
 *         0 if there's no more data to read.
 */
typedef ssize_t (*arc4_in_cb_t)(void *ctx, void *data, size_t size);

/** ARC4 type. */
typedef struct {
  /** User defined callback to handle output. */
  arc4_out_cb_t out_cb;

  /** User defined callback context passed to `out_cb`. */
  void *out_cb_ctx;

  /** User defined callback to get input. */
  arc4_in_cb_t in_cb;

  /** User defined callback context passed to `in_cb`. */
  void *in_cb_ctx;

  /** The ARC4 state array. */
  unsigned char state[ARC4_STATE_SIZE];

  /** Variables required by PRGA. */
  size_t i, j;

} arc4_t;

/** API */
bool arc4_create(arc4_t *arc4, const char *key, arc4_in_cb_t in_cb,
                 void *in_cb_ctx, arc4_out_cb_t out_cb, void *out_cb_ctx);
bool arc4_start(arc4_t *arc4);
void arc4_destroy(arc4_t *arc4);

#endif
