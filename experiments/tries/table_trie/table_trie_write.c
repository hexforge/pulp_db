#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "rtrie.h"
#include "tabletrie.h"

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

  int count = 0;
  //fp = fopen("foo", "r");
  fp = fopen("2mill_random.data", "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) 
  {
    //printf("Retrieved line of length %zu :\n", read);
    line[read-1] = '\0';
    //printf("adding line '%s'\n", line);
    n = rtrie__add(&view, line, read-1);
    if (n==NULL)
      error("ERROR: adding");
    n->result = count;
    count++;
  }
  
  
  printf("Converting to table trie\n");
  struct ttrie__obj tt;
  struct ttrie__obj *ttp = &tt;

  ttrie__open(ttp, 'w');
  ttrie__convert(ttp, &view);
  printf("Done: Converting to table trie\n");

  //ttrie__dprint(ttp);

  //char *ttrie_file_path = "foobar.ttrie";
  //printf("Writing to file %s \n", ttrie_file_path);
  //ttrie__write(ttp, ttrie_file_path);

  FILE * fp_read;

  //fp_read = fopen("foo", "r");
  fp_read = fopen("2mill_random.data", "r");
  if (fp_read == NULL)
    exit(EXIT_FAILURE);

  unsigned long long result;
  while ((read = getline(&line, &len, fp_read)) != -1) 
  {
    //printf("Retrieved line of length %zu :\n", read);
    line[read-1] = '\0';
    //printf("getting line '%s'\n", line);
    int rc = ttrie__get(ttp, line, read-1, &result);
    if (rc!=0)
      error("ERROR: getting");
    //printf("Got line '%s' %llu \n", line, result);
  }

  printf("Got line '%s' %llu \n", line, result);
  printf("Original count = %d\n", count);

  free(line);




  //printf("Done writing closning ttrie\n");
  ttrie__close(ttp);

  printf("closing trie\n");
  if (rtrie__close(&view))
    error("ERROR: closing");

  

  printf("done\n");

  exit(EXIT_SUCCESS);
}
