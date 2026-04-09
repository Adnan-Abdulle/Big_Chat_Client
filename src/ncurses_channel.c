#include "client.h"
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
enum { HISTORY_LIMIT = 50 };

/* load chat history from the server and store in message state */
static void load_history(int server, const char *username, const char *password,
                         uint8_t channel_id,
                         struct channel_message_state *msg_state) {
  static struct get_history_response history;
  static struct message_read_response msg;

  int result;

  memset(&history, 0, sizeof(history));

  get_history_request(server, username, password, 0, channel_id, HISTORY_LIMIT);
  result = get_history_response(server, &history);
  if (result == -1) {
    return;
  }

  for (int i = 0; i < history.result_len; i++) {
    message_read_request(server, username, password, history.timestamps[i],
                         channel_id, history.sender_ids[i]);

    memset(&msg, 0, sizeof(msg));
    result = message_read_response(server, &msg);
    if (result == 0) {
      msg_state->messages[msg_state->message_count].sender_id = msg.sender_id;
      msg_state->messages[msg_state->message_count].timestamp = msg.timestamp;
      memset(msg_state->messages[msg_state->message_count].text, 0,
             MAX_MESSAGE_SIZE);
      strncpy(msg_state->messages[msg_state->message_count].text, msg.message,
              MAX_MESSAGE_SIZE - 1);
      msg_state->message_count++;
      if (msg_state->message_count >= MAX_STORED_MESSAGES) {
        break;
      }
    }
  }
}

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
    int ch;
    int recv_result;

    draw_channel_header(header_win, focus, response.channel_name);

    recv_result = recv_buffer_read(&recv_buf, server);
    if (recv_result == RECV_EOF || recv_result == RECV_ERROR) {
      break;
    }

    drain_incoming_messages(&recv_buf, &msg_state);
    draw_messages(msg_win, &msg_state);

    ch = getch();

    if (ch != ERR) {
      if (handle_channel_input(ch, &focus, input, &input_len, server, username,
                               password, channel_id,
                               &msg_state) == INPUT_ACTION_BACK) {
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
