/*
 * Created by Adnan Abdulle on 2026-04-14.
 */

#ifndef MAIN_NCURSES_POPUP_CREATE_H
#define MAIN_NCURSES_POPUP_CREATE_H

#include "ncurses_popup_input.h"
#include <ncurses.h>
#include <string.h>

enum {
    POPUP_CREATE_HEIGHT = 7,
    POPUP_CREATE_WIDTH = 40,
    POPUP_CREATE_LABEL_ROW = 2,
    POPUP_CREATE_INPUT_ROW = 3,
    POPUP_CREATE_HINT_ROW = 5,
    POPUP_CREATE_INPUT_COL = 8
};

enum { POPUP_CANCEL = 0, POPUP_SUBMIT = 1 };

int create_channel_popup(char *channel_name);

#endif /* MAIN_NCURSES_POPUP_CREATE_H */
