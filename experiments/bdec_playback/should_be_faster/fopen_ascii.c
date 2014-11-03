#include "fopen_ascii.h"

static FILE *fp;
int setup_fmap_ascii(char *file_path)
{
    extern FILE *fp;
    if ((fp=fopen(file_path, "rb"))==NULL)
    {
        fprintf(stderr, "Can't open file '%s' \n", file_path);
        return 1;
    }
    return 0;
}
int close_fmap_ascii(void)
{
    extern FILE *fp;
    fclose(fp);   //< Whaqt does this return
    return 0;
}

int getdata(char *data, const int maxlen)
{
    if (fgets(data, maxlen, fp)==NULL)
    {
        fprintf(stderr, "Can't get line");
        return 0;
    }
    int len = strlen(data);
    //data[len-1] = 0;
    return len;
}
