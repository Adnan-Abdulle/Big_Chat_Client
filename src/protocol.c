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
  buffer[3] = header->reserved;

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

void serialize_channel_list_request(const struct channel_list_request *message,
                                    uint8_t *buffer) {
  memcpy(buffer, &message->auth, 32);
}

void deserialize_channel_list_response(const uint8_t *buffer,
                                       struct channel_list_response *message) {

  memcpy(&message->auth, buffer, 32);

  message->channel_id_len = buffer[32];

  memcpy(message->channel_ids, buffer + 33, message->channel_id_len);
}

void serialize_channel_read_request(const struct channel_read_request *message,
                                    uint8_t *buffer) {

  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);

  buffer[32] = message->channel_id;
}

void deserialize_channel_read_response(const uint8_t *buffer,
                                       struct channel_read_response *message) {

  memcpy(message->auth.username, buffer, USERNAME_SIZE);
  memcpy(message->auth.password, buffer + USERNAME_SIZE, PASSWORD_SIZE);

  memcpy(message->channel_name, buffer + 32, CHANNEL_NAME_SIZE);

  message->channel_id = buffer[48];
  message->user_id_len = buffer[49];

  memcpy(message->user_ids, buffer + 50, message->user_id_len);
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
  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);
  serialize_ipv4(&message->ip, buffer + USERNAME_SIZE + PASSWORD_SIZE);
  buffer[USERNAME_SIZE + PASSWORD_SIZE + 4] = message->account_status;
}

void deserialize_login_or_logout(const uint8_t *buffer,
                                 struct login_or_logout *message) {
  memcpy(message->auth.username, buffer, USERNAME_SIZE);
  memcpy(message->auth.password, buffer + USERNAME_SIZE, PASSWORD_SIZE);
  deserialize_ipv4(buffer + USERNAME_SIZE + PASSWORD_SIZE, &message->ip);
  message->account_status = buffer[USERNAME_SIZE + PASSWORD_SIZE + 4];
}
void deserialize_server_registration(const uint8_t *buffer,
                                     struct server_registration *message) {
  deserialize_ipv4(buffer, &message->ip);
  message->server_id = buffer[4];
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

  if (message_type == MESSAGE_TYPE_CHANNEL_LIST_READ_REQUEST) {
    return CHANNEL_LIST_REQUEST_BODY_SIZE;
  }

  return 0;
}