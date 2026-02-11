#include "manager.h"
#include "network.h"
#include "protocol.h"
#include <string.h>
#include <syslog.h>

/* generic helper to send a header + optional body */
static int send_message(int file_descriptor, uint8_t message_type,
                        uint8_t status, const uint8_t *body,
                        uint32_t body_size) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;

  header.version = PROTOCOL_VERSION;
  header.type = message_type;
  header.status = status;
  header.body_size = body_size;
  serialize_header(&header, header_buffer);

  if (write_exact(file_descriptor, header_buffer, HEADER_SIZE) <= 0) {
    return -1;
  }

  if (body_size > 0 && body != NULL) {
    if (write_exact(file_descriptor, body, body_size) <= 0) {
      return -1;
    }
  }

  return 0;
}

/* read an 8-byte header from the socket */
static int receive_header(int file_descriptor, struct protocol_header *header) {
  uint8_t header_buffer[HEADER_SIZE];

  if (read_exact(file_descriptor, header_buffer, HEADER_SIZE) <= 0) {
    return -1;
  }

  deserialize_header(header_buffer, header);
  return 0;
}

/* send registration request with server_id=0 (asking for an assignment) */
static int send_registration_request(int manager_file_descriptor,
                                     const struct ipv4_address *server_ip) {
  struct server_registration registration;
  uint8_t body_buffer[SERVER_REGISTRATION_BODY_SIZE];

  registration.ip = *server_ip;
  registration.server_id = 0;
  serialize_server_registration(&registration, body_buffer);

  return send_message(manager_file_descriptor,
                      MESSAGE_TYPE_SERVER_REGISTRATION_REQUEST, STATUS_OK,
                      body_buffer, SERVER_REGISTRATION_BODY_SIZE);
}

/* read the registration response and extract our assigned server_id */
static int receive_registration_response(int manager_file_descriptor,
                                         uint8_t *assigned_server_id) {
  struct protocol_header response_header;
  uint8_t response_body[SERVER_REGISTRATION_BODY_SIZE];
  struct server_registration response_registration;

  if (receive_header(manager_file_descriptor, &response_header) != 0) {
    syslog(LOG_ERR, "failed to receive registration response header");
    return -1;
  }

  if (response_header.type != MESSAGE_TYPE_SERVER_REGISTRATION_RESPONSE) {
    syslog(LOG_ERR, "unexpected response type: 0x%02X", response_header.type);
    return -1;
  }

  if (response_header.status != STATUS_OK) {
    syslog(LOG_ERR, "registration rejected with status: 0x%02X",
           response_header.status);
    return -1;
  }

  if (read_exact(manager_file_descriptor, response_body,
                 SERVER_REGISTRATION_BODY_SIZE) <= 0) {
    syslog(LOG_ERR, "failed to receive registration response body");
    return -1;
  }

  deserialize_server_registration(response_body, &response_registration);
  *assigned_server_id = response_registration.server_id;
  return 0;
}

int connect_and_register_with_manager(const char *manager_ip,
                                      uint16_t manager_port,
                                      const struct ipv4_address *server_ip,
                                      uint8_t *assigned_server_id) {
  int manager_file_descriptor;

  manager_file_descriptor = connect_to_remote(manager_ip, manager_port);
  if (manager_file_descriptor == -1) {
    syslog(LOG_ERR, "failed to connect to manager at %s:%u", manager_ip,
           manager_port);
    return -1;
  }

  if (send_registration_request(manager_file_descriptor, server_ip) != 0) {
    syslog(LOG_ERR, "failed to send registration request");
    return -1;
  }

  if (receive_registration_response(manager_file_descriptor,
                                    assigned_server_id) != 0) {
    return -1;
  }

  syslog(LOG_INFO, "registered with manager, assigned server id: %u",
         *assigned_server_id);
  return manager_file_descriptor;
}

int send_health_check_to_manager(int manager_file_descriptor,
                                 const struct ipv4_address *server_ip,
                                 uint8_t server_id) {
  struct server_registration health_check;
  uint8_t body_buffer[SERVER_HEALTH_CHECK_BODY_SIZE];

  health_check.ip = *server_ip;
  health_check.server_id = server_id;
  serialize_server_registration(&health_check, body_buffer);

  return send_message(manager_file_descriptor,
                      MESSAGE_TYPE_SERVER_HEALTH_CHECK_REQUEST, STATUS_OK,
                      body_buffer, SERVER_HEALTH_CHECK_BODY_SIZE);
}

