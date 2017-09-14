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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <unistd.h>

#include "../include/dmus2mid.h"
#include "../include/memio.h"
#include "../include/mid.h"
#include "../include/mus.h"

const unsigned char MUS2MID_EVENT_TABLE[] = {
  MIDI_NOTE_OFF,
  MIDI_NOTE_ON,
  MIDI_PITCH_BEND,
  MIDI_CTRL_EVENT,
  MIDI_CTRL_EVENT,
  MIDI_INSTR_CHNG,
  MIDI_END_TRACK,
  MIDI_META
};

const unsigned char MUS2MID_CTRL_TABLE[] = {
  0x00,
  MIDC_BANK_SELECT,
  MIDC_MOD_POT,
  MIDC_VOLUME,
  MIDC_PAN_POT,
  MIDC_EXPR_POT,
  MIDC_REVERB,
  MIDC_CHORUS,
  MIDC_HOLD,
  MIDC_SOFTP,
  MIDC_NOSND,
  MIDC_NONOTE,
  MIDC_MONO,
  MIDC_POLY,
  MIDC_RSTA
};

uint32_t mus2mid_delay_conv(uint32_t mus_delay, unsigned char *dtime)
{
  size_t i;
  uint32_t midi_delay;

  midi_delay = mus_delay & 0x7F;

  while( (mus_delay >>= 7) ) {
    midi_delay <<= 8;
    midi_delay |= 0x80;
    midi_delay += (mus_delay & 0x7F);
  }

  for(i = 0; i < MIDI_MAX_VARLEN && midi_delay; i++) {
    dtime[i] = (unsigned char)midi_delay;
    midi_delay >>= 8;
  }

  return midi_delay;
}

int isrunning(struct MIDIchan *chan, int arg_mask, unsigned char prev_chan)
{
  return arg_mask & ARGS_NORUNNING || arg_mask & ARGS_USERUNNING
         && (chan->event != chan->prev_event || chan->channel != prev_chan);
}

int args_parse(int argc,
              char **argv,
              char **fname_mus,
              char **fname_mid,
              uint16_t *tpqn)
{
  int arg;
  int mask;
  int stpqn;

  mask = 0;
  stpqn = MUS2MID_TPQN_DEFAULT;

  mask = DEFAULT_ARGS;

  while( (arg = getopt(argc, argv, "zreuqvt:") ) != -1) {
    switch(arg) {
      case 'r':
        mask |= ARGS_USERUNNING;
        mask &= ~(ARGS_NORUNNING);
        break;
      case 'z':
        mask |= ARGS_USEZEROVEL;
        mask &= ~(ARGS_NOZEROVEL);
        break;
      case 'e':
        mask |= ARGS_NOZEROVEL;
        mask &= ~(ARGS_USEZEROVEL);
        break;
      case 'u':
        mask |= ARGS_NORUNNING;
        mask &= ~(ARGS_USERUNNING);
        break;
      case 'q':
        mask |= ARGS_QUIET;
        mask &= ~(ARGS_VERBOSE);
        break;
      case 'v':
        mask |= ARGS_VERBOSE;
        mask &= ~(ARGS_QUIET);
        break;
      case 't':
        stpqn = strtol(optarg, NULL, 0);
        if(stpqn <= 0 || stpqn > MUS2MID_TPQN_MAX) {
          stpqn = MUS2MID_TPQN_DEFAULT;
          fputs("Ridiculous TPQN, ignoring\n", stderr);
        }
        break;
    }
  }

  *tpqn = stpqn;

  *fname_mus = argv[optind];
  *fname_mid = argv[optind + 1];

  return mask;
}

