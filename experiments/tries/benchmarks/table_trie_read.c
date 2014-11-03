#include <stdio.h>
#include <stdlib.h>
#include "trie.h"
#include "tabletrie.h"
#include <unistd.h>

void error(char *msg);
void error(char *msg)
{
    printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
  struct ttrie__obj tt;
  struct ttrie__obj *ttp = &tt;

  ttrie__open(ttp, 'w');

  //ttrie__dprint(ttp);

  char *ttrie_file_path = "foobar.ttrie";
  printf("Reading from file %s \n", ttrie_file_path);
  ttrie__read(ttp, ttrie_file_path);
  printf("Done reading\n");

  sleep(10);

  ttrie__close(ttp);



  printf("done\n");

}
