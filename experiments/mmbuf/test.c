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
    char *pb_path;
    char *out_path;
};

struct pb_struct
{
    char ip[16];
    char port[6];
    char time[18];
    unsigned char *header;
    unsigned int header_size;
    unsigned char *payload;
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

static struct arg_struct handle_args(int argc, char *argv[]);
struct arg_struct handle_args(int argc, char *argv[])
{
    struct arg_struct arg_data;
    if (argc==3)
    {
        arg_data.pb_path = argv[1];
        arg_data.out_path = argv[2];
        printf("pb_file ='%s'\n", arg_data.pb_path);
        printf("out_file='%s'\n", arg_data.out_path);
    }
    else
    {
        printf("We need two args pb_path, out_path  We got %d args.\n", argc);
        exit(1);
    }
    
    return arg_data;
}

int setup(struct mmbuf_obj *m, char *file_path, char mode);
int setup(struct mmbuf_obj *m, char *file_path, char mode)
{
    return mmbuf__setup(m, file_path, mode);
}

int teardown(struct mmbuf_obj *m);
int teardown(struct mmbuf_obj *m)
{
    return mmbuf__close(m);
}

int fill_till_slash(const unsigned char *data, char *target);
int fill_till_slash(const unsigned char *data, char *target)
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
int parse_header(unsigned char *data, struct pb_struct *p);
int parse_header(unsigned char *data, struct pb_struct *p)
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
    p->header = data;
    p->header_size = index;
    p->payload = data+index;
    p->payload_size = atoi(payload_size_str);

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //printf("DEBUG:: -2 of header %c\n", data[index-2]);
    //printf("DEBUG:: -1 of header %c\n", data[index-1]);
    //printf("DEBUG:: first char of message %c\n", data[index]);
    //exit_with_error("Blah\n");
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    return 0;
}

int get_msg(struct mmbuf_obj *m, struct pb_struct *pmsg);
int get_msg(struct mmbuf_obj *m, struct pb_struct *pmsg)
{
    static unsigned char *data;
    static unsigned long offset = 0;
    static unsigned long new_offset = 0;
    static int h_length = 0;
    static int p_length = 0;
    static int amount_got = 0;

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //char debug_string[MAX_HEADER_SIZE];  
    //int i;  
    //for (i=0; i<MAX_HEADER_SIZE; ++i)
    //    debug_string[i] = 0;
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    amount_got = mmbuf__get_data(m, &data, offset, MAX_HEADER_SIZE);
    //printf("offset=%ld, data_p=%p\n", offset, data);
    if (amount_got==0)
    {
        printf("no data remaining :)\n");
        return 1;
    }

    if ((parse_header(data, pmsg))!=0)
    {
        exit_with_error("Fail to parse header\n");
        return 2;
    }
    h_length = pmsg->header_size;
    p_length = pmsg->payload_size;
    new_offset = offset + h_length + p_length + 1;

    //DEBUG<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    //printf("offset=%ld, h_length=%d, p_length=%d, new_offset=%ld\n", offset, h_length, p_length, new_offset);
    //strncpy(debug_string, data, pmsg->header_size);
    //debug_string[pmsg->header_size] = 0;
    //printf("HEADER='%s'\n", debug_string);
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    amount_got = mmbuf__get_data(m, &data, offset+h_length, p_length);
    if (amount_got<p_length)
    {
        exit_with_error("Not enough payload data\n");
    }

    offset = new_offset;
    return 0;

}

static int parse_pb(struct mmbuf_obj *m, struct mmbuf_obj *out_obj);
int parse_pb(struct mmbuf_obj *m, struct mmbuf_obj *out_obj)
{
    int count = 0;
    int rc;
    struct pb_struct pb_data = {"0.0.0.0", "0", "12345.678", 0, 0};
    
    while (1)
    {
        rc = get_msg(m, &pb_data);
        if (rc==1)
        {
            printf("Finished");
            break;
        }
        else if (rc==2)
        {
            printf("Error");
            break;
        }
        count += 1;
        //printf("count=%d\n", count);
        //printf("%s\n", pb_data->time);
        mmbuf__write_data(out_obj, (unsigned char *) &(pb_data.time), 18);
        //mmbuf__free_data(m, offset,  new_offset);

    }
    printf("Parsed %d messages\n", count);
    return 0;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    struct arg_struct arg_data;
    arg_data = handle_args(argc, argv);

    struct mmbuf_obj pbf_obj;
    struct mmbuf_obj out_obj;

    if (setup(&pbf_obj, arg_data.pb_path, 'r')!=0)
        exit_with_error("Fail to setup playback\n");
    if (setup(&out_obj, arg_data.out_path, 'w')!=0)
        exit_with_error("Fail to setup outfile\n");

    if (parse_pb(&pbf_obj, &out_obj)!=0)
        exit_with_error("Fail to parse_pb\n");

    if (teardown(&out_obj)!=0)
        exit_with_error("Fail to outfile\n");
    if (teardown(&pbf_obj)!=0)
        exit_with_error("Fail to teardown\n");
    return rc;
}