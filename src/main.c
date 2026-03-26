#include "ncurses_ui.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

enum { EXPECTED_ARGC = 3 };
enum { STRTOL_DECIMAL = 10 };
enum { MIN_PORT = 1, MAX_PORT = 65535 };

/* entry point: parse ip and port then launch the ncurses ui */
int main(int argc, const char *const argv[]) {
  const char *ip;
  char *end;
  long port_long;
  uint16_t port;

  if (argc != EXPECTED_ARGC) {
    fprintf(stderr, "Invalid num of arguments. Need %d received %d\n",
            EXPECTED_ARGC, argc);
    return EXIT_FAILURE;
  }

  ip = argv[1];
  port_long = strtol(argv[2], &end, STRTOL_DECIMAL);

  if (*end != '\0') {
    fprintf(stderr, "Port must be a numeric value\n");
    return EXIT_FAILURE;
  }

  if (port_long < MIN_PORT || port_long > MAX_PORT) {
    fprintf(stderr, "Port must be between %d and %d\n", MIN_PORT, MAX_PORT);
    return EXIT_FAILURE;
  }

  port = (uint16_t)port_long;

  openlog("bigchat-client", LOG_PID | LOG_CONS, LOG_USER);
  run_ui(ip, port);
  closelog();

  return EXIT_SUCCESS;
}