enum { LOG_BODY_OVERHEAD = 4, LOG_MESSAGE_MAX_LENGTH = 256 };
enum { LOG_BUFFER_SIZE = 8 + 4 + 256 };

int send_log_to_manager(int manager_file_descriptor, uint8_t server_id,
                        const char *log_message) {
  uint8_t buffer[LOG_BUFFER_SIZE];
  size_t total_size;

  total_size =
      serialize_log_request(server_id, log_message, buffer, sizeof(buffer));
  if (total_size == 0) {
    syslog(LOG_ERR, "failed to serialize log message");
    return -1;
  }

  if (write_exact(manager_file_descriptor, buffer, total_size) <= 0) {
    syslog(LOG_ERR, "failed to send log to manager");
    return -1;
  }

  return 0;
}

/* respond to an activation request from the manager */
static int handle_activation_request(int manager_file_descriptor,
                                     uint8_t server_id, const uint8_t *body,
                                     int *is_activated) {
  struct server_activation activation;
  uint8_t response_body[SERVER_ACTIVATION_BODY_SIZE];

  deserialize_server_activation(body, &activation);

  if (activation.server_id != server_id) {
    syslog(LOG_WARNING,
           "activation request for wrong server id: %u (we are %u)",
           activation.server_id, server_id);
  }

  *is_activated = 1;
  syslog(LOG_INFO, "server activated");

  response_body[0] = server_id;
  return send_message(manager_file_descriptor,
                      MESSAGE_TYPE_SERVER_ACTIVATION_RESPONSE, STATUS_OK,
                      response_body, SERVER_ACTIVATION_BODY_SIZE);
}

/* respond to a deactivation request from the manager */
static int handle_deactivation_request(int manager_file_descriptor,
                                       uint8_t server_id, const uint8_t *body,
                                       int *is_activated) {
  struct server_activation deactivation;
  uint8_t response_body[SERVER_DEACTIVATION_BODY_SIZE];

  deserialize_server_activation(body, &deactivation);

  *is_activated = 0;
  syslog(LOG_INFO, "server deactivated");

  response_body[0] = server_id;
  return send_message(manager_file_descriptor,
                      MESSAGE_TYPE_SERVER_DEACTIVATION_RESPONSE, STATUS_OK,
                      response_body, SERVER_DEACTIVATION_BODY_SIZE);
}

/* figure out what the manager sent us and handle it */
static int dispatch_manager_message(int manager_file_descriptor,
                                    uint8_t server_id, int *is_activated,
                                    uint8_t message_type,
                                    const uint8_t *body_buffer) {
  if (message_type == MESSAGE_TYPE_SERVER_ACTIVATION_REQUEST) {
    return handle_activation_request(manager_file_descriptor, server_id,
                                     body_buffer, is_activated);
  }

  if (message_type == MESSAGE_TYPE_SERVER_DEACTIVATION_REQUEST) {
    return handle_deactivation_request(manager_file_descriptor, server_id,
                                       body_buffer, is_activated);
  }

  if (message_type == MESSAGE_TYPE_SERVER_HEALTH_CHECK_RESPONSE) {
    syslog(LOG_DEBUG, "received health check response");
    return 0;
  }

  if (message_type == MESSAGE_TYPE_SERVER_REGISTRATION_RESPONSE) {
    syslog(LOG_DEBUG,
           "received registration response (unexpected in this state)");
    return 0;
  }

  syslog(LOG_WARNING, "unexpected message type from manager: 0x%02X",
         message_type);
  return 0;
}

int handle_manager_message(int manager_file_descriptor, uint8_t server_id,
                           int *is_activated) {
  struct protocol_header header;
  uint8_t body_buffer[SERVER_REGISTRATION_BODY_SIZE];
  uint32_t expected_body_size;

  if (receive_header(manager_file_descriptor, &header) != 0) {
    return -1;
  }

  if (header.version != PROTOCOL_VERSION) {
    syslog(LOG_ERR, "unexpected protocol version: 0x%02X", header.version);
    return -1;
  }

  expected_body_size = get_body_size_for_type(header.type);
  if (expected_body_size == 0 || header.body_size != expected_body_size) {
    syslog(LOG_ERR, "unexpected body size for type 0x%02X: got %u, expected %u",
           header.type, header.body_size, expected_body_size);
    return -1;
  }

  if (read_exact(manager_file_descriptor, body_buffer, expected_body_size) <=
      0) {
    return -1;
  }

  return dispatch_manager_message(manager_file_descriptor, server_id,
                                  is_activated, header.type, body_buffer);
}
