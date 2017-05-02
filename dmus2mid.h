#ifndef DMUS2MID_H
#define DMUS2MID_H

#define MUS_HEADER_MAGIC "MUS\x1A"

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
}

#endif
