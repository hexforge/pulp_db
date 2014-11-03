#ifndef mmbufH
#define mmbufH

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/mman.h>   // memory map (mmap, madvise and flags)

#include <sys/types.h>

/*---
PUBLIC INTERFACE
---*/
#define BLOCKSIZE (1024*4L)                     // Must be a power of two.

struct mmbuf_obj
{
    char mode;
    signed int fd;
    unsigned char *map;                /* mmapped array of chars's */
    off_t filesize;
    unsigned long dealocated_block_hi;  // What can we advise the kernel to clean up?
    unsigned long write_offset;
};

int mmbuf__setup(struct mmbuf_obj *m, char *file_path, char mode);
int mmbuf__close(struct mmbuf_obj *m);
int mmbuf__get_data(struct mmbuf_obj *m, unsigned char **result_p, const unsigned long offset, const int length);
int mmbuf__free_data(struct mmbuf_obj *m, const unsigned long low, const unsigned long high);
int mmbuf__write_data(struct mmbuf_obj *m, const unsigned char *source, const unsigned int length);  //Should be called append

#endif //mmbufH
