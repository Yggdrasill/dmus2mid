#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/memio.h"

char *buffer_init(struct Buffer *buffer, size_t size)
{
  if(!buffer) return NULL;

  buffer->bufsize = size;
  buffer->length = size;
  buffer->offset = 0;
  buffer->io_count = 0;
  buffer->buffer = malloc(size);

  return buffer->buffer;
}

void buffer_free(struct Buffer *buffer)
{
  if(!buffer) return;
  if(!buffer->buffer) return;

  free(buffer->buffer);
}

size_t mread(struct Buffer *src,
            void *dst,
            size_t size,
            size_t nmemb,
            FILE *in)
{
  size_t bufsize;
  size_t length;
  size_t offset;
  size_t bytes;

  char *buffer;

  if(!src || !dst || !in) return 0;
  if(size == 0 || nmemb == 0) return 0;

  if(nmemb > SIZE_MAX / size || size * nmemb > src->bufsize) {
    fputs("mread(): read too large\n", stderr);
    return 0;
  }

  bufsize = src->bufsize;
  length  = src->length;
  offset  = src->offset;
  buffer  = src->buffer;
  bytes   = size * nmemb;

  if(!buffer) return 0;
  if(!length) return 0;

  if(!src->io_count) {
    length = fread(buffer, sizeof(*buffer), bufsize, in);
    offset = 0;
  }
  else if(bytes >= bufsize - offset) {
    memmove(buffer, buffer + offset, bufsize - offset);
    offset = bufsize - offset;
    length = fread(buffer + offset, sizeof(*buffer), bufsize - offset, in);
    offset = 0;
  }

  if(bytes == 1) {
    *(char *)dst = buffer[offset];
  } else {
    memcpy(dst, buffer + offset, bytes);
  }

  src->length = length;
  src->io_count += bytes;
  src->offset = offset + bytes;

  return bytes;
}

size_t mwrite(struct Buffer *dst,
              void *src,
              size_t size,
              size_t nmemb,
              FILE *out)
{
  size_t retval;
  size_t bufsize;
  size_t offset;
  size_t bytes;

  char *buffer;

  if(!src || !out) return 0;
  if(size == 0 || nmemb == 0) return 0;

  /* prevent integer overflow and buffer overflow */

  if(nmemb > SIZE_MAX / size && size * nmemb > dst->bufsize) {
    fputs("mwrite(): elements too large\n", stderr);
    return 0;
  }

  retval  = 0;
  bufsize  = dst->bufsize;
  offset  = dst->offset;
  buffer  = dst->buffer;
  bytes   = size * nmemb;

  if(!buffer) return 0;

  if(bytes > bufsize - offset) {
    retval = fwrite(buffer, sizeof(*buffer), offset, out);
    retval += fwrite(src, size, nmemb, out);
    offset = 0;
  } else if(bytes > 1) {
    memcpy(buffer, src, bytes);
    retval = bytes;
    offset += bytes;
  } else if(bytes < 2) {
    buffer[offset] = *(char *)src;
    retval = 1;
    offset++;
  }

  dst->offset = offset;
  dst->io_count += bytes;

  return retval;
}

size_t mread_byte(struct Buffer *src, unsigned char *byte, FILE *in)
{
  return mread(src, byte, 1, 1, in);
}

size_t mwrite_byte(struct Buffer *dst, char byte, FILE *out)
{
  return mwrite(dst, &byte, 1, 1, out);
}

size_t mflush(struct Buffer *src, FILE *out)
{
  return fwrite(src->buffer, 1, src->offset, out);
}

int msetoffset(struct Buffer *src, size_t offset)
{
  if(!src) return -1;
  if(offset > src->bufsize) return -2;

  src->offset = offset;

  return 0;
}
