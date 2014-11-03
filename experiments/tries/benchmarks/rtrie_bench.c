#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "rtrie.h"

void error(char *msg);
void error(char *msg)
{
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  
  struct rtrie__view view;
  struct rtrie__node *n;
  printf("Opening triee\n");
  if (rtrie__open(&view, "trees"))
    error("ERROR: opening");

  fp = fopen("wSrcTimes.data", "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) 
  {
    //printf("Retrieved line of length %zu :\n", read);
    line[read-2] = '\0';
    //printf("adding line '%s'\n", line);
    n = rtrie__add(&view, line, read-1);
    if (n==NULL)
      error("ERROR: adding");
    n->result = 42;
  }
  
  /*  
  printf("-----------iterating trie--------------\n");
  int len_tree = rtrie__len(&view);
  printf("init_iter\n");
  if (rtrie__iter(&view))
      error("ERROR: init_iter");

  int j;
  unsigned char fullk[STACK_SIZE];
  unsigned int len2;
  for (j=0; j<len_tree; ++j)
  {
      //printf("erm %d %d\n", len_tree, j);
      n = rtrie__next(&view);
      if (n==NULL)
          error("ERROR: next");
      rtrie__fullkey(&view, fullk, &len2);
      fullk[len2] = '\0';
      printf("Got element %d, key='%s', result='%lld'\n", j, fullk, n->result);
  }
  */
  


  printf("closing trie\n");
  if (rtrie__close(&view))
    error("ERROR: closing");

  free(line);

  printf("done\n");

  exit(EXIT_SUCCESS);
}
