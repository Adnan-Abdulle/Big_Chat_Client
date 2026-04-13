//
// Created by Adnan Abdulle on 2026-04-08.
//

#include "ncurses_input_helpers.h"
#include "client.h"
#include "ncurses_commands.h"
#include "protocol.h"
#include <ncurses.h>
#include <string.h>

static void handle_message_navigation(int ch, int *focus,
                                      struct channel_message_state *msg_state) {
  if (ch == KEY_UP) {
    if (msg_state->index > 0) {
      msg_state->index--;
    } else {
      *focus = FOCUS_BACK;
    }
  } else if (ch == KEY_DOWN) {
    if (msg_state->index < msg_state->message_count - 1) {
      msg_state->index++;
    } else {
      *focus = FOCUS_INPUT;
    }
  }
}
//depreciated
//static void remove_selected_message(struct channel_message_state *msg_state) {
//  int deleted_index;
//
//  deleted_index = msg_state->index;
//
//  for (int i = deleted_index; i < msg_state->message_count - 1; i++) {
//    msg_state->messages[i] = msg_state->messages[i + 1];
//  }
//
//  if (msg_state->message_count > 0) {
//    msg_state->message_count--;
//  }
//
//  if (msg_state->message_count == 0) {
//    msg_state->index = 0;
//  } else if (msg_state->index >= msg_state->message_count) {
//    msg_state->index = msg_state->message_count - 1;
//  }
//}

static void handle_message_action(int ch, int *focus, char *input,
                                  int *input_len, int server,
                                  const char *username, const char *password,
                                  uint8_t channel_id,
                                  struct channel_message_state *msg_state) {
  const struct stored_message *m;

  if (msg_state->message_count <= 0) {
    return;
  }

  m = &msg_state->messages[msg_state->index];

  if (ch == 'd') {
    delete_message_request(server, username, password, channel_id,
                           m->timestamp);

  } else if (ch == 'e') {
    strncpy(input, m->text, INPUT_MAX_LEN - 1);
    input[INPUT_MAX_LEN - 1] = '\0';
    *input_len = (int)strlen(input);
    msg_state->editing = 1;
    msg_state->editing_timestamp = m->timestamp;
    *focus = FOCUS_INPUT;
  }
}

static void handle_message_scroll(int ch,
                                  struct channel_message_state *msg_state) {
  if (ch == KEY_PPAGE) {
    handle_scroll_up(msg_state);
  } else if (ch == KEY_NPAGE) {
    handle_scroll_down(msg_state);
  }
}
void handle_messages_focus(int ch, int *focus, char *input, int *input_len,
                           int server, const char *username,
                           const char *password, uint8_t channel_id,
                           struct channel_message_state *msg_state) {
  if (ch == KEY_UP || ch == KEY_DOWN) {
    handle_message_navigation(ch, focus, msg_state);
    return;
  }

  if (ch == 'd' || ch == 'e') {
    handle_message_action(ch, focus, input, input_len, server, username,
                          password, channel_id, msg_state);
    return;
  }

  handle_message_scroll(ch, msg_state);
}

int handle_back_focus(int ch, int *focus) {
  if (ch == KEY_DOWN) {
    *focus = FOCUS_MESSAGES;
  } else if (ch == '\n' || ch == KEY_ENTER) {
    return INPUT_ACTION_BACK;
  }

  return INPUT_ACTION_NONE;
}

static void handle_input_navigation(int ch, int *focus,
                                    struct channel_message_state *msg_state) {
  if (ch == KEY_UP) {
    *focus = FOCUS_MESSAGES;
  } else if (ch == KEY_PPAGE) {
    handle_scroll_up(msg_state);
  } else if (ch == KEY_NPAGE) {
    handle_scroll_down(msg_state);
  }
}


