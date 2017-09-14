#ifndef MUS_H
#define MUS_H

#define MUS_HEADER_MAGIC  "MUS\x1A"

#define MUS_HEADER_LENGTH (sizeof(MUS_HEADER_MAGIC) - 1)

#define MUS_SYSTEM_MIN    10
#define MUS_SYSTEM_MAX    15

typedef struct Buffer;

unsigned char mus_msb_set(unsigned char);
unsigned char mus_msb_exclude(unsigned char);
uint32_t mus_delay_read(uint32_t, unsigned char);
unsigned char mus_event_type(unsigned char);
unsigned char mus_event_chan(unsigned char);
unsigned char mus_control_fix(unsigned char);
int mus_validate(FILE *, struct Buffer *);
int mus_metadata_read(FILE *, struct Buffer *, uint16_t *);

#endif
