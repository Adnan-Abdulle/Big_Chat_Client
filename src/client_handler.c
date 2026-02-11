#include "client_handler.h"
#include "network.h"
#include "protocol.h"
#include "server.h"
#include <syslog.h>

/* read one message from a client and dispatch it to the right handler */
int handle_client_message(int client_file_descriptor,
                          struct server_state *state) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  ssize_t bytes_read;
  uint32_t expected_size;
  uint8_t body_buffer[ACCOUNT_REGISTRATION_BODY_SIZE];

  bytes_read = read_exact(client_file_descriptor, header_buffer, HEADER_SIZE);
  if (bytes_read <= 0) {
    return -1;
  }

  deserialize_header(header_buffer, &header);

  if (header.version != PROTOCOL_VERSION) {
    syslog(LOG_ERR, "client sent wrong protocol version: 0x%02X",
           header.version);
    return -1;
  }

  if (header.type == MESSAGE_TYPE_ACCOUNT_REGISTRATION_REQUEST) {
    expected_size = ACCOUNT_REGISTRATION_BODY_SIZE;
  } else if (header.type == MESSAGE_TYPE_LOGIN_OR_LOGOUT_REQUEST) {
    expected_size = LOGIN_OR_LOGOUT_BODY_SIZE;
  } else {
    syslog(LOG_WARNING, "unexpected message type from client: 0x%02X",
           header.type);
    return -1;
  }

  if (header.body_size != expected_size) {
    syslog(LOG_ERR, "unexpected body size for type 0x%02X: got %u", header.type,
           header.body_size);
    return -1;
  }

  bytes_read = read_exact(client_file_descriptor, body_buffer, expected_size);
  if (bytes_read <= 0) {
    return -1;
  }

  if (header.type == MESSAGE_TYPE_ACCOUNT_REGISTRATION_REQUEST) {
    return handle_account_registration(client_file_descriptor, body_buffer,
                                       state);
  }

  return handle_login_or_logout(client_file_descriptor, body_buffer, state);
}
