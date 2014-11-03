/*
// This feels like the same problem with storing the refs 
// bit vs diff vs encoded vs plain
// We need to store offsets of keys.
// Say every first letter, 256 possibilities. Say only 3 first letters are used. But maybe 200 are used.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "trie.h"

static int goto_node_zero(struct trie__view *t);
static int goto_node_zero(struct trie__view *t)
{
    // -1 just temp value.  We re-evaluate it.
    //first child of every node all the way down
    struct trie__node *n = t->root;

    struct trie__frame tmp = {-1, n};
    t->stack_index = 0;
    t->stack[t->stack_index] = tmp;

    while (n->children != NULL)
    {
        t->stack[t->stack_index].stack_child_index = 0;
        n = n->children[0].node;  // Get first child
        
        struct trie__frame tmp = {-1, n};
        t->stack_index++;
        t->stack[t->stack_index] = tmp;
    }

    return 0;
}

static struct trie__node *next_node(struct trie__view *t);
static struct trie__node *next_node(struct trie__view *t)
{
    //printf("stack index =%d \n", t->stack_index);
    struct trie__node *n = t->stack[t->stack_index].node;
    //printf("Is it here?\n");
   // printf("t->stack_index %d\n", t->stack_index);
    //printf("t->children %p\n", n->children);

    if (t->stack_index < 0)
    {
        //printf("1?\n");
        return NULL;
    }
    else if (n->children == NULL)
    {
        //printf("!!!\n");
        return t->stack[t->stack_index--].node;
    }
    else if (t->stack[t->stack_index].stack_child_index == -1)  // -1 when children not null equals unexplored.
    {   
        //printf("coring here\n");
        t->stack[t->stack_index].stack_child_index = 0;         // Going into the first child
        t->stack_index++;
        t->stack[t->stack_index].stack_child_index = -1;
        t->stack[t->stack_index].node = t->stack[t->stack_index-1].node->children[0].node;
        return next_node(t);
    }
    else
    {
        //printf("key = %c\n", t->stack[t->stack_index].node->children[t->stack[t->stack_index].stack_child_index].key);
        //printf("things %d %d\n", t->stack_index, t->stack[t->stack_index].stack_child_index);
        if (t->stack[t->stack_index].stack_child_index < t->stack[t->stack_index].node->last_i)
        {
            //printf("or here %d %d\n", t->stack[t->stack_index].stack_child_index, t->stack[t->stack_index].node->last_i);
            (t->stack[t->stack_index].stack_child_index)++;   // Moving on to next child
            t->stack_index++;

            //printf("mmmkey = %c\n", t->stack[t->stack_index].node->children[t->stack[t->stack_index].stack_child_index].key);
            //printf("things %d %d\n", t->stack_index, t->stack[t->stack_index].stack_child_index);

            t->stack[t->stack_index].stack_child_index = -1;
            t->stack[t->stack_index].node = t->stack[t->stack_index-1].node->children[t->stack[t->stack_index-1].stack_child_index].node;
            //printf("moving on to next node\n");
            return next_node(t);
            //printf("post next node\n");
        }
        else
        {
            // Yield current and move up one
            //printf("???\n");
            return t->stack[t->stack_index--].node;
        }
    }
    return NULL;
}

static bool power_of_two(unsigned char i);
static bool power_of_two(unsigned char i)
{
    if (((i) & (i - 1)) == 0)   // See exercise 2.9 K&R
    {
        return true;
    }
    else
    {
        return false;
    }
}

static struct trie__node *new_child_node(struct trie__node *n, char key, unsigned char position);
static struct trie__node *new_child_node(struct trie__node *n, char key, unsigned char position)
{
    struct trie__node *new_child;
    if (n->children == NULL)
    {
        assert(position==0);
        n->last_i = 0;
        n->children = malloc(sizeof(struct trie__key_node[1]));
        n->children[0].key = key;
        new_child = n->children[0].node = malloc(sizeof(struct trie__node));
        new_child->children = NULL;
        new_child->last_i = 0;
        new_child->has_value = false;
        new_child->result = 0;
        return new_child;
    }
    else if (n->last_i == 255)
    {
        printf("Is not possible to add to a full node\n");
        exit(EXIT_FAILURE);
    }

    if (power_of_two(n->last_i + 1))
    {
        unsigned char old_size = n->last_i + 1;
        unsigned char new_size = old_size << 1;
        n->children = realloc(n->children, sizeof(struct trie__key_node[new_size]));
    }
    
    new_child = malloc(sizeof(struct trie__node));
    new_child->children = NULL;
    new_child->last_i = 0;
    new_child->has_value = false;
    new_child->result = 0;

    n->last_i++;
    bool append = (position == n->last_i);
    bool error = (position > n->last_i);

    if (append)
    {   
        assert(position==n->last_i);
    }
    else if (error)
    {
        printf("ERROR: can only use this to append or inplace insertion\n");
        exit(EXIT_FAILURE);
    }
    else  // inplace
    {
        int size = sizeof(n->children[position])*(n->last_i-position);   //inclusive last_i is now cur len due to ++
        memmove(n->children+position+1, n->children+position, size);  // Shift over by one
    }
    n->children[position].key = key;
    n->children[position].node = new_child;

    return new_child;
}

static unsigned char bisect_children(const struct trie__key_node *children, const char last_i, const char letter);
static unsigned char bisect_children(const struct trie__key_node *children, const char last_i, const char letter)
{
    /* Will return position where letter equals 
    or the position where that letter should be placed.
    */
    int low = 0;
    int high = last_i; 
    char c;
    unsigned char p = 0;
    while (low <= high)
    {
        p = (high+low)/2;
        c = children[p].key;
        //printf("fkeys %c\n", c);
        if (letter > c)
        {
            low = p + 1;
            p++;  //If we break straight after this is the position.
        }
        else if (letter < c)
        {
            high = p - 1;
        }
        else
        {
            return p;  // returning matching p allows us to use above as a binary search as well.
        }
    }
    return p;
}


