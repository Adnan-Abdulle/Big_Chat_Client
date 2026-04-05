#include "ncurses_input.h"
#include <ncurses.h>

enum { UI_CHROME_ROWS = 5 };

/* handle page up scrolling */
void handle_scroll_up(struct channel_message_state *msg_state) {
  int msg_win_height;

  msg_win_height = LINES - UI_CHROME_ROWS;
  if (msg_win_height < 1) {
    msg_win_height = 1;
  }

  msg_state->scroll_offset += msg_win_height;
  if (msg_state->scroll_offset > msg_state->message_count - 1) {
    msg_state->scroll_offset = msg_state->message_count - 1;
  }
  if (msg_state->scroll_offset < 0) {
    msg_state->scroll_offset = 0;
  }
}

/* handle page down scrolling */
void handle_scroll_down(struct channel_message_state *msg_state) {
  int msg_win_height;

  msg_win_height = LINES - UI_CHROME_ROWS;
  if (msg_win_height < 1) {
    msg_win_height = 1;
  }

  msg_state->scroll_offset -= msg_win_height;
  if (msg_state->scroll_offset < 0) {
    msg_state->scroll_offset = 0;
  }
}
