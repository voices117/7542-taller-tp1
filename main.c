#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arc4.h"
#include "server.h"

/* constants */
#define MSG_INVALID_INPUT_ARGS "Parámetros incorrectos.\n"

/**
 * Converts a sequence of numeric digits into an integer value.
 *
 * \param output The integer read from the input string (in case of success).
 * \param input The string containing the value to read.
 * \return false if it couldn't read an integer from the string.
 */
static bool strtoint(int *output, const char *input) {
  errno = 0;
  long int value = strtol(input, NULL, 10);
  if (errno != 0 || value > INT_MAX) {
    return false;
  }

  *output = value;
  return true;
}

static bool _write_hex_buf(FILE *f, const unsigned char *data, size_t size,
                           bool uppercase) {
  const char *fmt = uppercase ? "%02X" : "%02x";
  for (size_t i = 0; i < size; i++) {
    if (fprintf(f, fmt, data[i]) != 2) {
      return false;
    }
  }
  return true;
}

static ssize_t _client_arc4_read_cb(void *cb_ctx, void *data, size_t size) {
  FILE *f = cb_ctx;
  size_t bytes_read = fread(data, 1, size, f);

  /* checks for errors */
  if (bytes_read != size && ferror(f)) {
    return -1;
  }

  return bytes_read;
}

static bool _client_arc4_write_cb(void *cb_ctx, const void *data, size_t size) {
  tcp_socket_t *s = cb_ctx;

  /* prints the encoded data to STDOUT */
  const bool uppercase = false;
  if (!_write_hex_buf(stdout, data, size, uppercase)) {
    return false;
  }

  /* sends the encoded data to the server socket */
  return tcp_socket_write(s, data, size);
}

static ssize_t _server_arc4_read_cb(void *cb_ctx, void *data, size_t size) {
  tcp_socket_t *client = cb_ctx;

  /* reads a chunk from the client connection */
  ssize_t bytes_read = tcp_socket_read(client, data, 50);

  /* prints it to STDOUT */
  const bool uppercase = true;
  if (!_write_hex_buf(stdout, data, bytes_read, uppercase)) {
    return -1;
  }

  return bytes_read;
}

static bool _server_arc4_write_cb(void *cb_ctx, const void *data, size_t size) {
  FILE *output = cb_ctx;
  return (fwrite(data, 1, size, output) == size);
}

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

static int _main_server(int argc, const char **argv) {
  if (argc != 4) {
    return EXIT_FAILURE;
  }

  /* reads the port */
  int port;
  if (!strtoint(&port, argv[2])) {
    return EXIT_FAILURE;
  }

  const char *key = argv[3];

  server_t server;
  if (!server_create(&server, port)) {
    return EXIT_FAILURE;
  }

  if (!server_handle_client(&server, _server_handler, key)) {
    return EXIT_FAILURE;
  }

  server_destroy(&server);
  return EXIT_SUCCESS;
}

static int _main_client(int argc, const char **argv) {
  if (argc != 6) {
    return EXIT_FAILURE;
  }

  /* reads the port */
  int port;
  if (!strtoint(&port, argv[3])) {
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

int main(int argc, const char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "%s", MSG_INVALID_INPUT_ARGS);
    return EXIT_FAILURE;
  }

  int exit_code = EXIT_FAILURE;

  const char *app = argv[1];
  if (strcmp(app, "server") == 0) {
    exit_code = _main_server(argc, argv);
  } else if (strcmp(app, "client") == 0) {
    exit_code = _main_client(argc, argv);
  }

  if (exit_code != EXIT_SUCCESS) {
    fprintf(stderr, "%s", MSG_INVALID_INPUT_ARGS);
  }
  return exit_code;
}
