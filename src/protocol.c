#include "protocol.h"
#include <arpa/inet.h>
#include <string.h>

static void serialize_ipv4(const struct ipv4_address *ip, uint8_t *buffer) {
  buffer[0] = ip->a;
  buffer[1] = ip->b;
  buffer[2] = ip->c;
  buffer[3] = ip->d;
}

static void deserialize_ipv4(const uint8_t *buffer, struct ipv4_address *ip) {
  ip->a = buffer[0];
  ip->b = buffer[1];
  ip->c = buffer[2];
  ip->d = buffer[3];
}

void serialize_header(const struct protocol_header *header, uint8_t *buffer) {
  uint32_t network_size;

  buffer[0] = header->version;
  buffer[1] = header->type;
  buffer[2] = header->status;
  buffer[3] = 0;
  network_size = htonl(header->body_size);
  memcpy(buffer + 4, &network_size, sizeof(network_size));
}

void deserialize_header(const uint8_t *buffer, struct protocol_header *header) {
  uint32_t network_size;

  header->version = buffer[0];
  header->type = buffer[1];
  header->status = buffer[2];
  memcpy(&network_size, buffer + 4, sizeof(network_size));
  header->body_size = ntohl(network_size);
}

void serialize_server_registration(const struct server_registration *message,
                                   uint8_t *buffer) {
  serialize_ipv4(&message->ip, buffer);
  buffer[4] = message->server_id;
}

void deserialize_server_registration(const uint8_t *buffer,
                                     struct server_registration *message) {
  deserialize_ipv4(buffer, &message->ip);
  message->server_id = buffer[4];
}

void serialize_server_activation(const struct server_activation *message,
                                 uint8_t *buffer) {
  buffer[0] = message->server_id;
}

void deserialize_server_activation(const uint8_t *buffer,
                                   struct server_activation *message) {
  message->server_id = buffer[0];
}

void serialize_account_registration(const struct account_registration *message,
                                    uint8_t *buffer) {
  memcpy(buffer, message->username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->password, PASSWORD_SIZE);
  buffer[USERNAME_SIZE + PASSWORD_SIZE] = message->account_id;
}

void deserialize_account_registration(const uint8_t *buffer,
                                      struct account_registration *message) {
  memcpy(message->username, buffer, USERNAME_SIZE);
  memcpy(message->password, buffer + USERNAME_SIZE, PASSWORD_SIZE);
  message->account_id = buffer[USERNAME_SIZE + PASSWORD_SIZE];
}

void serialize_login_or_logout(const struct login_or_logout *message,
                               uint8_t *buffer) {
  memcpy(buffer, message->password, PASSWORD_SIZE);
  buffer[PASSWORD_SIZE] = message->account_id;
  buffer[PASSWORD_SIZE + 1] = message->account_status;
  serialize_ipv4(&message->ip, buffer + PASSWORD_SIZE + 2);
}

void deserialize_login_or_logout(const uint8_t *buffer,
                                 struct login_or_logout *message) {
  memcpy(message->password, buffer, PASSWORD_SIZE);
  message->account_id = buffer[PASSWORD_SIZE];
  message->account_status = buffer[PASSWORD_SIZE + 1];
  deserialize_ipv4(buffer + PASSWORD_SIZE + 2, &message->ip);
}

size_t serialize_log_request(uint8_t server_id, const char *log_message,
                             uint8_t *buffer, size_t buffer_size) {
  size_t message_length;
  size_t body_size;
  size_t total_size;
  struct protocol_header header;
  uint8_t *body;
  uint16_t network_length;

  message_length = strlen(log_message);
  if (message_length > UINT16_MAX) {
    message_length = UINT16_MAX;
  }

  body_size = 4 + message_length;
  total_size = HEADER_SIZE + body_size;

  if (total_size > buffer_size) {
    return 0;
  }

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_LOG_REQUEST;
  header.status = STATUS_OK;
  header.body_size = (uint32_t)body_size;
  serialize_header(&header, buffer);

  body = buffer + HEADER_SIZE;
  body[0] = server_id;
  body[1] = 0;

  network_length = htons((uint16_t)message_length);
  memcpy(body + 2, &network_length, sizeof(network_length));
  memcpy(body + 4, log_message, message_length);

  return total_size;
}

uint32_t get_body_size_for_type(uint8_t message_type) {
  if (message_type == MESSAGE_TYPE_SERVER_REGISTRATION_REQUEST ||
      message_type == MESSAGE_TYPE_SERVER_REGISTRATION_RESPONSE ||
      message_type == MESSAGE_TYPE_SERVER_HEALTH_CHECK_REQUEST ||
      message_type == MESSAGE_TYPE_SERVER_HEALTH_CHECK_RESPONSE) {
    return SERVER_REGISTRATION_BODY_SIZE;
  }

  if (message_type == MESSAGE_TYPE_SERVER_ACTIVATION_REQUEST ||
      message_type == MESSAGE_TYPE_SERVER_ACTIVATION_RESPONSE ||
      message_type == MESSAGE_TYPE_SERVER_DEACTIVATION_REQUEST ||
      message_type == MESSAGE_TYPE_SERVER_DEACTIVATION_RESPONSE) {
    return SERVER_ACTIVATION_BODY_SIZE;
  }

  if (message_type == MESSAGE_TYPE_ACCOUNT_REGISTRATION_REQUEST ||
      message_type == MESSAGE_TYPE_ACCOUNT_REGISTRATION_RESPONSE) {
    return ACCOUNT_REGISTRATION_BODY_SIZE;
  }

  if (message_type == MESSAGE_TYPE_LOGIN_OR_LOGOUT_REQUEST ||
      message_type == MESSAGE_TYPE_LOGIN_OR_LOGOUT_RESPONSE) {
    return LOGIN_OR_LOGOUT_BODY_SIZE;
  }

  return 0;
}
