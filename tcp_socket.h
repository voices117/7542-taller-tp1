#ifndef __TCP_SOCKET__
#define __TCP_SOCKET__

#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

/** Socket type */
typedef struct {
  /** Socket file descriptor */
  int fd;
} tcp_socket_t;

/* API */
bool tcp_socket_create(tcp_socket_t *s);
void tcp_socket_destroy(tcp_socket_t *s);
bool tcp_socket_write(tcp_socket_t *s, const void *data, size_t size);
ssize_t tcp_socket_read(tcp_socket_t *s, void *data, size_t size);

/* server */
bool tcp_socket_bind(tcp_socket_t *s, int port);
bool tcp_socket_listen(tcp_socket_t *s);
bool tcp_socket_accept(tcp_socket_t *s, tcp_socket_t *client);

/* client */
bool tcp_socket_connect(tcp_socket_t *s, const char *address, int port);

#endif
