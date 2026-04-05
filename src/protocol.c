#include "protocol.h"
#include <arpa/inet.h>
#include <string.h>

/* byte offsets within the protocol header */
enum {
  HDR_VERSION = 0,
  HDR_TYPE = 1,
  HDR_STATUS = 2,
  HDR_RESERVED = 3,
  HDR_SIZE = 4
};

/* byte offsets for message bodies that start with auth + timestamp */
enum {
  OFF_TIMESTAMP = AUTH_SIZE,
  OFF_MSG_LEN = AUTH_SIZE + TIMESTAMP_SIZE,
  OFF_MSG_CHANNEL = AUTH_SIZE + TIMESTAMP_SIZE + MESSAGE_LEN_FIELD_SIZE,
  OFF_MSG_SENDER = AUTH_SIZE + TIMESTAMP_SIZE + MESSAGE_LEN_FIELD_SIZE + 1,
  OFF_MSG_DATA = AUTH_SIZE + TIMESTAMP_SIZE + MESSAGE_LEN_FIELD_SIZE + 2
};

/* byte offsets for channel read bodies */
enum {
  OFF_CHAN_NAME = AUTH_SIZE,
  OFF_CHAN_ID = AUTH_SIZE + CHANNEL_NAME_SIZE,
  OFF_CHAN_USER_LEN = AUTH_SIZE + CHANNEL_NAME_SIZE + 1,
  OFF_CHAN_USERS = AUTH_SIZE + CHANNEL_NAME_SIZE + 2
};

/* serialize an ipv4 address into a buffer */
static void serialize_ipv4(const struct ipv4_address *ip, uint8_t *buffer) {
  buffer[0] = ip->a;
  buffer[1] = ip->b;
  buffer[2] = ip->c;
  buffer[3] = ip->d;
}

/* deserialize an ipv4 address from a buffer */
static void deserialize_ipv4(const uint8_t *buffer, struct ipv4_address *ip) {
  ip->a = buffer[0];
  ip->b = buffer[1];
  ip->c = buffer[2];
  ip->d = buffer[3];
}

/* pack the protocol header into 8 bytes of wire format */
void serialize_header(const struct protocol_header *header, uint8_t *buffer) {
  uint32_t network_size;

  buffer[HDR_VERSION] = header->version;
  buffer[HDR_TYPE] = header->type;
  buffer[HDR_STATUS] = header->status;
  buffer[HDR_RESERVED] = header->reserved;

  network_size = htonl(header->body_size);
  memcpy(buffer + HDR_SIZE, &network_size, sizeof(network_size));
}

/* unpack 8 bytes of wire format into a protocol header struct */
void deserialize_header(const uint8_t *buffer, struct protocol_header *header) {
  uint32_t network_size;

  header->version = buffer[HDR_VERSION];
  header->type = buffer[HDR_TYPE];
  header->status = buffer[HDR_STATUS];
  memcpy(&network_size, buffer + HDR_SIZE, sizeof(network_size));
  header->body_size = ntohl(network_size);
}

/* serialize a channel list request: auth + channel_id_len=0 */
void serialize_channel_list_request(const struct channel_list_request *message,
                                    uint8_t *buffer) {
  memcpy(buffer, &message->auth, AUTH_SIZE);
  buffer[AUTH_SIZE] = 0;
}

/* deserialize the channel list response from wire format */
void deserialize_channel_list_response(const uint8_t *buffer,
                                       struct channel_list_response *message) {
  memcpy(&message->auth, buffer, AUTH_SIZE);
  message->channel_id_len = buffer[AUTH_SIZE];
  memcpy(message->channel_ids, buffer + AUTH_SIZE + 1, message->channel_id_len);
}

/* serialize a channel read request: auth + channel_name + channel_id +
 * user_id_len=0 */
void serialize_channel_read_request(const struct channel_read_request *message,
                                    uint8_t *buffer) {
  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);

  memcpy(buffer + OFF_CHAN_NAME, message->channel_name, CHANNEL_NAME_SIZE);

  buffer[OFF_CHAN_ID] = message->channel_id;
  buffer[OFF_CHAN_USER_LEN] = 0;
}

/* deserialize a channel read response from wire format */
void deserialize_channel_read_response(const uint8_t *buffer,
                                       struct channel_read_response *message) {
  memcpy(message->auth.username, buffer, USERNAME_SIZE);
  memcpy(message->auth.password, buffer + USERNAME_SIZE, PASSWORD_SIZE);

  memcpy(message->channel_name, buffer + OFF_CHAN_NAME, CHANNEL_NAME_SIZE);

  message->channel_id = buffer[OFF_CHAN_ID];
  message->user_id_len = buffer[OFF_CHAN_USER_LEN];
  memcpy(message->user_ids, buffer + OFF_CHAN_USERS, message->user_id_len);
}

