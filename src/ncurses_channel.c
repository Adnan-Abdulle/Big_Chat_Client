#include "client.h"
#include "ncurses_channel_helpers.h"
#include "ncurses_input.h"
#include "ncurses_pages.h"
#include "ncurses_render.h"
#include "protocol.h"
#include "recv_buffer.h"
#include <ncurses.h>
#include <string.h>

/* layout constants for the channel page */
enum {
  UI_HEADER_WIN_H = 2,
  UI_INPUT_WIN_H = 3,
  UI_INPUT_WIN_PAD = 3,
  UI_MSG_WIN_PAD = 5,
  UI_POLL_DELAY_MS = 10
};

enum { INPUT_BUF_SIZE = 1024 };


/* display the channel messages and handle sending new ones */
void channel_page(int server, const char *username, const char *password,
                  uint8_t channel_id) {
  static struct recv_buffer recv_buf;
  static struct channel_message_state msg_state;

  struct channel_read_response response;
  int height;
  int width;
  WINDOW *header_win;
  WINDOW *msg_win;
  WINDOW *input_win;
  int focus;
  char input[INPUT_BUF_SIZE];
  int input_len;

  clear();
  refresh();

  memset(&response, 0, sizeof(response));
  memset(&msg_state, 0, sizeof(msg_state));
  msg_state.index = 0;
  msg_state.editing = 0;
  msg_state.editing_timestamp = 0;

  channel_read_request(server, username, password, channel_id);
  channel_read_response(server, &response);

  load_history(server, username, password, channel_id, &msg_state);


  getmaxyx(stdscr, height, width);

  header_win = newwin(UI_HEADER_WIN_H, width, 0, 0);
  msg_win = newwin(height - UI_MSG_WIN_PAD, width, UI_HEADER_WIN_H, 0);
  input_win = newwin(UI_INPUT_WIN_H, width, height - UI_INPUT_WIN_PAD, 0);

  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  focus = FOCUS_INPUT;
  memset(input, 0, sizeof(input));
  input_len = 0;

  recv_buffer_init(&recv_buf);

  while (1) {
    draw_channel_header(header_win, focus, response.channel_name);

    if (handle_server_input(&recv_buf, server, &msg_state) == -1) {
      break;
    }

    draw_messages(msg_win, &msg_state);

    if (handle_user_input(&focus, input, &input_len, server, username, password,
                          channel_id, &msg_state) == -1) {
      break;
    }

    draw_channel_input_bar(input_win, focus, input);
    napms(UI_POLL_DELAY_MS);
  }

  delwin(header_win);
  delwin(msg_win);
  delwin(input_win);
}
