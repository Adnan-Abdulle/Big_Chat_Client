/*
 * Created by Adnan Abdulle on 2026-04-14.
 */

#include "ncurses_popup_create.h"

/* shows a popup to type a new channel name */
int create_channel_popup(char *channel_name) {
  WINDOW *popup;
  int height;
  int width;
  int start_y;
  int start_x;
  int input_len;

  getmaxyx(stdscr, height, width);

  start_y = (height - POPUP_CREATE_HEIGHT) / 2;
  start_x = (width - POPUP_CREATE_WIDTH) / 2;

  popup = newwin(POPUP_CREATE_HEIGHT, POPUP_CREATE_WIDTH, start_y, start_x);
  keypad(popup, TRUE);

  memset(channel_name, 0, CHANNEL_NAME_SIZE);
  input_len = 0;

  while (1) {
    int ch;
    int action;

    werase(popup);
    box(popup, 0, 0);

    mvwprintw(popup, 1, 2, "Create Channel");
    mvwprintw(popup, POPUP_CREATE_LABEL_ROW, 2, "Name:");
    mvwprintw(popup, POPUP_CREATE_INPUT_ROW, POPUP_CREATE_INPUT_COL, "%s",
              channel_name);
    mvwprintw(popup, POPUP_CREATE_HINT_ROW, 2, "Enter = create, q = quit");

    wmove(popup, POPUP_CREATE_INPUT_ROW, POPUP_CREATE_INPUT_COL + input_len);
    wrefresh(popup);
    ch = wgetch(popup);

    action = handle_create_popup_key(ch, channel_name, &input_len);

    if (action == INPUT_QUIT) {
      delwin(popup);
      return POPUP_CANCEL;
    }

    if (action == INPUT_SUBMIT) {
      delwin(popup);
      return POPUP_SUBMIT;
    }
  }
}
