#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "dmus2mid.h"

int main(int argc, char **argv)
{
  struct stat st;

  FILE *mus;

  char buffer[BUFFER_SIZE];

  size_t size;

  int cmp;

  uint16_t mus_len;
  uint16_t mus_off;

  if(argc < 2) {
    printf("Too few arguments\n");
    exit(EXIT_FAILURE);
  }

  stat(argv[1], &st);
  size = st.st_size;

  mus = fopen(argv[1], "rb");

  fread(buffer, sizeof(*buffer), size, mus);

  cmp = strncmp(MUS_HEADER_MAGIC, buffer, strlen(MUS_HEADER_MAGIC) );

  fseek(mus, 4, SEEK_SET);
  fread(&mus_len, sizeof(mus_len), 1, mus);
  fread(&mus_off, sizeof(mus_off), 1, mus);

  printf("%d %d %d\n", cmp, mus_len, mus_off);

  fclose(mus);

  return 0;
}
