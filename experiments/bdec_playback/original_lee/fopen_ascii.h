#ifndef fopen_asciiH
#define fopen_asciiH

#include <stdio.h>
#include <string.h>

int setup_fmap_ascii(char *file_path);
int close_fmap_ascii(void);
int getdata(char *data, const int maxlen);

#endif  //fopen_asciiH