static void handle_input_editing(char *input, int *input_len, int server,
                                 const char *username, const char *password,
                                 uint8_t channel_id,
                                 struct channel_message_state *msg_state) {
  const struct stored_message *m;

  m = &msg_state->messages[msg_state->index];
  if (strcmp(input, m->text) != 0) {
    edit_message_request(server, username, password, channel_id,
                         msg_state->editing_timestamp, input);
  }

  msg_state->editing = 0;
  msg_state->editing_timestamp = 0;
  input[0] = '\0';
  *input_len = 0;
  msg_state->scroll_offset = 0;
}

static void handle_input_typing(int ch, char *input, int *input_len) {
  if (ch == KEY_BACKSPACE || ch == KEY_BS_ALT) {
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
static void handle_input_enter(char *input, int *input_len, int server,
                               const char *username, const char *password,
                               uint8_t channel_id,
                               struct channel_message_state *msg_state) {
  if (msg_state->editing) {
    handle_input_editing(input, input_len, server, username, password,
                         channel_id, msg_state);
    return;
  }

  if (*input_len > 0) {
    handle_send_message(input, input_len, server, username, password,
                        channel_id, msg_state);
  }

  input[0] = '\0';
  *input_len = 0;
  msg_state->scroll_offset = 0;
}
void handle_input_focus(int ch, int *focus, char *input, int *input_len,
                        int server, const char *username, const char *password,
                        uint8_t channel_id,
                        struct channel_message_state *msg_state) {
  if (ch == KEY_UP || ch == KEY_PPAGE || ch == KEY_NPAGE) {
    handle_input_navigation(ch, focus, msg_state);
    return;
  }

  if (ch == '\n') {
    handle_input_enter(input, input_len, server, username, password, channel_id,
                       msg_state);
    return;
  }

  handle_input_typing(ch, input, input_len);
}

static int find_message_index(const struct channel_message_state *msg_state,
                              uint8_t sender_id, uint64_t timestamp) {
  for (int i = 0; i < msg_state->message_count; i++) {
    if (msg_state->messages[i].sender_id == sender_id &&
        msg_state->messages[i].timestamp == timestamp) {
      return i;
    }
  }
  return -1;
}

static void remove_message_at_index(struct channel_message_state *msg_state,
                                    int index) {
  for (int i = index; i < msg_state->message_count - 1; i++) {
    msg_state->messages[i] = msg_state->messages[i + 1];
  }

  if (msg_state->message_count > 0) {
    msg_state->message_count--;
  }

  if (msg_state->message_count == 0) {
    msg_state->index = 0;
  } else if (msg_state->index >= msg_state->message_count) {
    msg_state->index = msg_state->message_count - 1;
  }

  if (msg_state->scroll_offset > msg_state->message_count - 1) {
    msg_state->scroll_offset = msg_state->message_count - 1;
  }

  if (msg_state->scroll_offset < 0) {
    msg_state->scroll_offset = 0;
  }
}

static void update_message_at_index(struct channel_message_state *msg_state,
                                    int index, const char *text) {
  strncpy(msg_state->messages[index].text, text, MAX_MESSAGE_SIZE - 1);
  msg_state->messages[index].text[MAX_MESSAGE_SIZE - 1] = '\0';
}

void handle_parsed_message(const struct protocol_header *msg_header,
                           const uint8_t *body,
                           struct channel_message_state *msg_state) {
  struct message_read_response msg;
  int existing_index;

  if (msg_header->type != MESSAGE_TYPE_MESSAGE_READ_RESPONSE ||
      msg_header->status != STATUS_OK) {
    return;
  }

  memset(&msg, 0, sizeof(msg));
  deserialize_message_read_response(body, &msg);

  existing_index = find_message_index(msg_state, msg.sender_id, msg.timestamp);

  if (existing_index >= 0) {
    if (msg.message_len == 0 || msg.message[0] == '\0') {
      remove_message_at_index(msg_state, existing_index);
    }
    else {
      update_message_at_index(msg_state, existing_index, msg.message);
    }

    return;
  }
  if (msg.message_len == 0 || msg.message[0] == '\0') {
    return;
  }
  store_message(msg_state, msg.sender_id, msg.timestamp, msg.message);
}