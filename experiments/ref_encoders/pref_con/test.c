#include <stdio.h>
#include <assert.h>
#include "cpref.h"
//#include "cpref.c"

/*
struct cpref__obj 
struct cpref__stream 

int cpref__open(struct cpref__obj *cp, const char *ref_file_path, const char mode);
int cpref__close(struct cpref__obj *cp);
void setup_write_stream(struct cpref__obj *cp, struct cpref__stream *stream, unsigned long long total_msgs); 
void setup_read_stream(struct cpref__obj *cp, struct cpref__stream *stream, unsigned long long offset);      
void close_stream(struct cpref__obj *cp, struct cpref__stream *stream);
void write_stream(struct cpref__stream *stream, signed long long *refs, int n);
signed long long cpref__next(struct cpref__stream *stream);   
signed long long cpref__prev(struct cpref__stream *stream);
signed long long cpref__get(struct cpref__stream *stream, unsigned long long i);
signed long long cpref__ge_ref(struct cpref__stream *stream, unsigned long long ref);
signed long long cpref__le_ref(struct cpref__stream *stream, unsigned long long ref);
*/



void print_stream_details(struct cpref__stream *stream)
{
    printf("mode=%c \n", stream->mode);
    printf("num_blocks=%d \n", stream->num_blocks);
    printf("meta_start_offset=%lld \n", stream->meta_start_offset);
    printf("meta_block_i=%d \n", stream->meta_block_i);
    printf("ref_start_offset=%lld \n", stream->ref_start_offset);
    printf("ref_block_i=%d \n", stream->ref_block_i);
    printf("nmsgs=%lld \n", stream->nmsgs);
    printf("low_ref=%lld \n", stream->low_ref);
    printf("high_ref=%lld \n", stream->high_ref);
    printf("current_ref_i=%lld \n", stream->current_ref_i);
    printf("current_block_ref_low=%lld \n", stream->current_block_ref_low);
    printf("current_block_ref_high=%lld \n", stream->current_block_ref_high);
}

void test_write(signed long long *refs, int n)
{    
    printf("-------------------test_write\n");
    struct cpref__obj c;
    struct cpref__obj *cp = &c;
    cpref__open(cp, "foobar.refs", 'w');

    struct cpref__stream s;
    struct cpref__stream *sp = &s;
    cpref__setup_write_stream(sp, cp, n);
    cpref__write_stream(sp, refs, n);

    print_stream_details(sp);
    cpref__close_stream(sp);

    cpref__close(cp);
}

void test_read(signed long long key)
{
    printf("-------------------test_read\n");
    struct cpref__obj c;
    struct cpref__obj *cp = &c;
    cpref__open(cp, "foobar.refs", 'r');

    struct cpref__stream s;
    struct cpref__stream *sp = &s;

    cpref__setup_read_stream(sp, cp, 0);
    print_stream_details(sp);

    signed long long result;
    result = cpref__get(sp, key);
    printf("result %lld\n", result);
    
    while ((result = cpref__next(sp))!=-1)
    {
        //printf("next %lld\n", result);
    }

    while ((result = cpref__prev(sp))!=-1)
    {
        //printf("next %lld\n", result);
    }

    cpref__close_stream(sp);
    cpref__close(cp);
}


void test_ge(signed long long *refs, int n)
{
    printf("-------------------test_ge_write\n");
    struct cpref__obj c;
    struct cpref__obj *cp = &c;
    cpref__open(cp, "foobar.refs", 'w');

    struct cpref__stream s;
    struct cpref__stream *sp = &s;
    cpref__setup_write_stream(sp, cp, n);
    cpref__write_stream(sp, refs, n);

    print_stream_details(sp);
    cpref__close_stream(sp);

    cpref__close(cp);

    printf("-------------------test_ge_read\n");
    struct cpref__obj c_read;
    struct cpref__obj *cp_read = &c_read;
    cpref__open(cp_read, "foobar.refs", 'r');

    struct cpref__stream s_read;
    struct cpref__stream *sp_read = &s_read;

    cpref__setup_read_stream(sp_read, cp_read, 0);
    print_stream_details(sp_read);

    signed long long result = 0;
    signed long long x = 0;

    signed long long max_ref = refs[n-1];
    
    while (1)
    {
        //printf("looking for %lld\n", x);
        result = cpref__ge_ref(sp_read, x);
        //printf("result=%lld for %lld\n", result, x);

        if (x>max_ref)
        {
            assert (result==-1);
            break;
        }

        if (x%2==0)
            assert (result==x+1);
        else
            assert (result==x);


        if (result==-1)
        {
            assert (0);
        }
        x++;
    }

    cpref__close_stream(sp_read);
    cpref__close(cp_read); 
}



void test_le(signed long long *refs, int n)
{

    printf("-------------------test_le_write\n");
    struct cpref__obj c;
    struct cpref__obj *cp = &c;
    cpref__open(cp, "foobar.refs", 'w');

    struct cpref__stream s;
    struct cpref__stream *sp = &s;
    cpref__setup_write_stream(sp, cp, n);
    cpref__write_stream(sp, refs, n);

    print_stream_details(sp);
    cpref__close_stream(sp);

    cpref__close(cp);

    printf("-------------------test_le_read\n");
    struct cpref__obj c_read;
    struct cpref__obj *cp_read = &c_read;
    cpref__open(cp_read, "foobar.refs", 'r');

    struct cpref__stream s_read;
    struct cpref__stream *sp_read = &s_read;

    cpref__setup_read_stream(sp_read, cp_read, 0);
    print_stream_details(sp_read);

    signed long long result = 0;
    signed long long x = 0;

    signed long long max_ref = refs[n-1];   
    signed long long min_ref = refs[0];

    while (1)
    {
        //printf("looking for %lld\n", x);
        result = cpref__le_ref(sp_read, x);
        //printf("result=%lld for %lld\n", result, x);

        if (x<min_ref)
        {
            assert (result==-1);
            x++;
            continue;
        }

        if (x>max_ref)
        {
            if (x>max_ref+2)
            {
                break;
            }
            x++;
            continue;
        }

        if (x%2==0)
            assert (result==x-1);
        else
            assert (result==x);
        x++;
    }

    cpref__close_stream(sp_read);
    cpref__close(cp_read); 
}


int main()
{
    printf("Hello world\n");

    signed long long msgs[2000];
    int n1 = sizeof(msgs)/sizeof(msgs[0]);
    for (int i=0; i<n1; i++)
        msgs[i] = i;

    test_write(msgs, n1);
    test_read(1800);

    //#######################################
    
    
    signed long long odd_nums[1000];
    int n = sizeof(odd_nums)/sizeof(odd_nums[0]);
    for (int i=0; i<n; ++i)
    {
        odd_nums[i] = i*2+1;
    }
    test_ge(odd_nums, n);
    test_le(odd_nums, n);
}
