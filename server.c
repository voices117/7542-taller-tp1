#include "server.h"

/**
 * Initializes a server instance.
 *
 * \param s Server structure to initialize.
 * \param port Port where the server listens.
 * \return false in case of error.
 */
bool server_create(server_t *s, int port) {
  if (!tcp_socket_create(&s->sock)) {
    return false;
  }

  if (!tcp_socket_bind(&s->sock, port)) {
    tcp_socket_destroy(&s->sock);
    return false;
  }

  if (!tcp_socket_listen(&s->sock)) {
    tcp_socket_destroy(&s->sock);
    return false;
  }

  return true;
}

/**
 * Destroys a server instance. It will no longer accept connections.
 *
 * \param s An initialized server instance.
 */
void server_destroy(server_t *s) { tcp_socket_destroy(&s->sock); }

/**
 * Waits for a client and handles the connection.
 *
 * \param s An initialized server instance.
 * \param handler A callback to handle the new connection.
 * \param handler_ctx An user defined context passed to the `handler`.
 * \return false In case of error.
 */
bool server_handle_client(server_t *s, server_handler_t handler,
                          const void *handler_ctx) {
  tcp_socket_t client;
  if (!tcp_socket_accept(&s->sock, &client)) {
    return false;
  }

  bool result = handler(&client, handler_ctx);
  tcp_socket_destroy(&client);
  return result;
}
