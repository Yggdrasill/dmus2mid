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

#ifndef MID_H
#define MID_H

#define MIDI_HEADER_MAGIC "MThd\x00\x00\x00\x06"
#define MIDI_HEADER_DATA  "\x00\x00\x00\x01"
#define MIDI_MTRK_MAGIC   "\x4d\x54\x72\x6b"
#define MIDI_MTRK_FILESZ  "\x00\x00\x00\x00"
#define MIDI_KEYSIG_MAGIC "\x00\xFF\x59\x02\x00\x00"
#define MIDI_TEMPO_MAGIC  "\x00\xFF\x51\x03"
#define MIDI_END_OF_TRACK "\xFF\x2F"

#define MIDI_HEADER_LENGTH  (sizeof(MIDI_HEADER_MAGIC) - 1)
#define MIDI_HDATA_LENGTH   (sizeof(MIDI_HEADER_DATA) - 1)
#define MIDI_MTRK_LENGTH    (sizeof(MIDI_MTRK_MAGIC) - 1)
#define MIDI_MTRK_FSZLEN    (sizeof(MIDI_MTRK_FILESZ) - 1)
#define MIDI_TEMPO_LENGTH   (sizeof(MIDI_TEMPO_MAGIC) - 1)
#define MIDI_EOT_LENGTH     (sizeof(MIDI_END_OF_TRACK) - 1)

#define MIDI_FACTOR_DEFAULT 1
#define MIDI_TEMPO_DEFAULT  0x000F4240L

#define MIDI_SIZE_PITCH   2
#define MIDI_MAX_VARLEN   4
#define MIDI_MAX_CHANS    16

unsigned char mid_channel_fix(unsigned char);
size_t mid_tempo_write(FILE *, uint8_t);
size_t mid_metadata_write(FILE *, uint16_t, uint8_t);

struct MIDIchan {
  unsigned char event;
  unsigned char prev_event;
  unsigned char channel;
  unsigned char volume;
  unsigned char ctrl;
  unsigned char args[2];
  unsigned char dtime[MIDI_MAX_VARLEN];
};

#endif
