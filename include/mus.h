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

#ifndef MUS_H
#define MUS_H

#define MUS_HEADER_MAGIC  "MUS\x1A"

#define MUS_HEADER_LENGTH (sizeof(MUS_HEADER_MAGIC) - 1)

#define MUS_SYSTEM_MIN    10
#define MUS_SYSTEM_MAX    15

typedef struct Buffer;

unsigned char mus_msb_set(unsigned char);
unsigned char mus_msb_exclude(unsigned char);
uint32_t mus_delay_read(uint32_t, unsigned char);
unsigned char mus_event_type(unsigned char);
unsigned char mus_event_chan(unsigned char);
unsigned char mus_control_fix(unsigned char);
int mus_validate(FILE *, struct Buffer *);
int mus_metadata_read(FILE *, struct Buffer *, uint16_t *);

#endif
