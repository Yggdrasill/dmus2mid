#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dmus2mid.h"

inline unsigned char mus_msb_set(unsigned char byte)
{
  return byte >> 7;
}

inline unsigned char mus_event_type(unsigned char byte)
{
  return byte >> 4 & 0x7;
}

inline unsigned char mus_event_chan(unsigned char byte)
{
  return byte & 0xF;
}

int main(int argc, char **argv)
{
  struct stat st;
  struct MIDIchan chans[16] = {0};

  FILE *mus;

  size_t size;

  int cmp;

  uint16_t mus_len;
  uint16_t mus_off;

  uint32_t total_delay;
  uint32_t mus_delay;
  uint32_t midi_delay;
  unsigned char cur_delay;

  char buffer[BUFFER_SIZE];

  unsigned char delay;
  unsigned char event;
  unsigned char channel;
  unsigned char midi_chan;
  unsigned char args[2];

  char byte;
  char last_vol[16] = {0};

  if(argc < 2) {
    printf("Too few arguments\n");
    exit(EXIT_FAILURE);
  }

  stat(argv[1], &st);
  size = st.st_size;
  byte = 0;
  delay = 0;

  total_delay = 0;
  cur_delay = 0;

  mus = fopen(argv[1], "rb");

  fread(buffer, sizeof(*buffer), size, mus);

  cmp = strncmp(MUS_HEADER_MAGIC, buffer, strlen(MUS_HEADER_MAGIC) );

  fseek(mus, 4, SEEK_SET);
  fread(&mus_len, sizeof(mus_len), 1, mus);
  fread(&mus_off, sizeof(mus_off), 1, mus);

  for(uint16_t i = mus_off; i < mus_len + mus_off; i++) {
    byte = buffer[i];
    delay = mus_msb_set(byte);
    event = mus_event_type(byte);
    channel = mus_event_chan(byte);
    midi_chan = channel == 0x0F ? 0x09 : channel == 0x09 ? 0x0F : channel;
    cur_delay = 0;
    total_delay = 0;
    midi_delay = 0;
    mus_delay = 0;

    args[0] = buffer[++i];
    args[1] = 0xFF;

    switch(event) {
      case MUS_NOTE_OFF:
        args[1] = 0x40;
      case MUS_PITCH_BEND:
      case MUS_FINISH:
        break;
      case MUS_NOTE_ON:
        if(mus_msb_set(args[0]) ) args[1] = buffer[++i];
        else args[1] = last_vol[channel];
        args[0] = args[0] & 0x7F;
        last_vol[channel] = args[1];
        break;
      case MUS_SYS_EVENT:
        args[0] = MUS2MID_CTRL_TABLE[args[0]];
        args[1] = 0x00;
        break;
      case MUS_CTRL_EVENT:
        if(args[0] != 0x00) {
          args[0] = MUS2MID_CTRL_TABLE[args[0]];
          args[1] = buffer[++i];
          args[1] = (args[1] & 0x80 ? 0x7F : args[1]);
        } else {
          event = 5;
          args[0] = buffer[++i];
        }
        break;
      default:
        exit(-1);
    }

    if(delay) {
      do {
        byte = buffer[++i];
        cur_delay = byte & 0x7F;
        total_delay = (total_delay << 7) + cur_delay;
      } while(byte & 0x80);

      mus_delay = total_delay;
      midi_delay = mus_delay & 0x7F;
      while( (mus_delay >>= 7) ) {
        midi_delay <<= 8;
        midi_delay |= 0x80;
        midi_delay += (mus_delay & 0x7F);
      }

      mus_delay = midi_delay;
      for(size_t j = 0; j < sizeof(mus_delay) && midi_delay; j++) {
        chans[midi_chan].dtime[j] = (unsigned char)midi_delay;
        midi_delay >>= 8;
      }
    }

    printf("%4x %d %d:%-16s%2x %2x %2x %2x\n", i, delay >> 7, event, MUS2MID_EVENT_STR[event], channel, args[0], args[1], total_delay);
  }

  printf("%d %d %d\n", cmp, mus_len, mus_off);

  fclose(mus);

  return 0;
}
