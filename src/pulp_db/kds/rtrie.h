#ifndef rtrieH
#define rtrieH

#include <stdbool.h>

/*
Api for a trie


field.vals
field.meta
field.refs

*/

// This needs to know the value being stored in order to serialise it,
// Order is good to have so merging doesn't mean load in full set.

// Could have two types of node.  node with data, note without. Have a union to represent both.

// Could shortcut the current value. Meaning, use the stack when adding,  If the same as just added.
// if add the same thing then we do nothing, if prefix then just reverse up the stack???


#define STACK_SIZE 256
#define RTRIE_NAME_SIZE 64

#define RTRIE_WALK_UP 6                     // Looks like up arrow.
#define RTRIE_WALK_ERROR 3                  // Who knows
#define RTRIE_WALK_DOWN 9                   // Looks like down arrow
#define RTRIE_STACK_STATE_L -1
#define RTRIE_STACK_STATE_R -2


// Can reduce the memory much more. Have child_keys, and child_nodes.  First key is len-2.
// Double +1 as max len is 257.
// Saves 8 + 2*8*256 -> 8 + 257. It would save us memory is every case.
// Len is more cache freindly????
// Basically get rid of rtrie__key_node
// Huffman encode would need to save us even more?  256*+1, 128*+2, 2*+256.  Could half the memory again.


union suffix_or_children
{   
    void *link;
    unsigned char* suffix;
    struct rtrie__node* children;
};      

struct rtrie__node
{                                                 
    unsigned char *keys;
    union suffix_or_children u;                   // If null empty, children are kept in sorted order.
    unsigned int result;                    // Could be just an int.
    // can combine these
    unsigned char last_i;                         // last_index with a value. if children null ignore.   // If need to save 8bytes could shift these into the result and keys.
    unsigned char suffix_len;    
    bool has_suffix;
    bool has_value;    
};
// Could reduce 4*8 to 3*8. Have a seperate array of (count, result) and have node result a int.


struct rtrie__frame
{
    signed short stack_child_index;
    struct rtrie__node *node;
};

struct rtrie__view    // This is a view on the data.  Using this shows the data can go away.
{
    unsigned long long len;          // Number of keys
    struct rtrie__node *root;

    signed short stack_index;
    struct rtrie__frame stack[STACK_SIZE];
};


// Reason for not returning rtrie__view from open is that would be an expensive copy.  Not a valid argument as it a one off cost.  WHat do we return on failure???
// Reason for not returning rtrie__view* from open is that is pretty much the same as it is now.  It does save an int though!!!!!!!.  Prehaps this is best.

int rtrie__open(struct rtrie__view *t);
int rtrie__close(struct rtrie__view *t);        

// Need to be able to flush to disk and merge at some point.

//int trie__write(struct trie__view *t, char *trie_file_path);
//int trie__read(struct trie__view *t, char *trie_file_path);

// Result can be found in the node count and result:
// Reasons
//result needs to be modifiable in one call.
//result needs to be able to be switched out of memory.

struct rtrie__node *rtrie__add(struct rtrie__view *t, void *key, int len, bool *new_entry);   // add result is set.
struct rtrie__node *rtrie__get(struct rtrie__view *t, void *key, int len);   // No side effect of adding. result is set if it is there else None.
// When t->result is different, then you most consider your previously help pointer out of scope.


// Node pointer or NULL: What is wrong with this?
// Add a get full key function and take result_key out of rtrie_view.
// Could make *root just root the struct.  Save a dereference.

// Here it would make sense to know what the key is.
// Perhaps iteration should be special. Should not need to start from root each time.
// A stack of nodes with index for which child we currently in :D

unsigned long long rtrie__len(struct rtrie__view *t);           // Number of keys in rtrie
bool rtrie__node_has_value(struct rtrie__node *n);
bool rtrie__node_has_children(struct rtrie__node *n);
bool rtrie__node_has_suffix(struct rtrie__node *n);

int rtrie__node_num_chidlren(struct rtrie__node *n);

// Iteration
void rtrie__iter(struct rtrie__view *t);
struct rtrie__node *rtrie__next(struct rtrie__view *t);          // Next stored key
int rtrie__fullkey(struct rtrie__view *t, unsigned char *dest, int *len);          // What is the key of the node we last iterated too.

#endif  //rtrieH
