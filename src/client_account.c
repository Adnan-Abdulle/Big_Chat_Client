#include "../include/client_handler.h"
#include "../include/manager.h"
#include "../include/network.h"
#include "../include/protocol.h"
#include "../include/server.h"
#include <stdio.h>
#include <string.h>

/* send a response header + optional body to a client */
static int send_response(int client_file_descriptor, uint8_t message_type,
                         uint8_t status_code, const uint8_t *body,
                         uint32_t body_size) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;

  header.version = PROTOCOL_VERSION;
  header.type = message_type;
  header.status = status_code;
  header.body_size = body_size;
  serialize_header(&header, header_buffer);

  if (write_exact(client_file_descriptor, header_buffer, HEADER_SIZE) <= 0) {
    return -1;
  }

  if (body_size > 0 && body != NULL) {
    if (write_exact(client_file_descriptor, body, body_size) <= 0) {
      return -1;
    }
  }

  return 0;
}

/* check one account slot against a padded username */
static int check_username_match(const struct account_record *record,
                                const char *padded_username) {
  if (record->account_id == 0) {
    return 0;
  }
  if (memcmp(record->username, padded_username, USERNAME_SIZE) == 0) {
    return 1;
  }
  return 0;
}

/* find an account by username, returns the account_id or 0 if not found */
static uint8_t find_account_by_username(const struct server_state *state,
                                        const char *username) {
  int i;
  char padded[USERNAME_SIZE];
  size_t length;

  memset(padded, 0, USERNAME_SIZE);
  length = strlen(username);
  if (length > USERNAME_SIZE) {
    length = USERNAME_SIZE;
  }
  memcpy(padded, username, length);

  for (i = 1; i < MAX_ACCOUNTS; i++) {
    if (check_username_match(&state->accounts[i], padded)) {
      return state->accounts[i].account_id;
    }
  }

  return 0;
}

/* handle an account registration request */
int handle_account_registration(int client_file_descriptor, const uint8_t *body,
                                struct server_state *state) {
  struct account_registration request;
  uint8_t response_body[ACCOUNT_REGISTRATION_BODY_SIZE];
  struct account_registration response;
  uint8_t new_id;

  deserialize_account_registration(body, &request);

  if (find_account_by_username(state, request.username) != 0) {
    printf("  account registration failed: username already exists\n");
    memcpy(response.username, request.username, USERNAME_SIZE);
    memset(response.password, 0, PASSWORD_SIZE);
    response.account_id = 0;
    serialize_account_registration(&response, response_body);
    return send_response(
        client_file_descriptor, MESSAGE_TYPE_ACCOUNT_REGISTRATION_RESPONSE,
        STATUS_SENDER_ERROR, response_body, ACCOUNT_REGISTRATION_BODY_SIZE);
  }

  new_id = state->next_account_id;
  if (new_id == 0) {
    printf("  account registration failed: no IDs available\n");
    return send_response(client_file_descriptor,
                         MESSAGE_TYPE_ACCOUNT_REGISTRATION_RESPONSE,
                         STATUS_RECEIVER_ERROR, NULL, 0);
  }

  memcpy(state->accounts[new_id].username, request.username, USERNAME_SIZE);
  memcpy(state->accounts[new_id].password, request.password, PASSWORD_SIZE);
  state->accounts[new_id].account_id = new_id;
  state->accounts[new_id].status = ACCOUNT_STATUS_OFFLINE;

  if (new_id < UINT8_MAX) {
    state->next_account_id = (uint8_t)(new_id + 1);
  } else {
    state->next_account_id = 0;
  }

  memcpy(response.username, request.username, USERNAME_SIZE);
  memset(response.password, 0, PASSWORD_SIZE);
  response.account_id = new_id;
  serialize_account_registration(&response, response_body);

  printf("  account registered: id=%u username=%.16s\n", new_id,
         request.username);
  send_log_to_manager(state->manager_file_descriptor, state->server_id,
                      "new account registered");

  return send_response(client_file_descriptor,
                       MESSAGE_TYPE_ACCOUNT_REGISTRATION_RESPONSE, STATUS_OK,
                       response_body, ACCOUNT_REGISTRATION_BODY_SIZE);
}

/* handle a login or logout request */
int handle_login_or_logout(int client_file_descriptor, const uint8_t *body,
                           struct server_state *state) {
  struct login_or_logout request;
  uint8_t response_body[LOGIN_OR_LOGOUT_BODY_SIZE];
  struct login_or_logout response;
  struct account_record *account;

  deserialize_login_or_logout(body, &request);

  if (request.account_id == 0 ||
      state->accounts[request.account_id].account_id == 0) {
    printf("  login/logout failed: account %u not found\n", request.account_id);
    memset(response.password, 0, PASSWORD_SIZE);
    response.account_id = request.account_id;
    response.account_status = ACCOUNT_STATUS_OFFLINE;
    response.ip = request.ip;
    serialize_login_or_logout(&response, response_body);
    return send_response(
        client_file_descriptor, MESSAGE_TYPE_LOGIN_OR_LOGOUT_RESPONSE,
        STATUS_SENDER_ERROR, response_body, LOGIN_OR_LOGOUT_BODY_SIZE);
  }

  account = &state->accounts[request.account_id];

  if (memcmp(account->password, request.password, PASSWORD_SIZE) != 0) {
    printf("  login/logout failed: wrong password for account %u\n",
           request.account_id);
    memset(response.password, 0, PASSWORD_SIZE);
    response.account_id = request.account_id;
    response.account_status = account->status;
    response.ip = request.ip;
    serialize_login_or_logout(&response, response_body);
    return send_response(
        client_file_descriptor, MESSAGE_TYPE_LOGIN_OR_LOGOUT_RESPONSE,
        STATUS_SENDER_ERROR, response_body, LOGIN_OR_LOGOUT_BODY_SIZE);
  }

  account->status = request.account_status;

  memset(response.password, 0, PASSWORD_SIZE);
  response.account_id = request.account_id;
  response.account_status = request.account_status;
  response.ip = request.ip;
  serialize_login_or_logout(&response, response_body);

  if (request.account_status == ACCOUNT_STATUS_ONLINE) {
    printf("  account %u logged in\n", request.account_id);
    send_log_to_manager(state->manager_file_descriptor, state->server_id,
                        "user logged in");
  } else {
    printf("  account %u logged out\n", request.account_id);
    send_log_to_manager(state->manager_file_descriptor, state->server_id,
                        "user logged out");
  }

  return send_response(client_file_descriptor,
                       MESSAGE_TYPE_LOGIN_OR_LOGOUT_RESPONSE, STATUS_OK,
                       response_body, LOGIN_OR_LOGOUT_BODY_SIZE);
}
