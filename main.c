#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_client.h"
#include "main_server.h"

/* constants */
#define MSG_INVALID_INPUT_ARGS "Parámetros incorrectos.\n"

int main(int argc, const char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "%s", MSG_INVALID_INPUT_ARGS);
    return EXIT_FAILURE;
  }

  int exit_code = EXIT_FAILURE;

  const char *app = argv[1];
  if (strcmp(app, "server") == 0) {
    exit_code = main_server(argc, argv);
  } else if (strcmp(app, "client") == 0) {
    exit_code = main_client(argc, argv);
  }

  if (exit_code != EXIT_SUCCESS) {
    fprintf(stderr, "%s", MSG_INVALID_INPUT_ARGS);
  }
  return exit_code;
}
