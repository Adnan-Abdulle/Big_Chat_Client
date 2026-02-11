#ifndef MANAGER_H
#define MANAGER_H

#include "protocol.h"
#include <stdint.h>

/* connect to the manager, register, and return the socket fd (-1 on failure) */
int connect_and_register_with_manager(const char *manager_ip,
                                      uint16_t manager_port,
                                      const struct ipv4_address *server_ip,
                                      uint8_t *assigned_server_id);

/* send a health check to the manager */
int send_health_check_to_manager(int manager_file_descriptor,
                                 const struct ipv4_address *server_ip,
                                 uint8_t server_id);

/* send a log message to the manager */
int send_log_to_manager(int manager_file_descriptor, uint8_t server_id,
                        const char *log_message);

/* read and handle one message from the manager, returns -1 on connection loss */
int handle_manager_message(int manager_file_descriptor, uint8_t server_id,
                           int *is_activated);

#endif
