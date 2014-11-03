#ifndef __tabletrie_H
#define __tabletrie_H

#include "rtrie.h"

/**
* Initial approach.
* First iteration only can be converted from 'first_trie'
* Immutable
* Concentrate on loading all from memory for now.  Ignore memory waste in structs.
*/
// Once this is working can do anaylsis on it.

//<fieldname>.keys   ## trie data.
//<fieldname>.meta   ## Count primary node
//<fieldname>.values ## refs

#define STACK_SIZE 256
#define TTRIE_WALK_UP 6                     // Looks like up arrow.
#define TTRIE_WALK_ERROR 3                  // Who knows
#define TTRIE_WALK_DOWN 9                   // Looks like down arrow
#define TTRIE_STACK_STATE_L -1
#define TTRIE_STACK_STATE_R -2


/*
# We can see the node types easy as we already have the trie.
# We have 5 types of state. We are trying to minimise the amount of data in a node.  
*/

#define CHOICE_TABLE 0                 // Will have choice of targets we pick up an element
#define TERMINATOR_TABLE 1             // Will have a value
#define PASSTHROUGH_TABLE 2            // Will have value and suffix
#define SUFFIX_TABLE 3      // Will have a value and a single target
#define INFIX_TABLE 4               // Will have a infix and a single target

#define MAX_CHOICE_TABLE_DEPTH 256

#define ORDER_BY_ID 1
#define ORDER_BY_NAME 2

/*
// Could separate the different lengths ones
struct table_name_union
{
    unsigned char len;
    table_name_union{
        char (*name0)[0];
        char (*name1)[1]
        char (*name2)[2]
        char (*name3)[3]
        char (*name4)[4]
        char (*name5)[5]
        char (*name6)[6]
        char (*name7)[7]
    } u;
};
*/

//Design a choice table api.

//find_by_string   // This is used on creation only.  This data structure is tempory and can be thrown away.  Hash table or trie.   
//find_by_number   // This is used when reading.  number is insertion order number.


// Tables are quite common, 
struct ttrie__choice_table
{
    unsigned char *name;                 // First element = last_i -1, not null terminated string!!!!!  len_minus_2
    unsigned char *ids;                  // First element[0] is the number of following rows in the array. len_minus_2

    // To save on slop moving both of these below into the first element of their assoicated arrays.
    //unsigned char name_len;      
    //unsigned int last_row_i;            // This costs us 8bytes.

    // Size progression
    // 1,2,4,8,16,32,64,128,256 STOP
};

struct ttrie__terminator
{
    unsigned long long *results;
    unsigned long long last_i;
};

// Not reusing suffixes.
struct ttrie__suffix
{
    unsigned long long *results;
    unsigned char *suffix_lens;
    unsigned char **suffixs;
    unsigned long long last_i;
};

struct ttrie__passthrough
{    
    unsigned char (*ids)[5]; 
    unsigned long long *results;
    unsigned long long last_i;
};

// Not reusing infixs
struct ttrie__infix
{
    unsigned char (*ids)[5];
    unsigned char *infix_lens;
    unsigned char **infixs;
    unsigned long long last_i;
};

struct ttrie__obj
{
    unsigned long long len;                 // Number of nodes in ttrie
    char mode;
    unsigned char root_id[5];                // Should be zero                

    bool index_table_names;
    struct rtrie__view *table_names_index;

    unsigned int last_choice_i;              
    struct ttrie__choice_table *choice_tables;

    struct ttrie__terminator term_table;
    struct ttrie__passthrough pass_table;

    struct ttrie__suffix suffix_table;
    struct ttrie__infix infix_table;
};

struct ttrie__frame
{
    unsigned char table_type;
    unsigned char id[5];
    signed char child_index;                       //<--- Only needed for child nodes.
    unsigned char key[STACK_SIZE];
    int key_len;
};

struct ttrie__iterator                             // This is a view on the data.  Using this shows the data can go away.
{
    struct ttrie__obj *view;
    struct ttrie__frame *frames;
    signed int stack_index;                             
};




// If you load all table_id is fine
// If you want to do from disk.  Then we are in trouble.
// We will not know the offsets until we are finished.  This is due to table reuse.
// On disk the table type is specifed.  len, type, data.  In memory the data is filled into a struct but passed around with

/*
id is 5 bytes long.  
    starts with 1. table_num = [1], row_num = [4], // Can be quite flexible here.
    else table_num = [4]-1bit row_num = [1]
*/

//16bytes a choice table



//////////////////////////////////////////////////////////////////////


/**
 * General
 */
int ttrie__open(struct ttrie__obj *tt, char mode);
int ttrie__convert(struct ttrie__obj *tt, struct rtrie__view *t);
int ttrie__write(struct ttrie__obj *tt, char *trie_file_name);
int ttrie__close(struct ttrie__obj *tt);

/** 
 * Read only function
 */
int ttrie__read(struct ttrie__obj *tt, char *trie_file_name); 

int ttrie__get(struct ttrie__obj *tt, void *key, int keylen, unsigned long long *result);
unsigned long long ttrie__len(struct ttrie__obj *tt);           // Number of keys in trie

void ttrie__dprint(struct ttrie__obj *tt);
void ttrie__unindex(struct ttrie__obj *tt);       // This should be more hidden
void ttrie__print_root(struct ttrie__obj *tt);

// Iteration
int ttrie__iter(struct ttrie__iterator *iter, struct ttrie__obj *tt);
int ttrie__next_node(struct ttrie__iterator *iter, void *key, int *key_len, unsigned long long **value);
int ttrie__close_iter(struct ttrie__iterator *iter);

#endif  //__tabletrie_H
