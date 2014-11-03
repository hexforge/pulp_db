#ifndef trieH
#define trieH

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
#define TRIE_NAME_SIZE 64


// Can reduce the memory much more. Have child_keys, and child_nodes.  First key is len-2.
// Double +1 as max len is 257.
// Saves 8 + 2*8*256 -> 8 + 257. It would save us memory is every case.
// Len is more cache freindly????
// Basically get rid of trie__key_node
// Huffman encode would need to save us even more?  256*+1, 128*+2, 2*+256.  Could half the memory again.

/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
These are the memory important ones
*/
struct trie__node
{                                                 
                            
    struct trie__key_node *children;              // If null empty, these are kept in sorted order.
    unsigned char last_i;                         // last_index with a value. if children null ignore.
    bool has_value;
    // Values
    unsigned long long result;                    // Probably going to be initial offset of refstore.
};

struct trie__key_node
{
    char key;                                     // Can have many char in here going to be padded anyway.
    struct trie__node *node;
};
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/

struct trie__frame
{
    signed short stack_child_index;
    struct trie__node *node;
};

struct trie__view    // This is a view on the data.  Using this shows the data can go away.
{
    char trie_name[TRIE_NAME_SIZE];
    unsigned long long len;          // Number of keys
    struct trie__node *root;

    signed short stack_index;
    struct trie__frame stack[STACK_SIZE];
};


// Reason for not returning trie__view from open is that would be an expensive copy.  Not a valid argument as it a one off cost.  WHat do we return on failure???
// Reason for not returning trie__view* from open is that is pretty much the same as it is now.  It does save an int though!!!!!!!.  Prehaps this is best.

int trie__open(struct trie__view *t, char trie_name[TRIE_NAME_SIZE]);
int trie__close(struct trie__view *t);        

// Need to be able to flush to disk and merge at some point.

//int trie__write(struct trie__view *t, char *trie_file_path);
//int trie__read(struct trie__view *t, char *trie_file_path);

// Result can be found in the node count and result:
// Reasons
//result needs to be modifiable in one call.
//result needs to be able to be switched out of memory.

struct trie__node *trie__add(struct trie__view *t, void *key, unsigned int len);   // add result is set.
struct trie__node *trie__get(struct trie__view *t, void *key, unsigned int len);   // No side effect of adding. result is set if it is there else None.
// When t->result is different, then you most consider your previously help pointer out of scope.


// Node pointer or NULL: What is wrong with this?
// Add a get full key function and take result_key out of trie_view.
// Could make *root just root the struct.  Save a dereference.

// Here it would make sense to know what the key is.
// Perhaps iteration should be special. Should not need to start from root each time.
// A stack of nodes with index for which child we currently in :D

unsigned long long trie__len(struct trie__view *t);           // Number of keys in trie
bool trie__has_value(struct trie__node *n);
int trie__has_children(struct trie__node *n);

// Iteration
int trie__iter(struct trie__view *t);
struct trie__node *trie__next(struct trie__view *t);          // Next stored key
int trie__fullkey(struct trie__view *t, unsigned char *dest, unsigned int *len);          // What is the key of the node we last iterated too.

#endif  //trieH
