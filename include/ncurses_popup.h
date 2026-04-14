//
// Created by Adnan Abdulle on 2026-04-14.
//

#ifndef MAIN_NCURSES_POPUP_H
#define MAIN_NCURSES_POPUP_H
#include "protocol.h"
#include <ncurses.h>
#include <string.h>
enum {
    POPUP_HEIGHT = 7,
    POPUP_WIDTH = 40,
    POPUP_LABEL_ROW = 2,
    POPUP_INPUT_ROW = 3,
    POPUP_HINT_ROW = 5,
    POPUP_INPUT_COL = 8
};

enum { POPUP_CANCEL = 0, POPUP_SUBMIT = 1 };
enum { POPUP_NO = 0, POPUP_YES = 1 };

int create_channel_popup(char *channel_name);
int confirm_channel_delete_popup(const char *channel_name);

#endif //MAIN_NCURSES_POPUP_H
