#ifndef RECV_BUFFER_H
#define RECV_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include "protocol.h"

/* recv_buffer_read return codes */
enum {
  RECV_OK = 0,       /* read some bytes (or EAGAIN, nothing available) */
  RECV_EOF = -1,     /* server closed connection */
  RECV_ERROR = -2    /* read error */
};

/* how many max-size messages the buffer can hold at once */
enum { RECV_BUFFER_MSG_CAPACITY = 4 };
enum {
  RECV_BUFFER_SIZE = RECV_BUFFER_MSG_CAPACITY *
                     (HEADER_SIZE + MIN_MESSAGE_READ_RESPONSE_BODY_SIZE +
                      MAX_MESSAGE_SIZE)
};

/* return code when body exceeds caller capacity */
enum { RECV_PARSE_SKIPPED = -1 };

struct recv_buffer {
  uint8_t data[RECV_BUFFER_SIZE];
  size_t filled;
  size_t needed;
  int have_header;
  struct protocol_header parsed_header;
};

/* zero the buffer and set initial state */
void recv_buffer_init(struct recv_buffer *buffer);

/* do one non-blocking read from fd, append to buffer.
 * returns RECV_OK, RECV_EOF, or RECV_ERROR */
int recv_buffer_read(struct recv_buffer *buffer, int fd);

/* try to parse a complete message from the buffer.
 * returns 1 if a full message was extracted, 0 if not enough data yet,
 * RECV_PARSE_SKIPPED if the body was too large for body_capacity (message
 * is consumed and discarded).
 * body is a caller-provided buffer of body_capacity bytes.
 * the caller must consume body before calling this function again. */
int recv_buffer_try_parse(struct recv_buffer *buffer,
                          struct protocol_header *header,
                          uint8_t *body, size_t body_capacity,
                          uint32_t *body_size);

#endif
