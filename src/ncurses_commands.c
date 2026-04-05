#include "ncurses_commands.h"
#include "client.h"
#include "protocol.h"
#include <string.h>
#include <time.h>

enum { EDIT_CMD_LEN = 6 };

enum { MS_PER_SEC = 1000 };
enum { NS_PER_MS = 1000000 };

/* get the current time in milliseconds since unix epoch */
static uint64_t current_time_ms(void) {
  struct timespec ts;

  clock_gettime(CLOCK_REALTIME, &ts);
  return ((uint64_t)ts.tv_sec * MS_PER_SEC) +
         ((uint64_t)ts.tv_nsec / NS_PER_MS);
}

/* add a message to the state, dropping the oldest if full */
void store_message(struct channel_message_state *state, uint8_t sender_id,
                   uint64_t timestamp, const char *text) {
  struct stored_message *slot;

  if (state->message_count >= MAX_STORED_MESSAGES) {
    memmove(&state->messages[0], &state->messages[1],
            sizeof(struct stored_message) * (MAX_STORED_MESSAGES - 1));
    state->message_count = MAX_STORED_MESSAGES - 1;
  }

  slot = &state->messages[state->message_count];
  slot->sender_id = sender_id;
  slot->timestamp = timestamp;
  memset(slot->text, 0, MAX_MESSAGE_SIZE);
  strncpy(slot->text, text, MAX_MESSAGE_SIZE - 1);
  state->message_count++;
}

/* handle the /edit command */
void handle_edit_command(const char *input, int *input_len, int server,
                         const char *username, const char *password,
                         uint8_t channel_id,
                         const struct channel_message_state *msg_state) {
  const char *new_text;

  if (msg_state->last_sent_timestamp == 0) {
    return;
  }

  new_text = input + EDIT_CMD_LEN;
  edit_message_request(server, username, password, channel_id,
                       msg_state->last_sent_timestamp, new_text);

  *input_len = 0;
}

/* handle the /delete command */
void handle_delete_command(int *input_len, int server, const char *username,
                           const char *password, uint8_t channel_id,
                           struct channel_message_state *msg_state) {
  if (msg_state->last_sent_timestamp == 0) {
    return;
  }

  delete_message_request(server, username, password, channel_id,
                         msg_state->last_sent_timestamp);

  msg_state->last_sent_timestamp = 0;
  *input_len = 0;
}

/* handle sending a normal message */
void handle_send_message(const char *input, int *input_len, int server,
                         const char *username, const char *password,
                         uint8_t channel_id,
                         struct channel_message_state *msg_state) {
  uint64_t timestamp;

  timestamp = current_time_ms();
  message_create_request(server, username, password, channel_id, input,
                         timestamp);
  msg_state->last_sent_timestamp = timestamp;
  *input_len = 0;
}
