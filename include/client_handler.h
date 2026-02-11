#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <stdint.h>

struct server_state;

/* read one message from a client and handle it, returns -1 on connection loss */
int handle_client_message(int client_file_descriptor,
                          struct server_state *state);

/* handle an account registration request (defined in client_account.c) */
int handle_account_registration(int client_file_descriptor,
                                const uint8_t *body,
                                struct server_state *state);

/* handle a login or logout request (defined in client_account.c) */
int handle_login_or_logout(int client_file_descriptor, const uint8_t *body,
                           struct server_state *state);

#endif
