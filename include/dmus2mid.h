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

#ifndef DMUS2MID_H
#define DMUS2MID_H

/*
 * It is unreasonable to ask any software sequencer
 * to keep up with less than 1 msec per tick. Hardware
 * sequencers or extremely low-latency systems could do
 * it, but most software sequencers can't.
 */

#define MUS2MID_TPQN_MAX      1000
#define MUS2MID_TPQN_DEFAULT  140

#define BUFFER_SIZE       65536

uint32_t mus2mid_delay_conv(uint32_t, unsigned char *);
unsigned char mid_channel_fix(unsigned char);

enum ARGS {
  ARGS_TPQN         = 0x01,
  ARGS_USEZEROVEL   = 0x02,
  ARGS_USERUNNING   = 0x04,
  ARGS_NOZEROVEL    = 0x08,
  ARGS_NORUNNING    = 0x10,
  ARGS_QUIET        = 0x20,
  ARGS_VERBOSE      = 0x40
};

#define DEFAULT_ARGS (ARGS_USERUNNING | ARGS_NOZEROVEL)

enum MUS {
  MUS_NOTE_OFF      = 0x00,
  MUS_NOTE_ON       = 0x01,
  MUS_PITCH_BEND    = 0x02,
  MUS_SYS_EVENT     = 0x03,
  MUS_CTRL_EVENT    = 0x04,
  MUS_UNKNOWN1      = 0x05,
  MUS_FINISH        = 0x06,
  MUS_UNKNOWN2      = 0x07
};

enum MIDI {
  MIDI_NOTE_OFF     = 0x80,
  MIDI_NOTE_ON      = 0x90,
  MIDI_PITCH_BEND   = 0xE0,
  MIDI_CTRL_EVENT   = 0xB0,
  MIDI_INSTR_CHNG   = 0xC0,
  MIDI_END_TRACK    = 0x2F,
  MIDI_META         = 0xFF
};

enum MIDC {
  MIDC_BANK_SELECT  = 0x20,
  MIDC_MOD_POT      = 0x01,
  MIDC_VOLUME       = 0x07,
  MIDC_PAN_POT      = 0x0A,
  MIDC_EXPR_POT     = 0x0B,
  MIDC_REVERB       = 0x5B,
  MIDC_CHORUS       = 0x5D,
  MIDC_HOLD         = 0x40,
  MIDC_SOFTP        = 0x43,
  MIDC_NOSND        = 0x78,
  MIDC_NONOTE       = 0x7B,
  MIDC_MONO         = 0x7E,
  MIDC_POLY         = 0x7F,
  MIDC_RSTA         = 0x79
};

enum MUSC {
  MUSC_INSTR_CHNG   = 0x00,
  MUSC_BANK_SELECT  = 0x01,
  MUSC_MOD_POT      = 0x02,
  MUSC_VOLUME       = 0x03,
  MUSC_PAN_POT      = 0x04,
  MUSC_EXPR_POT     = 0x05,
  MUSC_REVERB       = 0x06,
  MUSC_CHORUS       = 0x07,
  MUSC_SUSTAIN      = 0x08,
  MUSC_SOFT_PEDAL   = 0x09,
  MUSC_MUTE_ALL     = 0x0A,
  MUSC_NOTEOFF_ALL  = 0x0B,
  MUSC_MONO         = 0x0C,
  MUSC_POLY         = 0x0D,
  MUSC_RESET_CTRLS  = 0x0E
};

extern const unsigned char MUS2MID_EVENT_TABLE[];
extern const unsigned char MUS2MID_CTRL_TABLE[];

#endif
