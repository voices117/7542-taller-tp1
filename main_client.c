#include "main_client.h"
#include <stdio.h>
#include <stdlib.h>
#include "arc4.h"
#include "tcp_socket.h"
#include "utils.h"

/** The size of the chunks read/written by the client. */
#define CLIENT_CHUNK_SIZE 64

/**
 * Read callback for ARC4 that gets data from an input file.
 *
 * \param cb_ctx Callback context (input file).
 * \param data Output buffer for ARC4.
 * \param size Size of the output buffer.
 * \return ssize_t Bytes written into the output buffer.
 */
static ssize_t _client_arc4_read_cb(void *cb_ctx, void *data, size_t size) {
  FILE *f = cb_ctx;
  size_t bytes_read = fread(data, 1, CLIENT_CHUNK_SIZE, f);

  /* checks for errors */
  if (bytes_read != size && ferror(f)) {
    return -1;
  }

  return bytes_read;
}

/**
 * Write callback for ARC4 that sends data through a socket.
 *
 * \param cb_ctx Callback context (socket).
 * \param data Encoded ARC4 data.
 * \param size Size of the buffer.
 * \return false if failed to send the data.
 */
static bool _client_arc4_write_cb(void *cb_ctx, const void *data, size_t size) {
  tcp_socket_t *s = cb_ctx;

  /* prints the encoded data to STDOUT */
  const bool uppercase = false;
  if (!utils_write_hex_buf(stdout, data, size, uppercase)) {
    return false;
  }

  /* sends the encoded data to the server socket */
  return tcp_socket_write(s, data, size);
}

/**
 * ARGV: client <IP> <PORT> <Key> <input file>
 *
 * \param argc
 * \param argv
 * \return int Exit code.
 */
int main_client(int argc, const char **argv) {
  if (argc != 6) {
    return EXIT_FAILURE;
  }

  /* reads the port */
  int port;
  if (!utils_strtoint(&port, argv[3])) {
    return EXIT_FAILURE;
  }

  /* opens the input file */
  FILE *input = fopen(argv[5], "r");
  if (input == NULL) {
    return EXIT_FAILURE;
  }

  const char *server_address = argv[2];

  tcp_socket_t socket;
  if (!tcp_socket_create(&socket)) {
    fclose(input);
    return EXIT_FAILURE;
  }

  if (!tcp_socket_connect(&socket, server_address, port)) {
    fclose(input);
    tcp_socket_destroy(&socket);
    fprintf(stderr, "No se pudo conectar al servidor\n");
    return EXIT_FAILURE;
  }

  const char *key = argv[4];

  int exit_code = EXIT_SUCCESS;

  /* encrypts the input */
  arc4_t arc4;
  arc4_create(&arc4, key, _client_arc4_read_cb, input, _client_arc4_write_cb,
              &socket);
  if (!arc4_start(&arc4)) {
    exit_code = EXIT_FAILURE;
  }

  /* cleanup */
  arc4_destroy(&arc4);
  tcp_socket_destroy(&socket);
  fclose(input);
  return exit_code;
}
