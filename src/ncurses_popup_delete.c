/*
 * Created by Adnan Abdulle on 2026-04-14.
 */

#include "ncurses_popup_delete.h"

/* shows a confirmation popup before deleting a channel */
int confirm_channel_delete_popup(const char *channel_name) {
  WINDOW *popup;
  int height;
  int width;
  int start_y;
  int start_x;

  getmaxyx(stdscr, height, width);

  start_y = (height - POPUP_DELETE_HEIGHT) / 2;
  start_x = (width - POPUP_DELETE_WIDTH) / 2;

  popup = newwin(POPUP_DELETE_HEIGHT, POPUP_DELETE_WIDTH, start_y, start_x);
  keypad(popup, TRUE);

  while (1) {
    int ch;

    werase(popup);
    box(popup, 0, 0);

    mvwprintw(popup, 1, 2, "Delete Channel?");
    mvwprintw(popup, POPUP_DELETE_LABEL_ROW, 2, "Name:");
    mvwprintw(popup, POPUP_DELETE_INPUT_ROW, POPUP_DELETE_INPUT_COL, "%.15s",
              channel_name);
    mvwprintw(popup, POPUP_DELETE_HINT_ROW, 2, "y = delete, q = cancel");

    wrefresh(popup);

    ch = wgetch(popup);

    if (ch == 'q') {
      delwin(popup);
      return POPUP_NO;
    }

    if (ch == 'y' || ch == 'Y') {
      delwin(popup);
      return POPUP_YES;
    }
  }
}
