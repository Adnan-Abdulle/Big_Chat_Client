#ifndef NCURSES_INPUT_H
#define NCURSES_INPUT_H

#include "protocol.h"
#include <ncurses.h>
#include <stdint.h>

struct recv_buffer;

enum { INPUT_ACTION_NONE = 0, INPUT_ACTION_BACK = 1 };

enum { MAX_STORED_MESSAGES = 256 };

struct stored_message {
  uint8_t sender_id;
  uint64_t timestamp;
  char text[MAX_MESSAGE_SIZE];
};

struct channel_message_state {
  struct stored_message messages[MAX_STORED_MESSAGES];
  int message_count;
  int scroll_offset;
  uint64_t last_sent_timestamp;
};

/* process a keypress in the channel page, returns INPUT_ACTION_BACK to exit */
int handle_channel_input(int ch, int *focus, char *input, int *input_len,
                         int server, const char *username,
                         const char *password, uint8_t channel_id,
                         struct channel_message_state *msg_state);

/* drain all complete messages from the recv buffer into the message state */
void drain_incoming_messages(struct recv_buffer *recv_buf,
                             struct channel_message_state *msg_state);

/* draw the stored messages based on current scroll position */
void draw_messages(WINDOW *msg_win,
                   const struct channel_message_state *msg_state);

/* draw the channel header bar with back button and channel name */
void draw_channel_header(WINDOW *header_win, int focus,
                         const char *channel_name);

/* draw the input bar with the current input text */
void draw_channel_input_bar(WINDOW *input_win, int focus, const char *input);

/* handle page up scrolling */
void handle_scroll_up(struct channel_message_state *msg_state);

/* handle page down scrolling */
void handle_scroll_down(struct channel_message_state *msg_state);

#endif
