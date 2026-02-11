#ifndef SERVER_H
#define SERVER_H

#include "protocol.h"
#include <poll.h>
#include <stdint.h>
#include <time.h>

enum { MAX_CLIENTS = 64 };
enum { MAX_POLL_FILE_DESCRIPTORS = 2 + 64 };
enum { MANAGER_POLL_INDEX = 0 };
enum { LISTENER_POLL_INDEX = 1 };
enum { FIRST_CLIENT_POLL_INDEX = 2 };
enum { HEALTH_CHECK_INTERVAL_SECONDS = 5 };
enum { MAX_ACCOUNTS = 256 };

/* one account stored in memory */
struct account_record {
    char username[USERNAME_SIZE];
    char password[PASSWORD_SIZE];
    uint8_t account_id;
    uint8_t status;
};

/* everything the server needs to operate */
struct server_state {
    uint8_t server_id;
    struct ipv4_address server_ip;
    int is_activated;
    int running;

    int manager_file_descriptor;
    int listener_file_descriptor;

    struct pollfd poll_file_descriptors[MAX_POLL_FILE_DESCRIPTORS];
    int number_of_connections;

    struct account_record accounts[MAX_ACCOUNTS];
    uint8_t next_account_id;
    time_t last_health_check_time;
};

void initialize_server_state(struct server_state *state);
int run_server_event_loop(struct server_state *state);
void shutdown_server(struct server_state *state);

#endif
