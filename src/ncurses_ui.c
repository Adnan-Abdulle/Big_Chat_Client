#include "ncurses_ui.h"
#include <ncurses.h>

void run_ui(const char *ip, uint16_t port) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);

  mvprintw(0, 0, "Connecting...");
  refresh();

  int server = connect_server_manager(ip, port);

  if (server == -1) {
    mvprintw(3, 5, "Failed to connect to server");
    refresh();
    getch();
    endwin();
    return;
  }

  while (1) {
    menu_ui(server);
  }

  endwin();
}

void menu_ui(int server) {

  clear();
  refresh();

  mvprintw(0, 15, "BIG CHAT");
  mvprintw(6, 15, "1.Create account");
  mvprintw(7, 15, "2.Login");

  refresh();

  int choice = getch();

  if (choice == '1') {
    create_account_ui(server);
  }

  if (choice == '2') {
    login_ui(server);
  }
}

void create_account_ui(int server) {

  clear();
  refresh();

  const char *title = "BIG CHAT ACCOUNT CREATION";

  attron(A_BOLD);
  mvprintw(0, 15, "%s", title);
  attroff(A_BOLD);

  mvprintw(4, 3, "Username: ");
  mvprintw(8, 3, "Password: ");

  refresh();

  int box_width = 30;

  WINDOW *userwin = newwin(3, box_width, 3, 15);
  box(userwin, 0, 0);
  wrefresh(userwin);

  WINDOW *passwin = newwin(3, box_width, 7, 15);
  box(passwin, 0, 0);
  wrefresh(passwin);

  wmove(userwin, 1, 1);

  char username[USERNAME_SIZE];

  echo();
  wgetnstr(userwin, username, USERNAME_SIZE - 1);
  noecho();

  wmove(passwin, 1, 1);

  char password[PASSWORD_SIZE];

  echo();
  wgetnstr(passwin, password, PASSWORD_SIZE - 1);
  noecho();

  wrefresh(passwin);

  account_creation(server, username, password);

  uint16_t id = account_creation_resp(server);

  if (id != 0) {
    mvprintw(12, 3, "Account created successfully!");
  } else {
    mvprintw(12, 3, "Account creation failed.");
  }

  refresh();
  getch();

  delwin(userwin);
  delwin(passwin);
}

void login_ui(int server) {

  clear();
  refresh();

  const char *title = "BIG CHAT LOGIN";

  attron(A_BOLD);
  mvprintw(0, 15, "%s", title);
  attroff(A_BOLD);

  mvprintw(4, 3, "Username: ");
  mvprintw(8, 3, "Password: ");

  refresh();

  int box_width = 30;

  WINDOW *userwin = newwin(3, box_width, 3, 15);
  box(userwin, 0, 0);
  wrefresh(userwin);

  WINDOW *passwin = newwin(3, box_width, 7, 15);
  box(passwin, 0, 0);
  wrefresh(passwin);

  wmove(userwin, 1, 1);

  char username[USERNAME_SIZE];

  echo();
  wgetnstr(userwin, username, USERNAME_SIZE - 1);
  noecho();

  wmove(passwin, 1, 1);

  char password[PASSWORD_SIZE];

  echo();
  wgetnstr(passwin, password, PASSWORD_SIZE - 1);
  noecho();

  wrefresh(passwin);

  login(server, password, username);

  if (login_logout_response(server) == 1) {

    delwin(userwin);
    delwin(passwin);

    flushinp();

    main_page(server, password, username);
    return;
  }

  mvprintw(12, 3, "Login failed");
  refresh();
  getch();

  delwin(userwin);
  delwin(passwin);
}

void main_page(int server, char *password, char *username) {

  int ch;

  while (1) {

    clear();

    mvprintw(4, 5, "Logged in!");
    mvprintw(8, 5, "Channel List");
    mvprintw(15, 5, "Press 'q' to logout");

    channel_list_request(server, username, password);

    struct channel_list_response response;

    if (channel_list_response(server, &response) == -1) {

      mvprintw(10, 5, "Failed to retrieve channel list");
      refresh();
      getch();
      return;
    }

    if (response.channel_id_len == 0) {

      mvprintw(10, 5, "No channels available");

    } else {

      for (int i = 0; i < response.channel_id_len; ++i) {

        struct channel_read_response channelReadResponse;
        channel_read_request(server, username, password,
                             response.channel_ids[i]);
        if (channel_read_response(server, &channelReadResponse) == 0) {
          mvprintw(9 + i, 5, "ID: %u Name: %.16s # of Users: %u",
                   response.channel_ids[i], channelReadResponse.channel_name,
                   channelReadResponse.user_id_len);
        }
      }
    }

    refresh();

    ch = getch();

    if (ch == 'q') {

      logout(server, password, username);

      if (login_logout_response(server) == 1) {

        mvprintw(8, 5, "Logout Successful");
        refresh();
        getch();
        return;

      } else {

        mvprintw(8, 5, "Logout failed");
        refresh();
        getch();
      }
    }
  }
}

void channel_page() {
  while (1) {
    clear();
    refresh();
  }
}