/*
No need for this.
*/

#ifndef fopenH
#define fopenH

#define _FILE_OFFSET_BITS 64

/*---
PUBLIC INTERFACE
---*/
#define BLOCKSIZE (1024*4L)                     // Must be a power of two.

struct fopen_obj {
    FILE *fp;
    char *file_path;
    unsigned long current_pos;
};

int fopen__setup(struct fopen_obj *fobj, char *file_path, char mode);
int fopen__close(struct fopen_obj *fobj);
int fopen__get_data(struct fopen_obj *fobj, unsigned char *result_p, const unsigned long offset, const int length);
//int fopen__free_data(unsigned char *result);
int fopen__write(struct fopen_obj *fobj, const unsigned char *data, const int length);

#endif //fopenH