/*
 * Created by Adnan Abdulle on 2026-04-14.
 */

#ifndef MAIN_NCURSES_POPUP_INPUT_H
#define MAIN_NCURSES_POPUP_INPUT_H

#include "protocol.h"
#include <ncurses.h>

enum { ASCII_DEL = 127, ASCII_PRINTABLE_MIN = 32, ASCII_PRINTABLE_MAX = 126 };

enum { INPUT_CONTINUE = 0, INPUT_QUIT = 1, INPUT_SUBMIT = 2 };

int handle_create_popup_key(int ch, char *channel_name, int *input_len);

#endif /* MAIN_NCURSES_POPUP_INPUT_H */
