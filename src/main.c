#include "manager.h"
#include "network.h"
#include "protocol.h"
#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

enum { REQUIRED_ARGUMENT_COUNT = 5 };
enum { DECIMAL_BASE = 10 };

static void setup_signal_handlers(void) { signal(SIGPIPE, SIG_IGN); }

static void print_usage(const char *program_name) {
  fprintf(stderr,
          "Usage: %s <interface> <manager_ip> <manager_port> <server_port>\n",
          program_name);
}

/* parse a port string into a uint16_t, returns -1 on bad input */
static int parse_port(const char *port_string, uint16_t *port_out) {
  char *end_pointer;
  long int value;

  value = strtol(port_string, &end_pointer, DECIMAL_BASE);

  if (*end_pointer != '\0' || value <= 0 || value > UINT16_MAX) {
    return -1;
  }

  *port_out = (uint16_t)value;
  return 0;
}

/* resolve interface, create listener socket */
static int open_resources(struct ipv4_address *server_ip,
                          const char *interface_name, uint16_t server_port) {
  int listener_file_descriptor;

  memset(server_ip, 0, sizeof(*server_ip));
  if (resolve_interface_to_ipv4(interface_name, server_ip) != 0) {
    fprintf(stderr, "Failed to resolve interface %s\n", interface_name);
    return -1;
  }

  printf("  server IP:    %u.%u.%u.%u\n", server_ip->a, server_ip->b,
         server_ip->c, server_ip->d);

  listener_file_descriptor = create_listening_socket(server_port);
  if (listener_file_descriptor == -1) {
    fprintf(stderr, "Failed to create listening socket on port %u\n",
            server_port);
    return -1;
  }

  printf("  listening on port %u\n", server_port);
  return listener_file_descriptor;
}

/* connect to manager and register */
static int register_with_manager(const char *manager_ip, uint16_t manager_port,
                                 const struct ipv4_address *server_ip,
                                 uint8_t *server_id) {
  int manager_file_descriptor;

  *server_id = 0;
  manager_file_descriptor = connect_and_register_with_manager(
      manager_ip, manager_port, server_ip, server_id);
  if (manager_file_descriptor == -1) {
    fprintf(stderr, "Failed to register with manager\n");
    return -1;
  }

  printf("  registered with manager, server id: %u\n", *server_id);
  return manager_file_descriptor;
}

int main(int argc, char *argv[]) {
  const char *interface_name;
  const char *manager_ip;
  uint16_t manager_port;
  uint16_t server_port;
  struct server_state state;

  if (argc != REQUIRED_ARGUMENT_COUNT) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  interface_name = argv[1];
  manager_ip = argv[2];

  if (parse_port(argv[3], &manager_port) != 0) {
    fprintf(stderr, "Invalid manager port: %s\n", argv[3]);
    return EXIT_FAILURE;
  }

  if (parse_port(argv[4], &server_port) != 0) {
    fprintf(stderr, "Invalid server port: %s\n", argv[4]);
    return EXIT_FAILURE;
  }

  openlog("big-chat-server", LOG_PID | LOG_CONS, LOG_USER);
  setup_signal_handlers();
  initialize_server_state(&state);

  printf("B.I.G Chat Server starting\n");
  printf("  interface:    %s\n", interface_name);
  printf("  manager:      %s:%u\n", manager_ip, manager_port);
  printf("  server port:  %u\n", server_port);

  state.listener_file_descriptor =
      open_resources(&state.server_ip, interface_name, server_port);
  if (state.listener_file_descriptor == -1) {
    closelog();
    return EXIT_FAILURE;
  }

  state.manager_file_descriptor = register_with_manager(
      manager_ip, manager_port, &state.server_ip, &state.server_id);
  if (state.manager_file_descriptor == -1) {
    close(state.listener_file_descriptor);
    closelog();
    return EXIT_FAILURE;
  }

  run_server_event_loop(&state);
  shutdown_server(&state);
  closelog();
  return EXIT_SUCCESS;
}
