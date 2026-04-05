#ifndef NCURSES_COMMANDS_H
#define NCURSES_COMMANDS_H

#include "ncurses_input.h"
#include <stdint.h>

/* add a message to the state, dropping the oldest if full */
void store_message(struct channel_message_state *state, uint8_t sender_id,
                   uint64_t timestamp, const char *text);

/* handle the /edit command */
void handle_edit_command(const char *input, int *input_len, int server,
                         const char *username, const char *password,
                         uint8_t channel_id,
                         const struct channel_message_state *msg_state);

/* handle the /delete command */
void handle_delete_command(int *input_len, int server, const char *username,
                           const char *password, uint8_t channel_id,
                           struct channel_message_state *msg_state);

/* handle sending a normal message */
void handle_send_message(const char *input, int *input_len, int server,
                         const char *username, const char *password,
                         uint8_t channel_id,
                         struct channel_message_state *msg_state);

#endif
