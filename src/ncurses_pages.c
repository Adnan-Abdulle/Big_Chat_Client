#include "ncurses_pages.h"
#include "client.h"
#include "protocol.h"
#include <ncurses.h>
#include <string.h>

/* layout constants for the main page */
enum {
  UI_INDENT_COL = 5,
  UI_CHAN_START_ROW = 9,
  UI_FIELD_ROW_USER = 4,
  UI_FIELD_ROW_PASS = 8,
  UI_MSG_ROW = 10,
  UI_HINT_ROW = 15
};

/* show the channel list and let the user pick one */
void main_page(int server, const char *username, const char *password) {
  int selected;
  struct channel_list_response response;
  struct channel_read_response channel_read_resp;

  selected = 0;

  while (1) {
    int ch;

    clear();

    mvprintw(UI_FIELD_ROW_USER, UI_INDENT_COL, "Logged in!");
    mvprintw(UI_FIELD_ROW_PASS, UI_INDENT_COL, "Channel List");
    mvprintw(UI_HINT_ROW, UI_INDENT_COL, "Press 'q' to logout");

    channel_list_request(server, username, password);

    memset(&response, 0, sizeof(response));
    if (channel_list_response(server, &response) == -1) {

      mvprintw(UI_MSG_ROW, UI_INDENT_COL, "Failed to retrieve channel list");
      refresh();
      getch();
      return;
    }

    if (response.channel_id_len == 0) {

      mvprintw(UI_MSG_ROW, UI_INDENT_COL, "No channels available");

    } else {

      for (int i = 0; i < response.channel_id_len; ++i) {

        memset(&channel_read_resp, 0, sizeof(channel_read_resp));

        channel_read_request(server, username, password,
                             response.channel_ids[i]);

        channel_read_response(server, &channel_read_resp);

        if (i == selected) {
          attron(A_REVERSE);
        }

        mvprintw(UI_CHAN_START_ROW + i, UI_INDENT_COL, "ID: %u Name: %.16s",
                 response.channel_ids[i], channel_read_resp.channel_name);

        if (i == selected) {
          attroff(A_REVERSE);
        }
      }
    }

    refresh();

    ch = getch();

    if (ch == KEY_UP) {

      if (selected > 0) {
        selected--;
      }

    } else if (ch == KEY_DOWN) {

      if (selected < response.channel_id_len - 1) {
        selected++;
      }

    } else if (ch == '\n') {
      channel_page(server, username, password, response.channel_ids[selected]);

    } else if (ch == 'q') {

      logout(server, password, username);

      if (login_logout_response(server) == 1) {

        mvprintw(UI_FIELD_ROW_PASS, UI_INDENT_COL, "Logout Successful");
        refresh();
        getch();
        return;
      }

      mvprintw(UI_FIELD_ROW_PASS, UI_INDENT_COL, "Logout failed");
      refresh();
      getch();
    }
  }
}
