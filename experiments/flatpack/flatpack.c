#include <stdio.h>
#include "flatpack.h"

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
        target[i].msg_size = (*(unsigned short int*) page)++;
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
        (*(unsigned short int*) page) = source[i].msg_size;
        page += 2;
    }
    return 0;
}

int flatpack__pgread(unsigned char *page, struct index_packet *target, int little_endian)
{
    if (little_endian)
        return pgread_little_endian(page, target);
    else
        return pgread_big_endian(page, target);
}

int flatpack__pgwrite(unsigned char *page, struct index_packet *source, int little_endian)
{
    if (little_endian)
        return pgwrite_little_endian(page, source);
    else
        return pgwrite_big_endian(page, source);
}

int flatpack__print_structs(struct index_packet *packets, unsigned int npacks)
{
    printf("PRINTING STRUCTS >>>>>>>>>\n");
    int p;
    for (p=0; p<npacks ; ++p)
    { 
        printf("+++++++++packet number='%d'\n", p);
        flatpack__print_struct(packets+p);
    }
    printf("END PRINTING STRUCTS <<<<<<<<\n");
    return 0;
}

int flatpack__print_struct(struct index_packet *packet)
{
    printf("offset=%ld\n", packet->offset);
    printf("size=%d\n", packet->msg_size);
    return 0;
}

