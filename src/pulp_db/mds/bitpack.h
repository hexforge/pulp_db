/*

16*3 Composite modulus so can avoid division.  
see http://homepage.cs.uiowa.edu/~jones/bcd/mod.shtml#exmod6

1024bits                         #<-------------- Favorite1
20bits*(16*3)+64
= --max-message-size=1mb

#48*4*8=1536 keys per page. 512*3
#32 blocks per page

2 2/3 bytes per each
0.6gb extra per billion messages
*/

#ifndef bitpackH
#define bitpackH

/*---
PUBLIC INTERFACE
---*/
#define PAGE_SIZE 4096
#define NUM_CLUMPS_IN_PAGE 32
#define MAX_MSG_IN_CLUMP 48


struct bitpack__clump {
    long long offsets[MAX_MSG_IN_CLUMP];
    unsigned int msg_sizes[MAX_MSG_IN_CLUMP];
};


int bitpack__decode(char *source, struct bitpack__clump *target, int little_endian);
int bitpack__encode(char *target, struct bitpack__clump *source, int little_endian);
int bitpack__print_struct(struct bitpack__clump *clump);

#endif       //bitpackH




