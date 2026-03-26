#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <arpa/inet.h>
#include <stddef.h>
#include <stdint.h>

/* portable big-endian 64-bit conversion using only POSIX htonl/ntohl */
static inline uint64_t big_host_to_net_64(uint64_t host) {
  uint32_t high;
  uint32_t low;

  high = htonl((uint32_t)(host >> 32));
  low = htonl((uint32_t)(host & 0xFFFFFFFFU));
  return ((uint64_t)low << 32) | high;
}

static inline uint64_t big_net_to_host_64(uint64_t net) {
  uint32_t high;
  uint32_t low;

  high = (uint32_t)(net & 0xFFFFFFFFU);
  low = (uint32_t)(net >> 32);
  return ((uint64_t)ntohl(high) << 32) | ntohl(low);
}

enum { PROTOCOL_VERSION = 0x02 };

enum { HEADER_SIZE = 8 };
enum { USERNAME_SIZE = 16, PASSWORD_SIZE = 16, CHANNEL_NAME_SIZE = 16 };
enum { AUTH_SIZE = USERNAME_SIZE + PASSWORD_SIZE };
enum { MAX_IDS = 256 };
enum { TIMESTAMP_SIZE = 8 };
enum { MESSAGE_LEN_FIELD_SIZE = 2 };
enum { IPV4_SIZE = 4 };
enum { IP_STRING_SIZE = 16 };

enum { SERVER_REGISTRATION_BODY_SIZE = IPV4_SIZE + 1 };
enum { SERVER_HEALTH_CHECK_BODY_SIZE = IPV4_SIZE + 1 };
enum { SERVER_ACTIVATION_BODY_SIZE = IPV4_SIZE + 1 };
enum { SERVER_DEACTIVATION_BODY_SIZE = IPV4_SIZE + 1 };
enum { ACCOUNT_REGISTRATION_BODY_SIZE = AUTH_SIZE + 1 };
enum { LOGIN_OR_LOGOUT_BODY_SIZE = AUTH_SIZE + IPV4_SIZE + 1 };
enum { CHANNEL_LIST_REQUEST_BODY_SIZE = AUTH_SIZE + 1 };
enum { CHANNEL_READ_REQUEST_BODY_SIZE = AUTH_SIZE + CHANNEL_NAME_SIZE + 1 + 1 };
enum {
  CHANNEL_RESPONSE_MAX_BODY_SIZE = CHANNEL_READ_REQUEST_BODY_SIZE + MAX_IDS
};
enum { MAX_MESSAGE_SIZE = 2048 };
enum {
  MIN_MESSAGE_CREATE_BODY_SIZE =
      AUTH_SIZE + TIMESTAMP_SIZE + MESSAGE_LEN_FIELD_SIZE + 1
};
enum {
  MIN_MESSAGE_READ_RESPONSE_BODY_SIZE =
      AUTH_SIZE + TIMESTAMP_SIZE + MESSAGE_LEN_FIELD_SIZE + 1 + 1
};
enum {
  MESSAGE_READ_REQUEST_BODY_SIZE =
      AUTH_SIZE + TIMESTAMP_SIZE + MESSAGE_LEN_FIELD_SIZE + 1 + 1
};

enum {
  MESSAGE_TYPE_SERVER_REGISTRATION_REQUEST = 0x00,
  MESSAGE_TYPE_SERVER_REGISTRATION_RESPONSE = 0x01,
  MESSAGE_TYPE_SERVER_HEALTH_CHECK_REQUEST = 0x04,
  MESSAGE_TYPE_SERVER_HEALTH_CHECK_RESPONSE = 0x05,
  MESSAGE_TYPE_SERVER_ACTIVATION_REQUEST = 0x08,
  MESSAGE_TYPE_SERVER_ACTIVATION_RESPONSE = 0x09,
  MESSAGE_TYPE_GET_ACTIVATED_SERVER_REQUEST = 0x0A,
  MESSAGE_TYPE_GET_ACTIVATED_SERVER_RESPONSE = 0x0B,
  MESSAGE_TYPE_SERVER_DEACTIVATION_REQUEST = 0x0E,
  MESSAGE_TYPE_SERVER_DEACTIVATION_RESPONSE = 0x0F,
  MESSAGE_TYPE_ACCOUNT_REGISTRATION_REQUEST = 0x10,
  MESSAGE_TYPE_ACCOUNT_REGISTRATION_RESPONSE = 0x11,
  MESSAGE_TYPE_LOGIN_OR_LOGOUT_REQUEST = 0x14,
  MESSAGE_TYPE_LOGIN_OR_LOGOUT_RESPONSE = 0x15,
  MESSAGE_TYPE_LOG_REQUEST = 0x18,
  MESSAGE_TYPE_LOG_RESPONSE = 0x19,

