#include "tcp_socket.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

/**
 * Initializes a new `tcp_socket` structure.
 *
 * \param s Socket to initialize.
 * \return false in case of error.
 */
bool tcp_socket_create(tcp_socket_t *s) {
  s->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (s->fd == -1) {
    perror("socket");
    return false;
  }

  return true;
}

/**
 * Destroys and releases all resources of a `tcp_socket`.
 *
 * \param s An initialized socket instance.
 */
void tcp_socket_destroy(tcp_socket_t *s) {
  if (shutdown(s->fd, SHUT_RDWR) == -1) {
    perror("shutdown");
  }
  if (close(s->fd) == -1) {
    perror("close");
  }
}

/**
 * Sends data through the socket.
 *
 * \param s An initialized socket instance.
 * \param data Buffer containing the data to send.
 * \param size Size of the buffer `data`.
 * \return false In case of error.
 */
bool tcp_socket_write(tcp_socket_t *s, const void *data, size_t size) {
  /* casts the data buffer to a byte array */
  const unsigned char *out = data;
  ssize_t total_bytes_written = 0;
  do {
    ssize_t bytes_written = send(s->fd, out + total_bytes_written,
                                 size - total_bytes_written, MSG_NOSIGNAL);
    if (bytes_written < 0) {
      perror("write");
      break;
    }

    total_bytes_written += bytes_written;
  } while (total_bytes_written < size);

  return (total_bytes_written == size);
}

/**
 * Reads a chunk of data from the socket.
 *
 * \param s An initialized socket instance.
 * \param data Buffer where the read data is placed.
 * \param size Size of the `data` buffer.
 * \return Number of bytes written into the output buffer.
 *
 * \note This function reads the required amount of bytes, except for the last
 * chunk.
 */
ssize_t tcp_socket_read(tcp_socket_t *s, void *data, size_t size) {
  ssize_t bytes_read = recv(s->fd, data, size, MSG_WAITALL);
  if (bytes_read < 0) {
    perror("write");
  }

  return bytes_read;
}

/**
 * Binds a socket to a port.
 *
 * \param s An initialized socket instance.
 * \param port Port to bind the socket.
 * \return false in case of error.
 */
bool tcp_socket_bind(tcp_socket_t *s, int port) {
  struct sockaddr_in addr = {
      .sin_port = htons(port),
      .sin_family = AF_INET,
      .sin_addr = {.s_addr = htonl(INADDR_ANY)},
  };

  /* binds the socket */
  if (bind(s->fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind");
    return false;
  }

  return true;
}

/**
 * Makes a socket passive so it can accept connection.
 *
 * \param s An initialized and binded socket.
 * \return false in case of error.
 */
bool tcp_socket_listen(tcp_socket_t *s) {
  if (listen(s->fd, 0) == -1) {
    perror("listen");
    return false;
  }

  return true;
}

/**
 * Waits for a client to connect to the socket.
 *
 * \param s An initialized, binded and listening socket.
 * \param client An uninitialized socket that will represent the connection to
 *               the client once established (output).
 * \return false in case of error.
 *
 * \note The client socket should be destroyed once no longer used.
 */
bool tcp_socket_accept(tcp_socket_t *s, tcp_socket_t *client) {
  struct sockaddr_in cli_addr = {0};
  size_t addr_size = sizeof(struct sockaddr_in);

  int client_fd =
      accept(s->fd, (struct sockaddr *)&cli_addr, (socklen_t *)&addr_size);
  if (client_fd == -1) {
    perror("accept");
    return false;
  }

  /* initializes the client socket */
  client->fd = client_fd;
  return true;
}

/**
 * Connects a socket to an address and port.
 *
 * \param s An initialized socket.
 * \param address Address to connect to.
 * \param port Port to connect to.
 * \return false in case of error.
 */
bool tcp_socket_connect(tcp_socket_t *s, const char *address, int port) {
  struct sockaddr_in server_address = {
      .sin_port = htons(port),
      .sin_family = AF_INET,
  };

  if (inet_pton(AF_INET, address, &server_address.sin_addr) <= 0) {
    perror("inet_pton");
    return false;
  }

  if (connect(s->fd, (struct sockaddr *)&server_address,
              sizeof(struct sockaddr_in)) == -1) {
    perror("connect");
    return false;
  }

  return true;
}
