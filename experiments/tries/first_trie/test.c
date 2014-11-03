#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"


/*
int trie__open(struct trie__view *t, char trie_name[TRIE_NAME_SIZE]);
int trie__close(struct trie__view *t);
int trie__add(struct trie__view *t, char *field, unsigned long long result);
int trie__get(struct trie__view *t, char *field);
int trie__geti(struct trie__view *t, unsigned long long index);
int trie__iter(struct trie__view *t);
int trie__next(struct trie__view *t);       
unsigned long long trie__len(struct trie__view *t);
*/

static char *STRINGS[] = {"abada", "abc", "abcd", "erm", "abc", "erq"};
static unsigned int STRING_LENS[] = {6, 4, 5, 4, 4, 4};              // Done so we can test a null in middle string easy.

static char *SEARCH_STRINGS[] = {"erm", "not_there", "erq", "abc", "abcd"};
static unsigned int SEARCH_STRING_LENS[] = {4, 10, 4, 4, 5};

void error(char *msg);
void error(char *msg)
{
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    printf("hello world\n");
    
    struct trie__view view;
    struct trie__node *n;    

    printf("-----------Opening trie--------------\n");
    if (trie__open(&view, "trees"))
        error("ERROR: opening");
    
    printf("-----------Adding elements to trie--------------\n");
    int num_elements = sizeof(STRINGS) / sizeof(STRINGS[0]);
    for (int i=0; i<num_elements; ++i)
    {
        printf("Adding %s, %d\n", STRINGS[i], i);
        n = trie__add(&view, STRINGS[i], STRING_LENS[i]);
        if (n==NULL)
            error("ERROR: adding");
        n->result = i;
    }
    
    printf("-----------Checking number of elements in trie--------------\n");
    int len_tree = trie__len(&view);
    printf("Number of elements %d\n", len_tree);

    printf("-----------Getting elements from trie--------------\n");
    int num_search_elements = sizeof(SEARCH_STRINGS) / sizeof(SEARCH_STRINGS[0]);
    for (int i=0; i<num_search_elements; ++i)
    {
        printf("Getting %s, %d\n", SEARCH_STRINGS[i], i);

        n = trie__get(&view, SEARCH_STRINGS[i], SEARCH_STRING_LENS[i]);
        if (n==NULL)
        {
            printf("Not found: key='%s'\n", SEARCH_STRINGS[i]);
        }
        else
        {
            printf("FOUND: key %s, result %lld \n", SEARCH_STRINGS[i], n->result);
        }
    }

    printf("-----------iterating trie--------------\n");
    printf("init_iter\n");
    if (trie__iter(&view))
        error("ERROR: init_iter");

    int j;
    unsigned char fullk[STACK_SIZE];
    unsigned int len;
    for (j=0; j<len_tree; ++j)
    {
        //printf("erm %d %d\n", len_tree, j);
        n = trie__next(&view);
        if (n==NULL)
            error("ERROR: next");
        trie__fullkey(&view, fullk, &len);
        printf("Got element %d, key='%s', result='%lld'\n", j, fullk, n->result);
    }

    printf("-----------Closing trie--------------\n");
    if (trie__close(&view))
        error("ERROR: closing");

}
