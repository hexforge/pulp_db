#ifndef mmbufH
#define mmbufH

#define _GNU_SOURCE 1
#include <unistd.h>
#include <sys/mman.h>   // memory map (mmap, madvise and flags)

#include <sys/types.h>

/*---
PUBLIC INTERFACE
---*/
#define BLOCKSIZE (1024*4L)                     // Must be a power of two.

struct mmbuf__obj   //<------------------ Missing second underscore
{
    char mode;
    signed int fd;
    void *map;                /* mmap */
    off_t filesize;
    off_t free_boundary;  // What can we advise the kernel to clean up?
    off_t offset;
};

int mmbuf__setup(struct mmbuf__obj *m, const char *file_path, const char *mode);    //-> error code, zero = success
int mmbuf__teardown(const struct mmbuf__obj *m);                                    //-> error code, zero = success

// Read
unsigned int mmbuf__get_data(const struct mmbuf__obj *m, void **result_p, const off_t offset, const unsigned int length); //-> ammount of data got
int mmbuf__free_data(struct mmbuf__obj *m, const off_t low, const off_t high); //-> error code, zero = success

// Write
int mmbuf__append(struct mmbuf__obj *m, const void *source, const unsigned int length); //-> error code, zero = success

    //(Pointer is tmp) Return a pointer to memory that can be written.  Must be used before the next mmbuf operation.  Pointer may become invalid if file resizes.
void *mmbuf__alloc(struct mmbuf__obj *m, const unsigned int length); //-> pointer (tempory) to allocated memory.

    //Positional writes.  (position is persistant)
off_t mmbuf__pos_alloc(struct mmbuf__obj *m, const unsigned int length);   //-> position (persistant)
void *mmbuf__pos_tmpptr(struct mmbuf__obj *m, const off_t position); //-> pointer (tempory) to memory at that position.



#endif //mmbufH
