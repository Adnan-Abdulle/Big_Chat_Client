#ifndef NETWORK_H
#define NETWORK_H

#include "protocol.h"
#include <stdint.h>
#include <sys/types.h>

/* look up the IPv4 address for a named network interface (e.g. "lo", "eth0") */
int resolve_interface_to_ipv4(const char *interface_name,
                              struct ipv4_address *result);

/* create a TCP socket bound to the given port, listening for connections */
int create_listening_socket(uint16_t port);

/* create a TCP socket and connect to a remote IP:port */
int connect_to_remote(const char *ip_string, uint16_t port);

/* read exactly number_of_bytes from the fd, looping on partial reads */
ssize_t read_exact(int file_descriptor, uint8_t *buffer,
                   size_t number_of_bytes);

/* write exactly number_of_bytes to the fd, looping on partial writes */
ssize_t write_exact(int file_descriptor, const uint8_t *buffer,
                    size_t number_of_bytes);

#endif
