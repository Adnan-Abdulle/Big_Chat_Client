/*
 * Created by Adnan Abdulle on 2026-02-11.
 */

#include "client.h"
#include "network.h"
#include "protocol.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

/* sends an account creation request to the server */
void account_creation(int server_fd, const char *username,
                      const char *password) {
  uint8_t buffer[HEADER_SIZE + ACCOUNT_REGISTRATION_BODY_SIZE];
  struct protocol_header header;
  struct account_registration account_reg;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_ACCOUNT_REGISTRATION_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = ACCOUNT_REGISTRATION_BODY_SIZE;
  serialize_header(&header, buffer);

  memset(&account_reg, 0, sizeof(account_reg));

  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }
  memcpy(account_reg.username, username, strlen(username));
  memcpy(account_reg.password, password, strlen(password));
  account_reg.account_id = 0;

  serialize_account_registration(&account_reg, buffer + HEADER_SIZE);

  write_exact(server_fd, buffer, HEADER_SIZE + ACCOUNT_REGISTRATION_BODY_SIZE);
}

/* receives the account creation response from the server */
uint8_t account_creation_resp(int server_fd) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  uint8_t body_buffer[ACCOUNT_REGISTRATION_BODY_SIZE];
  struct account_registration reg_resp;

  read_exact(server_fd, header_buffer, HEADER_SIZE);
  deserialize_header(header_buffer, &header);

  if (header.status != STATUS_OK) {
    syslog(LOG_ERR, "Account registration failed: status is not OK");
    return 0;
  }

  if (header.body_size != ACCOUNT_REGISTRATION_BODY_SIZE) {
    syslog(LOG_ERR, "Account creation body size error");
    return 0;
  }

  memset(&reg_resp, 0, sizeof(reg_resp));
  read_exact(server_fd, body_buffer, ACCOUNT_REGISTRATION_BODY_SIZE);
  deserialize_account_registration(body_buffer, &reg_resp);

  if (reg_resp.account_id == 0) {
    syslog(LOG_ERR, "Account registration failed invalid ID");
    return 0;
  }

  return reg_resp.account_id;
}

/* sends the login request to the server */
void login(int server_fd, const char *password, const char *username) {
  uint8_t buffer[HEADER_SIZE + LOGIN_OR_LOGOUT_BODY_SIZE];
  struct protocol_header header;
  struct login_or_logout login_req;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_LOGIN_OR_LOGOUT_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = LOGIN_OR_LOGOUT_BODY_SIZE;
  serialize_header(&header, buffer);

  memset(&login_req, 0, sizeof(login_req));
  login_req.account_status = ACCOUNT_STATUS_ONLINE;

  login_req.ip.a = 0;
  login_req.ip.b = 0;
  login_req.ip.c = 0;
  login_req.ip.d = 0;
  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }
  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  memcpy(login_req.auth.username, username, strlen(username));
  memcpy(login_req.auth.password, password, strlen(password));

  serialize_login_or_logout(&login_req, buffer + HEADER_SIZE);

  write_exact(server_fd, buffer, HEADER_SIZE + LOGIN_OR_LOGOUT_BODY_SIZE);
}

/* receives the login or logout response from the server */
uint8_t login_logout_response(int server_fd) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  uint8_t body_buffer[LOGIN_OR_LOGOUT_BODY_SIZE];
  struct login_or_logout login_logout_resp;

  read_exact(server_fd, header_buffer, HEADER_SIZE);
  deserialize_header(header_buffer, &header);

  if (header.status != STATUS_OK) {
    syslog(LOG_ERR, "Login/logout failed");
    return 0;
  }
  if (header.body_size != LOGIN_OR_LOGOUT_BODY_SIZE) {
    syslog(LOG_ERR, "Login/logout body size error");
    return 0;
  }

  memset(&login_logout_resp, 0, sizeof(login_logout_resp));
  read_exact(server_fd, body_buffer, LOGIN_OR_LOGOUT_BODY_SIZE);
  deserialize_login_or_logout(body_buffer, &login_logout_resp);

  return 1;
}

