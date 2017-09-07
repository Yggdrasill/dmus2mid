#ifndef DMUS2MID_H
#define DMUS2MID_H

#define MUS_HEADER_MAGIC  "MUS\x1A"

#define MIDI_HEADER_MAGIC "MThd\x00\x00\x00\x06"
#define MIDI_HEADER_DATA  "\x00\x00\x00\x01"
#define MIDI_KEYSIG_MAGIC "\x00\xFF\x59\x02\x00\x00"
#define MIDI_TEMPO_MAGIC  "\x00\0xFF\x51\x03\x0F\x42\x40"

#define MUS_SYSTEM_MIN    10
#define MUS_SYSTEM_MAX    15

#define MIDI_SIZE_PITCH   2
#define MIDI_MAX_VARLEN   4
#define MIDI_MAX_CHANS    16

#define BUFFER_SIZE       65536

enum MUS {
  MUS_NOTE_OFF      = 0x00,
  MUS_NOTE_ON       = 0x01,
  MUS_PITCH_BEND    = 0x02,
  MUS_SYS_EVENT     = 0x03,
  MUS_CTRL_EVENT    = 0x04,
  MUS_FINISH        = 0x06
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

struct MIDIchan {
  unsigned char cur_event;
  unsigned char prev_event;
  unsigned char chan;
  unsigned char vol;
  unsigned char ctrl;
  unsigned char c_arg;
  unsigned char instr;
  unsigned char pitch[MIDI_SIZE_PITCH];
  unsigned char dtime[MIDI_MAX_VARLEN];
};

const unsigned char MUS2MID_EVENT_TABLE[] = {
  MIDI_NOTE_OFF,
  MIDI_NOTE_ON,
  MIDI_PITCH_BEND,
  MIDI_CTRL_EVENT,
  MIDI_CTRL_EVENT,
  0x00,
  MIDI_END_TRACK,
  0x00
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

#endif
