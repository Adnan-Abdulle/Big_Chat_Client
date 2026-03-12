#include "ncurses_ui.h"
#include <ncurses.h>

void run_ui(const char *ip, uint16_t port) {
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);

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

    main_page(server, username, password);
    return;
  }

  mvprintw(12, 3, "Login failed");
  refresh();
  getch();

  delwin(userwin);
  delwin(passwin);
}
void main_page(int server, char *username, char *password) {

  int ch;
  int selected = 0;

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

        channel_read_response(server, &channelReadResponse);

        if (i == selected) {
          attron(A_REVERSE);
        }

        mvprintw(9 + i, 5, "ID: %u Name: %.16s", response.channel_ids[i],
                 channelReadResponse.channel_name);

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

    } else if (ch == 10) {
      channel_page(server, username, password, response.channel_ids[selected]);

    } else if (ch == 'q') {

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
void channel_page(int server, char *username, char *password,
                  uint8_t channel_id) {

  clear();
  refresh();

  struct channel_read_response response;

  channel_read_request(server, username, password, channel_id);
  channel_read_response(server, &response);

  int height, width;
  getmaxyx(stdscr, height, width);

  WINDOW *header_win = newwin(2, width, 0, 0);
  WINDOW *msg_win = newwin(height - 5, width, 2, 0);
  WINDOW *input_win = newwin(3, width, height - 3, 0);

  scrollok(msg_win, TRUE);

  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  int focus = 1;
  int ch;

  char input[256] = {0};
  int input_len = 0;

  while (1) {

    werase(header_win);

    if (focus == 0)
      wattron(header_win, A_REVERSE);
    mvwprintw(header_win, 0, 0, "< Back");
    if (focus == 0)
      wattroff(header_win, A_REVERSE);

    mvwprintw(header_win, 0, 10, "%.16s", response.channel_name);

    wrefresh(header_win);

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    select(server + 1, &readfds, NULL, NULL, &tv);

    if (FD_ISSET(server, &readfds)) {

      struct message_read_response msg;

      if (message_read_response(server, &msg) == 0) {

        wprintw(msg_win, "%u: %.*s\n", msg.sender_id, msg.message_len,
                msg.message);

        wrefresh(msg_win);
      }
    }

    ch = getch();

    if (ch != ERR) {

      if (ch == KEY_UP)
        focus = 0;

      else if (ch == KEY_DOWN)
        focus = 1;

      else if (focus == 0 && (ch == '\n' || ch == KEY_ENTER)) {
        break;
      }

      else if (focus == 1) {

        if (ch == '\n') {

          message_create_request(server, username, password, channel_id, input);
          message_create_response(server);

          channel_read_request(server, username, password, channel_id);
          channel_read_response(server, &response);

          input_len = 0;
          input[0] = '\0';
        }

        else if (ch == KEY_BACKSPACE || ch == 127) {

          if (input_len > 0) {
            input_len--;
            input[input_len] = '\0';
          }
        }

        else if (input_len < 255 && ch >= 32 && ch <= 126) {

          input[input_len++] = ch;
          input[input_len] = '\0';
        }
      }
    }

    werase(input_win);
    box(input_win, 0, 0);

    if (focus == 1)
      wattron(input_win, A_REVERSE);
    mvwprintw(input_win, 1, 1, "> %s", input);
    if (focus == 1)
      wattroff(input_win, A_REVERSE);

    wrefresh(input_win);
  }

  delwin(header_win);
  delwin(msg_win);
  delwin(input_win);
}