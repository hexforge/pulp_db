#include <stdio.h>
#include <string.h>

#include "fline.h"

/*---
DATA
---*/
static FILE *fp;

/*---
FUNCTIONS
---*/
int fline__setup(char *file_path)
{
    extern FILE *fp;
    if ((fp=fopen(file_path, "rb"))==NULL)
    {
        fprintf(stderr, "Can't open file '%s' \n", file_path);
        return 1;
    }
    return 0;
}
int fline__close(void)
{
    extern FILE *fp;
    fclose(fp);   //< Whaqt does this return
    return 0;
}

int fline__get_line(char *data, const int maxlen)
{
    if (fgets(data, maxlen, fp)==NULL)
    {
        fprintf(stderr, "Can't get line");
        return 0;
    }
    int len = strlen(data);
    data[len-1] = 0;
    return len;
}
