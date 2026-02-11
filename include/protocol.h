#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

enum { PROTOCOL_VERSION = 0x01 };

enum { HEADER_SIZE = 8 };
enum { USERNAME_SIZE = 16, PASSWORD_SIZE = 16 };

enum { SERVER_REGISTRATION_BODY_SIZE = 5 };
enum { SERVER_HEALTH_CHECK_BODY_SIZE = 5 };
enum { SERVER_ACTIVATION_BODY_SIZE = 1 };
enum { SERVER_DEACTIVATION_BODY_SIZE = 1 };
enum { ACCOUNT_REGISTRATION_BODY_SIZE = 33 };
enum { LOGIN_OR_LOGOUT_BODY_SIZE = 22 };

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
    MESSAGE_TYPE_LOG_RESPONSE = 0x19
};

enum {
    STATUS_OK = 0x00,
    STATUS_SENDER_ERROR = 0x10,
    STATUS_RECEIVER_ERROR = 0x20
};

enum {
    ACCOUNT_STATUS_OFFLINE = 0x00,
    ACCOUNT_STATUS_ONLINE = 0x01
};

struct protocol_header {
    uint8_t version;
    uint8_t type;
    uint8_t status;
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

struct login_or_logout {
    char password[PASSWORD_SIZE];
    uint8_t account_id;
    uint8_t account_status;
    struct ipv4_address ip;
};

void serialize_header(const struct protocol_header *header, uint8_t *buffer);
void deserialize_header(const uint8_t *buffer, struct protocol_header *header);

void serialize_server_registration(const struct server_registration *message,
                                   uint8_t *buffer);
void deserialize_server_registration(const uint8_t *buffer,
                                     struct server_registration *message);

void serialize_server_activation(const struct server_activation *message,
                                 uint8_t *buffer);
void deserialize_server_activation(const uint8_t *buffer,
                                   struct server_activation *message);

void serialize_account_registration(const struct account_registration *message,
                                    uint8_t *buffer);
void deserialize_account_registration(const uint8_t *buffer,
                                      struct account_registration *message);

void serialize_login_or_logout(const struct login_or_logout *message,
                               uint8_t *buffer);
void deserialize_login_or_logout(const uint8_t *buffer,
                                 struct login_or_logout *message);

size_t serialize_log_request(uint8_t server_id, const char *log_message,
                             uint8_t *buffer, size_t buffer_size);

uint32_t get_body_size_for_type(uint8_t message_type);

#endif
