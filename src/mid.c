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

#include <stdio.h>
#include <stdint.h>

#include <arpa/inet.h>

#include "../include/mid.h"

unsigned char mid_channel_fix(unsigned char byte)
{
  return byte == 0x0F ? 0x09 : byte == 0x09 ? 0x0F : byte;
}

size_t mid_metadata_write(FILE *mid, uint16_t tpqn)
{
  size_t retval;

  retval = fwrite(MIDI_HEADER_MAGIC, 1, MIDI_HEADER_LENGTH, mid);
  retval += fwrite(MIDI_HEADER_DATA, 1, MIDI_HDATA_LENGTH, mid);

  tpqn = htons(tpqn);
  retval += fwrite(&tpqn, sizeof(tpqn), 1, mid);

  retval += fwrite(MIDI_MTRK_MAGIC, 1, MIDI_MTRK_LENGTH, mid);
  retval += fwrite(MIDI_MTRK_FILESZ, 1, MIDI_MTRK_FSZLEN, mid);

  return retval;
}

