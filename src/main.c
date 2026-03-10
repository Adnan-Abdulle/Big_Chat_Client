#include "client.h"
#include "ncurses_ui.h"
#include "network.h"
#include "protocol.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Invalid num of arguments. Need 3 recieved %d", argc);
  }
  const char *ip = argv[1];
  char *end;
  long port_long = strtol(argv[2], &end, 10);
  uint16_t port = (uint16_t)port_long;
  openlog("bigchat-client", LOG_PID | LOG_CONS, LOG_USER);
  run_ui(ip, port);
  closelog();
  return EXIT_SUCCESS;
}
