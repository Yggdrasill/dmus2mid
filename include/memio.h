/*
 * Copyright (C) 2017 Yggdrasill (kaymeerah@lambda.is)
 *
 * This file is part of dmus2mid.
 *
 * dmus2mid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dmus2mid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dmus2mid.  If not, see <http://www.gnu.org/licenses/>.
 */

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
