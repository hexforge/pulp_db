
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

static int grow_file(struct mmbuf_obj *m, unsigned long amount);
static int grow_file(struct mmbuf_obj *m, unsigned long amount)
{
    int result = lseek(m->fd, amount-1, SEEK_CUR);
    if (result == -1) 
    {
        close(m->fd);
        perror("Error calling lseek() to 'stretch' the file");
        exit(EXIT_FAILURE);
    }
    unsigned long old_file_size = m->filesize;
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

int mmbuf__setup(struct mmbuf_obj *m, char *file_path, char mode)
{
    m->mode = mode;
    // OPEN THE FILE
    if (mode=='r')
        m->fd = open(file_path, O_RDONLY, 0754);
    else if (mode=='w')
    {
        
        m->fd = open(file_path, O_RDWR|O_TRUNC|O_CREAT, 0600);
    }
    if (m->fd == -1) 
    {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    // GET THE FILE SIZE CREATE THE MAP
    m->map = 0;
    if (mode=='r')
    {
        m->filesize = lseek(m->fd, 0L, SEEK_END);
        off_t curpos = lseek(m->fd, 0L, SEEK_SET);
        if (curpos!=0)
        {
            fprintf(stderr, "Seek failed %ld\n", curpos);
            perror("Error");
            return -1; 
        }
        m->map = mmap(0, m->filesize, PROT_READ, MAP_SHARED, m->fd, 0);
    }
    else if (mode=='w')
    {
        grow_file(m, BLOCKSIZE*BLOCKSIZE);
        m->map = mmap(0, m->filesize, PROT_WRITE, MAP_SHARED, m->fd, 0);
        m->write_offset = 0;
    }
    if (m->map == MAP_FAILED)
    {
        handle_mmap_fail(m);
    }

    unsigned char madvise_ret = madvise(m->map, m->filesize, MADV_SEQUENTIAL);
    if (madvise_ret != 0) 
    {
        handle_madvise_error(m);
    }
    m->dealocated_block_hi = 0;

    return 0;
}

int mmbuf__close(struct mmbuf_obj *m)
{
    if (m->mode=='w')
    {
        ftruncate(m->fd, m->write_offset-1);
    }
    if (munmap(m->map, m->filesize) == -1) 
    {
        perror("Error un-mmapping the file");
    }
    close(m->fd);
    return 0;
}

int mmbuf__get_data(struct mmbuf_obj *m, unsigned char **result_p, const unsigned long offset, const int length)
{
    int available_data;
    if (offset>(m->filesize))
    {
        available_data = 0;
    }
    if (offset+length>(m->filesize))
    {
        available_data = (m->filesize) - offset;
    }
    else
    {
        available_data = length;
    }

    //printf("hello %c %p\n", m->map[offset], m->map+offset);
    
    *result_p = m->map+offset;

    //printf("gets data got request for data %ld: %c\n", offset, m->map[offset]);
    
    return available_data;
}

int mmbuf__free_data(struct mmbuf_obj *m, const unsigned long low, const unsigned long high)
{
    if (low==0)
    {   
        return 0;
    }

    unsigned long length;
    unsigned long data_not_in_use_limit = low - 1;
    unsigned long lnearest_block_boundary = data_not_in_use_limit &~(BLOCKSIZE-1);

    if (lnearest_block_boundary > (m->dealocated_block_hi))
    {
        //printf("data_in_use low='%d' lnearest='%d', block_hi='%d'\n", low, lnearest_block_boundary, m->dealocated_block_hi);
        
        length = lnearest_block_boundary - m->dealocated_block_hi;
        int madvise_ret = madvise(m->map+m->dealocated_block_hi, length, MADV_DONTNEED);
        if (madvise_ret != 0) 
        {
            printf("Error number %d\n", errno);
            handle_madvise_error(m);
        }
        else
        {
            m->dealocated_block_hi = lnearest_block_boundary;
        }
    }
    return 0;
}

int mmbuf__write_data(struct mmbuf_obj *m, const unsigned char *source, const unsigned int length)
{
    //Writing data may change the mapping if max size is hit
    //printf("%p %p %d %d %ld\n", m->map, source, length, m->filesize, m->write_offset);
    if ((m->write_offset + length) > (m->filesize))
    {
        printf("coring here \n");
        printf("old_size='%ld' old_map_pt='%p'\n", m->filesize, m->map);
        grow_file(m, m->filesize*1.5);
        printf("new_size='%ld' new_map_pt='%p'\n", m->filesize, m->map);
    }
    memcpy((m->map)+(m->write_offset), source, length);
    m->write_offset += length;
    return 0;
}

