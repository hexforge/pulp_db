/*
// This feels like the same problem with storing the refs 
// bit vs diff vs encoded vs plain
// We need to store offsets of keys.
// Say every first letter, 256 possibilities. Say only 3 first letters are used. But maybe 200 are used.
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>  // for NULL
#include <stdlib.h>
#include <assert.h>
#include "rtrie.h"

static int goto_node_zero(struct rtrie__view *t);
static int goto_node_zero(struct rtrie__view *t)
{
    // -1 just temp value.  We re-evaluate it.
    //first child of every node all the way down
    struct rtrie__node *n = t->root;

    struct rtrie__frame tmp = {-1, n};
    t->stack_index = 0;
    t->stack[t->stack_index] = tmp;

    while ((n->u.link != NULL)  && (n->has_suffix==false))
    {
        t->stack[t->stack_index].stack_child_index = 0;
        n = (struct rtrie__node *)n->u.children + 0;  // Get first child
        
        struct rtrie__frame tmp = {-1, n};
        t->stack_index++;
        t->stack[t->stack_index] = tmp;
    }

    return 0;
}

static struct rtrie__node *next_node(struct rtrie__view *t);
static struct rtrie__node *next_node(struct rtrie__view *t)
{
    //printf("stack index =%d \n", t->stack_index);
    struct rtrie__node *n = t->stack[t->stack_index].node;
    //printf("Is it here?\n");
   // printf("t->stack_index %d\n", t->stack_index);
    //printf("t->children %p\n", n->children);

    if (t->stack_index < 0)
    {
        //printf("1?\n");
        return NULL;
    }
    else if (n->u.children == NULL)
    {
        //printf("!!!\n");
        return t->stack[t->stack_index--].node;  //yield node then move back one
    }
    else if (n->has_suffix==true)
    {
        return t->stack[t->stack_index--].node;  //yield node then move back one
    }
    else if (t->stack[t->stack_index].stack_child_index == -1)  // -1 when children not null equals unexplored set of chieldren.
    {   
        //printf("coring here\n");
        t->stack[t->stack_index].stack_child_index = 0;         // Going into the first child
        t->stack_index++;
        t->stack[t->stack_index].stack_child_index = -1;
        t->stack[t->stack_index].node = t->stack[t->stack_index-1].node->u.children + 0;
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
            t->stack[t->stack_index].node = t->stack[t->stack_index-1].node->u.children + t->stack[t->stack_index-1].stack_child_index;  // This addition >0 as in this clause.
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

static void rtrie__init_node(struct rtrie__node *new_node);
static void rtrie__init_node(struct rtrie__node *new_node)
{
    new_node->keys = NULL;
    new_node->u.link = NULL;
    new_node->last_i = 0;
    new_node->has_value = false;
    new_node->has_suffix = false;
    new_node->result = 0;
    new_node->suffix_len = 0;
}

static struct rtrie__node *new_child_node(struct rtrie__node *n, char key, unsigned char position);
static struct rtrie__node *new_child_node(struct rtrie__node *n, char key, unsigned char position)
{
    //printf("new_child_node %c\n", key);
    assert (n->has_suffix==false);

    if (n->u.link == NULL)
    {
        assert(position==0);
        n->last_i = 0;
        
        n->u.children = malloc(sizeof(struct rtrie__node[1]));
        n->keys = malloc(sizeof(char));

        rtrie__init_node((struct rtrie__node *)n->u.children + 0);
        n->keys[0] = key;
        return n->u.children + 0;
    }
    else if (n->last_i == 255)
    {
        //printf("Is not possible to add to a full node\n");
        exit(EXIT_FAILURE);
    }

    if (power_of_two(n->last_i + 1))
    {
        unsigned char old_size = n->last_i + 1;
        unsigned char new_size = old_size << 1;
        n->keys = realloc(n->keys, sizeof(char [new_size]));
        n->u.children = realloc(n->u.children, sizeof(struct rtrie__node [new_size]));
    }
    
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
        int ksize = sizeof(n->keys[position])*(n->last_i-position);   //inclusive last_i is now cur len due to ++
        memmove(n->keys+position+1, n->keys+position, ksize);

        int size = sizeof(n->u.children[position])*(n->last_i-position);   //inclusive last_i is now cur len due to ++
        memmove(n->u.children + position+1, n->u.children + position, size);  // Shift over by one
    }
    struct rtrie__node *new_child = n->u.children + position;
    rtrie__init_node(new_child);
    n->keys[position] = key;
    return new_child;
}

static unsigned char bisect_children(const struct rtrie__node *n, const char letter);
static unsigned char bisect_children(const struct rtrie__node *n, const char letter)
{
    /* Will return position where letter equals 
    or the position where that letter should be placed.
    */
    assert (n->has_suffix==false);

    unsigned char last_i = n->last_i;

    int low = 0;
    int high = last_i; 
    char c;
    unsigned char p = 0;
    while (low <= high)
    {
        p = (high+low)/2;
        c = n->keys[p];
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


static int compare_suffix(unsigned char *suffix_string, int suffix_len, unsigned char *str, int str_len);
static int compare_suffix(unsigned char *suffix_string, int suffix_len, unsigned char *str, int str_len)
{
    if (suffix_len!=str_len)
        return suffix_len-str_len;
    return memcmp(suffix_string, str, str_len);
}

static int breakout_suffix(struct rtrie__node *n);
static int breakout_suffix(struct rtrie__node *n)
{
    // This function replaces a node with a suffix with a normal node with a child under the first letter of the suffix.  The child has the suffix -1.
    assert  (rtrie__node_has_suffix(n));
    unsigned char letter = n->u.suffix[0];
    
    // Prepare for child suffix
    unsigned char *new_suffix;
    unsigned char new_suffix_len;
    if (n->suffix_len > 1)
    {
        new_suffix_len = n->suffix_len - 1;

        new_suffix = malloc(new_suffix_len);
        memcpy(new_suffix, n->u.suffix +1, new_suffix_len);
        free(n->u.suffix);
        //OR
        //new_suffix = memmove(new_suffix, suffix+1, new_suffix_len);   // Faster but wastes a bit of space
    }
    else
    {
        free(n->u.suffix);
        new_suffix_len = 0;
        new_suffix = NULL;
    }
    n->u.suffix = NULL;
    n->has_suffix = false;
    n->suffix_len = 0;

    bool tmp_has_value = n->has_value;
    unsigned long long tmp_result = n->result;
    n->has_value = false;
    n->result = 0;

    int position = 0;
    n = new_child_node(n, letter, position);  // n is now the child node.
    if (n==NULL)
    {
        printf("Couldn't make child node");
        return 1;
    }

    if (new_suffix!=NULL)
    {
        n->u.suffix = new_suffix;
        n->has_suffix = true;
        n->suffix_len = new_suffix_len;
    }

    if (tmp_has_value==true)
    {
        n->has_value = tmp_has_value;
        n->result = tmp_result;
    }
    return 0;
}


/*---
 GLOBAL METHODS
---*/
int rtrie__open(struct rtrie__view *t, char *rtrie_name)
{
    strncpy(t->rtrie_name, rtrie_name, RTRIE_NAME_SIZE);
    t->len = 0;
    t->stack_index = -1;
    //t->stack[0] = NULL;

    t->root = malloc(sizeof(struct rtrie__node));
    rtrie__init_node(t->root);
    return 0;
}


struct rtrie__node *rtrie__add(struct rtrie__view *t, void *key, unsigned int len)
{
    struct rtrie__node *n = t->root;
    unsigned char letter;
    int letter_index;
    int position;
    unsigned char *_key = key;

    // Find correct node.

    for (letter_index=0; letter_index<len; ++letter_index)
    {
        letter = _key[letter_index];
        //printf("fletter='%c'\n", _key[letter_index]);

        // Find position of next node
        if (n->u.link == NULL)
        {
            if (n->has_value==true)
            {
                position = 0;
                n = new_child_node(n, letter, position);
                if (n==NULL)
                {
                    printf("Couldn't make child node");
                    return NULL;
                }
            }
            else
            {
                //printf("Making suffix with %.*s\n", len-letter_index, _key+letter_index);
                n->u.suffix = malloc(len-letter_index);
                memcpy(n->u.suffix, _key+letter_index, len-letter_index);
                n->has_suffix = true;
                n->suffix_len = len-letter_index;
                break;
            }
        }
        else if (n->has_suffix == true)
        {
            
            if (compare_suffix(n->u.suffix, n->suffix_len, _key+letter_index, len-letter_index)==0)
            {
                assert(n->has_value==true);
                break;
            }
            else
            {   //Suffixs differ breakout a child node and redo.
                breakout_suffix(n);
                letter_index--;
            }    
        }
        else
        {
            position = bisect_children(n, letter);

            if  (position > n->last_i)
            {
                n = new_child_node(n, letter, position);
                if (n==NULL)
                {
                    printf("Couldn't make child node");
                    return NULL;
                }
            }
            else if (n->keys[position] == letter)
            {
                //printf("Found %c %d\n", letter, position);
                n = n->u.children + position;
            }
            else
            {
                //printf("Not Found %c %d\n", letter, position);
                n = new_child_node(n, letter, position);
                if (n==NULL)
                {
                    printf("Couldn't make child node");
                    return NULL;
                }
            }            
        }
    }

    // If we run of of letters but end up on a suffix, then we must shift it down and use it's position to store our current val.
    if (letter_index == len)
    {
        if (n->has_suffix == true)
        {
            breakout_suffix(n);
        }
    }

    if (n->has_value==false)
    {
        n->has_value = true;
        n->result = 0;
        t->len++;
    }
    
    return n;
}


struct rtrie__node *rtrie__get(struct rtrie__view *t, void *key, unsigned int len)
{
    struct rtrie__node *n = t->root;
    unsigned char *_key = key;
    char letter;
    bool found = false;

    int letter_index;
    int position;
    for (letter_index=0; letter_index<len; ++letter_index)
    {
        found = false;
        if (n->u.link == NULL)
        {
            //printf("no children\n");
            break;
        }
        else if (n->has_suffix == true)
        {

            //printf("in suffix %u %.*s %u %.*s\n", n->suffix_len, n->suffix_len, n->u.suffix, len-letter_index, len-letter_index, _key+letter_index);
            if (compare_suffix(n->u.suffix, n->suffix_len, _key+letter_index, len-letter_index)==0)
            {
                return n;
            }
            break;
        }
        else
        {
            letter = _key[letter_index];
            //printf("foobar %c children %.*s\n", letter, n->last_i+1, n->keys);
            position = bisect_children(n, letter);
            if ((position <= n->last_i) && (n->keys[position] == letter))
            {
                //printf("Found %c %d\n", letter, position);
                n = n->u.children + position;
                found = true;
                //printf("found %c %.*s\n", letter, n->last_i+1, n->keys);
            }
        }

        if (!found)
            break;
    }

    //printf("am here ok %u\n", found);

    if (found)
    {
        if (n->has_value==false)
        {
            //printf("beep\n");
            found = false;       // FOund a compatable node but no value.
        }
        else
        {
            //printf("boop\n");
            if (letter_index == len)  // If we run out of letters end end up on a node with a suffix it's value is not for us.
                if (n->has_suffix == true)
                    found = false;   
        }
    }

    if (found)
        return n;
    else
        return NULL;
}


unsigned long long rtrie__len(struct rtrie__view *t)
{
    return t->len;
}

bool rtrie__node_has_value(struct rtrie__node *n)
{
    return (n->has_value);
}

int rtrie__node_has_children(struct rtrie__node *n)
{
    if (n->u.link == NULL)
    {
        return 0;
    }
    else
    {
        if (rtrie__node_has_suffix(n))
        {
            return 0;
        }
        else
        {
            return n->last_i + 1;
        }
    }
}

int rtrie__node_has_suffix(struct rtrie__node *n)
{
    if (n->has_suffix == true)
    {
        assert (n->u.suffix != NULL);
        return 1;
    }
    else
    {
        return 0;
    }

}

int rtrie__close(struct rtrie__view *t)
{
    printf("Pre goto_node_zero \n");
    goto_node_zero(t);
    printf("Post goto_node_zero \n");

    while (true)
    {
        //printf("pre next \n");
        struct rtrie__node *n = next_node(t);

        //unsigned char tmp[256];
        //unsigned int len = 0;
        //rtrie__fullkey(t, tmp, &len);
        //tmp[len] = '\0';
        //printf("full_key='%s'\n", tmp);
        //printf("foobar %lld\n", n->result);
        //printf("post next \n");
        if (n == NULL)
            break;
        
        if (n->u.link != NULL)
        {
            //printf("childrern = '%.*s'\n", n->last_i+1, n->keys);

            //printf("Freeing children array \n");
            free(n->u.link);        // Maybe dangerous
            n->u.link = NULL;
            free(n->keys);
            n->keys = NULL;
        }
    }
    printf("freeing root\n");
    free(t->root);
    t->root = NULL;
    return 0;
}

int rtrie__iter(struct rtrie__view *t)
{
    return goto_node_zero(t);
}

struct rtrie__node *rtrie__next(struct rtrie__view *t)
{
    struct rtrie__node *node =  NULL;
    while (true)
    {
        node = next_node(t);
        if (node == NULL)
            break;
        
        if (node->has_value==true)
        {
            break;
        }
    }
    return node;
}

int rtrie__fullkey(struct rtrie__view *t, unsigned char *dest, unsigned int *len)  // Refactor this
{
    unsigned int i;
    for (i = 0; i <= t->stack_index; ++i)
    {
        if (t->stack[i].node->has_suffix)
            break;
        int c_i = t->stack[i].stack_child_index;
        if (c_i==-1)
            break;
        dest[i] = t->stack[i].node->keys[c_i];
    }
    *len = i;


    // Now do the last node, could be a suffix
    if (rtrie__node_has_children(t->stack[i].node))  // I think this if can be removed.
    {
        int c_i = t->stack[i].stack_child_index;

        struct rtrie__node *child = t->stack[i].node->u.children +c_i;
        if (rtrie__node_has_suffix(child))
        {
            int suffix_len = child->suffix_len;
            memcpy(dest + i, child->u.suffix, suffix_len);
            *len += suffix_len;
        }
    } // The root node could have a suffix
    else if (rtrie__node_has_suffix(t->stack[i].node))
    {
        int suffix_len = t->stack[i].node->suffix_len;
        memcpy(dest + i, t->stack[i].node->u.suffix, suffix_len);
        *len += suffix_len; 
    }
    return 0;
}