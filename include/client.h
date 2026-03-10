//
// Created by Adnan Abdulle on 2026-02-11.
//

#ifndef MAIN_CLIENT_H
#define MAIN_CLIENT_H
#include <stdint.h>
#include "protocol.h"

void account_creation(int server_fd, char* username, char* password);

uint8_t account_creation_resp(int server_fd);

void login(int server_fd, char *password, char* username);

uint8_t login_logout_response(int server_fd);

void logout(int server_fd, char *password, char* username);

int connect_socket(const char* ip, uint16_t port);

void active_server_request(int server_manager);

char* active_server_rsp(int server_manager);

int connect_server_manager(const char* ip, uint16_t port);

void channel_list_request(int server_fd, char* username, char* password);

int channel_list_response(int server_fd, struct channel_list_response *response);
#endif //MAIN_CLIENT_H
