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

