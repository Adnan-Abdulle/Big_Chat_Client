#include "server.h"
#include "client_handler.h"
#include "manager.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>

enum { POLL_TIMEOUT_MILLISECONDS = 5000 };

/* zero out the server state and prepare it for use */
void initialize_server_state(struct server_state *state) {
  int i;

  memset(state, 0, sizeof(*state));
  state->next_account_id = 1;
  state->running = 1;
  state->manager_file_descriptor = -1;
  state->listener_file_descriptor = -1;

  for (i = 0; i < MAX_POLL_FILE_DESCRIPTORS; i++) {
    state->poll_file_descriptors[i].fd = -1;
  }
}

/* accept a new client and add it to the poll set */
static void accept_new_client(struct server_state *state) {
  int client_file_descriptor;
  int slot;

  client_file_descriptor = accept(state->listener_file_descriptor, NULL, NULL);
  if (client_file_descriptor == -1) {
    syslog(LOG_WARNING, "accept failed");
    return;
  }

  slot = -1;
  for (int i = FIRST_CLIENT_POLL_INDEX; i < MAX_POLL_FILE_DESCRIPTORS; i++) {
    if (state->poll_file_descriptors[i].fd == -1) {
      slot = i;
      break;
    }
  }

  if (slot == -1) {
    syslog(LOG_WARNING, "max clients reached, rejecting connection");
    close(client_file_descriptor);
    return;
  }

  state->poll_file_descriptors[slot].fd = client_file_descriptor;
  state->poll_file_descriptors[slot].events = POLLIN;
  state->number_of_connections++;
  printf("  client connected (slot %d, total %d)\n", slot,
         state->number_of_connections);
}

/* close a client connection and remove it from the poll set */
static void disconnect_client(struct server_state *state, int slot) {
  printf("  client disconnected (slot %d)\n", slot);
  close(state->poll_file_descriptors[slot].fd);
  state->poll_file_descriptors[slot].fd = -1;
  state->poll_file_descriptors[slot].events = 0;
  state->poll_file_descriptors[slot].revents = 0;
  state->number_of_connections--;
}

/* check one client poll slot and handle its message if ready */
static void check_one_client(struct server_state *state, int slot) {
  short revents;
  int result;

  if (state->poll_file_descriptors[slot].fd == -1) {
    return;
  }

  revents = state->poll_file_descriptors[slot].revents;
  if ((revents & (POLLIN | POLLERR | POLLHUP)) == 0) {
    return;
  }

  result = handle_client_message(state->poll_file_descriptors[slot].fd, state);
  if (result == -1) {
    disconnect_client(state, slot);
  }
}

/* send a health check if enough time has passed */
static void maybe_send_health_check(struct server_state *state) {
  time_t now;

  now = time(NULL);
  if (now - state->last_health_check_time >= HEALTH_CHECK_INTERVAL_SECONDS) {
    send_health_check_to_manager(state->manager_file_descriptor,
                                 &state->server_ip, state->server_id);
    state->last_health_check_time = now;
  }
}

/* the main server event loop */
int run_server_event_loop(struct server_state *state) {
  state->poll_file_descriptors[MANAGER_POLL_INDEX].fd =
      state->manager_file_descriptor;
  state->poll_file_descriptors[MANAGER_POLL_INDEX].events = POLLIN;
  state->poll_file_descriptors[LISTENER_POLL_INDEX].fd =
      state->listener_file_descriptor;
  state->poll_file_descriptors[LISTENER_POLL_INDEX].events = POLLIN;
  state->last_health_check_time = time(NULL);

  printf("  entering event loop\n");

  while (state->running) {
    int poll_result;

    poll_result = poll(state->poll_file_descriptors, MAX_POLL_FILE_DESCRIPTORS,
                       POLL_TIMEOUT_MILLISECONDS);

    if (poll_result == -1) {
      if (errno == EINTR) {
        continue;
      }
      syslog(LOG_ERR, "poll failed");
      return -1;
    }

    maybe_send_health_check(state);

    if (state->poll_file_descriptors[MANAGER_POLL_INDEX].revents & POLLIN) {
      if (handle_manager_message(state->manager_file_descriptor,
                                 state->server_id,
                                 &state->is_activated) == -1) {
        syslog(LOG_ERR, "lost connection to manager");
        state->running = 0;
        break;
      }
    }

    if (state->is_activated &&
        (state->poll_file_descriptors[LISTENER_POLL_INDEX].revents & POLLIN)) {
      accept_new_client(state);
    }

    for (int i = FIRST_CLIENT_POLL_INDEX; i < MAX_POLL_FILE_DESCRIPTORS; i++) {
      check_one_client(state, i);
    }
  }

  return 0;
}

/* close all connections and clean up */
void shutdown_server(struct server_state *state) {
  for (int i = FIRST_CLIENT_POLL_INDEX; i < MAX_POLL_FILE_DESCRIPTORS; i++) {
    if (state->poll_file_descriptors[i].fd != -1) {
      close(state->poll_file_descriptors[i].fd);
      state->poll_file_descriptors[i].fd = -1;
    }
  }

  if (state->listener_file_descriptor != -1) {
    close(state->listener_file_descriptor);
    state->listener_file_descriptor = -1;
  }

  if (state->manager_file_descriptor != -1) {
    close(state->manager_file_descriptor);
    state->manager_file_descriptor = -1;
  }

  printf("  server shut down\n");
}
