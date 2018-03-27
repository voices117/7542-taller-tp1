#include "main_server.h"
#include <stdio.h>
#include <stdlib.h>
#include "arc4.h"
#include "server.h"
#include "tcp_socket.h"
#include "utils.h"

/** The size of the chunks read by the server. */
#define SERVER_CHUNK_SIZE 50

/**
 * ARC4 read callback that gets data from a socket.
 *
 * \param cb_ctx Callback context (socket).
 * \param data Output buffer for ARC4.
 * \param size Size of the output buffer.
 * \return ssize_t Bytes written into the output buffer.
 */
static ssize_t _server_arc4_read_cb(void *cb_ctx, void *data, size_t size) {
  tcp_socket_t *client = cb_ctx;

  /* reads a chunk from the client connection */
  ssize_t bytes_read = tcp_socket_read(client, data, SERVER_CHUNK_SIZE);

  return bytes_read;
}

/**
 * ARC4 write callback that writes to an output file.
 *
 * \param cb_ctx Callback context (output file).
 * \param data Buffer with the decoded data.
 * \param size Size of the buffer.
 * \return false in case of error.
 */
static bool _server_arc4_write_cb(void *cb_ctx, const void *data, size_t size) {
  FILE *output = cb_ctx;

  /* prints it to STDOUT */
  const bool uppercase = false;
  if (!utils_write_hex_buf(stdout, data, size, uppercase)) {
    return -1;
  }

  return (fwrite(data, 1, size, output) == size);
}

/**
 * Server handler that reads and decodes ARC4 data from a socket and writes it
 * to a file.
 *
 * \param client Client connection.
 * \param handler_ctx Handler context (the ARC4 key).
 * \return false In case of an error.
 */
static bool _server_handler(tcp_socket_t *client, const void *handler_ctx) {
  const char *key = handler_ctx;

  /* creates/truncates the output file */
  FILE *output = fopen("out", "w");

  arc4_t arc4;
  if (!arc4_create(&arc4, key, _server_arc4_read_cb, client,
                   _server_arc4_write_cb, output)) {
    fclose(output);
    return false;
  }

  bool result = arc4_start(&arc4);

  arc4_destroy(&arc4);
  fclose(output);
  return result;
}

/**
 * ARGV: server <PORT> <Key>
 *
 * \param argc
 * \param argv
 * \return int Exit code.
 */
int main_server(int argc, const char **argv) {
  if (argc != 4) {
    return EXIT_FAILURE;
  }

  /* reads the port */
  int port;
  if (!utils_strtoint(&port, argv[2])) {
    return EXIT_FAILURE;
  }

  const char *key = argv[3];

  /* creates the server */
  server_t server;
  if (!server_create(&server, port)) {
    return EXIT_FAILURE;
  }

  /* waits for a client to connect and decode it's data */
  if (!server_handle_client(&server, _server_handler, key)) {
    return EXIT_FAILURE;
  }

  server_destroy(&server);
  return EXIT_SUCCESS;
}
