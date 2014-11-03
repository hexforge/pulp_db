
/*
ToDo: http://man7.org/linux/man-pages/man2/readahead.2.html
*/

#include "mmbuf.h"
#include <stdio.h>  

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*---
DATA
---*/

/*---
FUNCTIONS
---*/
static void handle_mmap_fail(struct mmbuf_obj *m);
void handle_mmap_fail(struct mmbuf_obj *m)
{
    close(m->fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
}

static void handle_madvise_error(struct mmbuf_obj *m);
void handle_madvise_error(struct mmbuf_obj *m)
{
    //printf("I am he22re\n");
    close(m->fd);
    //printf("I am her2e\n");
    perror("Error madvise the file");
    //printf("I am her3e\n");
    exit(EXIT_FAILURE);
    //printf("I am he4re\n");
}

static int grow_file(struct mmbuf_obj *const m, const long long amount);
static int grow_file(struct mmbuf_obj *const m, const long long amount)
{
    long long old_file_size = m->filesize;

    int result = lseek(m->fd, amount-1, SEEK_CUR);
    if (result == -1) 
    {
        close(m->fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }
    
    m->filesize = amount-1;

    // Write to end of file to make it take on the size.
    result = write(m->fd, "", 1);

    if (m->map!=0)
    {
        printf("Warning remapping the file");
        void *new_mapping = mremap(m->map, old_file_size, m->filesize, MREMAP_MAYMOVE);
        if (new_mapping == MAP_FAILED)
        {
            close(m->fd);
            perror("Error remapping the file\n");
            exit(EXIT_FAILURE);
        }
        m->map = new_mapping;
    }
    return 0;
}

int mmbuf__setup(struct mmbuf_obj *m, const char *file_path, const char *mode)
{
    //printf("opening the file %s\n", file_path);
    // OPEN THE FILE
    if (mode[0]=='r')
        m->fd = open(file_path, O_RDONLY, 0754);
    else if (mode[0]=='w')
    {
        m->fd = open(file_path, O_RDWR|O_TRUNC|O_CREAT, 0600);
    }
    else
    {
        printf("I don't understand the mode %s, should start with r, or w", mode);
        exit(EXIT_FAILURE);
    }
    if (m->fd == -1) 
    {
        perror("Error opening file");
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
            perror("Error");
            return 1; 
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
        handle_mmap_fail(m);
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
        handle_madvise_error(m);
    }  
    m->free_boundary = 0;

    return 0;
}

int mmbuf__teardown(const struct mmbuf_obj *m)
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

int mmbuf__get_data(const struct mmbuf_obj *m, void **result_p, const long long offset, const int length)  // This should be a void pointer
{
    if (offset>(m->filesize))
    {
        return 0;
    }

    int available_data;
    if (offset+length>(m->filesize))
    {
        available_data = (m->filesize) - offset;
    }
    else
    {
        available_data = length;
    }

    //printf("hello %c %p\n", m->map[offset], m->map+offset);
    *result_p = (unsigned char *)m->map + offset;
    //printf("gets data got request for data %ld: %c\n", offset, m->map[offset]);
    
    return available_data;
}

int mmbuf__free_data(struct mmbuf_obj *const m, const long long low, const long long high)   // Not using high at the moment, jsut freeing everything upto low.
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
            handle_madvise_error(m);
        }
        else
        {
            m->free_boundary = lnearest_block_boundary;
        }
    }
    return 0;
}

int mmbuf__append(struct mmbuf_obj *const m, const void *source, const unsigned int length)
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

void *mmbuf__alloc(struct mmbuf_obj *const m, const unsigned int length)
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

signed long long mmbuf__offalloc(struct mmbuf_obj *const m, const unsigned int length)
{
    while ((m->offset + length) > (m->filesize))
    {
        printf("old_size='%ld' old_map_pt='%p'\n", m->filesize, m->map);
        grow_file(m, m->filesize*1.5);
        printf("new_size='%ld' new_map_pt='%p'\n", m->filesize, m->map);
    }
    signed long long offset = m->offset;
    m->offset += length;
    return offset;
}