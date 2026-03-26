#include "client.h"
#include "ncurses_input.h"
#include "ncurses_pages.h"
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

  channel_read_request(server, username, password, channel_id);
  channel_read_response(server, &response);

  message_read_request(server, username, password, 0, channel_id);

  getmaxyx(stdscr, height, width);

  header_win = newwin(UI_HEADER_WIN_H, width, 0, 0);
  msg_win = newwin(height - UI_MSG_WIN_PAD, width, UI_HEADER_WIN_H, 0);
  input_win = newwin(UI_INPUT_WIN_H, width, height - UI_INPUT_WIN_PAD, 0);

  scrollok(msg_win, TRUE);

  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  focus = 1;
  memset(input, 0, sizeof(input));
  input_len = 0;

  recv_buffer_init(&recv_buf);

  while (1) {
    int ch;
    int recv_result;

    draw_channel_header(header_win, focus, response.channel_name);

    recv_result = recv_buffer_read(&recv_buf, server);
    if (recv_result == RECV_EOF || recv_result == RECV_ERROR) {
      break;
    }

    drain_incoming_messages(&recv_buf, msg_win);

    ch = getch();

    if (ch != ERR) {
      if (handle_channel_input(ch, &focus, input, &input_len, server, username,
                               password, channel_id) == INPUT_ACTION_BACK) {
        break;
      }
    }

    draw_channel_input_bar(input_win, focus, input);

    napms(UI_POLL_DELAY_MS);
  }

  delwin(header_win);
  delwin(msg_win);
  delwin(input_win);
}
