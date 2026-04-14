/*
 * Created by Adnan Abdulle on 2026-04-14.
 */

#ifndef MAIN_NCURSES_POPUP_DELETE_H
#define MAIN_NCURSES_POPUP_DELETE_H

#include "protocol.h"
#include <ncurses.h>
#include <string.h>

enum {
    POPUP_DELETE_HEIGHT = 7,
    POPUP_DELETE_WIDTH = 40,
    POPUP_DELETE_LABEL_ROW = 2,
    POPUP_DELETE_INPUT_ROW = 3,
    POPUP_DELETE_HINT_ROW = 5,
    POPUP_DELETE_INPUT_COL = 8
};

enum { POPUP_NO = 0, POPUP_YES = 1 };

int confirm_channel_delete_popup(const char *channel_name);

#endif /* MAIN_NCURSES_POPUP_DELETE_H */
