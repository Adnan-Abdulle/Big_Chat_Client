//
// Created by Adnan Abdulle on 2026-04-09.
//

#include "ncurses_channel_helpers.h"

#include "client.h"
#include "ncurses_input.h"
#include "protocol.h"
#include <ncurses.h>
#include <string.h>

enum { HISTORY_LIMIT = 50 };

void load_history(int server, const char *username, const char *password,
                  uint8_t channel_id, struct channel_message_state *msg_state) {
  static struct get_history_response history;
  static struct message_read_response msg;
  int result;

  memset(&history, 0, sizeof(history));

  msg_state->message_count = 0;
  msg_state->scroll_offset = 0;

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

  if (msg_state->message_count == 0) {
    msg_state->index = 0;
  } else if (msg_state->index >= msg_state->message_count) {
    msg_state->index = msg_state->message_count - 1;
  }
}

int handle_server_input(struct recv_buffer *recv_buf, int server,
                        struct channel_message_state *msg_state) {
  int recv_result;

  recv_result = recv_buffer_read(recv_buf, server);
  if (recv_result == RECV_EOF || recv_result == RECV_ERROR) {
    return -1;
  }

  drain_incoming_messages(recv_buf, msg_state);
  return 0;
}

int handle_user_input(int *focus, char *input, int *input_len, int server,
                      const char *username, const char *password,
                      uint8_t channel_id,
                      struct channel_message_state *msg_state) {
  int ch;

  ch = getch();
  if (ch == ERR) {
    return 0;
  }

  if (handle_channel_input(ch, focus, input, input_len, server, username,
                           password, channel_id,
                           msg_state) == INPUT_ACTION_BACK) {
    return -1;
  }

  return 0;
}
