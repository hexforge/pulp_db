/*
TODO::  http://yarchive.net/comp/linux/o_direct.html
*/

#define _GNU_SOURCE    //odirect

#include <stdio.h>
#include <unistd.h>  //contains read
#include <sys/stat.h> // http://linux.die.net/man/3/open
#include <fcntl.h>    // http://linux.die.net/man/3/open
#include <string.h>   // memcpy
#include <stdlib.h>   //exit

#include "cfbuf.h"
/*
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
//posix_memalign(&buffer, 512, 512);  //buffer[BUFFER_SIZE];
//#include <malloc.h>
*/

/*---
DATA
---*/
#define BLOCKSIZE 4096
#define MAX_MSG_SIZE (1024*1024)
#define NUM_BLOCKS_IN_BUFFER (32*1024)
#define NUM_BLOCKS_IN_MAXMSG (MAX_MSG_SIZE/BLOCKSIZE)
#define NUM_MAXMSG_IN_BUFFER ((NUM_BLOCKS_IN_BUFFER*BLOCKSIZE)/MAX_MSG_SIZE)
#define BUFFER_SIZE (NUM_BLOCKS_IN_BUFFER*BLOCKSIZE)
#define HIGH_BOUNDARY ((NUM_MAXMSG_IN_BUFFER-1)*NUM_BLOCKS_IN_MAXMSG*BLOCKSIZE)  
// This warns of the disconitinutit. Leave enough room for at least one.


/*---
FUNCTIONS
---*/
static void handle_io_fail(struct cfbuf_obj *f);
void handle_io_fail(struct cfbuf_obj *f)
{
    perror("Error reading the file1\n");
    cfbuf__close(f);
    exit(EXIT_FAILURE);
}

static void handle_fill_fail(struct cfbuf_obj *f);
void handle_fill_fail(struct cfbuf_obj *f)
{
    perror("Error filling the buffer");
    cfbuf__close(f);
    exit(EXIT_FAILURE);
}

static int copy_x_block_into_buffer(struct cfbuf_obj *f, const int offset, const unsigned int num_blocks);
int copy_x_block_into_buffer(struct cfbuf_obj *f, const int offset, const unsigned int num_blocks)
{
    ssize_t read_amount;
    ssize_t actual_eof;
    ssize_t expected_eof;

    //printf("num blocks in buffer %d\n", num_blocks);

    unsigned char *target_buffer;
    target_buffer = f->buffer + offset;
    
    unsigned int amount_to_read = BLOCKSIZE*num_blocks;

    read_amount = read(f->fd, target_buffer, amount_to_read);
    if (read_amount == -1)
    {
        handle_io_fail(f);
        exit(1);
    }
    else if (read_amount!=amount_to_read)
    {
        //printf("%p %p %d\n", target_buffer, buffer, read_amount);
        actual_eof = f->start_offset + read_amount + offset;
        expected_eof = f->max_offset;

        printf("max_offset='%ld' start_offset='%ld'\n", f->max_offset, f->start_offset);
        if (actual_eof==expected_eof)
        {
            printf("Finished reading the file \n");
        }
        else
        {
            // DEBUG <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            //printf("%d read_amount%ld\n", i, read_amount);
            //printf("Error expected EOF='%ld', actual EOF='%ld'\n", expected_eof, actual_eof);
            // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            handle_io_fail(f);
        }
    }
    
    return 0;
}

static int fill(struct cfbuf_obj *f);
int fill(struct cfbuf_obj *f)
{
    // The second the free crosses the boundary we shift everything
    if (f->current_pos >= f->buffer+HIGH_BOUNDARY)
    {
        memcpy(f->buffer, f->buffer+HIGH_BOUNDARY, MAX_MSG_SIZE);
        f->start_offset += HIGH_BOUNDARY;
        f->current_pos = (f->current_pos-HIGH_BOUNDARY);
        copy_x_block_into_buffer(f, MAX_MSG_SIZE, NUM_BLOCKS_IN_BUFFER-NUM_BLOCKS_IN_MAXMSG);
    }
    else
    {
        printf("ERROR: current_pos='%p', buffer='%p', HIGH_BOUNDARY='%d'", f->current_pos, f->buffer, HIGH_BOUNDARY);
        handle_fill_fail(f);
    }
    return 0;
}

static int init_buffer(struct cfbuf_obj *f);     // What happens in the case we don't have enough data?
int init_buffer(struct cfbuf_obj *f)
{
    copy_x_block_into_buffer(f, 0, NUM_BLOCKS_IN_BUFFER);
    f->current_pos = f->buffer;
    return 0;
}

static int flush_write(struct cfbuf_obj *f, unsigned long amount);
int flush_write(struct cfbuf_obj *f, unsigned long amount)
{
    if ((amount & (BLOCKSIZE-1)) !=0)
    {
        printf("Error: need to write on block size. Write amount='%ld'\n", amount);
    }

    int write_amount = write(f->fd, f->buffer, amount);
    if (write_amount==-1 || write_amount!=amount)
    {
        perror("write error idea\n");
        exit(EXIT_FAILURE);
    }
    printf("Flushing %d %d\n", write_amount, f->buffer_size);
    f->current_pos = f->buffer;

    return 0;
}

