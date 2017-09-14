#ifndef MEMIO_H
#define MEMIO_H

struct Buffer {
  size_t bufsize;
  size_t length;
  size_t offset;
  size_t io_count;
  char *buffer;
};

char *buffer_init(struct Buffer *, size_t);
void buffer_free(struct Buffer *);
size_t mread(struct Buffer *, void *, size_t, size_t, FILE *);
size_t mwrite(struct Buffer *, void *, size_t, size_t, FILE *);
size_t mread_byte(struct Buffer *src, unsigned char *, FILE *);
size_t mwrite_byte(struct Buffer *, char, FILE *);
size_t mflush(struct Buffer *, FILE *);
int msetoffset(struct Buffer *, size_t);

#endif
