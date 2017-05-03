#ifndef DMUS2MID_H
#define DMUS2MID_H

#define MUS_HEADER_MAGIC  "MUS\x1A"

#define MIDI_HEADER_MAGIC "MThd\x00\x00\x00\x06"
#define MIDI_HEADER_DATA  "\x00\x00\x00\x01"
#define MIDI_KEYSIG_MAGIC "\x00\xFF\x59\x02\x00\x00"
#define MIDI_TEMPO_MAGIC  "\x00\0xFF\x51\x03\x0F\x42\x40"

#define MUS_SYSTEM_MIN    10
#define MUS_SYSTEM_MAX    15

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

#endif
