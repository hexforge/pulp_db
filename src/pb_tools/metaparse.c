//*M*E*T*A*S*T*A*R*T*/4/16/224.0.65.10/50005/00:20:09.177/1361406009.177703//*M*E*T*A*E*N*D*177703��S!

/*
#Takes a pointer that has enough data :)
# Get the size,ip,port,time and endpointer out of the data.
*/
// *M*E*T*A*S*T*A*R*T* = 19chars
// *M*E*T*A*E*N*D*= 15

#include <stdio.h>
#include <string.h>    // strncpy
#include <stdlib.h>    // atoi
#include "mmbuf.h"
#include "metaparse.h"

#define MAX_HEADER_SIZE 200

/*---
DATA
---*/

/*---
FUNCTIONS
---*/
static void exit_with_error(char *err_msg);
void exit_with_error(char *err_msg)
{
    printf("%s\n", err_msg);
    exit(1);
}

static int fill_till_slash(const char *data, char *target);
int fill_till_slash(const char *data, char *target)
{
    //printf("new fill %p\n", data);
    int i = 0;
    char c;
    while ((c=data[i])!='/')
    {  
        //printf("parsing char %d, %c\n", i, c);
        target[i++] = c;
    }
    
    target[i] = '\0';
    //printf("%dtarget='%s'\n", i, target);
    return i+1;   // +1 to move past the the slash
}

static char payload_size_str[8];
int parse_header(char *const data, struct metaparse__msg *const p);
int parse_header(char *const data, struct metaparse__msg *const p)
{
    extern char payload_size_str[8];
    if (data[0]!='*' || data[1]!='M')
    {
        printf("debug: next data boundary incorrect\n");
        int i;
        for (i=0; i<MAX_HEADER_SIZE; ++i)
        {
            printf("%c", data[i]);
        }
        exit_with_error("\nFail to hit next message\n");
    }
    int index = 22; //skip metastart
    index += fill_till_slash(data+index, payload_size_str);
    index += fill_till_slash(data+index, p->ip);
    index += fill_till_slash(data+index, p->port);
    index += 13;
    index += fill_till_slash(data+index, p->time);
    index += 16;
    p->msg = data;
    p->header_size = index;
    //p->payload = data+index;
    p->payload_size = atoi(payload_size_str);

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //printf("DEBUG:: -2 of header %c\n", data[index-2]);
    //printf("DEBUG:: -1 of header %c\n", data[index-1]);
    //printf("DEBUG:: first char of message %c\n", data[index]);
    //exit_with_error("Blah\n");
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    return 0;
}

int metaparse__setup(struct metaparse__pb *pb, const char *file_path, const char *mode)
{
    struct mmbuf__obj *m = malloc(sizeof(struct mmbuf__obj));
    pb->m = m;
    pb->offset = 0;
    return mmbuf__setup(m, file_path, mode);
}

int metaparse__teardown(struct metaparse__pb *pb)
{
    int rc = mmbuf__teardown(pb->m);
    free(pb->m);
    pb->m = NULL;
    return rc;
}

int metaparse__get_msg(struct metaparse__pb *pb, struct metaparse__msg *pmsg)
{
    struct mmbuf__obj *m = pb->m;
    int amount_got = 0;

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //char debug_string[MAX_HEADER_SIZE];  
    //int i;  
    //for (i=0; i<MAX_HEADER_SIZE; ++i)
    //    debug_string[i] = 0;
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    amount_got = mmbuf__get_data(m, (void **) &pb->data, pb->offset, MAX_HEADER_SIZE);
    //printf("offset=%ld, data_p=%p\n", offset, data);
    if (amount_got==0)
    {
        printf("no data remaining :)\n");
        return 1;
    }

    if ((parse_header(pb->data, pmsg))!=0)
    {
        exit_with_error("Fail to parse header\n");
        return 2;
    }
    const int h_length = pmsg->header_size;
    const int p_length = pmsg->payload_size;
    const long long new_offset = pb->offset + h_length + p_length + 1;

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //printf("offset=%ld, h_length=%d, p_length=%d, new_offset=%ld\n", pb->offset, h_length, p_length, new_offset);
    //strncpy(debug_string, pb->data, pmsg->header_size);
    //debug_string[pmsg->header_size] = 0;
    //printf("HEADER='%s'\n", debug_string);
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    amount_got = mmbuf__get_data(m, (void **) &pb->data, pb->offset+h_length, p_length);
    if (amount_got<p_length)
    {
        exit_with_error("Not enough payload data\n");
    }

    pb->offset = new_offset;
    return 0;
}

