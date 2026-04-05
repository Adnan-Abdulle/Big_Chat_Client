#include "protocol.h"
#include <arpa/inet.h>
#include <string.h>

/* byte offsets for get history bodies */
enum {
  OFF_HIST_LIMIT = AUTH_SIZE + TIMESTAMP_SIZE,
  OFF_HIST_LEN = AUTH_SIZE + TIMESTAMP_SIZE + 2,
  OFF_HIST_CHANNEL = AUTH_SIZE + TIMESTAMP_SIZE + 4,
  OFF_HIST_RESERVED = AUTH_SIZE + TIMESTAMP_SIZE + 5,
  OFF_HIST_RESERVED_END = AUTH_SIZE + TIMESTAMP_SIZE + 8
};

/* serialize a delete message request: auth + timestamp + channel_id */
void serialize_delete_message(const struct delete_message_request *message,
                              uint8_t *buffer) {
  uint64_t timestamp_net;

  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);

  timestamp_net = big_host_to_net_64(message->timestamp);
  memcpy(buffer + AUTH_SIZE, &timestamp_net, TIMESTAMP_SIZE);

  buffer[AUTH_SIZE + TIMESTAMP_SIZE] = message->channel_id;
}

/* serialize a get history request into wire format */
void serialize_get_history_request(const struct get_history_request *message,
                                   uint8_t *buffer) {
  uint64_t timestamp_net;
  uint16_t limit_net;
  uint16_t len_net;

  memcpy(buffer, message->auth.username, USERNAME_SIZE);
  memcpy(buffer + USERNAME_SIZE, message->auth.password, PASSWORD_SIZE);

  timestamp_net = big_host_to_net_64(message->start_timestamp);
  memcpy(buffer + AUTH_SIZE, &timestamp_net, TIMESTAMP_SIZE);

  limit_net = htons(message->result_len_limit);
  memcpy(buffer + OFF_HIST_LIMIT, &limit_net, sizeof(limit_net));

  len_net = htons(message->result_len);
  memcpy(buffer + OFF_HIST_LEN, &len_net, sizeof(len_net));

  buffer[OFF_HIST_CHANNEL] = message->channel_id;

  memset(buffer + OFF_HIST_RESERVED, 0,
         OFF_HIST_RESERVED_END - OFF_HIST_RESERVED);
}

/* deserialize a get history response from wire format */
void deserialize_get_history_response(const uint8_t *buffer,
                                      struct get_history_response *message) {
  uint64_t timestamp_net;
  uint16_t limit_net;
  uint16_t len_net;
  uint16_t safe_len;
  size_t timestamps_offset;
  size_t sender_ids_offset;

  memcpy(message->auth.username, buffer, USERNAME_SIZE);
  memcpy(message->auth.password, buffer + USERNAME_SIZE, PASSWORD_SIZE);

  memcpy(&timestamp_net, buffer + AUTH_SIZE, TIMESTAMP_SIZE);
  message->start_timestamp = big_net_to_host_64(timestamp_net);

  memcpy(&limit_net, buffer + OFF_HIST_LIMIT, sizeof(limit_net));
  message->result_len_limit = ntohs(limit_net);

  memcpy(&len_net, buffer + OFF_HIST_LEN, sizeof(len_net));
  message->result_len = ntohs(len_net);

  message->channel_id = buffer[OFF_HIST_CHANNEL];

  safe_len = message->result_len;
  if (safe_len > MAX_HISTORY_RESULTS) {
    safe_len = MAX_HISTORY_RESULTS;
  }
  message->result_len = safe_len;

  timestamps_offset = GET_HISTORY_FIXED_BODY_SIZE;
  sender_ids_offset =
      GET_HISTORY_FIXED_BODY_SIZE + ((size_t)safe_len * TIMESTAMP_SIZE);

  for (int i = 0; i < safe_len; i++) {
    memcpy(&timestamp_net,
           buffer + timestamps_offset + ((size_t)i * TIMESTAMP_SIZE),
           TIMESTAMP_SIZE);
    message->timestamps[i] = big_net_to_host_64(timestamp_net);
  }

  memcpy(message->sender_ids, buffer + sender_ids_offset, safe_len);
}

/* number of entries in the body size lookup table */
enum { BODY_SIZE_TABLE_ENTRIES = 19 };

/* lookup table entry mapping a message type to its body size */
struct body_size_entry {
  uint8_t message_type;
  uint32_t body_size;
};

/* table of message types with fixed body sizes */
static const struct body_size_entry body_size_table[BODY_SIZE_TABLE_ENTRIES] = {
    {MESSAGE_TYPE_SERVER_REGISTRATION_REQUEST, SERVER_REGISTRATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_REGISTRATION_RESPONSE, SERVER_REGISTRATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_HEALTH_CHECK_REQUEST, SERVER_REGISTRATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_HEALTH_CHECK_RESPONSE, SERVER_REGISTRATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_ACTIVATION_REQUEST, SERVER_ACTIVATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_ACTIVATION_RESPONSE, SERVER_ACTIVATION_BODY_SIZE},
    {MESSAGE_TYPE_GET_ACTIVATED_SERVER_REQUEST, SERVER_ACTIVATION_BODY_SIZE},
    {MESSAGE_TYPE_GET_ACTIVATED_SERVER_RESPONSE, SERVER_ACTIVATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_DEACTIVATION_REQUEST, SERVER_ACTIVATION_BODY_SIZE},
    {MESSAGE_TYPE_SERVER_DEACTIVATION_RESPONSE, SERVER_ACTIVATION_BODY_SIZE},
    {MESSAGE_TYPE_ACCOUNT_REGISTRATION_REQUEST, ACCOUNT_REGISTRATION_BODY_SIZE},
    {MESSAGE_TYPE_ACCOUNT_REGISTRATION_RESPONSE,
     ACCOUNT_REGISTRATION_BODY_SIZE},
    {MESSAGE_TYPE_LOGIN_OR_LOGOUT_REQUEST, LOGIN_OR_LOGOUT_BODY_SIZE},
    {MESSAGE_TYPE_LOGIN_OR_LOGOUT_RESPONSE, LOGIN_OR_LOGOUT_BODY_SIZE},
    {MESSAGE_TYPE_DELETE_USER_REQUEST, DELETE_USER_BODY_SIZE},
    {MESSAGE_TYPE_DELETE_USER_RESPONSE, DELETE_USER_BODY_SIZE},
    {MESSAGE_TYPE_CHANNEL_LIST_READ_REQUEST, CHANNEL_LIST_REQUEST_BODY_SIZE},
    {MESSAGE_TYPE_DELETE_MESSAGE_REQUEST, DELETE_MESSAGE_BODY_SIZE},
    {MESSAGE_TYPE_DELETE_MESSAGE_RESPONSE, DELETE_MESSAGE_BODY_SIZE},
};

/* return the expected body size for a given message type */
uint32_t get_body_size_for_type(uint8_t message_type) {
  for (int i = 0; i < BODY_SIZE_TABLE_ENTRIES; i++) {
    if (body_size_table[i].message_type == message_type) {
      return body_size_table[i].body_size;
    }
  }

  return 0;
}
