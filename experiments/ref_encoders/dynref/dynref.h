#ifndef _dynref_H_
#define _dynref_H_

#include "mmbuf.h"

#define DYNREF_N_ELEMENTS 4096   // This is only approx.  Will only flush on a change of message num.  +0 Do not count.
#define DYNREF_MAX_ELEMENTS DYNREF_N_ELEMENTS * 2   // This implies that a single msg can't be repeated more than N_ELEMENTS_times.  Boundary condition.

/* dynref FILE STRUCTURE. 
[
    (meta:: total_counts, n_msgs, counts_offset)  #<---- This is allocated but no written till the end.
    (type, size, block), 
    (type, size, block), 
    (type, size, block), 
    (type, size, block), 
    (type, size, block), 
    (type_counts, [counts]), #<-- This is key information lets us allocate the file super fast.  Just add them and no need to resize.  We know the positional stuff too.
]
*/

struct dynref__obj 
{
    char mode;
    struct mmbuf_obj *ref_file;
    char ref_file_path[256];   // Should this just be malloced.  More generally correct no max size.
    
    // This is needed so we know say val_id=13 will need x space in the cpref. Or other store.  Useful even for things like encoding to tell most common.
    unsigned long long num_val_ids;        // This is the size of the array below.
    unsigned long long total_count;        
    unsigned long long *val_id_counts;     // This is just an array indexed by the val_id.  The val_id is stored inside the trie,
    
    unsigned long long num_uniq_msgs_ids;  // This can be counted! just by counting the number of times msg_id/last_msg_id increases. (msg_id can never increa)
    bool msg_id_always_uniq;               // One to one surjective (reaping RHS)

    // Here we store before flush.  Once buffer is full we can decide how best to serialise the data from the buffer.
    int num_in_buffer;
    unsigned long long *msg_ids_buffer;   // Msg ids can be repeated in the buffers above.
    unsigned long long *val_ids_buffer;
    
    // He we can work out if continuous like block.  Else sparse.
    signed long long low_msg_id;
    signed long long high_msg_id;
    signed long long last_msg_id;    // The last msg_id, -1 if there was no last
    bool plus_zero_present;          // last_msg_id==msg_id somewhere in the block.  "one to one" or "many to one" mapping in just this block.

    // We then review the contents of val_ids buffer.  If lots of repeating fields.  Can looks at hauffmann encoding them.
};

struct dynref__iter
{
    struct dynref__obj *d;
    signed long long i;                   // Current i, -1 if not started.
    signed long long offset;              // Current file offset.  -1 if not started.
    int num_in_buffer;
    unsigned long long *msg_ids_buffer;   // Msg ids can be repeated in the buffers above.
    unsigned long long *val_ids_buffer;
};


/*---
 GLOBAL METHODS
---*/
int dynref__open(struct dynref__obj *const p, const char *ref_file_path, const char mode);
int dynref__close(struct dynref__obj *p, bool del);

/*---
 WRITE METHODS
---*/
signed long long dynref__append(struct dynref__obj *const p, struct dynref__buffer *const buf, const signed long long ref);      
int finalise(struct dynref__obj *const p); // This can only be called once.  
// Write usage:: open, append, append, append, finalise, iterate sizes, close

/*---
 GLOBAL ANAYLSIS
---*/
// What are these used for, some of these could be quite expensive.
unsigned long long dynref__nmsgs(const struct dynref__obj *p);   // msg1, msg1, msg2, msg6 :: 3 uniq_messages
unsigned long long dynref__nlinks(const struct dynref__obj *p);  // msg1->val1, msg1->val2, msg2->val2, msg6->val3 :: 4 links
unsigned long long dynref__nvals(const struct dynref__obj *p);   // msg1->val1, msg1->val2, msg2->val2, msg6->val3 :: 3 uniq_vals

// One to one but possible repeating right elements.
bool dynref__surjective(const struct dynref__obj *p);             // msg1->val1, msg1->val2, msg2->val2, msg6->val3 :: False, msg1 dublicated.

/*---
 READ METHODS
---*/
signed long long dynref__iter__(const struct dynref__obj *p, const struct dynref__iter *i);   
// Negative if end of stream. Thus can create a len out of this.
signed long long dynref__next(const struct dynref__iter *i);                                  
// Negative if end of stream. Thus can create a len out of this.

#endif       //_dynref_H_


/*

###############################################

The val_ids are random.  How to efficiently store these.  They can be anything in the domain that is less than msg_id.
We know that we will have less than x of these.

Really want to reuse 
Imagine a flag in each terminal node.  

[ , , , , , , , , , ] array index is val_id

When to store continuously.  When to store separate???
If can use positionally this will be the most space efficient mode.  It will be a bit slower to query.
If is very sparse, then using per key might be a better storage solution.

In this temporary store. Space efficiency is best as everything need to be re-evaluated anyway.


Numbers on a domain. 

Detecting val_id resuse is a hard problem. Maybe.  
In the trie objects.
    * In this we could tag a node to say this has been accessed.
    * 


##################################################

Data comes in like
        msg_id=1, key_id=1
        msg_id=2, key_id=2
        msg_id=3, key_id=3
    or could be
        msg_id=10000, key_id=1
        msg_id=20323, key_id=4
        msg_id=23233, key_id=2

msg_id will always increase or be equal with prev msg_id.
key_id can be anything, but must be less or equal to msg_id.

Local ranges:

    #Many common domains.
     * wMsgType: 1tomany Every msg_id (high density every message) has many val_ids (small set)
     * wSrcTime: 1to1 Every msg_id (high density every message) has a new key_id
     * wIP: 1to1 Every msg_id (high density every message) has one key_id (small set)
    
    #We can generalise this to:
     * msg_id: 1to1, 1tomany. dense, sparse.
     * key_id: large_set, small_set. distributions frequency.
    
    #Special cases
     * High msg_id density: (close to every message), 1to1 msg_id has one key_id.  These are great as can be positional, great storage properties.
     * Common ids. Lots of repeation, we can do something like huffman encoding
     ? If it has a huge domain of numbers but only 10 numbers represented can something be done here?  Especially for the case where the numbers tend to be clustered.  10, 11, 12.  13,14,15.  etc.
            # This would be like wSrcTime. Where each new message pretty much has a new key id.
     * Lower message density.  The msg_ids can be stored as diffs.
        plus_zero_present
        [ 
          (+1, key_id),                # (unsigned long long. i can be an int.)
          (+1, key_id),
          (+1212, key_id),
          (+2, key_id),
          (+131, key_id),
          (+0, key_id)
        ]
     * Many to one domains: Keep this simple for now.

    #How do we tell
     * Do we have any msg_id repetition in the buffer? 
     ? Store numbers in an intermediate state that makes the decision clear?
    

# On disk formats
    # normal local domain  where the number of elements 
        [size][type][entries]

    # Big domain little local
        [size][type][num_locals][...][num_entries][....]  # Think num_locals is a mapping. # Might need first and then diff


###############################################################################################
###############################################################################################
Thoughs on temp storage.

How about two diff tries. Used to store the msg and id.
We can look at the trie and decide the best storage properties.

Could store msg_id, val_id or val_id, msg_ids.  
When adding can just append msg_id. But then how to we efficiently append count numbers.  
Cost of random access.

*/