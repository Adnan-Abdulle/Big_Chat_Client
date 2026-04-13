//
// Created by Adnan Abdulle on 2026-04-08.
//

#include "ncurses_render.h"

static int get_bottom_index(const struct channel_message_state *msg_state) {
  return msg_state->message_count - msg_state->scroll_offset;
}

static int get_top_index(int bottom, int height) {
  int top;

  top = bottom - height;
  if (top < 0) {
    top = 0;
  }

  return top;
}

static void draw_one_message(WINDOW *msg_win,
                             const struct channel_message_state *msg_state,
                             int row, int message_index) {
  const struct stored_message *m;

  m = &msg_state->messages[message_index];

  if (message_index == msg_state->index) {
    wattron(msg_win, A_REVERSE);
  }

  mvwprintw(msg_win, row, 0, "%u: %s", m->sender_id, m->text);

  if (message_index == msg_state->index) {
    wattroff(msg_win, A_REVERSE);
  }
}

static void draw_visible_messages(WINDOW *msg_win,
                                  const struct channel_message_state *msg_state,
                                  int top, int bottom) {
  for (int i = top; i < bottom; i++) {
    draw_one_message(msg_win, msg_state, i - top, i);
  }
}
void draw_messages(WINDOW *msg_win,
                   const struct channel_message_state *msg_state) {
  int height;
  int width;
  int bottom;
  int top;

  werase(msg_win);
  getmaxyx(msg_win, height, width);
  (void)width;

  if (msg_state->message_count == 0) {
    wrefresh(msg_win);
    return;
  }

  bottom = get_bottom_index(msg_state);
  top = get_top_index(bottom, height);

  if (bottom > msg_state->message_count) {
    bottom = msg_state->message_count;
  }

  draw_visible_messages(msg_win, msg_state, top, bottom);
  wrefresh(msg_win);
}

enum { UI_CHAN_NAME_COL = 10 };

/* draw the channel header bar with back button and channel name */
void draw_channel_header(WINDOW *header_win, int focus,
                         const char *channel_name) {
  werase(header_win);

  if (focus == 0) {
    wattron(header_win, A_REVERSE);
  }
  mvwprintw(header_win, 0, 0, "< Back");
  if (focus == 0) {
    wattroff(header_win, A_REVERSE);
  }

  mvwprintw(header_win, 0, UI_CHAN_NAME_COL, "%.16s", channel_name);
  wrefresh(header_win);
}

/* draw the input bar with the current input text */
void draw_channel_input_bar(WINDOW *input_win, int focus, const char *input) {
  werase(input_win);
  box(input_win, 0, 0);

  if (focus == FOCUS_INPUT) {
    wattron(input_win, A_REVERSE);
  }
  mvwprintw(input_win, 1, 1, "> %s", input);
  if (focus == FOCUS_INPUT) {
    wattroff(input_win, A_REVERSE);
  }

  wrefresh(input_win);
}
