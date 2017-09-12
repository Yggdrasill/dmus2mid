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

#define NORUNNING   arg_mask & ARGS_NORUNNING \
                    || arg_mask & ARGS_USERUNNING \
                    && (chans[midi_chan].event != chans[midi_chan].prev_event \
                    || midi_chan != prev_chan)

inline unsigned char mus_msb_set(unsigned char byte)
{
  return byte >> 7;
}

inline unsigned char mus_msb_exclude(unsigned char byte)
{
  return byte & 0x7F;
}

inline uint32_t mus_delay_read(uint32_t delay, unsigned char byte)
{
  uint32_t retval;

  byte = byte & 0x7F;
  retval = (delay << 7) + byte;

  return retval;
}

inline unsigned char mus_event_type(unsigned char byte)
{
  return byte >> 4 & 0x7;
}

inline unsigned char mus_event_chan(unsigned char byte)
{
  return byte & 0xF;
}

inline unsigned char mus_control_fix(unsigned char byte)
{
  return byte & 0x80 ? 0x7F : byte;
}

inline uint32_t mus2mid_delay_conv(uint32_t mus_delay, char *dtime)
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

inline unsigned char mid_channel_fix(unsigned char byte)
{
  return byte == 0x0F ? 0x09 : byte == 0x09 ? 0x0F : byte;
}

int args_parse(int argc, char **argv, char **fname_mus,
               char **fname_mid, uint16_t *tpqn)
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
          printf("Ridiculous TPQN, ignoring\n",
                 MUS2MID_TPQN_DEFAULT);
        }
        break;
    }
  }

  *tpqn = stpqn;

  *fname_mus = argv[optind];
  *fname_mid = argv[optind + 1];

  return mask;
}


