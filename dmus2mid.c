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

  FILE *mus;

  size_t size;

  int cmp;

  uint16_t mus_len;
  uint16_t mus_off;

  uint32_t total_delay;
  unsigned char cur_delay;

  char buffer[BUFFER_SIZE];

  unsigned char delay;
  unsigned char event;
  unsigned char channel;
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
    cur_delay = 0;
    total_delay = 0;

    args[0] = buffer[++i];
    args[1] = 0x00;

    if(event != MUS_NOTE_ON && event != MUS_CTRL_EVENT) {
      args[1] = 0xFF;
    } else if(event == MUS_NOTE_ON && mus_msb_set(args[0]) ) {
      args[1] = buffer[++i];
      last_vol[channel] = args[1];
    } else if(event == MUS_NOTE_ON) {
      args[1] = last_vol[channel];
    } else if(event == MUS_CTRL_EVENT && args[0] != 0x00) {
      args[1] = buffer[++i];
    } else if(event == MUS_CTRL_EVENT) {
      args[0] = buffer[++i];
      args[1] = 0xFF;
      event = 5;
    }

    if(delay) {
      do {
        byte = buffer[++i];
        cur_delay = byte & 0x7F;
        total_delay = (total_delay << 7) + cur_delay;
      } while(byte & 0x80);
    }

    printf("%4x %d %d:%-16s%2x %2x %2x %2x\n", i, delay >> 7, event, MUS2MID_EVENT_STR[event], channel, args[0], args[1], total_delay);
  }

  printf("%d %d %d\n", cmp, mus_len, mus_off);

  fclose(mus);

  return 0;
}
