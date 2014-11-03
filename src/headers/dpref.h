#ifndef prefH
#define prefH

#include "mmbuf.h"



/*
PLAIN_REF

A ref is just a message number.  If we have refs, 13, 18.  Means messages 13 and 18.
We use it to point to the relevant messages in the master store.

We call it plain as we are literally on disk just storing the numbers.

A pref store allows just to store different stream of references.

Why discontinuous.


-----      ---         ---   Substores
---------------------------  Global store (disk)

We always appending, the substores grow and would have to be moved around to be continuous.

For a simple first implementation.
Instead we use a linked list to 

     |---------.                Stream A
---- ---- ---- ---- ----        ALL data (as in the file)
|---------^---------^           Stream B

We always just append.  We worry about each stream independently once everything is done.
We shall then know more about the data and know how to best encode the message numbers to a compressed format on disk.

Each stream gets it's own buffer.

This is just an intermediate store.  It doesn't have the full api required of the other key stores.

// Reading
// Need to be able to analysis the key.  
// So we know how to transform the data.  
// Then need to be able to iterate over the data.

*/
#define DPREF_BUFFER_OPEN true
#define DPREF_BUFFER_CLOSED false

#define DPREF_PAGE_SIZE 4096
#define DPREF_SIZE_OF_ARRAY (DPREF_PAGE_SIZE/sizeof(signed long long))
static const int DPREF_NUM_REF_PER_PAGE = (DPREF_SIZE_OF_ARRAY-1);
static const int DPREF_NEXT_PAGE_INDEX = (DPREF_SIZE_OF_ARRAY-1);

struct dpref__obj 
{
    char mode;
    struct mmbuf_obj *ref_file;
    char ref_file_path[256];
    unsigned long long num_msgs;
};

struct dpref__buffer
{
    signed int refs_index;                              // 0 <=0 x < DPREF_NUM_REF_PER_PAGE. Index of refs the next write/read will go to.
    bool state;
    unsigned long long global_index;               // Global index, we have stored 1883 messages global index will be 1883. Rename to Num_msgs??
    signed long long first_page_offset;          // Is this is negative we don't have a first page
    signed long long current_page_offset;        // pointer to signed long long refs[DPREF_SIZE_OF_ARRAY]
                                                 // We use the last ref to store the next page offset. If negative it is the last one.
                                                 // negative ref means end to refs.
};


/*---
 GLOBAL METHODS
---*/
int dpref__open(struct dpref__obj *const p, const char *ref_file_path, const char mode);
int dpref__close(struct dpref__obj *p, bool del);

unsigned long long dpref__len(const struct dpref__obj *p);

int dpref__setup_buffer(struct dpref__obj *const p, struct dpref__buffer *const pref_b, ...);  // offset only used in read mode;
int dpref__teardown_buffer(struct dpref__obj *const p, struct dpref__buffer *const pref_b);

/*---
 WRITE METHODS
---*/
signed long long dpref__append(struct dpref__obj *const p, struct dpref__buffer *const buf, const signed long long ref);

/*---
 READ METHODS
---*/
signed long long dpref__geti(const struct dpref__obj *p, struct dpref__buffer *const buf, const unsigned long long index);   // Negative if end of stream. Thus can create a len out of this.

#endif       //prefH