int main(int argc, char **argv)
{
  struct stat st;
  struct MIDIchan chans[16] = {0};

  FILE *mus;
  FILE *mid;

  size_t size;

  long mtrk_len_offset;

  uint32_t pos;
  uint32_t mus_delay;

  int arg_mask;

  uint16_t mus_len;
  uint16_t mus_off;
  uint16_t tpqn;

  char read_buffer[BUFFER_SIZE];
  char *write_buffer = calloc(BUFFER_SIZE, sizeof(*write_buffer) );

  unsigned char delay;
  unsigned char event;
  unsigned char channel;
  unsigned char midi_chan;
  unsigned char prev_chan;
  unsigned char args[2];

  char byte;

  char *fname_mus;
  char *fname_mid;

  if(argc < 3) {
    printf("Too few arguments\n");
    exit(EXIT_FAILURE);
  }

  arg_mask = args_parse(argc, argv, &fname_mus, &fname_mid, &tpqn);

  if(!fname_mus) {
    puts("MUS filename invalid\n");
    exit(EXIT_FAILURE);
  }

  if(!fname_mid) {
    puts("MID filename invalid\n");
    exit(EXIT_FAILURE);
  }

  if(stat(fname_mus, &st) ) {
    perror(fname_mus);
    exit(EXIT_FAILURE);
  }

  size = st.st_size;
  byte = 0;
  delay = 0;
  pos = 0;
  prev_chan = 0xFF;

  mus = fopen(fname_mus, "rb");

  if(!mus) {
    perror(fname_mus);
    exit(EXIT_FAILURE);
  }

  fread(read_buffer, sizeof(*read_buffer), size, mus);

  if(memcmp(MUS_HEADER_MAGIC, read_buffer, sizeof(MUS_HEADER_MAGIC) - 1) ) {
    printf("Not a MUS file!\n");
    exit(EXIT_FAILURE);
  }

  mid = fopen(fname_mid, "wb");

  if(!mid) {
    perror(fname_mid);
    exit(EXIT_FAILURE);
  }

  fwrite(MIDI_HEADER_MAGIC, 1, sizeof(MIDI_HEADER_MAGIC) - 1, mid);
  fwrite(MIDI_HEADER_DATA, 1, sizeof(MIDI_HEADER_DATA) - 1, mid);

  tpqn = htons(tpqn);

  fwrite(&tpqn, sizeof(tpqn), 1, mid);
  fwrite(MIDI_MTRK_MAGIC, 1, sizeof(MIDI_MTRK_MAGIC) - 1, mid);

  memcpy(write_buffer, MIDI_TEMPO_MAGIC, sizeof(MIDI_TEMPO_MAGIC) - 1);
  pos += sizeof(MIDI_TEMPO_MAGIC) - 1;
  write_buffer[pos] = 0x00;
  pos++;
  mtrk_len_offset = ftell(mid);
  fwrite(MIDI_MTRK_LENGTH, 1, sizeof(MIDI_MTRK_LENGTH) - 1, mid);

  fseek(mus, 4, SEEK_SET);
  fread(&mus_len, sizeof(mus_len), 1, mus);
  fread(&mus_off, sizeof(mus_off), 1, mus);

  for(uint16_t i = mus_off; i < mus_len + mus_off; i++) {
    byte = read_buffer[i];
    delay = mus_msb_set(byte);
    event = mus_event_type(byte);
    channel = mus_event_chan(byte);
    midi_chan = mid_channel_fix(channel);
    mus_delay = 0;

    args[0] = read_buffer[++i];
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
      case MUS_FINISH:
        event = MUS_UNKNOWN2;
        midi_chan = 0;
        args[0] = 0x2F;
        delay = 0;
        chans[midi_chan].dtime[0] = 0;
        break;
      case MUS_NOTE_ON:
        if(mus_msb_set(args[0]) ) args[1] = read_buffer[++i];
        else args[1] = chans[midi_chan].volume;
        args[0] = mus_msb_exclude(args[0]);
        chans[midi_chan].volume = args[1];
        break;
      case MUS_SYS_EVENT:
        args[0] = MUS2MID_CTRL_TABLE[args[0]];
        args[1] = 0x00;
        break;
      case MUS_CTRL_EVENT:
        if(args[0] != 0x00) {
          args[0] = MUS2MID_CTRL_TABLE[args[0]];
          args[1] = read_buffer[++i];
          args[1] = mus_control_fix(args[1]);
        } else {
          event = 5;
          args[0] = read_buffer[++i];
        }
        break;
      default:
        exit(-1);
    }

    chans[midi_chan].event = MUS2MID_EVENT_TABLE[event];
    chans[midi_chan].channel = midi_chan;
    chans[midi_chan].args[0] = args[0];
    chans[midi_chan].args[1] = args[1];

    if(delay) {
      do {
        delay = read_buffer[++i];
        mus_delay = mus_delay_read(mus_delay, delay);
      } while(mus_msb_set(delay) );

      mus2mid_delay_conv(mus_delay, chans[midi_chan].dtime);
    }

    if(NORUNNING) {
      write_buffer[pos++] = chans[midi_chan].event |
                            chans[midi_chan].channel;
    }

    write_buffer[pos++] = chans[midi_chan].args[0];

    if(chans[midi_chan].args[1] != 0xFF) {
      write_buffer[pos++] = chans[midi_chan].args[1];
    }

    int j = 0;
    do {
      write_buffer[pos++] = chans[midi_chan].dtime[j];
      chans[midi_chan].dtime[j] = 0;
    } while(chans[midi_chan].dtime[++j] && j < MIDI_MAX_VARLEN);

    chans[midi_chan].prev_event = chans[midi_chan].event;
    prev_chan = midi_chan;
  }

  fwrite(write_buffer, 1, pos, mid);
  fwrite("\xFF\x2F\x00", 1, 4, mid);
  free(write_buffer);
  fseek(mid, mtrk_len_offset, SEEK_SET);
  pos = htonl(pos);
  fwrite(&pos, sizeof(pos), 1, mid);

  fclose(mus);
  fclose(mid);

  return 0;
}
