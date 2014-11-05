

#include "mmbuf.h"


#include <stdio.h>      //perror, fprintf, printf
#include <sys/types.h>  //off_t
#include <sys/stat.h>   //O_RDONLY
#include <fcntl.h>      //close, write, open, O_RDWR, O_TRUNC, O_CREAT
#include <unistd.h>     //ftruncate, lseek, SEEK_CUR, SEEK_END, SEEK_SET
#include <errno.h>
#include <stdlib.h>     //exit, EXIT_FAILURE
#include <stddef.h>     //ptrdiff_t
#include <string.h>     //memcpy
#include <sys/mman.h>   //mmap, mremap, madvise, munmap, 
                        //MREMAP_MAYMOVE, MAP_FAILED, PROT_READ, MAP_SHARED, PROT_WRITE, MADV_SEQUENTIAL, MADV_RANDOM, MADV_NORMAL

/*---
DATA
---*/

/*---
FUNCTIONS
---*/
static void handle_error(const struct mmbuf__obj *m, const char *err_msg);
static void handle_error(const struct mmbuf__obj *m, const char *err_msg)
{
    close(m->fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
}

static int grow_file(struct mmbuf__obj *m, const off_t amount);
static int grow_file(struct mmbuf__obj *m, const off_t amount)
{
    off_t result = lseek(m->fd, amount-1, SEEK_CUR);
    if (result == -1) 
    {
        handle_error(m, "Error calling lseek() to 'stretch' the file");
    }
    
    off_t old_file_size = m->filesize;
    m->filesize = amount-1;

    // Write to end of file to make it take on the size.
    write(m->fd, "", 1);

    if (m->map!=0)
    {
        printf("Warning remapping the file");
        void *new_mapping = mremap(m->map, old_file_size, m->filesize, MREMAP_MAYMOVE);
        if (new_mapping == MAP_FAILED)
        {
            handle_error(m, "Error remapping the file\n");
        }
        m->map = new_mapping;
    }
    return 0;
}

int mmbuf__setup(struct mmbuf__obj *m, const char *file_path, const char *mode)
{
    //printf("opening the file %s\n", file_path);
    // OPEN THE FILE
    if (mode[0]=='r')
    {
        m->fd = open(file_path, O_RDONLY, 0754);
    }
    else if (mode[0]=='w')
    {
        m->fd = open(file_path, O_RDWR|O_TRUNC|O_CREAT, 0600);
    }
    else
    {
        fprintf(stderr, "I don't understand the mode %s, should start with r, or w", mode);
        exit(EXIT_FAILURE);
    }
    if (m->fd == -1) 
    {
        fprintf(stderr, "Error opening file");
        exit(EXIT_FAILURE);
    }
    m->mode = mode[0];

    // GET THE FILE SIZE CREATE THE MAP
    m->map = 0;
    m->offset = 0;
    if (mode[0]=='r')
    {
        m->filesize = lseek(m->fd, 0L, SEEK_END);
        off_t curpos = lseek(m->fd, 0L, SEEK_SET);
        if (curpos!=0 || m->filesize < 0)
        {
            fprintf(stderr, "Seek failed %ld\n", curpos);
            exit(EXIT_FAILURE); 
        }
        m->map = mmap(0, m->filesize, PROT_READ, MAP_SHARED, m->fd, 0);
    }
    else if (mode[0]=='w')
    {
        grow_file(m, BLOCKSIZE*BLOCKSIZE);
        m->map = mmap(0, m->filesize, PROT_WRITE, MAP_SHARED, m->fd, 0);
    }
    if (m->map == MAP_FAILED)
    {
        handle_error(m, "Error mmapping the file");
    }
    
    int flags;
    if (mode[1]=='s')
        flags = MADV_SEQUENTIAL;
    else if (mode[1]=='r')
        flags = MADV_RANDOM;
    else
        flags = MADV_NORMAL;
    char madvise_ret = madvise(m->map, m->filesize, flags);
    if (madvise_ret != 0) 
    {
        handle_error(m, "Error madvise the file");
    }  
    m->free_boundary = 0;

    return 0;
}

int mmbuf__teardown(const struct mmbuf__obj *m)
{
    if (m->mode=='w')
    {
        int rc = ftruncate(m->fd, m->offset);
        if (rc==-1)
        {
            perror("Truncation error\n");
        }
    }
    if (munmap(m->map, m->filesize) == -1) 
    {
        perror("Error un-mmapping the file");
    }
    close(m->fd);
    return 0;
}

unsigned int mmbuf__get_data(const struct mmbuf__obj *m, void **result_p, const off_t offset, const unsigned int length)  // This should be a void pointer
{
    if (offset>(m->filesize))
        return 0;

    unsigned int available_data;
    if (offset+length>(m->filesize))
        available_data = (m->filesize) - offset;
    else
        available_data = length;

    //printf("hello %c %p\n", m->map[offset], m->map+offset);
    *result_p = (unsigned char *)m->map + offset;
    //printf("gets data got request for data %ld: %c\n", offset, m->map[offset]);
    
    return available_data;
}

int mmbuf__free_data(struct mmbuf__obj *m, const off_t low, const off_t high)   // Not using high at the moment, jsut freeing everything upto low.
{
    if (low==0)
    {   
        return 0;
    }

    long long length;
    long long data_not_in_use_limit = low - 1;
    long long lnearest_block_boundary = data_not_in_use_limit &~(BLOCKSIZE-1);

    if (lnearest_block_boundary > (m->free_boundary))
    {
        //printf("data_in_use low='%d' lnearest='%d', block_hi='%d'\n", low, lnearest_block_boundary, m->free_boundary);
        
        length = lnearest_block_boundary - m->free_boundary;
        int madvise_ret = madvise((unsigned char *)m->map + m->free_boundary, length, MADV_DONTNEED);
        if (madvise_ret != 0) 
        {
            printf("Error number %d\n", errno);
            handle_error(m, "Error madvise the file");
        }
        else
        {
            m->free_boundary = lnearest_block_boundary;
        }
    }
    return 0;
}

int mmbuf__append(struct mmbuf__obj *m, const void *source, const unsigned int length)
{
    //Writing data may change the mapping if max size is hit
    //printf("%p %p %d %d %ld\n", m->map, source, length, m->filesize, m->offset);
    while ((m->offset + length) > (m->filesize))
    {
        printf("old_size='%ld' old_map_pt='%p'\n", m->filesize, m->map);
        grow_file(m, m->filesize*1.5);
        printf("new_size='%ld' new_map_pt='%p'\n", m->filesize, m->map);
    }
    memcpy((char *)(m->map)+(m->offset), source, length);
    m->offset += length;
    return 0;
}

off_t mmbuf__pos_alloc(struct mmbuf__obj *m, const unsigned int length)
{
    while ((m->offset + length) > (m->filesize))
    {
        printf("old_size='%ld' old_map_pt='%p'\n", m->filesize, m->map);
        grow_file(m, m->filesize*1.5);
        printf("new_size='%ld' new_map_pt='%p'\n", m->filesize, m->map);
    }
    off_t offset = m->offset;
    m->offset += length;
    return offset;
}

void *mmbuf__pos_tmpptr(struct mmbuf__obj *m, const off_t position)
{
    return (void *) ((char *)(m->map)+position);
}


void *mmbuf__alloc(struct mmbuf__obj *m, const unsigned int length)
{
    while ((m->offset + length) > (m->filesize))
    {
        printf("old_size='%ld' old_map_pt='%p'\n", m->filesize, m->map);
        grow_file(m, m->filesize*1.5);
        printf("new_size='%ld' new_map_pt='%p'\n", m->filesize, m->map);
    }
    void *free_space = (char *)(m->map) + (m->offset);
    m->offset += length;
    return free_space;
}

