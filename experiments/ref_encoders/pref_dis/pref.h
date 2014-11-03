#ifndef prefH
#define prefH

#include "mmbuf.h"

#define PAGE_SIZE 4096
#define SIZE_OF_ARRAY PAGE_SIZE/sizeof(signed long long)
#define NUM_REF_PER_PAGE (SIZE_OF_ARRAY-1)
#define NEXT_PAGE_INDEX NUM_REF_PER_PAGE


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

struct pref__obj 
{
    char mode;
    struct mmbuf_obj *ref_file;
    signed long long offset;
    char ref_file_path[256];
};

struct pref__buffer
{
    int refs_index;                              // 0 <=0 x < NUM_REF_PER_PAGE. Index of refs the next write/read will go to.
    signed long long global_index;               // Global index, we have stored 1883 messages global index will be 1883. Rename to Num_msgs??
    signed long long first_page_offset;          // Is this is negative we don't have a first page
    signed long long current_page_offset;        // Space in the file ready to be written too, negative if we don't have one yet.
    signed long long *refs;                       // pointer to signed long long refs[SIZE_OF_ARRAY]
                                                 // We use the last ref to store the next page offset. If negative it is the last one.
                                                 // negative ref means end to refs.
};



/*---
 GLOBAL METHODS
---*/
int pref__open(struct pref__obj *const p, const char *ref_file_path, const char mode);
int pref__close(const struct pref__obj *p);

int pref__setup_buffer(struct pref__obj *const p, struct pref__buffer *const pref_b, ...);  // offset only used in read mode;
int pref__teardown_buffer(struct pref__obj *const p, struct pref__buffer *const pref_b);

/*---
 WRITE METHODS
---*/
signed long long pref__append(struct pref__obj *const p, struct pref__buffer *const buf, const signed long long ref);

/*---
 READ METHODS
---*/
signed long long pref__geti(const struct pref__obj *p, struct pref__buffer *const buf, const unsigned long long index);   // Negative if end of stream. Thus can create a len out of this.

#endif       //prefH
