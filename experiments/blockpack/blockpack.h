#ifndef blockpackH
#define blockpackH

/*---
PUBLIC INTERFACE
---*/
#define PAGE_SIZE 4096
#define NUM_PACKS_IN_PAGE (PAGE_SIZE/sizeof(struct index_packet))
#define NUM_MSG_IN_PACKET 1

struct index_packet {
    unsigned long offset;
    short unsigned int msg_sizes[4];
};

int blockpack__pgread(unsigned char *page, struct index_packet *target, int little_endian);
int blockpack__pgwrite(unsigned char *page, struct index_packet *source, int little_endian);
int blockpack__print_structs(struct index_packet *packets, unsigned int npacks);
int blockpack__print_struct(struct index_packet *packet);

#endif       //blockpackH