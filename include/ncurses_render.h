//
// Created by Adnan Abdulle on 2026-04-08.
//

#ifndef MAIN_NCURSES_RENDER_H
#define MAIN_NCURSES_RENDER_H
#include "ncurses_input.h"
#include <ncurses.h>

/* draw the stored messages based on current scroll position */
void draw_messages(WINDOW *msg_win,
                   const struct channel_message_state *msg_state);

/* draw the channel header bar with back button and channel name */
void draw_channel_header(WINDOW *header_win, int focus,
                         const char *channel_name);

/* draw the input bar with the current input text */
void draw_channel_input_bar(WINDOW *input_win, int focus, const char *input);

#endif //MAIN_NCURSES_RENDER_H
