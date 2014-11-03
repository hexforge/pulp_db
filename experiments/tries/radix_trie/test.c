#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtrie.h"


/*
int rtrie__open(struct rtrie__view *t, char rtrie_name[RTRIE_NAME_SIZE]);
int rtrie__close(struct rtrie__view *t);
int rtrie__add(struct rtrie__view *t, char *field, unsigned long long result);
int rtrie__get(struct rtrie__view *t, char *field);
int rtrie__geti(struct rtrie__view *t, unsigned long long index);
int rtrie__iter(struct rtrie__view *t);
int rtrie__next(struct rtrie__view *t);       
unsigned long long rtrie__len(struct rtrie__view *t);
*/

static char *STRINGS[] = {"abc", "abcd", "erm", "abc", "erq", "123456789", "1234xxxx", "1234567890"};
static unsigned int STRING_LENS[] = {3, 4, 3, 3, 3, 9, 8, 10};              // Done so we can test a null in middle string easy.

static char *SEARCH_STRINGS[] = {"erm", "not_there", "erq", "abc", "abcd", "abce", "123456789", "1234xxxx", "1234567890"};
static unsigned int SEARCH_STRING_LENS[] = {3, 9, 3, 3, 4, 4, 9, 8, 10};

//static char *STRINGS[] = {"erm", "erq", "abc", "abcd"};
//static unsigned int STRING_LENS[] = {3, 3, 3, 4};              // Done so we can test a null in middle string easy.

//static char *SEARCH_STRINGS[] = {"erm", "erq", "abc", "abcd"};
//static unsigned int SEARCH_STRING_LENS[] = {3, 3, 3, 4};



void error(char *msg);
void error(char *msg)
{
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    printf("hello world\n");
    
    struct rtrie__view view;
    struct rtrie__node *n;    

    printf("-----------Opening rtrie--------------\n");
    if (rtrie__open(&view, "trees"))
        error("ERROR: opening");
    
    printf("-----------Adding elements to rtrie--------------\n");
    int num_elements = sizeof(STRINGS) / sizeof(STRINGS[0]);
    for (int i=0; i<num_elements; ++i)
    {
        printf("Adding %s, %d\n", STRINGS[i], i);
        n = rtrie__add(&view, STRINGS[i], STRING_LENS[i]);
        if (n==NULL)
            error("ERROR: adding");
        n->result = i;
    }
    
    printf("-----------Checking number of elements in rtrie--------------\n");
    int len_tree = rtrie__len(&view);
    printf("Number of elements %d\n", len_tree);

    printf("-----------Getting elements from rtrie--------------\n");
    int num_search_elements = sizeof(SEARCH_STRINGS) / sizeof(SEARCH_STRINGS[0]);
    for (int i=0; i<num_search_elements; ++i)
    {
        printf("Getting %s, %d\n", SEARCH_STRINGS[i], i);

        n = rtrie__get(&view, SEARCH_STRINGS[i], SEARCH_STRING_LENS[i]);
        if (n==NULL)
        {
            printf("Not found: key='%s'\n", SEARCH_STRINGS[i]);
        }
        else
        {
            printf("FOUND: key %s, result %lld \n", SEARCH_STRINGS[i], n->result);
        }
    }

    printf("-----------iterating rtrie--------------\n");
    printf("init_iter\n");
    if (rtrie__iter(&view))
        error("ERROR: init_iter");

    int j;
    unsigned char fullk[STACK_SIZE];
    unsigned int len;
    for (j=0; j<len_tree; ++j)
    {
        //printf("erm %d %d\n", len_tree, j);
        n = rtrie__next(&view);
        if (n==NULL)
            error("ERROR: next");
        rtrie__fullkey(&view, fullk, &len);
        fullk[len] = '\0';
        printf("Got element %d, key='%s', result='%lld'\n", j, fullk, n->result);
    }

    printf("-----------Closing rtrie--------------\n");
    if (rtrie__close(&view))
        error("ERROR: closing");

}