static struct trie__node *trie__setdefault(struct trie__view *t, unsigned char *key, unsigned int len, bool not_found_init);
static struct trie__node *trie__setdefault(struct trie__view *t, unsigned char *key, unsigned int len, bool not_found_init)
{
    struct trie__node *n = t->root;
    char letter;
    bool found = false;

    unsigned int letter_index;

    for (letter_index=0; letter_index<len; ++letter_index)
    {
        letter = key[letter_index];
        //printf("fletter='%u'\n", field[letter_index]);

        found = false;
        if (n->children == NULL)
            break; // n has no children

        // binary search here? Better if children split into nodes[] and keys[]???  Cache?
        int position = bisect_children(n->children, n->last_i, letter);
        if (position > n->last_i)
        {
            //printf("in new code\n");
            //printf("Not Found %c %d\n", letter, position);
        }
        else if (n->children[position].key == letter)
        {
            //printf("len= %u\n", n->last_i);
            //printf("trie__setdefault::len= %u\n", n->last_i);
            //if (not_found_init==false)
            //    printf("trie__setdefault::Found '%c' '%c' %d\n", letter, n->children[position].key, position);


            // Issue here if node matches lettter but doesn't have value.

            n = n->children[position].node;
            found = true;
        }
        else
        {
            //printf("Not Found %c %d\n", letter, position);
        }

        if (!found)
            break;
    }

    if (not_found_init && !found)
    {
        while (letter_index<len)
        {
            letter = key[letter_index];
            //printf("iletter='%u'\n", field[letter_index]);
            int position; 
            if (n->children == NULL)
            {
                position = 0;
            }
            else
            {
                position = bisect_children(n->children, n->last_i, letter);
            }
            //printf("Adding node %c %d %d\n", letter, letter_index, position);
            n = new_child_node(n, letter, position);
            if (n==NULL)
            {
                printf("Couldn't make child node");
                return NULL;
            }
            ++letter_index;
        }
        n->has_value = true;
        n->result = 0;
        found = true;
        t->len++;
    }

    if (found)
    {
        //printf("n->result=%llu\n", n->result);
        return n;
    }
    else
    {
        return NULL;
    }
}

/*---
 GLOBAL METHODS
---*/
int trie__open(struct trie__view *t, char *trie_name)
{
    strncpy(t->trie_name, trie_name, TRIE_NAME_SIZE);
    t->len = 0;
    t->stack_index = -1;
    //t->stack[0] = NULL;

    t->root = malloc(sizeof(struct trie__node));
    t->root->last_i = 0;
    t->root->children = NULL;
    t->root->has_value = false;
    t->root->result = 0;
    return 0;
}

struct trie__node *trie__get(struct trie__view *t, void *key, unsigned int len)
{
    return trie__setdefault(t, key, len, false);
}

struct trie__node *trie__add(struct trie__view *t, void *key, unsigned int len)
{
    return trie__setdefault(t, key, len, true);
}

unsigned long long trie__len(struct trie__view *t)
{
    return t->len;
}

bool trie__has_value(struct trie__node *n)
{
    return (n->has_value);
}

int trie__has_children(struct trie__node *n)
{
    if (n->children == NULL)
    {
        return 0;
    }
    else
        return n->last_i + 1;
}

int trie__close(struct trie__view *t)
{
    //printf("Pre goto_node_zero \n");
    goto_node_zero(t);
    //printf("Post goto_node_zero \n");

    while (true)
    {
        //printf("pre next \n");
        struct trie__node *n = next_node(t);
        //printf("post next \n");
        if (n == NULL)
            break;
        else if (n->children != NULL)
        {
            //printf("Pre for loop \n");
            for (int i=0; i <= n->last_i; ++i)
            {
                //printf("Freeing %c \n", n->children[i].key);
                free(n->children[i].node);
                n->children[i].node = NULL;
            }
            //printf("Freeing children array \n");
            free(n->children);
            n->children = NULL;
        }
    }
    //printf("freeing root\n");
    free(t->root);
    t->root = NULL;
    return 0;
}

int trie__iter(struct trie__view *t)
{
    return goto_node_zero(t);
}

struct trie__node *trie__next(struct trie__view *t)
{
    struct trie__node *node =  NULL;
    while (true)
    {
        node = next_node(t);
        if (node == NULL)
            break;
        
        if (node->has_value)
        {
            break;
        }
    }
    return node;
}

int trie__fullkey(struct trie__view *t, unsigned char *dest, unsigned int *len)
{
    unsigned int i;
    for (i = 0; i <= t->stack_index; ++i)
    {
        int c_i = t->stack[i].stack_child_index;

        dest[i] = t->stack[i].node->children[c_i].key;
    }
    //dest[i] = t->stack[i].node->key;
    //++i;
    *len = i;

    return 0;
}