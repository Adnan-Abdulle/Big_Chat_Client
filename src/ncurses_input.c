#include "ncurses_input.h"
#include "client.h"
#include "ncurses_commands.h"
#include "protocol.h"
#include "recv_buffer.h"
#include <ncurses.h>
#include <string.h>

enum {
  INPUT_MAX_LEN = 1023,
  ASCII_PRINT_MIN = 32,
  ASCII_PRINT_MAX = 126,
  KEY_BS_ALT = 127
};

enum { EDIT_CMD_LEN = 6 };

/* process a keypress in the channel page, returns INPUT_ACTION_BACK to exit */
int handle_channel_input(int ch, int *focus, char *input, int *input_len,
                         int server, const char *username, const char *password,
                         uint8_t channel_id,
                         struct channel_message_state *msg_state) {

  if (ch == KEY_UP) {
    *focus = 0;
  } else if (ch == KEY_DOWN) {
    *focus = 1;
  } else if (ch == KEY_PPAGE) {
    handle_scroll_up(msg_state);
  } else if (ch == KEY_NPAGE) {
    handle_scroll_down(msg_state);
  } else if (*focus == 0 && (ch == '\n' || ch == KEY_ENTER)) {
    return INPUT_ACTION_BACK;
  } else if (*focus == 1) {
    if (ch == '\n') {
      if (strncmp(input, "/edit ", EDIT_CMD_LEN) == 0 &&
          *input_len > EDIT_CMD_LEN) {
        handle_edit_command(input, input_len, server, username, password,
                            channel_id, msg_state);
      } else if (strcmp(input, "/delete") == 0) {
        handle_delete_command(input_len, server, username, password, channel_id,
                              msg_state);
      } else if (*input_len > 0) {
        handle_send_message(input, input_len, server, username, password,
                            channel_id, msg_state);
      }
      input[0] = '\0';
      msg_state->scroll_offset = 0;
    } else if (ch == KEY_BACKSPACE || ch == KEY_BS_ALT) {
      if (*input_len > 0) {
        (*input_len)--;
        input[*input_len] = '\0';
      }
    } else if (*input_len < INPUT_MAX_LEN && ch >= ASCII_PRINT_MIN &&
               ch <= ASCII_PRINT_MAX) {
      input[(*input_len)++] = (char)ch;
      input[*input_len] = '\0';
    }
  }

  return INPUT_ACTION_NONE;
}

/* drain all complete messages from the recv buffer into the message state */
void drain_incoming_messages(struct recv_buffer *recv_buf,
                             struct channel_message_state *msg_state) {
  static uint8_t body[MIN_MESSAGE_READ_RESPONSE_BODY_SIZE + MAX_MESSAGE_SIZE];
  static struct message_read_response msg;

  struct protocol_header msg_header;
  uint32_t body_size;
  int parse_result;

  parse_result = recv_buffer_try_parse(recv_buf, &msg_header, body,
                                       sizeof(body), &body_size);
  while (parse_result != 0) {
    if (parse_result == 1 &&
        msg_header.type == MESSAGE_TYPE_MESSAGE_READ_RESPONSE &&
        msg_header.status == STATUS_OK) {
      memset(&msg, 0, sizeof(msg));
      deserialize_message_read_response(body, &msg);
      store_message(msg_state, msg.sender_id, msg.timestamp, msg.message);
    }
    parse_result = recv_buffer_try_parse(recv_buf, &msg_header, body,
                                         sizeof(body), &body_size);
  }
}

/* draw the stored messages based on current scroll position */
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

  bottom = msg_state->message_count - msg_state->scroll_offset;
  top = bottom - height;
  if (top < 0) {
    top = 0;
  }
  if (bottom > msg_state->message_count) {
    bottom = msg_state->message_count;
  }

  for (int i = top; i < bottom; i++) {
    const struct stored_message *m;

    m = &msg_state->messages[i];
    mvwprintw(msg_win, i - top, 0, "%u: %s", m->sender_id, m->text);
  }

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

  if (focus == 1) {
    wattron(input_win, A_REVERSE);
  }
  mvwprintw(input_win, 1, 1, "> %s", input);
  if (focus == 1) {
    wattroff(input_win, A_REVERSE);
  }

  wrefresh(input_win);
}
