/*
 * Created by Adnan Abdulle on 2026-02-11.
 */

#ifndef MAIN_CLIENT_H
#define MAIN_CLIENT_H
#include "protocol.h"
#include <stdint.h>

void account_creation(int server_fd, const char *username,
                      const char *password);

uint8_t account_creation_resp(int server_fd);

void login(int server_fd, const char *password, const char *username);

uint8_t login_logout_response(int server_fd);

void logout(int server_fd, const char *password, const char *username);

int connect_server_manager(const char *ip, uint16_t port);

void channel_list_request(int server_fd, const char *username,
                          const char *password);

int channel_list_response(int server_fd,
                          struct channel_list_response *response);

void channel_read_request(int server_fd, const char *username,
                          const char *password, uint8_t channel_id);

int channel_read_response(int server_fd,
                          struct channel_read_response *response);

void message_create_request(int server, const char *username,
                            const char *password, uint8_t channel_id,
                            const char *message, uint64_t timestamp);
int message_create_response(int server);

void message_read_request(int server, const char *username,
                          const char *password, uint64_t timestamp,
                          uint8_t channel_id, uint8_t sender_id);

int message_read_response(int server, struct message_read_response *response);

void delete_user_request(int server_fd, const char *username,
                         const char *password);
int delete_user_response(int server_fd);

void edit_message_request(int server, const char *username,
                          const char *password, uint8_t channel_id,
                          uint64_t timestamp, const char *message);

void delete_message_request(int server, const char *username,
                            const char *password, uint8_t channel_id,
                            uint64_t timestamp);

void get_history_request(int server, const char *username,
                         const char *password, uint64_t start_timestamp,
                         uint8_t channel_id, uint16_t limit);

int get_history_response(int server, struct get_history_response *response);

#endif /* MAIN_CLIENT_H */
