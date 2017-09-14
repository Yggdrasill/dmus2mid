#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "dmus2mid.h"

unsigned char mus_msb_set(unsigned char byte)
{
  return byte >> 7;
}

unsigned char mus_msb_exclude(unsigned char byte)
{
  return byte & 0x7F;
}

uint32_t mus_delay_read(uint32_t delay, unsigned char byte)
{
  uint32_t retval;

  byte = byte & 0x7F;
  retval = (delay << 7) + byte;

  return retval;
}

unsigned char mus_event_type(unsigned char byte)
{
  return byte >> 4 & 0x7;
}

unsigned char mus_event_chan(unsigned char byte)
{
  return byte & 0xF;
}

unsigned char mus_control_fix(unsigned char byte)
{
  return byte & 0x80 ? 0x7F : byte;
}

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

unsigned char mid_channel_fix(unsigned char byte)
{
  return byte == 0x0F ? 0x09 : byte == 0x09 ? 0x0F : byte;
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

size_t mread_byte(struct Buffer *src, unsigned char *byte, FILE *in)
{
  return mread(src, byte, 1, 1, in);
}

int msetoffset(struct Buffer *src, size_t offset)
{
  if(!src) return -1;
  if(offset > src->bufsize) return -2;

  src->offset = offset;

  return 0;
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

size_t mwrite_byte(struct Buffer *dst,
                  char byte,
                  FILE *out)
{
  return mwrite(dst, &byte, 1, 1, out);
}

size_t mflush(struct Buffer *src, FILE *out)
{
  return fwrite(src->buffer, 1, src->offset, out);
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

int mus_validate(FILE *mus, struct Buffer *read_buffer)
{
  char mus_header[MUS_HEADER_LENGTH];

  mread(read_buffer, mus_header, sizeof(*mus_header), MUS_HEADER_LENGTH, mus);

  if(memcmp(MUS_HEADER_MAGIC, mus_header, MUS_HEADER_LENGTH) ) {
    fputs("Not a MUS file!\n", stderr);
    exit(EXIT_FAILURE);
  }

  return 0;
}

int mus_metadata_read(FILE *mus, struct Buffer *read_buffer, uint16_t *mus_channels)
{
  uint16_t mus_len;
  uint16_t mus_off;

  mread(read_buffer, &mus_len, sizeof(mus_len), 1, mus);
  mread(read_buffer, &mus_off, sizeof(mus_off), 1, mus);
  mread(read_buffer, mus_channels, sizeof(mus_channels), 1, mus);

  if(mus_len <= mus_off) {
    fputs("Unexpected end of file\n", stderr);
    exit(EXIT_FAILURE);
  }

  msetoffset(read_buffer, mus_off);

  return 0;
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

  fseek(mid, mtrk_len_offset, SEEK_SET);
  mtrk_size = htonl( (uint32_t)write_buffer.io_count);
  fwrite(&mtrk_size, sizeof(mtrk_size), 1, mid);

  mflush(&write_buffer, mid);

  buffer_free(&read_buffer);
  buffer_free(&write_buffer);

  fclose(mus);
  fclose(mid);

  return 0;
}
