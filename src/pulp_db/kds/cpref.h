#ifndef cprefH
#define cprefH

// CONTINOUS PLAIN REFS

// This is to be simple.  
// A faster implementation would be a y fast trie.

// Consideration of size.
// 1 billion messages takes 8gb to store. Use unsigned long long for future proofing.
// If a block is 4096.
// One block stores 512 messages.
// 2million blocks needed to store the messages.
// If the meta for the blocks is 24bytes.
// 48mb will be enough to read all the required meta data for a query.  This is feasible.

/*
Why do we want prev???
    OPens more possibilties can do double end merges.
    Can answer questions like what was the previous trade.
    Because we can, it all the same problem.
*/

/*
# Think about pythons __iter__ fuction that returns an iterator that stores a little state.
# This a more general version, storing more state.  
# It returns a view in which we store the state.  This state can be used for more that iteration: things like jump to >= value or skip to nth value.
*/


// This data base, concentartes on caputring data as fasta rate a possible, doning very little real tim eprocessing on the data.  Data is compressed after.



#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define CPREF_PAGE_SIZE 4096
#define CPREF_SIZE_OF_ARRAY (CPREF_PAGE_SIZE/sizeof(signed long long))
//assert CPREF_SIZE_OF_ARRAY is power of 2.

struct cpref__obj 
{
    char mode;
    struct mmbuf_obj *ref_file;
    char ref_file_path[256];
    unsigned long long size;
};

// More local than three zipped arrays, data used together.  Could move offset it only used on success.
struct cpref__meta_block
{
    signed long long min;        // refs
    signed long long max;        // refs
};

/*---
 WRITE METHODS
---*/
struct cpref__stream 
{
    unsigned char mode;
    unsigned char *allocated_area;
    signed int num_blocks;

    unsigned long long meta_start_offset;
    signed int meta_block_i;

    signed long long ref_start_offset;
    signed int ref_block_i;

    unsigned long long nmsgs;
    signed long long low_ref;
    signed long long high_ref;
    
    signed long long current_i;              // In read mode this is the last written i, in read it is the last read
    signed long long current_block_ref_low;
    signed long long current_block_ref_high;
};

int cpref__open(struct cpref__obj *cp, const char *ref_file_path, const char mode);
int cpref__close(struct cpref__obj *cp);

signed long long cpref__setup_write_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long total_msgs);      // allocates meta storage area in file. 
void cpref__setup_read_stream(struct cpref__stream *stream, struct cpref__obj *cp, unsigned long long offset);           // allocates meta storage area in file. 
void cpref__close_stream(struct cpref__stream *stream);

/*---
 WRITE METHODS
---*/
void cpref__write_stream(struct cpref__stream *stream, signed long long *refs, int n);

/*---
 READ METHODS
---*/
// In the following calls a negative ref is a fail, out of bounds most likely
signed long long cpref__len(struct cpref__stream *stream);  
signed long long cpref__next(struct cpref__stream *stream);   
signed long long cpref__prev(struct cpref__stream *stream);
signed long long cpref__get(struct cpref__stream *stream, unsigned long long i);
signed long long cpref__ge_ref(struct cpref__stream *stream, signed long long ref);
signed long long cpref__le_ref(struct cpref__stream *stream, signed long long ref);

#endif       //cprefH


// Design higher level api for abstracting between different ones.