int mus2mid_convert(FILE *mid,
                    FILE *mus,
                    struct Buffer *read_buffer,
                    struct Buffer *write_buffer,
                    int arg_mask,
                    uint16_t mus_channels)
{
  struct MIDIchan channels[MIDI_MAX_CHANS];
  struct MIDIchan *channel;

  uint32_t mus_delay;

  int j;

  unsigned char delay;
  unsigned char event;
  unsigned char mus_channel;
  unsigned char midi_channel;
  unsigned char prev_chan;
  unsigned char args[2];

  unsigned char byte;

  memset(channels, 0x00, sizeof(channels) );

  event = 0;
  byte = 0;
  delay = 0;
  prev_chan = 0xFF;

  mwrite(write_buffer, MIDI_TEMPO_MAGIC, 1, MIDI_TEMPO_LENGTH, mid);

  for(midi_channel = 0; midi_channel < MIDI_MAX_CHANS; midi_channel++) {
    mwrite_byte(write_buffer, 0xB0 | midi_channel, mid);
    mwrite_byte(write_buffer, 0x07, mid);
    mwrite_byte(write_buffer, 0x7F, mid);
    mwrite_byte(write_buffer, 0x00, mid);
  }

  while(event != MUS_UNKNOWN2) {
    mread_byte(read_buffer, &byte, mus);
    delay = mus_msb_set(byte);
    event = mus_event_type(byte);
    mus_channel = mus_event_chan(byte);
    midi_channel = mid_channel_fix(mus_channel);
    channel = channels + midi_channel;
    mus_delay = 0;

    mread_byte(read_buffer, &args[0], mus);
    args[1] = 0xFF;

    switch(event) {
      case MUS_NOTE_OFF:
        if(arg_mask & ARGS_USEZEROVEL) {
          event = MUS_NOTE_ON;
          args[1] = 0x00;
        } else {
          args[1] = 0x40;
        }
        break;

      case MUS_PITCH_BEND:
        args[1] = args[0] >> 1;
        args[0] = (args[0] & 1) << 6;
        break;

      case MUS_FINISH:
        event = MUS_UNKNOWN2;
        midi_channel = 0;
        args[0] = 0x2F;
        delay = 0;
        channel->dtime[0] = 0;
        break;

      case MUS_NOTE_ON:
        if(mus_msb_set(args[0]) ) mread_byte(read_buffer, &args[1], mus);
        else args[1] = channel->volume;
        args[0] = mus_msb_exclude(args[0]);
        channel->volume = args[1];
        break;

      case MUS_SYS_EVENT:
        args[0] = MUS2MID_CTRL_TABLE[args[0]];
        args[1] = args[0] == MIDC_MONO ? mus_channels : 0x00;
        break;

      case MUS_CTRL_EVENT:
        if(args[0] != 0x00) {
          args[0] = MUS2MID_CTRL_TABLE[args[0]];
          mread_byte(read_buffer, &args[1], mus);
          args[1] = mus_control_fix(args[1]);
        } else {
          event = MUS_UNKNOWN1;
          mread_byte(read_buffer, &args[0], mus);
        }
        break;

      default:
        fprintf(stderr, "Unknown event %x\n", event);
        exit(-1);
    }

    channel->event = MUS2MID_EVENT_TABLE[event];
    channel->channel = midi_channel;
    channel->args[0] = args[0];
    channel->args[1] = args[1];

    if(delay) {
      do {
        mread_byte(read_buffer, &delay, mus);
        mus_delay = mus_delay_read(mus_delay, delay);
      } while(mus_msb_set(delay) );

      mus2mid_delay_conv(mus_delay, channel->dtime);
    }

    if(isrunning(channel, arg_mask, prev_chan) ) {
      mwrite_byte(write_buffer, channel->event | channel->channel, mid);
    }

    mwrite_byte(write_buffer, channel->args[0], mid);

    if(channel->args[1] != 0xFF) {
      mwrite_byte(write_buffer, channel->args[1], mid);
    }

    j = 0;
    do {
      mwrite_byte(write_buffer, channel->dtime[j], mid);
    } while(mus_msb_set(channel->dtime[j]) && ++j < MIDI_MAX_VARLEN);

    memset(channel->dtime, 0x00, MIDI_MAX_VARLEN);

    channel->prev_event = channel->event;
    prev_chan = midi_channel;
  }

  return 0;
}

int main(int argc, char **argv)
{
  struct Buffer read_buffer;
  struct Buffer write_buffer;

  FILE *mus;
  FILE *mid;

  long mtrk_len_offset;

  uint32_t mtrk_size;

  int arg_mask;

  uint16_t tpqn;
  uint16_t mus_channels;

  char *fname_mus;
  char *fname_mid;

  if(argc < 3) {
    fputs("Too few arguments\n", stderr);
    exit(EXIT_FAILURE);
  }

  arg_mask = args_parse(argc, argv, &fname_mus, &fname_mid, &tpqn);

  if(!fname_mus) {
    fputs("MUS filename invalid\n", stderr);
    exit(EXIT_FAILURE);
  }

  if(!fname_mid) {
    fputs("MID filename invalid\n", stderr);
    exit(EXIT_FAILURE);
  }

  mus = fopen(fname_mus, "rb");

  if(!mus) {
    perror(fname_mus);
    exit(EXIT_FAILURE);
  }

  mid = fopen(fname_mid, "wb");

  if(!mid) {
    perror(fname_mid);
    exit(EXIT_FAILURE);
  }

  if(!buffer_init(&read_buffer, BUFFER_SIZE) ) {
    perror(argv[0]);
    exit(EXIT_FAILURE);
  }

  if(!buffer_init(&write_buffer, BUFFER_SIZE) ) {
    perror(argv[0]);
    exit(EXIT_FAILURE);
  }

  mid_metadata_write(mid, tpqn);
  mtrk_len_offset = ftell(mid) - MIDI_MTRK_FSZLEN;

  mus_validate(mus, &read_buffer);
  mus_metadata_read(mus, &read_buffer, &mus_channels);

  mus2mid_convert(mid, mus, &read_buffer,
                  &write_buffer, arg_mask,
                  mus_channels);

  mflush(&write_buffer, mid);

  fseek(mid, mtrk_len_offset, SEEK_SET);
  mtrk_size = htonl( (uint32_t)write_buffer.io_count);
  fwrite(&mtrk_size, sizeof(mtrk_size), 1, mid);

  buffer_free(&read_buffer);
  buffer_free(&write_buffer);

  fclose(mus);
  fclose(mid);

  return 0;
}