/* send the logout request to the server */
void logout(int server_fd, const char *password, const char *username) {
  uint8_t buffer[HEADER_SIZE + LOGIN_OR_LOGOUT_BODY_SIZE];
  struct protocol_header header;
  struct login_or_logout logout_req;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_LOGIN_OR_LOGOUT_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = LOGIN_OR_LOGOUT_BODY_SIZE;
  serialize_header(&header, buffer);

  memset(&logout_req, 0, sizeof(logout_req));
  logout_req.account_status = ACCOUNT_STATUS_OFFLINE;

  logout_req.ip.a = 0;
  logout_req.ip.b = 0;
  logout_req.ip.c = 0;
  logout_req.ip.d = 0;

  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }
  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  memcpy(logout_req.auth.username, username, strlen(username));
  memcpy(logout_req.auth.password, password, strlen(password));

  serialize_login_or_logout(&logout_req, buffer + HEADER_SIZE);

  write_exact(server_fd, buffer, HEADER_SIZE + LOGIN_OR_LOGOUT_BODY_SIZE);
}

/* request the list of channels from the server */
void channel_list_request(int server_fd, const char *username,
                          const char *password) {
  uint8_t buffer[HEADER_SIZE + CHANNEL_LIST_REQUEST_BODY_SIZE];
  struct protocol_header header;
  struct channel_list_request channel_list_req;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_CHANNEL_LIST_READ_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = CHANNEL_LIST_REQUEST_BODY_SIZE;
  serialize_header(&header, buffer);

  memset(&channel_list_req, 0, sizeof(channel_list_req));
  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }
  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  memcpy(channel_list_req.auth.username, username, strlen(username));
  memcpy(channel_list_req.auth.password, password, strlen(password));

  serialize_channel_list_request(&channel_list_req, buffer + HEADER_SIZE);
  write_exact(server_fd, buffer, HEADER_SIZE + CHANNEL_LIST_REQUEST_BODY_SIZE);
}

/* read the channel list response from the server */
int channel_list_response(int server_fd,
                          struct channel_list_response *response) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  uint8_t body_buffer[CHANNEL_RESPONSE_MAX_BODY_SIZE];

  read_exact(server_fd, header_buffer, HEADER_SIZE);

  deserialize_header(header_buffer, &header);

  if (header.status != STATUS_OK) {
    return -1;
  }

  if (header.type != MESSAGE_TYPE_CHANNEL_LIST_READ_RESPONSE) {
    return -1;
  }

  if (header.body_size > sizeof(body_buffer)) {
    return -1;
  }

  if (header.body_size < AUTH_SIZE + 1) {
    return -1;
  }

  read_exact(server_fd, body_buffer, header.body_size);

  memset(response, 0, sizeof(*response));
  deserialize_channel_list_response(body_buffer, response);

  if (response->channel_id_len > header.body_size - AUTH_SIZE - 1) {
    response->channel_id_len = 0;
    return -1;
  }

  return 0;
}

/* request channel details from the server */
void channel_read_request(int server_fd, const char *username,
                          const char *password, uint8_t channel_id) {
  uint8_t buffer[HEADER_SIZE + CHANNEL_READ_REQUEST_BODY_SIZE];
  struct protocol_header header;
  struct channel_read_request channel_read_req;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_CHANNEL_READ_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = CHANNEL_READ_REQUEST_BODY_SIZE;
  serialize_header(&header, buffer);

  memset(&channel_read_req, 0, sizeof(channel_read_req));
  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }
  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  memcpy(channel_read_req.auth.username, username, strlen(username));
  memcpy(channel_read_req.auth.password, password, strlen(password));
  channel_read_req.channel_id = channel_id;

  serialize_channel_read_request(&channel_read_req, buffer + HEADER_SIZE);
  write_exact(server_fd, buffer, HEADER_SIZE + CHANNEL_READ_REQUEST_BODY_SIZE);
}

