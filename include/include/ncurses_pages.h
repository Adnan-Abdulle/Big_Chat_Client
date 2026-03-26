#ifndef NCURSES_PAGES_H
#define NCURSES_PAGES_H

#include <stdint.h>

void main_page(int server, const char *username, const char *password);
void channel_page(int server, const char *username, const char *password,
                  uint8_t channel_id);

#endif
