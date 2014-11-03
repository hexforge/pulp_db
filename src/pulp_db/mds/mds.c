#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mds.h"
#include "hex.h"


#define MASTERDS_DATA_STORE "/master.data"
#define MASTERDS_DATA_INDEX "/master.index"

/*---
DATA
---*/
// The first page could contain meta information??

/*---
FUNCTIONS
---*/

// Flush a full set
static int flush(const struct mds_obj *m);
int flush(const struct mds_obj *m)
{
    char *assigned_space = mmbuf__alloc(m->index_file, 128);
    bitpack__encode(assigned_space, m->index, 1);    
    return 0;
}

// Flush a partial set
static int partial_flush(struct mds_obj *m);
int partial_flush(struct mds_obj *m)
{
    if (m->num_in_clump==0)
        return 0;

    while (m->num_in_clump <= (MAX_MSG_IN_CLUMP))
    {
        m->index->msg_sizes[m->num_in_clump] = 0;
        m->num_in_clump += 1;
    }
    flush(m);
    return 0;
}

// Flush a possibly partial set and add a zeroed out set at the end.
static int final_flush(struct mds_obj *m);
int final_flush(struct mds_obj *m)
{
    partial_flush(m);
    char *assigned_space = mmbuf__alloc(m->index_file, 128);
    int i;
    for (i=0; i<MAX_MSG_IN_CLUMP; i++)
    {
        assigned_space[i] = 0;
    }
    return 0;
}

/*---
API
---*/
int mds__setup(struct mds_obj *m, char *dirpath, char mode)
{
    // In read mode we should be random!!!!!!!
    m->index = malloc(sizeof(struct bitpack__clump));
    m->num_in_clump = 0;
    m->msg_count = 0;
    m->current_block_index = 0;
    
    char ds_path[256];
    char index_path[256];
    
    strcpy(ds_path, dirpath);
    strcat(ds_path, MASTERDS_DATA_STORE);
    strcpy(index_path, dirpath);
    strcat(index_path, MASTERDS_DATA_INDEX);
    
    char mode_with_hint[3];
    
    if (mode=='r')
        strcpy(mode_with_hint, "r");   //'rr' Read random, 'rs' sometimes want to read sequential!!!!!!!!
    else if (mode=='w')
        strcpy(mode_with_hint, "ws");   // Write sequential
    else
    {
        printf("Bad mode %c \n", mode);
        exit(EXIT_FAILURE);
    }
    m->mode = mode;

    m->data_file = malloc(sizeof(struct mmbuf_obj));
    mmbuf__setup(m->data_file, ds_path, mode_with_hint);

    m->index_file = malloc(sizeof(struct mmbuf_obj));
    mmbuf__setup(m->index_file, index_path, mode_with_hint);
    return 0;
}

int mds__teardown(struct mds_obj *m)
{
    if (m->mode=='w' && m->num_in_clump!=0)
    {
        final_flush(m);
        m->num_in_clump = 0;
    }
    free(m->index);
    m->index = NULL;
    mmbuf__teardown(m->data_file);
    mmbuf__teardown(m->index_file);
    free(m->data_file);
    m->data_file = NULL;
    free(m->index_file);
    m->index_file = NULL;
    return 0;
}

signed long long mds__append(struct mds_obj *m, char *data, int length)
{
    long long offset = m->data_file->offset;
    mmbuf__append(m->data_file, data, length);

    m->index->offsets[m->num_in_clump] = offset;
    m->index->msg_sizes[m->num_in_clump] = length;
    
    m->num_in_clump += 1;
    m->msg_count += 1;   // Could merge these two counts and use a bit mask.

    if (m->num_in_clump >= (MAX_MSG_IN_CLUMP))
    {
        flush(m);
        m->num_in_clump = 0;
    }
    return m->msg_count-1;   //-1 if problem
}

unsigned int mds__get(struct mds_obj *m, long long index, char **result)
{
    /*
    Calculate clump_index
    Calculate local_index
    if not already in cache
        read clump data
        decode
    
    lookup strut for index and rolling sizes.
    calculate true index sum.
    read get len from index_sum into data store.

    0 problem
    else
    return size read if matches
    */
    long long  block_index = index/48;     // Need to specialise this 3*16=(4-1)*16 can be worked out with modulus
    unsigned int  clump_index = index%48;

    if (m->num_in_clump==0 || m->current_block_index!=block_index)
    {
        //printf("READING for %lld\n", block_index);
        int num_decoded = bitpack__decode((char*)(m->index_file->map)+(block_index*128), m->index, 1);
        m->current_block_index = block_index;
        m->num_in_clump = num_decoded;
    }

    if (clump_index >= m->num_in_clump)
    {
        return 0;
    }
    long long  msg_offset = m->index->offsets[clump_index];
    unsigned int  msg_size = m->index->msg_sizes[clump_index];
    *result =  (char *)m->data_file->map + msg_offset;
    return msg_size;
}

long long mds__len(struct mds_obj *m)
{
    /*
    File split into 128byte blocks.
    Last block in file should be all zero.
    Get addresss of block before the last.
    There is 48 messages per block.
    Add on the messages in that block and we done
    */

    // This would be better with meta data. Meta data can be stored in a data file.  Only effects offsets.
    if (m->mode=='w')
    {
        return m->msg_count;
    }
    else if (m->mode=='r')
    {
        if (m->msg_count!=0)
        {
            return m->msg_count;
        }
    }
    // Need to calculate number of messages based on file size of index.
    // Is this an acceptable idea or a very very bad one?
    long long current_pos = lseek(m->index_file->fd, 0L, SEEK_CUR);
    long long filesize = lseek(m->index_file->fd, 0L, SEEK_END);
    lseek(m->index_file->fd, current_pos, SEEK_SET);  

    if ((filesize&0177)!=0)
    {
        printf("Filesize not divisable %lld\n ", filesize);
        exit(EXIT_FAILURE);
    }

    long long zero_block = (filesize-1)&~0177;
    int i;
    for (i=0; i<128; i++)
    {  
        if (((char *)m->index_file->map)[i+zero_block]!=0)
        {
            printf("Non zero byte detected error %d\n ", i);
            exit(EXIT_FAILURE);
        }
    }

    long long last_data_block = zero_block-128;
    long long num_of_msgs = ((last_data_block>>7))*48;
    
    struct bitpack__clump result;
    int num_decoded = bitpack__decode((char *)m->index_file->map+last_data_block, &result, 1);
    num_of_msgs += num_decoded;
    return num_of_msgs;
}
