//
// Created by Adnan Abdulle on 2026-04-14.
//

#include "ncurses_popup.h"
#include "protocol.h"

int create_channel_popup(char *channel_name) {
    WINDOW *popup;
    int height;
    int width;
    int start_y;
    int start_x;
    int input_len;

    getmaxyx(stdscr, height, width);

    start_y = (height - POPUP_HEIGHT) / 2;
    start_x = (width - POPUP_WIDTH) / 2;

    popup = newwin(POPUP_HEIGHT, POPUP_WIDTH, start_y, start_x);
    keypad(popup, TRUE);

    memset(channel_name, 0, CHANNEL_NAME_SIZE);
    input_len = 0;

    while (1) {

        int ch;
        werase(popup);
        box(popup, 0, 0);

        mvwprintw(popup, 1, 2, "Create Channel");
        mvwprintw(popup, POPUP_LABEL_ROW, 2, "Name:");
        mvwprintw(popup, POPUP_INPUT_ROW, POPUP_INPUT_COL, "%s", channel_name);
        mvwprintw(popup, POPUP_HINT_ROW, 2, "Enter = create, q = quit");

        wmove(popup, POPUP_INPUT_ROW, POPUP_INPUT_COL + input_len);
        wrefresh(popup);
        ch = wgetch(popup);

        if (ch == 'q') {
            delwin(popup);
            return POPUP_CANCEL;
        }

        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            if (input_len > 0) {
                delwin(popup);
                return POPUP_SUBMIT;
            }
            continue;
        }

        if (ch == KEY_BACKSPACE || ch == 127) {
            if (input_len > 0) {
                input_len--;
                channel_name[input_len] = '\0';
            }
            continue;
        }

        if (ch >= 32 && ch <= 126 && input_len < CHANNEL_NAME_SIZE - 1) {
            channel_name[input_len++] = (char)ch;
            channel_name[input_len] = '\0';
        }
    }
}

int confirm_channel_delete_popup(const char *channel_name) {
    WINDOW *popup;
    int height;
    int width;
    int start_y;
    int start_x;

    getmaxyx(stdscr, height, width);

    start_y = (height - POPUP_HEIGHT) / 2;
    start_x = (width - POPUP_WIDTH) / 2;

    popup = newwin(POPUP_HEIGHT, POPUP_WIDTH, start_y, start_x);
    keypad(popup, TRUE);

    while (1) {
        int ch;

        werase(popup);
        box(popup, 0, 0);

        mvwprintw(popup, 1, 2, "Delete Channel?");
        mvwprintw(popup, POPUP_LABEL_ROW, 2, "Name:");
        mvwprintw(popup, POPUP_INPUT_ROW, POPUP_INPUT_COL, "%.15s", channel_name);
        mvwprintw(popup, POPUP_HINT_ROW, 2, "y = delete, q = cancel");

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
