#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  if(argc < 2) {
    printf("Too few arguments\n");
    exit(EXIT_FAILURE);
  }

  stat(argv[1], &st);
  size = st.st_size;

  mus = fopen(argv[1], "rb");

  fread(buffer, sizeof(*buffer), size, mus);

  printf("%d\n", strncmp(MUS_HEADER_MAGIC, buffer, strlen(MUS_HEADER_MAGIC) ) );

  fclose(mus);

  return 0;
}
