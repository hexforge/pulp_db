#include <stdio.h>
#include <stdlib.h>    // atoi

#include "metaparse.h"
#include "mmbuf.h"

/*---
DATA
---*/
struct arg_struct
{
    char *pb_path;
    char *out_path;
};

/*---
FUNCTIONS
---*/
static void exit_with_error(char *err_msg);
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

static int test(struct metaparse__pb *m, struct mmbuf__obj *out_obj);
int test(struct metaparse__pb *m, struct mmbuf__obj *out_obj)
{
    int count = 0;
    int rc;
    struct metaparse__msg pb_data = {"0.0.0.0", "0", "12345.678", 0, 0};
    
    while (1)
    {
        rc = metaparse__get_msg(m, &pb_data);
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
        mmbuf__append(out_obj, (char *) &(pb_data.time), 18);
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

    struct metaparse__pb pbf_obj;
    struct mmbuf__obj out_obj;
    
    if (metaparse__setup(&pbf_obj, arg_data.pb_path, "r")!=0)
        exit_with_error("Fail to setup playback\n");
    if (mmbuf__setup(&out_obj, arg_data.out_path, "w")!=0)
        exit_with_error("Fail to setup outfile\n");

    if (test(&pbf_obj, &out_obj)!=0)
        exit_with_error("Fail to test\n");

    if (mmbuf__teardown(&out_obj)!=0)
        exit_with_error("Fail to outfile\n");
    if (metaparse__teardown(&pbf_obj)!=0)
        exit_with_error("Fail to teardown\n");
    return rc;
}