  MESSAGE_TYPE_CHANNEL_READ_REQUEST = 0x22,
  MESSAGE_TYPE_CHANNEL_READ_RESPONSE = 0x23,
  MESSAGE_TYPE_CHANNEL_LIST_READ_REQUEST = 0x2A,
  MESSAGE_TYPE_CHANNEL_LIST_READ_RESPONSE = 0x2B,
  MESSAGE_TYPE_MESSAGE_CREATE_REQUEST = 0x30,
  MESSAGE_TYPE_MESSAGE_CREATE_RESPONSE = 0x31,
  MESSAGE_TYPE_MESSAGE_READ_REQUEST = 0x32,
  MESSAGE_TYPE_MESSAGE_READ_RESPONSE = 0x33

};

enum {
  STATUS_OK = 0x00,
  STATUS_INVALID_VERSION = 0x40,
  STATUS_INVALID_TYPE = 0x41,
  STATUS_INVALID_SIZE = 0x42,
  STATUS_MALFORMED_REQUEST = 0x43,
  STATUS_INVALID_CREDENTIALS = 0x44,
  STATUS_NOT_FOUND = 0x45,
  STATUS_ALREADY_EXISTS = 0x46,
  STATUS_NOT_REGISTERED = 0x47,
  STATUS_FORBIDDEN = 0x48,
  STATUS_NOT_CHANNEL_MEMBER = 0x49,
  STATUS_INTERNAL_ERROR = 0x80,
  STATUS_SERVICE_UNAVAILABLE = 0x81,
  STATUS_RESOURCE_EXHAUSTED = 0x82,
  STATUS_MESSAGE_TOO_LARGE = 0x83,
  STATUS_TIMEOUT = 0x84
};

enum {
  STATUS_CATEGORY_MASK = 0xC0,
  STATUS_CATEGORY_SUCCESS = 0x00,
  STATUS_CATEGORY_SENDER_ERROR = 0x40,
  STATUS_CATEGORY_RECEIVER_ERROR = 0x80
};

enum { ACCOUNT_STATUS_OFFLINE = 0x00, ACCOUNT_STATUS_ONLINE = 0x01 };

struct protocol_header {
  uint8_t version;
  uint8_t type;
  uint8_t status;
  uint8_t reserved;
  uint32_t body_size;
};

struct ipv4_address {
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
};

struct server_registration {
  struct ipv4_address ip;
  uint8_t server_id;
};

struct server_activation {
  uint8_t server_id;
};

struct account_registration {
  char username[USERNAME_SIZE];
  char password[PASSWORD_SIZE];
  uint8_t account_id;
};

struct auth {
  char username[USERNAME_SIZE];
  char password[PASSWORD_SIZE];
};

struct login_or_logout {
  struct auth auth;
  struct ipv4_address ip;
  uint8_t account_status;
};

struct channel_list_request {
  struct auth auth;
};

struct channel_list_response {
  struct auth auth;
  uint8_t channel_id_len;
  uint8_t channel_ids[MAX_IDS];
};

struct channel_read_request {
  struct auth auth;
  char channel_name[CHANNEL_NAME_SIZE];
  uint8_t channel_id;
  uint8_t user_id_len;
};

struct channel_read_response {
  struct auth auth;
  char channel_name[CHANNEL_NAME_SIZE];
  uint8_t channel_id;
  uint8_t user_id_len;
  uint8_t user_ids[MAX_IDS];
};

struct message_create_request {
  struct auth auth;
  uint64_t timestamp;
  uint16_t message_len;
  uint8_t channel_id;
  char message[MAX_MESSAGE_SIZE];
};

struct message_read_request {
  struct auth auth;
  uint64_t timestamp;
  uint16_t message_len;
  uint8_t channel_id;
  uint8_t sender_id;
};

struct message_read_response {
  struct auth auth;
  uint64_t timestamp;
  uint16_t message_len;
  uint8_t channel_id;
  uint8_t sender_id;
  char message[MAX_MESSAGE_SIZE];
};

void serialize_header(const struct protocol_header *header, uint8_t *buffer);
void deserialize_header(const uint8_t *buffer, struct protocol_header *header);

void serialize_account_registration(const struct account_registration *message,
                                    uint8_t *buffer);
void deserialize_account_registration(const uint8_t *buffer,
                                      struct account_registration *message);

void serialize_login_or_logout(const struct login_or_logout *message,
                               uint8_t *buffer);
void deserialize_login_or_logout(const uint8_t *buffer,
                                 struct login_or_logout *message);

uint32_t get_body_size_for_type(uint8_t message_type);

void deserialize_server_registration(const uint8_t *buffer,
                                     struct server_registration *message);

void serialize_channel_list_request(const struct channel_list_request *message,
                                    uint8_t *buffer);

void deserialize_channel_list_response(const uint8_t *buffer,
                                       struct channel_list_response *message);

void serialize_channel_read_request(const struct channel_read_request *message,
                                    uint8_t *buffer);

void deserialize_channel_read_response(const uint8_t *buffer,
                                       struct channel_read_response *message);

void serialize_message_create_request(
    const struct message_create_request *message, uint8_t *buffer);

void serialize_message_read_request(const struct message_read_request *message,
                                    uint8_t *buffer);

void deserialize_message_read_response(const uint8_t *buffer,
                                       struct message_read_response *message);

#endif
