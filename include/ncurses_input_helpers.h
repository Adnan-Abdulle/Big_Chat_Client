//
// Created by Adnan Abdulle on 2026-04-08.
//

#ifndef MAIN_NCURSES_INPUT_HELPERS_H
#define MAIN_NCURSES_INPUT_HELPERS_H

#ifndef NCURSES_HELPERS_H
#define NCURSES_HELPERS_H

#include "ncurses_input.h"
#include <stdint.h>

enum {
    INPUT_MAX_LEN = 1023,
    ASCII_PRINT_MIN = 32,
    ASCII_PRINT_MAX = 126,
    KEY_BS_ALT = 127
};

void handle_messages_focus(int ch, int *focus, char *input,
                           int *input_len, int server,
                           const char *username, const char *password,
                           uint8_t channel_id,
                           struct channel_message_state *msg_state);

int handle_back_focus(int ch, int *focus);

void handle_input_focus(int ch, int *focus, char *input, int *input_len,
                        int server, const char *username,
                        const char *password, uint8_t channel_id,
                        struct channel_message_state *msg_state);

void handle_parsed_message(const struct protocol_header *msg_header,
                           const uint8_t *body,
                           struct channel_message_state *msg_state);

#endif
#endif //MAIN_NCURSES_INPUT_HELPERS_H
