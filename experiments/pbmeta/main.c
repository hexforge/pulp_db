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

#define MAX_HEADER_SIZE 200

/*---
DATA
---*/
struct arg_struct
{
    char *file_path;
};

struct pb_struct
{
    char ip[16];
    char port[6];
    char time[18];
    unsigned int header_size;
    unsigned int payload_size;
};

/*---
FUNCTIONS
---*/
void exit_with_error(char *err_msg);
void exit_with_error(char *err_msg)
{
    printf("%s\n", err_msg);
    exit(1);
}

struct arg_struct handle_args(int argc, char *argv[]);
struct arg_struct handle_args(int argc, char *argv[])
{
    struct arg_struct arg_data;
    if (argc!=1)
    {
        arg_data.file_path = argv[1];
        printf("Plyabck file location '%s'\n", arg_data.file_path);
    }
    else
    {
        printf("We need one argument the playback path.  We got %d args.\n", argc);
        exit(1);
    }
    
    return arg_data;
}

int setup(char *file_path);
int setup(char *file_path)
{
    return mmbuf__setup(file_path);
}

int teardown();
int teardown()
{
    return mmbuf__close();
}

int fill_till_slash(const char *data, char *target);
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

static char payload_size[8];
int parse_header(const char *data, struct pb_struct *p);
int parse_header(const char *data, struct pb_struct *p)
{
    extern char payload_size[8];
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
    index += fill_till_slash(data+index, payload_size);
    index += fill_till_slash(data+index, p->ip);
    index += fill_till_slash(data+index, p->port);
    index += 13;
    index += fill_till_slash(data+index, p->time);
    index += 16;
    p->header_size = index;
    p->payload_size = atoi(payload_size);

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //printf("DEBUG:: -2 of header %c\n", data[index-2]);
    //printf("DEBUG:: -1 of header %c\n", data[index-1]);
    //printf("DEBUG:: first char of message %c\n", data[index]);
    //exit_with_error("Blah\n");
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    return 0;
}

int parse_pb();
int parse_pb()
{
    unsigned char *data;
    unsigned long offset = 0;
    unsigned long new_offset = 0;
    int h_length = 0;
    int p_length = 0;
    int amount_got = 0;
    int count = 0;

    const int length = MAX_HEADER_SIZE;
    struct pb_struct pb_data = {"0.0.0.0", "0", "12345.678", 0, 0};
    struct pb_struct *p_pb_data = &pb_data;
    
    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //char debug_string[MAX_HEADER_SIZE];  
    //int i;  
    //for (i=0; i<MAX_HEADER_SIZE; ++i)
    //    debug_string[i] = 0;
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    while (1)
    {
        amount_got = mmbuf__get_data(&data, offset, length);
        //printf("offset=%ld, data_p=%p\n", offset, data);
        if (amount_got==0)
        {
            printf("no data remaining :)\n");
            break;
        }

        if ((parse_header(data, p_pb_data))!=0)
        {
            exit_with_error("Fail to parse header\n");
        }
        count += 1;
        h_length = p_pb_data->header_size;
        p_length = p_pb_data->payload_size;
        new_offset = offset + h_length + p_length + 1;

        //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        //printf("offset=%ld, h_length=%d, p_length=%d, new_offset=%ld\n", offset, h_length, p_length, new_offset);
        //strncpy(debug_string, data, p_pb_data->header_size);
        //debug_string[p_pb_data->header_size] = 0;
        //printf("HEADER='%s'\n", debug_string);
        //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

        amount_got = mmbuf__get_data(&data, offset+h_length, p_length);
        if (amount_got<p_length)
        {
            exit_with_error("Not enough payload data\n");
        }
        //mmbuf__free_data(offset,  new_offset);
        offset = new_offset;
    }
    printf("Parsed %d messages\n", count);
    return 0;
}

int main(int argc, char *argv[])
{
    struct arg_struct arg_data;
    arg_data = handle_args(argc, argv);

    if (setup(arg_data.file_path)!=0)
        exit_with_error("Fail to setup\n");

    if (parse_pb()!=0)
        exit_with_error("Fail to parse_pb\n");

    if (teardown()!=0)
        exit_with_error("Fail to teardown\n");
}
