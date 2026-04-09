//
// Created by Adnan Abdulle on 2026-04-08.
//

#include "ncurses_render.h"

void draw_messages(WINDOW *msg_win,
                   const struct channel_message_state *msg_state) {
    int height;
    int width;
    int bottom;
    int top;

    werase(msg_win);
    getmaxyx(msg_win, height, width);
    (void)width;

    if (msg_state->message_count == 0) {
        wrefresh(msg_win);
        return;
    }

    bottom = msg_state->message_count - msg_state->scroll_offset;
    top = bottom - height;
    if (top < 0) {
        top = 0;
    }
    if (bottom > msg_state->message_count) {
        bottom = msg_state->message_count;
    }

    for (int i = top; i < bottom; i++) {
        const struct stored_message *m;

        m = &msg_state->messages[i];

        if (i == msg_state->index) {
            wattron(msg_win, A_REVERSE);
        }

        mvwprintw(msg_win, i - top, 0, "%u: %s", m->sender_id, m->text);

        if (i == msg_state->index) {
            wattroff(msg_win, A_REVERSE);
        }
    }

    wrefresh(msg_win);
}

enum { UI_CHAN_NAME_COL = 10 };

/* draw the channel header bar with back button and channel name */
void draw_channel_header(WINDOW *header_win, int focus,
                         const char *channel_name) {
    werase(header_win);

    if (focus == 0) {
        wattron(header_win, A_REVERSE);
    }
    mvwprintw(header_win, 0, 0, "< Back");
    if (focus == 0) {
        wattroff(header_win, A_REVERSE);
    }

    mvwprintw(header_win, 0, UI_CHAN_NAME_COL, "%.16s", channel_name);
    wrefresh(header_win);
}

/* draw the input bar with the current input text */
void draw_channel_input_bar(WINDOW *input_win, int focus, const char *input) {
    werase(input_win);
    box(input_win, 0, 0);

    if (focus == FOCUS_INPUT) {
        wattron(input_win, A_REVERSE);
    }
    mvwprintw(input_win, 1, 1, "> %s", input);
    if (focus == FOCUS_INPUT) {
        wattroff(input_win, A_REVERSE);
    }

    wrefresh(input_win);
}
