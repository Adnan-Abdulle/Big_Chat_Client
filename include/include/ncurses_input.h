#ifndef NCURSES_INPUT_H
#define NCURSES_INPUT_H

#include <ncurses.h>
#include <stdint.h>

struct recv_buffer;

enum { INPUT_ACTION_NONE = 0, INPUT_ACTION_BACK = 1 };

/* process a keypress in the channel page, returns INPUT_ACTION_BACK to exit */
int handle_channel_input(int ch, int *focus, char *input, int *input_len,
                         int server, const char *username,
                         const char *password, uint8_t channel_id);

/* drain all complete messages from the recv buffer and display them */
void drain_incoming_messages(struct recv_buffer *recv_buf, WINDOW *msg_win);

/* draw the channel header bar with back button and channel name */
void draw_channel_header(WINDOW *header_win, int focus,
                         const char *channel_name);

/* draw the input bar with the current input text */
void draw_channel_input_bar(WINDOW *input_win, int focus, const char *input);

#endif
