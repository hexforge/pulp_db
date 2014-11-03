#include <stdio.h>
#include "blockpack.h"

/*---
DATA
---*/


/*---
FUNCTIONS
---*/
static int pgread_big_endian(unsigned char *page, struct index_packet *target);
int pgread_big_endian(unsigned char *page, struct index_packet *target)
{
    return -1;
}

static int pgwrite_big_endian(unsigned char *page, struct index_packet *source);
int pgwrite_big_endian(unsigned char *page, struct index_packet *source)
{
    return -1;
}

static int pgread_little_endian(unsigned char *page, struct index_packet *target);
int pgread_little_endian(unsigned char *page, struct index_packet *target)
{
    int i;
    for (i=0; i<NUM_PACKS_IN_PAGE; ++i)
    {
        target[i].offset = (*(unsigned long*) page)++;
        target[i].msg_sizes[0] = (*(unsigned short int*) page)++;
        target[i].msg_sizes[1] = (*(unsigned short int*) page)++;
        target[i].msg_sizes[2] = (*(unsigned short int*) page)++;
        target[i].msg_sizes[3] = (*(unsigned short int*) page)++;
    }
    return 0;
}

static int pgwrite_little_endian(unsigned char *page, struct index_packet *source);
int pgwrite_little_endian(unsigned char *page, struct index_packet *source)
{
    int i;
    for (i=0; i<NUM_PACKS_IN_PAGE; ++i)
    {
        (*(unsigned long*) page) =  source[i].offset;
        page += 8;
        (*(unsigned short int*) page) = source[i].msg_sizes[0];
        page += 2;
        (*(unsigned short int*) page) = source[i].msg_sizes[1];
        page += 2;
        (*(unsigned short int*) page) = source[i].msg_sizes[2];
        page += 2;
        (*(unsigned short int*) page) = source[i].msg_sizes[3];
        page += 2;
    }
    return 0;
}

int blockpack__pgread(unsigned char *page, struct index_packet *target, int little_endian)
{
    if (little_endian)
        return pgread_little_endian(page, target);
    else
        return pgread_big_endian(page, target);
}

int blockpack__pgwrite(unsigned char *page, struct index_packet *source, int little_endian)
{
    if (little_endian)
        return pgwrite_little_endian(page, source);
    else
        return pgwrite_big_endian(page, source);
}

int blockpack__print_structs(struct index_packet *packets, unsigned int npacks)
{
    printf("PRINTING STRUCTS >>>>>>>>>\n");
    int p;
    for (p=0; p<npacks ; ++p)
    { 
        printf("+++++++++packet number='%d'\n", p);
        blockpack__print_struct(packets+p);
    }
    printf("END PRINTING STRUCTS <<<<<<<<\n");
    return 0;
}

int blockpack__print_struct(struct index_packet *packet)
{
    printf("offset=%ld\n", packet->offset);
    printf("size=%d\n", packet->msg_sizes[0]);
    printf("size=%d\n", packet->msg_sizes[1]);
    printf("size=%d\n", packet->msg_sizes[2]);
    printf("size=%d\n", packet->msg_sizes[3]);
    return 0;
}