/* read the channel details response from the server */
int channel_read_response(int server_fd,
                          struct channel_read_response *response) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  uint8_t body_buffer[CHANNEL_RESPONSE_MAX_BODY_SIZE];

  read_exact(server_fd, header_buffer, HEADER_SIZE);

  deserialize_header(header_buffer, &header);

  if (header.status != STATUS_OK) {
    return -1;
  }

  if (header.type != MESSAGE_TYPE_CHANNEL_READ_RESPONSE) {
    return -1;
  }

  if (header.body_size > sizeof(body_buffer)) {
    return -1;
  }

  if (header.body_size < CHANNEL_READ_REQUEST_BODY_SIZE) {
    return -1;
  }

  read_exact(server_fd, body_buffer, header.body_size);

  memset(response, 0, sizeof(*response));
  deserialize_channel_read_response(body_buffer, response);

  if (response->user_id_len >
      header.body_size - CHANNEL_READ_REQUEST_BODY_SIZE) {
    response->user_id_len = 0;
    return -1;
  }

  return 0;
}

/* connects to any socket as long as we got ip and port number */
static int connect_socket(const char *ip, uint16_t port) {
  int fd;
  struct sockaddr_in addr;

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    syslog(LOG_ERR, "socket creation failed");
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
    syslog(LOG_ERR, "inet_pton failed");
    close(fd);
    return -1;
  }

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    syslog(LOG_ERR, "connect failed");
    close(fd);
    return -1;
  }

  return fd;
}

/* ask the server manager which server is currently active */
static void active_server_request(int server_manager) {
  uint8_t buffer[HEADER_SIZE + SERVER_ACTIVATION_BODY_SIZE];
  struct protocol_header header;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_GET_ACTIVATED_SERVER_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = SERVER_ACTIVATION_BODY_SIZE;
  serialize_header(&header, buffer);

  memset(buffer + HEADER_SIZE, 0, SERVER_ACTIVATION_BODY_SIZE);

  write_exact(server_manager, buffer,
              HEADER_SIZE + SERVER_ACTIVATION_BODY_SIZE);
}

/* receives the server IP from the server manager */
static char *active_server_rsp(int server_manager) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  uint8_t body_buffer[SERVER_ACTIVATION_BODY_SIZE];
  struct server_registration server_reg;
  size_t ip_size;
  char *server_ip;

  read_exact(server_manager, header_buffer, HEADER_SIZE);
  deserialize_header(header_buffer, &header);

  if (header.status != STATUS_OK) {
    syslog(LOG_ERR, "Manager error");
    return NULL;
  }
  if (header.body_size != SERVER_ACTIVATION_BODY_SIZE) {
    syslog(LOG_ERR, "Server registration body size error");
    return NULL;
  }

  memset(&server_reg, 0, sizeof(server_reg));
  read_exact(server_manager, body_buffer, SERVER_ACTIVATION_BODY_SIZE);
  deserialize_server_registration(body_buffer, &server_reg);

  ip_size = IP_STRING_SIZE;
  server_ip = malloc(ip_size);
  if (server_ip != NULL) {
    snprintf(server_ip, ip_size, "%u.%u.%u.%u", server_reg.ip.a,
             server_reg.ip.b, server_reg.ip.c, server_reg.ip.d);
  }

  return server_ip;
}

/* send a message to a channel */
void message_create_request(int server, const char *username,
                            const char *password, uint8_t channel_id,
                            const char *message) {
  size_t raw_len;
  uint16_t message_len;
  uint8_t buffer[HEADER_SIZE + MIN_MESSAGE_CREATE_BODY_SIZE + MAX_MESSAGE_SIZE];
  struct protocol_header header;
  struct message_create_request request;
  time_t now;

  raw_len = strlen(message);
  if (raw_len > MAX_MESSAGE_SIZE) {
    raw_len = MAX_MESSAGE_SIZE;
  }
  message_len = (uint16_t)raw_len;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_MESSAGE_CREATE_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = (uint32_t)(MIN_MESSAGE_CREATE_BODY_SIZE + message_len);

  serialize_header(&header, buffer);

  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }

  memset(&request, 0, sizeof(request));

  memcpy(request.auth.username, username, strlen(username));
  memcpy(request.auth.password, password, strlen(password));

  now = time(NULL);
  request.timestamp = (uint64_t)now;
  request.message_len = message_len;
  request.channel_id = channel_id;

  memcpy(request.message, message, message_len);

  serialize_message_create_request(&request, buffer + HEADER_SIZE);

  write_exact(server, buffer, HEADER_SIZE + header.body_size);
}

