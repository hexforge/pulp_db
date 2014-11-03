#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "trie.h"

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
  
  struct trie__view view;
  struct trie__node *n;
  printf("Opening triee\n");
  if (trie__open(&view, "trees"))
    error("ERROR: opening");

  fp = fopen("words.data", "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) 
  {
    //printf("Retrieved line of length %zu :\n", read);
    line[read-2] = '\0';
    //printf("adding line '%s'\n", line);
    n = trie__add(&view, line, read-1);
    if (n==NULL)
      error("ERROR: adding");
    n->result = 42;
  }
  
  printf("closing trie\n");
  if (trie__close(&view))
    error("ERROR: closing");

  free(line);

  printf("done\n");

  exit(EXIT_SUCCESS);
}
