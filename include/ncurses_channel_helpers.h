//
// Created by Adnan Abdulle on 2026-04-09.
//

#ifndef NCURSES_CHANNEL_HELPERS_H
#define NCURSES_CHANNEL_HELPERS_H

#include "ncurses_input.h"
#include "recv_buffer.h"
#include <stdint.h>

uint64_t current_time_ms(void);

void load_history(int server, const char *username, const char *password,
                  uint8_t channel_id,
                  struct channel_message_state *msg_state);

int handle_server_input(struct recv_buffer *recv_buf, int server,
                        struct channel_message_state *msg_state);

int handle_user_input(int *focus, char *input, int *input_len, int server,
                      const char *username, const char *password,
                      uint8_t channel_id,
                      struct channel_message_state *msg_state);

#endif
