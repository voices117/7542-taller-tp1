#ifndef __SERVER__
#define __SERVER__

#include <stdbool.h>
#include "tcp_socket.h"

/**
 * Callback type to handle client connections.
 *
 * \param client The client's socket.
 * \param handler_ctx User defined callback context recevied from
 * `server_handle_client`. \return false in case of error.
 */
typedef bool (*server_handler_t)(tcp_socket_t *client, const void *handler_ctx);

/** Server type */
typedef struct {
  tcp_socket_t sock;
} server_t;

/** API */
bool server_create(server_t *s, int port);
void server_destroy(server_t *s);

bool server_handle_client(server_t *s, server_handler_t handler,
                          const void *handler_ctx);

#endif
