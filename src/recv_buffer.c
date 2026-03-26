#include "recv_buffer.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>

/* zero the buffer and set initial state */
void recv_buffer_init(struct recv_buffer *buffer) {
  memset(buffer, 0, sizeof(*buffer));
  buffer->needed = HEADER_SIZE;
  buffer->have_header = 0;
}

/* do one non-blocking read from fd, append to buffer.
 * returns RECV_OK, RECV_EOF, or RECV_ERROR */
int recv_buffer_read(struct recv_buffer *buffer, int fd) {
  ssize_t n;

  /* buffer is full, caller must drain via try_parse first */
  if (buffer->filled >= sizeof(buffer->data)) {
    return RECV_OK;
  }

  n = recv(fd, buffer->data + buffer->filled,
           sizeof(buffer->data) - buffer->filled, MSG_DONTWAIT);

  if (n > 0) {
    buffer->filled += (size_t)n;
    return RECV_OK;
  }

  if (n == 0) {
    return RECV_EOF;
  }

  /* n == -1 */
  if (errno == EAGAIN || errno == EINTR) {
    return RECV_OK;
  }
#if EAGAIN != EWOULDBLOCK
  if (errno == EWOULDBLOCK) {
    return RECV_OK;
  }
#endif

  return RECV_ERROR;
}

/* try to parse a complete message from the buffer.
 * returns 1 if a full message was extracted, 0 if not enough data yet,
 * RECV_PARSE_SKIPPED if the body was too large for body_capacity */
int recv_buffer_try_parse(struct recv_buffer *buffer,
                          struct protocol_header *header, uint8_t *body,
                          size_t body_capacity, uint32_t *body_size) {
  size_t remainder;

  /* step 1: try to deserialize the header if we haven't yet */
  if (!buffer->have_header && buffer->filled >= HEADER_SIZE) {
    deserialize_header(buffer->data, &buffer->parsed_header);
    buffer->needed = HEADER_SIZE + (size_t)buffer->parsed_header.body_size;
    buffer->have_header = 1;

    /* message too large to ever fit in the buffer, discard everything */
    if (buffer->needed > sizeof(buffer->data)) {
      recv_buffer_init(buffer);
      return 0;
    }
  }

  /* step 2: check if we have a complete message */
  if (!buffer->have_header || buffer->filled < buffer->needed) {
    return 0;
  }

  /* we have a complete message */
  if (buffer->parsed_header.body_size > body_capacity) {
    /* body too large for caller buffer; skip this message */
    remainder = buffer->filled - buffer->needed;
    memmove(buffer->data, buffer->data + buffer->needed, remainder);
    buffer->filled = remainder;
    buffer->have_header = 0;
    buffer->needed = HEADER_SIZE;
    return RECV_PARSE_SKIPPED;
  }

  /* copy header to caller */
  *header = buffer->parsed_header;
  *body_size = buffer->parsed_header.body_size;

  /* copy body to caller-provided buffer */
  memcpy(body, buffer->data + HEADER_SIZE, buffer->parsed_header.body_size);

  /* shift remaining data to front of buffer */
  remainder = buffer->filled - buffer->needed;
  memmove(buffer->data, buffer->data + buffer->needed, remainder);
  buffer->filled = remainder;

  /* reset for next message */
  buffer->have_header = 0;
  buffer->needed = HEADER_SIZE;

  return 1;
}
