#include "ncurses_input.h"

#include "ncurses_input_helpers.h"
#include "protocol.h"
#include "recv_buffer.h"
#include <ncurses.h>

enum { EDIT_CMD_LEN = 6 };

/* process a keypress in the channel page, returns INPUT_ACTION_BACK to exit */
int handle_channel_input(int ch, int *focus, char *input, int *input_len,
                         int server, const char *username, const char *password,
                         uint8_t channel_id,
                         struct channel_message_state *msg_state) {
  if (*focus == FOCUS_MESSAGES) {
    handle_messages_focus(ch, focus, input, input_len, server, username,
                          password, channel_id, msg_state);
  } else if (*focus == FOCUS_BACK) {
    return handle_back_focus(ch, focus);
  } else if (*focus == FOCUS_INPUT) {
    handle_input_focus(ch, focus, input, input_len, server, username, password,
                       channel_id, msg_state);
  }

  return INPUT_ACTION_NONE;
}

/* drain all complete messages from the recv buffer into the message state */
void drain_incoming_messages(struct recv_buffer *recv_buf,
                             struct channel_message_state *msg_state) {
  static uint8_t body[MIN_MESSAGE_READ_RESPONSE_BODY_SIZE + MAX_MESSAGE_SIZE];
  struct protocol_header msg_header;
  uint32_t body_size;
  int parse_result;

  parse_result = recv_buffer_try_parse(recv_buf, &msg_header, body,
                                       sizeof(body), &body_size);

  while (parse_result != 0) {
    if (parse_result == 1) {
      handle_parsed_message(&msg_header, body, msg_state);
    }

    parse_result = recv_buffer_try_parse(recv_buf, &msg_header, body,
                                         sizeof(body), &body_size);
  }
}
/* draw the stored messages based on current scroll position */
