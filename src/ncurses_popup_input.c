/*
 * Created by Adnan Abdulle on 2026-04-14.
 */

#include "ncurses_popup_input.h"
#include <string.h>

/* handles a single keypress in the create-channel popup */
int handle_create_popup_key(int ch, char *channel_name, int *input_len) {
  if (ch == 'q') {
    return INPUT_QUIT;
  }

  if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
    if (*input_len > 0) {
      return INPUT_SUBMIT;
    }
    return INPUT_CONTINUE;
  }

  if (ch == KEY_BACKSPACE || ch == ASCII_DEL) {
    if (*input_len > 0) {
      (*input_len)--;
      channel_name[*input_len] = '\0';
    }
    return INPUT_CONTINUE;
  }

  if (ch >= ASCII_PRINTABLE_MIN && ch <= ASCII_PRINTABLE_MAX &&
      *input_len < CHANNEL_NAME_SIZE - 1) {
    channel_name[*input_len] = (char)ch;
    (*input_len)++;
    channel_name[*input_len] = '\0';
  }

  return INPUT_CONTINUE;
}
