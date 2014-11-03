#ifndef flatpackH
#define flatpackH

/*---
PUBLIC INTERFACE
---*/
#define PAGE_SIZE 4096
#define NUM_PACKS_IN_PAGE 409     //4096/10
#define NUM_MSG_IN_PACKET 1

struct index_packet {
    unsigned long offset;
    unsigned short int msg_size;
};

int flatpack__pgread(unsigned char *page, struct index_packet *target, int little_endian);
int flatpack__pgwrite(unsigned char *page, struct index_packet *source, int little_endian);
int flatpack__print_structs(struct index_packet *packets, unsigned int npacks);
int flatpack__print_struct(struct index_packet *packet);

#endif       //flatpackH