/* serialize a message create request into wire format */
void serialize_message_create_request(
    const struct message_create_request *message, uint8_t *buffer) {
  uint64_t timestamp_net;
  uint16_t len_net;

  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);

  timestamp_net = big_host_to_net_64(message->timestamp);
  memcpy(buffer + OFF_TIMESTAMP, &timestamp_net, TIMESTAMP_SIZE);

  len_net = htons(message->message_len);
  memcpy(buffer + OFF_MSG_LEN, &len_net, MESSAGE_LEN_FIELD_SIZE);

  buffer[OFF_MSG_CHANNEL] = message->channel_id;

  memcpy(buffer + OFF_MSG_CHANNEL + 1, message->message, message->message_len);
}

/* serialize a message read request into wire format */
void serialize_message_read_request(const struct message_read_request *message,
                                    uint8_t *buffer) {
  uint64_t timestamp_net;
  uint16_t len_net;

  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);

  timestamp_net = big_host_to_net_64(message->timestamp);
  memcpy(buffer + OFF_TIMESTAMP, &timestamp_net, TIMESTAMP_SIZE);

  len_net = htons(message->message_len);
  memcpy(buffer + OFF_MSG_LEN, &len_net, MESSAGE_LEN_FIELD_SIZE);

  buffer[OFF_MSG_CHANNEL] = message->channel_id;
  buffer[OFF_MSG_SENDER] = message->sender_id;
}

/* deserialize a message read response, capping message length to avoid overflow
 */
void deserialize_message_read_response(const uint8_t *buffer,
                                       struct message_read_response *message) {
  uint64_t timestamp_net;
  uint16_t len_net;
  uint16_t safe_len;

  memcpy(message->auth.username, buffer, USERNAME_SIZE);
  memcpy(message->auth.password, buffer + USERNAME_SIZE, PASSWORD_SIZE);

  memcpy(&timestamp_net, buffer + OFF_TIMESTAMP, TIMESTAMP_SIZE);
  message->timestamp = big_net_to_host_64(timestamp_net);

  memcpy(&len_net, buffer + OFF_MSG_LEN, MESSAGE_LEN_FIELD_SIZE);
  message->message_len = ntohs(len_net);

  message->channel_id = buffer[OFF_MSG_CHANNEL];
  message->sender_id = buffer[OFF_MSG_SENDER];

  safe_len = message->message_len;
  if (safe_len > MAX_MESSAGE_SIZE - 1) {
    safe_len = MAX_MESSAGE_SIZE - 1;
  }

  memcpy(message->message, buffer + OFF_MSG_DATA, safe_len);
  message->message[safe_len] = '\0';
}

/* serialize an account registration into wire format */
void serialize_account_registration(const struct account_registration *message,
                                    uint8_t *buffer) {
  memcpy(buffer, message->username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->password, PASSWORD_SIZE);
  buffer[AUTH_SIZE] = message->account_id;
}

/* deserialize an account registration from wire format */
void deserialize_account_registration(const uint8_t *buffer,
                                      struct account_registration *message) {
  memcpy(message->username, buffer, USERNAME_SIZE);
  memcpy(message->password, buffer + USERNAME_SIZE, PASSWORD_SIZE);
  message->account_id = buffer[AUTH_SIZE];
}

/* serialize a login or logout request into wire format */
void serialize_login_or_logout(const struct login_or_logout *message,
                               uint8_t *buffer) {
  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);
  serialize_ipv4(&message->ip, buffer + AUTH_SIZE);
  buffer[AUTH_SIZE + IPV4_SIZE] = message->account_status;
}

/* deserialize a login or logout response from wire format */
void deserialize_login_or_logout(const uint8_t *buffer,
                                 struct login_or_logout *message) {
  memcpy(message->auth.username, buffer, USERNAME_SIZE);
  memcpy(message->auth.password, buffer + USERNAME_SIZE, PASSWORD_SIZE);
  deserialize_ipv4(buffer + AUTH_SIZE, &message->ip);
  message->account_status = buffer[AUTH_SIZE + IPV4_SIZE];
}

/* deserialize a server registration from wire format */
void deserialize_server_registration(const uint8_t *buffer,
                                     struct server_registration *message) {
  deserialize_ipv4(buffer, &message->ip);
  message->server_id = buffer[IPV4_SIZE];
}
