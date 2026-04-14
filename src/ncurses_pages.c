#include "ncurses_pages.h"
#include "client.h"
#include "ncurses_popup.h"
#include "protocol.h"
#include <ncurses.h>
#include <string.h>
#include <sys/syslog.h>

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
    int total_items;

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

    total_items = response.channel_id_len + 1;

    if (selected >= total_items) {
      selected = total_items - 1;
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

    if (selected == response.channel_id_len) {
      attron(A_REVERSE);
    }

    mvprintw(UI_CHAN_START_ROW + response.channel_id_len, UI_INDENT_COL,
             "+ New Channel");

    if (selected == response.channel_id_len) {
      attroff(A_REVERSE);
    }

    refresh();
    ch = getch();

    if (ch == KEY_UP) {
      if (selected > 0) {
        selected--;
      }

    } else if (ch == KEY_DOWN) {
      if (selected < total_items - 1) {
        selected++;
      }

    } else if (ch == '\n') {
      if (selected < response.channel_id_len) {
        channel_page(server, username, password,
                     response.channel_ids[selected]);
      } else {
        char channel_name[CHANNEL_NAME_SIZE];
        struct channel_create_response create_response;

        if (create_channel_popup(channel_name) == POPUP_SUBMIT) {
          memset(&create_response, 0, sizeof(create_response));

          channel_create_request(server, username, password, channel_name);

          if (channel_create_response(server, &create_response) == 0) {
            syslog(LOG_INFO, "Channel created: %s", channel_name);
          } else {
            syslog(LOG_ERR, "Channel creation failed");
          }

          refresh();
          getch();
        }
      }
    } else if (ch == 'd') {
      if (selected < response.channel_id_len) {

        uint8_t channel_id = response.channel_ids[selected];
        char channel_name[CHANNEL_NAME_SIZE];

        struct channel_read_response channel_read_resp_local;
        memset(&channel_read_resp_local, 0, sizeof(channel_read_resp_local));

        channel_read_request(server, username, password, channel_id);
        channel_read_response(server, &channel_read_resp_local);

        memcpy(channel_name, channel_read_resp_local.channel_name,
               CHANNEL_NAME_SIZE);
        channel_name[CHANNEL_NAME_SIZE - 1] = '\0';

        if (confirm_channel_delete_popup(channel_name) == POPUP_YES) {

          channel_delete_request(server, username, password, channel_id);
          struct channel_delete_response del_resp;
          memset(&del_resp, 0, sizeof(del_resp));

          if (channel_delete_response(server, &del_resp) == 0) {
            syslog(LOG_INFO, "Channel deleted: %s", channel_name);
          } else {
            syslog(LOG_ERR, "Channel deletion failed");
          }
          refresh();
          getch();
        }
      }
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
