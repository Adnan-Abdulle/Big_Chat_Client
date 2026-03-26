#include "ncurses_ui.h"
#include "client.h"
#include "ncurses_pages.h"
#include "protocol.h"
#include <ncurses.h>
#include <string.h>
#include <unistd.h>

/* layout constants for the ui */
enum {
  UI_TITLE_COL = 15,
  UI_LABEL_COL = 3,
  UI_INDENT_COL = 5,
  UI_INPUT_BOX_WIDTH = 30,
  UI_INPUT_BOX_COL = 15,
  UI_BOX_HEIGHT = 3
};

enum {
  UI_ERR_ROW = 3,
  UI_FIELD_ROW_USER = 4,
  UI_MENU_ROW_1 = 6,
  UI_MENU_ROW_2 = 7,
  UI_MENU_ROW_3 = 8,
  UI_FIELD_ROW_PASS = 8,
  UI_STATUS_ROW = 12
};

enum { UI_USERWIN_ROW = 3, UI_PASSWIN_ROW = 7 };

enum { MENU_CONTINUE = 0, MENU_QUIT = 1 };

static int menu_ui(int server);
static void create_account_ui(int server);
static void login_ui(int server);

/* connect to the server manager and launch the main menu loop */
void run_ui(const char *ip, uint16_t port) {
  int server;

  initscr();
  cbreak();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);

  mvprintw(0, 0, "Connecting...");
  refresh();

  server = connect_server_manager(ip, port);

  if (server == -1) {
    mvprintw(UI_ERR_ROW, UI_INDENT_COL, "Failed to connect to server");
    refresh();
    getch();
    endwin();
    return;
  }

  while (menu_ui(server) != MENU_QUIT) {
    /* keep looping until user quits */
  }

  close(server);
  endwin();
}

/* show the main menu and dispatch to the chosen action, returns MENU_QUIT to
 * exit */
static int menu_ui(int server) {
  int choice;

  clear();
  refresh();

  mvprintw(0, UI_TITLE_COL, "BIG CHAT");
  mvprintw(UI_MENU_ROW_1, UI_TITLE_COL, "1.Create account");
  mvprintw(UI_MENU_ROW_2, UI_TITLE_COL, "2.Login");
  mvprintw(UI_MENU_ROW_3, UI_TITLE_COL, "q.Quit");

  refresh();

  choice = getch();

  if (choice == '1') {
    create_account_ui(server);
  }

  if (choice == '2') {
    login_ui(server);
  }

  if (choice == 'q') {
    return MENU_QUIT;
  }

  return MENU_CONTINUE;
}

/* prompt for username and password then create an account */
static void create_account_ui(int server) {
  const char *title = "BIG CHAT ACCOUNT CREATION";
  WINDOW *userwin;
  WINDOW *passwin;
  char username[USERNAME_SIZE];
  char password[PASSWORD_SIZE];
  uint16_t id;

  clear();
  refresh();

  attron(A_BOLD);
  mvprintw(0, UI_TITLE_COL, "%s", title);
  attroff(A_BOLD);

  mvprintw(UI_FIELD_ROW_USER, UI_LABEL_COL, "Username: ");
  mvprintw(UI_FIELD_ROW_PASS, UI_LABEL_COL, "Password: ");

  refresh();

  userwin = newwin(UI_BOX_HEIGHT, UI_INPUT_BOX_WIDTH, UI_USERWIN_ROW,
                   UI_INPUT_BOX_COL);
  box(userwin, 0, 0);
  wrefresh(userwin);

  passwin = newwin(UI_BOX_HEIGHT, UI_INPUT_BOX_WIDTH, UI_PASSWIN_ROW,
                   UI_INPUT_BOX_COL);
  box(passwin, 0, 0);
  wrefresh(passwin);

  wmove(userwin, 1, 1);

  echo();
  wgetnstr(userwin, username, USERNAME_SIZE - 1);
  noecho();

  wmove(passwin, 1, 1);

  echo();
  wgetnstr(passwin, password, PASSWORD_SIZE - 1);
  noecho();

  wrefresh(passwin);

  account_creation(server, username, password);

  id = account_creation_resp(server);

  if (id != 0) {
    mvprintw(UI_STATUS_ROW, UI_LABEL_COL, "Account created successfully!");
  } else {
    mvprintw(UI_STATUS_ROW, UI_LABEL_COL, "Account creation failed.");
  }

  refresh();
  getch();

  delwin(userwin);
  delwin(passwin);
}

/* prompt for username and password then log in */
static void login_ui(int server) {
  const char *title = "BIG CHAT LOGIN";
  WINDOW *userwin;
  WINDOW *passwin;
  char username[USERNAME_SIZE];
  char password[PASSWORD_SIZE];

  clear();
  refresh();

  attron(A_BOLD);
  mvprintw(0, UI_TITLE_COL, "%s", title);
  attroff(A_BOLD);

  mvprintw(UI_FIELD_ROW_USER, UI_LABEL_COL, "Username: ");
  mvprintw(UI_FIELD_ROW_PASS, UI_LABEL_COL, "Password: ");

  refresh();

  userwin = newwin(UI_BOX_HEIGHT, UI_INPUT_BOX_WIDTH, UI_USERWIN_ROW,
                   UI_INPUT_BOX_COL);
  box(userwin, 0, 0);
  wrefresh(userwin);

  passwin = newwin(UI_BOX_HEIGHT, UI_INPUT_BOX_WIDTH, UI_PASSWIN_ROW,
                   UI_INPUT_BOX_COL);
  box(passwin, 0, 0);
  wrefresh(passwin);

  wmove(userwin, 1, 1);

  echo();
  wgetnstr(userwin, username, USERNAME_SIZE - 1);
  noecho();

  wmove(passwin, 1, 1);

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

  mvprintw(UI_STATUS_ROW, UI_LABEL_COL, "Login failed");
  refresh();
  getch();

  delwin(userwin);
  delwin(passwin);
}
