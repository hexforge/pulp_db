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

struct mmbuf_obj
{
    char mode;
    signed int fd;
    void *map;                /* mmapped array of chars's */
    off_t filesize;
    long long free_boundary;  // What can we advise the kernel to clean up?
    long long offset;
};

int mmbuf__setup(struct mmbuf_obj *const m, const char *const file_path, const char *mode);
int mmbuf__teardown(const struct mmbuf_obj *m);

// Read
int mmbuf__get_data(const struct mmbuf_obj *m, void **result_p, const long long offset, const int length);
int mmbuf__free_data(struct mmbuf_obj *const m, const long long low, const long long high);

// Write
int mmbuf__append(struct mmbuf_obj *const m, const void *source, const unsigned int length);
void *mmbuf__alloc(struct mmbuf_obj *const m, const unsigned int length);  //returns a pointer to some space at end of file moves on offset, basically so user can write directly into the space.
signed long long mmbuf__offalloc(struct mmbuf_obj *const m, const unsigned int length); 
// Should we be using (void *) instead if (char *) above.

#endif //mmbufH