/* read the message create response from the server */
int message_create_response(int server) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;

  read_exact(server, header_buffer, HEADER_SIZE);

  deserialize_header(header_buffer, &header);

  if (header.type != MESSAGE_TYPE_MESSAGE_CREATE_RESPONSE) {
    return -1;
  }

  if (header.status != STATUS_OK) {
    return -1;
  }

  if (header.body_size > MAX_MESSAGE_SIZE) {
    return -1;
  }

  if (header.body_size > 0) {
    uint8_t body_buffer[MAX_MESSAGE_SIZE];
    read_exact(server, body_buffer, header.body_size);
  }

  return 0;
}

/* request messages from a channel, starting at a given timestamp */
void message_read_request(int server, const char *username,
                          const char *password, uint64_t timestamp,
                          uint8_t channel_id) {
  uint8_t buffer[HEADER_SIZE + MESSAGE_READ_REQUEST_BODY_SIZE];
  struct protocol_header header;
  struct message_read_request request;

  header.version = PROTOCOL_VERSION;
  header.type = MESSAGE_TYPE_MESSAGE_READ_REQUEST;
  header.status = STATUS_OK;
  header.reserved = 0;
  header.body_size = MESSAGE_READ_REQUEST_BODY_SIZE;

  serialize_header(&header, buffer);

  if (strlen(username) >= USERNAME_SIZE) {
    return;
  }
  if (strlen(password) >= PASSWORD_SIZE) {
    return;
  }

  memset(&request, 0, sizeof(request));

  memcpy(request.auth.username, username, strlen(username));
  memcpy(request.auth.password, password, strlen(password));

  request.timestamp = timestamp;
  request.channel_id = channel_id;

  serialize_message_read_request(&request, buffer + HEADER_SIZE);

  write_exact(server, buffer, HEADER_SIZE + MESSAGE_READ_REQUEST_BODY_SIZE);
}

/* read a message read response from the server */
int message_read_response(int server, struct message_read_response *response) {
  uint8_t header_buffer[HEADER_SIZE];
  struct protocol_header header;
  uint8_t body_buffer[MIN_MESSAGE_READ_RESPONSE_BODY_SIZE + MAX_MESSAGE_SIZE];

  read_exact(server, header_buffer, HEADER_SIZE);

  deserialize_header(header_buffer, &header);

  if (header.status != STATUS_OK) {
    return -1;
  }

  if (header.type != MESSAGE_TYPE_MESSAGE_READ_RESPONSE) {
    return -1;
  }

  if (header.body_size > sizeof(body_buffer)) {
    return -1;
  }

  read_exact(server, body_buffer, header.body_size);

  memset(response, 0, sizeof(*response));
  deserialize_message_read_response(body_buffer, response);

  return 0;
}

/* connects the client to the server manager, gets the server ip, and then
 * connects to the server */
int connect_server_manager(const char *ip, uint16_t port) {
  int server_manager;
  char *server_ip;
  int server;

  server_manager = connect_socket(ip, port);
  if (server_manager == -1) {
    syslog(LOG_ERR, "connect_socket failed");
    return -1;
  }

  active_server_request(server_manager);
  server_ip = active_server_rsp(server_manager);
  if (server_ip == NULL) {
    close(server_manager);
    return -1;
  }

  close(server_manager);
  server = connect_socket(server_ip, port);
  if (server == -1) {
    syslog(LOG_ERR, "connect_socket failed");
    free(server_ip);
    return -1;
  }

  free(server_ip);
  return server;
}
