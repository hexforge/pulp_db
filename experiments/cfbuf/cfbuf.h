#ifndef cfbufH
#define cfbufH

#define _FILE_OFFSET_BITS 64

/*---
PUBLIC INTERFACE
---*/
struct cfbuf_obj {
    unsigned char mode;
    unsigned char *buffer;
    unsigned char *current_pos;
    int fd;
    unsigned int buffer_size;
    unsigned long max_offset;
    unsigned long start_offset;
    
};

int cfbuf__setup(struct cfbuf_obj *f,
                 const char *file_path, 
                 const unsigned char mode);   // Could put max message size and things in here.

int cfbuf__close(struct cfbuf_obj *f);

int cfbuf__get_data(struct cfbuf_obj *f, 
                    unsigned char **target, 
                    const unsigned long offset, 
                    const unsigned int min_length);

int cfbuf__free_data(struct cfbuf_obj *f, 
                     const unsigned long offset_to);   // Should this be from to

int cfbuf__write_data(struct cfbuf_obj *f, 
                      const unsigned char *target, 
                      const unsigned int length);

#endif //cfbufH