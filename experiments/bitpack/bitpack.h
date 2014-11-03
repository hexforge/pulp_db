/*

16*3 Composite modulus so can avoid division.  
see http://homepage.cs.uiowa.edu/~jones/bcd/mod.shtml#exmod6

1024bits                         #<-------------- Favorite1
20bits*(16*3)+40
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
#define NUM_PACKS_IN_PAGE 32
#define NUM_MSG_IN_PACKET 48

struct index_packet {
    unsigned long offset;
    unsigned int msg_sizes[NUM_MSG_IN_PACKET];
};


int bitpack__pgread(unsigned char *page, struct index_packet *target, int little_endian);
int bitpack__pgwrite(unsigned char *page, struct index_packet *source, int little_endian);
int bitpack__print_structs(struct index_packet *packets, unsigned int npacks);
int bitpack__print_struct(struct index_packet *packet);

// TODO

//lookup :: offset block_address, block_index   # Needs to buffer so can look up 1,3,5 super fast as cached.
//iterate ::

#endif       //bitpackH




