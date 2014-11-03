#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include "fopen.h"

/*---
DATA
---*/

/*---
FUNCTIONS
---*/
int fopen__setup(struct fopen_obj *fobj, char *file_path, char mode)
{   
    fobj->file_path = file_path;
    if (mode=='r')
        fobj->fp=fopen(file_path, "rb");
    else if (mode=='w')
        fobj->fp=fopen(file_path, "wb");
    else
        fobj->fp = NULL;
    if (fobj->fp==NULL)
    {
        fprintf(stderr, "Can't open file '%s' \n", file_path);
        return 1;
    }
    return 0;
}

int fopen__close(struct fopen_obj *fobj)
{
    fclose(fobj->fp);   //< Whaqt does this return
    return 0;
}

int fopen__get_data(struct fopen_obj *fobj, unsigned char *result_p, const unsigned long offset, const int length)
{
    int fs_rc;
    size_t amount_read;

    if (fobj->current_pos==offset)
    {
        amount_read = fread(result_p, length, 1, fobj->fp);
    }
    else
    {
        fs_rc = fseek(fobj->fp, offset, SEEK_SET);
        if (fs_rc!=0)
        {
            fprintf(stderr, "Seek failed %d\n", fs_rc);
            perror("Error");
            return -1; 
        }
        amount_read = fread(result_p, 1, length, fobj->fp);
    }

    if (amount_read==0)
    {
        if (errno==0)
        {
            return 0;
        }
        else
        {
            // Need to check if end of file here.
            fprintf(stderr, "Read failed %d\n", errno);
            perror("foobar");
            return -1;
        }
    
    }
    else if (amount_read==1)
    {
        amount_read = length; 
    }
    fobj->current_pos = offset + amount_read;
    return amount_read;
}

/*
int fopen__free_data(unsigned char *result)
{
    free(result);
}
*/

int fopen__write(struct fopen_obj *fobj, const unsigned char *data, const int length)
{
    size_t amount_wrote;
    amount_wrote = fwrite(data, length, 1, fobj->fp);
    if (amount_wrote==0)
    {
        fprintf(stderr, "Write failed");
        return -1;
    }
    return amount_wrote;
}