int cfbuf__setup(struct cfbuf_obj *f, const char *file_path, const unsigned char mode)
{  
    int rc = posix_memalign((void **)&(f->buffer), BLOCKSIZE, BUFFER_SIZE);
    if (rc!=0)
    {   
        printf("%d", rc);
        perror("Error memalign");
        exit(EXIT_FAILURE);
    }

    f->mode = mode;
    f->max_offset = 0;
    f->start_offset = 0;
    f->buffer_size = BUFFER_SIZE;
    f->current_pos = f->buffer;

    //fd = open(file_path, O_RDONLY|O_DIRECT|O_SYNC);
    if (mode == 'r')
    {
        printf("opening for reading %s\n", file_path);
        f->fd = open(file_path, O_RDONLY|O_DIRECT, 0);
    }
    else
    {
        printf("opening for writing %s\n", file_path);
        f->fd = open(file_path, O_WRONLY|O_DIRECT|O_CREAT|O_TRUNC, 0754);
    }

    if (f->fd == -1)
    {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }
 
    if (mode=='r')
    {
        //find mum bytes in file
        f->max_offset = lseek(f->fd, 0L, SEEK_END);
        unsigned long curpos = lseek(f->fd, 0L, SEEK_SET);
        if (curpos!=0)
        {
            fprintf(stderr, "Seek failed %ld\n", curpos);
            perror("Error");
            return -1; 
        }
        //Fill the buffer
        init_buffer(f);
    }
    return 0;
}

int cfbuf__close(struct cfbuf_obj *f)
{
    printf("closing\n");
    if (f->mode=='w')
    {
        //Must always write in blocks

        unsigned long amount = f->current_pos-f->buffer;
        if (amount > 0);
        {
            unsigned long nearest_block = amount&~(BLOCKSIZE-1); // Rounded down
            int overwrite_amount = 0;
            int block_remainder = amount&(BLOCKSIZE-1);
            if (block_remainder!=0)
            {
                nearest_block += BLOCKSIZE;
                overwrite_amount = BLOCKSIZE - block_remainder;
            }
            printf("amount %ld, %p %p\n", nearest_block, f->current_pos, f->buffer);
            flush_write(f, nearest_block);

            if (overwrite_amount)
            {
                unsigned long file_size = lseek(f->fd, 0L, SEEK_END);
                ftruncate(f->fd, file_size-overwrite_amount-1);
            }
        }
        
    }

    free(f->buffer);
    if (close(f->fd) == -1) 
    {
        perror("Error closing the file");
    }
    return 0;
}

int cfbuf__get_data(struct cfbuf_obj *f, 
                    unsigned char **target, 
                    const unsigned long offset, 
                    const unsigned int min_length
                    )
{   
    if (min_length>MAX_MSG_SIZE)
    {
        printf("Requested too much data\n");
        exit(1);
    }
    if ((offset) < (f->start_offset))
    {
        printf("Something has gone wrong offset='%ld' start_offset='%ld'\n", offset, f->start_offset);
        exit(1);
    }

    *target = (f->buffer + offset) - (f->start_offset);
    if ((offset+min_length) > (f->max_offset))
    {
        return (f->max_offset) - (offset);
    }
    else
    {
        return min_length;
    }

}

int cfbuf__free_data(struct cfbuf_obj *f, const unsigned long offset_to)
{
    if ((offset_to) < (f->start_offset))
    {
        printf("Something has gone wrong offset_to='%ld' start_offset='%ld'\n", offset_to, f->start_offset);
        exit(1);
    }

    f->current_pos = f->buffer + (offset_to - f->start_offset);
    if (offset_to - (f->start_offset) > HIGH_BOUNDARY)
    {
        // DEBUG: <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        //printf("current_pos='%p', buffer='%p', HIGH_BOUNDARY='%d'\n", f->current_pos, f->buffer, HIGH_BOUNDARY);
        //printf("offset_to='%ld', start_offset='%ld'\n", offset_to, start_offset);
        //printf("-------------\n");
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        fill(f);
    }
    return 0;
}

int cfbuf__write_data(struct cfbuf_obj *f,                      
                      const unsigned char *target, 
                      const unsigned int length)
{
    //current_pos is next free pointer

    int remaining = f->buffer_size - (f->current_pos - f->buffer);
    //printf("%d %d\n", remaining, length);
    if (remaining > length)
    {
        memcpy(f->current_pos, target, length);
        f->current_pos += length;
        //printf("Wrote message %s\n", f->current_pos);
    }
    else
    {
        memcpy(f->current_pos, target, remaining);
        f->current_pos += remaining;
        target += remaining;
        flush_write(f, f->buffer_size);   // This sets current_pos to start of buffer
        memcpy(f->current_pos, target, length-remaining);
        f->current_pos += length-remaining;
    }
    
    return -1;
}

