#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/gallows.h"

void print_gallows(const char  stage,
                   const char  gallows_height,
                   const char  gallows_width,
                   const char *gallows_file)
{
  char to_line = (stage+1)*gallows_height;
  char from_line = to_line-gallows_height;

  FILE *fp = fopen(gallows_file, "rw+");
  if (fp == NULL)
  {
	  fprintf(stderr, "Error opening %s file.\n", gallows_file);
	  exit(2);
  }

	int current_line = 0 ;
  char buffer[gallows_width+2]; // +1 for newline
  do
  {
    fgets(buffer , sizeof(buffer) , fp);
    if(current_line >= from_line)
    {
      printf("%s", buffer);
		}
		current_line++;
  } while(current_line < to_line);
  fclose(fp);
}
