#include "network.h"
#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>

enum {
  IP_BYTE_A_SHIFT = 24,
  IP_BYTE_B_SHIFT = 16,
  IP_BYTE_C_SHIFT = 8,
  IP_BYTE_MASK = 0xFF
};

/* walk the system interface list and find the IPv4 address for interface_name
 */
int resolve_interface_to_ipv4(const char *interface_name,
                              struct ipv4_address *result) {
  struct ifaddrs *interface_list;
  const struct ifaddrs *current;
  int found;
  uint32_t ip_network;
  uint32_t ip_host;

  if (getifaddrs(&interface_list) == -1) {
    syslog(LOG_ERR, "getifaddrs failed: %s", strerror(errno));
    return -1;
  }

  found = 0;

  for (current = interface_list; current != NULL; current = current->ifa_next) {
    struct sockaddr_in socket_address;

    if (current->ifa_addr == NULL) {
      continue;
    }
    if (current->ifa_addr->sa_family != AF_INET) {
      continue;
    }
    if (strcmp(current->ifa_name, interface_name) != 0) {
      continue;
    }

    memcpy(&socket_address, current->ifa_addr, sizeof(socket_address));
    ip_network = socket_address.sin_addr.s_addr;
    ip_host = ntohl(ip_network);
    result->a = (uint8_t)((ip_host >> IP_BYTE_A_SHIFT) & IP_BYTE_MASK);
    result->b = (uint8_t)((ip_host >> IP_BYTE_B_SHIFT) & IP_BYTE_MASK);
    result->c = (uint8_t)((ip_host >> IP_BYTE_C_SHIFT) & IP_BYTE_MASK);
    result->d = (uint8_t)(ip_host & IP_BYTE_MASK);
    found = 1;
    break;
  }

  freeifaddrs(interface_list);

  if (!found) {
    syslog(LOG_ERR, "interface %s not found or has no IPv4 address",
           interface_name);
    return -1;
  }

  return 0;
}

/* set up a TCP server socket: create, set reuse, bind, listen */
int create_listening_socket(uint16_t port) {
  int socket_file_descriptor;
  int option_value;
  struct sockaddr_in bind_address;

  socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_file_descriptor == -1) {
    syslog(LOG_ERR, "socket creation failed: %s", strerror(errno));
    return -1;
  }

  option_value = 1;
  if (setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR,
                 &option_value, sizeof(option_value)) == -1) {
    syslog(LOG_ERR, "setsockopt SO_REUSEADDR failed: %s", strerror(errno));
    close(socket_file_descriptor);
    return -1;
  }

  memset(&bind_address, 0, sizeof(bind_address));
  bind_address.sin_family = AF_INET;
  bind_address.sin_addr.s_addr = htonl(INADDR_ANY);
  bind_address.sin_port = htons(port);

  if (bind(socket_file_descriptor, (struct sockaddr *)&bind_address,
           sizeof(bind_address)) == -1) {
    syslog(LOG_ERR, "bind failed on port %u: %s", port, strerror(errno));
    close(socket_file_descriptor);
    return -1;
  }

  if (listen(socket_file_descriptor, SOMAXCONN) == -1) {
    syslog(LOG_ERR, "listen failed: %s", strerror(errno));
    close(socket_file_descriptor);
    return -1;
  }

  return socket_file_descriptor;
}

/* create a TCP socket and connect to a remote address */
int connect_to_remote(const char *ip_string, uint16_t port) {
  int socket_file_descriptor;
  struct sockaddr_in remote_address;

  socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_file_descriptor == -1) {
    syslog(LOG_ERR, "socket creation failed: %s", strerror(errno));
    return -1;
  }

  memset(&remote_address, 0, sizeof(remote_address));
  remote_address.sin_family = AF_INET;
  remote_address.sin_port = htons(port);

  if (inet_pton(AF_INET, ip_string, &remote_address.sin_addr) != 1) {
    syslog(LOG_ERR, "invalid IP address: %s", ip_string);
    close(socket_file_descriptor);
    return -1;
  }

  if (connect(socket_file_descriptor, (struct sockaddr *)&remote_address,
              sizeof(remote_address)) == -1) {
    syslog(LOG_ERR, "connect to %s:%u failed: %s", ip_string, port,
           strerror(errno));
    close(socket_file_descriptor);
    return -1;
  }

  return socket_file_descriptor;
}

/* keep reading until we have exactly number_of_bytes, or hit EOF/error */
ssize_t read_exact(int file_descriptor, uint8_t *buffer,
                   size_t number_of_bytes) {
  size_t total_bytes_read;

  total_bytes_read = 0;

  while (total_bytes_read < number_of_bytes) {
    ssize_t bytes_read;

    bytes_read = read(file_descriptor, buffer + total_bytes_read,
                      number_of_bytes - total_bytes_read);

    if (bytes_read < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }

    if (bytes_read == 0) {
      return 0;
    }

    total_bytes_read += (size_t)bytes_read;
  }

  return (ssize_t)total_bytes_read;
}

/* keep writing until all number_of_bytes are sent, or hit error */
ssize_t write_exact(int file_descriptor, const uint8_t *buffer,
                    size_t number_of_bytes) {
  size_t total_bytes_written;

  total_bytes_written = 0;

  while (total_bytes_written < number_of_bytes) {
    ssize_t bytes_written;

    bytes_written = write(file_descriptor, buffer + total_bytes_written,
                          number_of_bytes - total_bytes_written);

    if (bytes_written < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }

    total_bytes_written += (size_t)bytes_written;
  }

  return (ssize_t)total_bytes_written;
}
