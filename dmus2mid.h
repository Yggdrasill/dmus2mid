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

#define MUS_HEADER_MAGIC  "MUS\x1A"

#define MIDI_HEADER_MAGIC "MThd\x00\x00\x00\x06"
#define MIDI_HEADER_DATA  "\x00\x00\x00\x01"
#define MIDI_MTRK_MAGIC   "\x4d\x54\x72\x6b"
#define MIDI_MTRK_LENGTH  "\x00\x00\x00\x00"
#define MIDI_KEYSIG_MAGIC "\x00\xFF\x59\x02\x00\x00"
#define MIDI_TEMPO_MAGIC  "\x00\xFF\x51\x03\x0F\x42\x40"

#define MUS_SYSTEM_MIN    10
#define MUS_SYSTEM_MAX    15

#define MIDI_SIZE_PITCH   2
#define MIDI_MAX_VARLEN   4
#define MIDI_MAX_CHANS    16

#define BUFFER_SIZE       65536

unsigned char mus_msb_set(unsigned char);
unsigned char mus_msb_exclude(unsigned char);
uint32_t mus_delay_read(uint32_t, unsigned char);
unsigned char mus_event_type(unsigned char);
unsigned char mus_event_chan(unsigned char);
unsigned char mus_control_fix(unsigned char);
uint32_t mus2mid_delay_conv(uint32_t, char *);
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

struct Buffer {
  size_t bufsize;
  size_t length;
  size_t offset;
  size_t io_count;
  char *buffer;
};

struct MIDIchan {
  unsigned char event;
  unsigned char prev_event;
  unsigned char channel;
  unsigned char volume;
  unsigned char ctrl;
  unsigned char args[2];
  unsigned char dtime[MIDI_MAX_VARLEN];
};

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

const char MUS2MID_CTRL_TABLE[] = {
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

#endif
