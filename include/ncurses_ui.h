#ifndef MAIN_NCURSES_H
#define MAIN_NCURSES_H

#include <ncurses.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "client.h"
#include "protocol.h"
#include <sys/select.h>
#include <unistd.h>

void menu_ui(int server);
void create_account_ui(int server);
void login_ui(int server);
void run_ui(const char* ip, uint16_t port);
void main_page(int server, char* username, char* password);
void channel_page(int server, char* username, char* password, uint8_t channel_id);

#endif
