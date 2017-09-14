#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "../include/mus.h"
#include "../include/memio.h"

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

int mus_metadata_read(FILE *mus,
                      struct Buffer *read_buffer,
                      uint16_t *mus_channels)
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
