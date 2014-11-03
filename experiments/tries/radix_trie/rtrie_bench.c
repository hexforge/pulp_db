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
  struct rtrie__node *n = NULL;
  printf("Opening rtriee\n");
  if (rtrie__open(&view, "trees"))
    error("ERROR: opening");

  //fp = fopen("5mill_random.data", "r");
  fp = fopen("5mill_random.data", "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  int count = 0;
  while ((read = getline(&line, &len, fp)) != -1) 
  {
    //printf("Rertrieved line of length %zu :\n", read);
    line[read-1] = '\0';
    //printf("adding line '%s'\n", line);
    n = rtrie__add(&view, line, read-1);
    if (n==NULL)
      error("ERROR: adding");
    n->result = count;
    count++;
  }
  
  /*  
  printf("-----------iterating rtrie--------------\n");
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
  
  FILE * fp_read;

  fp_read = fopen("5mill_random.data", "r");
  //fp_read = fopen("5mill_random.data", "r");
  if (fp_read == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp_read)) != -1) 
  {
    //printf("Retrieved line of length %zu :\n", read);
    line[read-1] = '\0';
    //printf("getting line '%s'\n", line);
    n = rtrie__get(&view, line, read-1);
    if (n==NULL)
      error("ERROR: getting");
    //printf("Got line '%s' %llu \n", line, n->result);
  }
  
  if (n!=NULL)
  {
    printf("Got line '%s' %llu \n", line, n->result);
  }
  printf("Original count = %d\n", count);

  free(line);

  printf("closing rtrie\n");
  if (rtrie__close(&view))
    error("ERROR: closing");


  printf("done\n");

  exit(EXIT_SUCCESS);
}
