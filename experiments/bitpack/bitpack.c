/*
Here we use pages. 4096bytes.  A page contains 32 index packets 
Into this we are packing:
    an offset of 64bits (In reality unlimited addressable), 
    and 48 20bit sizes (max message size 1mb)
This results in 2 2/3 bytes per message indexed.

Notice 32 is a power of 2.
Notice 48 is a composite modulus 16*3 when looking up which quarter page block
Thus so we can use http://homepage.cs.uiowa.edu/~jones/bcd/mod.shtml#exmod6

    https://github.com/gpakosz/PackedArray
    http://stackoverflow.com/questions/1429558/a-c-preprocessor-macro-to-pack-bitfields-into-a-byte
    http://stackoverflow.com/questions/1664563/what-is-a-better-method-for-packing-4-bytes-into-3-than-this
    http://stackoverflow.com/questions/6066157/combining-17bit-data-into-byte-array/6066425#6066425
    http://stackoverflow.com/questions/18961420/reverse-the-endianness-of-a-c-structure/18962229#18962229
    http://stackoverflow.com/questions/6556961/use-of-the-bitwise-operators-to-pack-multiple-values-in-one-int/6557022#6557022
    http://stackoverflow.com/questions/7913760/c-storing-2-numbers-in-one-byte/7913817#7913817


    16*3 Composite modulus so can avoid division.  
    see http://homepage.cs.uiowa.edu/~jones/bcd/mod.shtml#exmod6

    1024bits                         #<-------------- Favorite1
    20bits*(16*3)+64
    = --max-message-size=1mb

    #48*4*8=1536 keys per page. 512*3
    #32 blocks per page

    2 2/3 bytes per each
    0.6gb extra per billion messages

    40bits      5bytes
    48*20bits   
    ==
    5bytes + 24*5bytes


ALTERNATIVE IS TO REVERSE THE WHOLE THING AND HAVE 32 per packet with 

32*20+~40  # Would be the spacing
48 per block
*/
#include <stdio.h>
#include "bitpack.h"


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
    int p;
    for (p=0; p<32; ++p)
    {
        target[p].offset = (*(unsigned long*)page);
        page += 8;
        int i;
        for (i=0; i<24; ++i)
        {
            target[p].msg_sizes[i*2] = (*(unsigned int*)page)&0x0FFFFF;
            target[p].msg_sizes[i*2+1] = ((*(unsigned int*)(page+2))&0x0FFFFF0)>>4;
            page += 5;
        }
    }
    return 0;
}

static int pgwrite_little_endian(unsigned char *page, struct index_packet *source);
int pgwrite_little_endian(unsigned char *page, struct index_packet *source)
{
    //unsigned char *page_old = page;
    printf("Page root %p\n", page);
    int p;
    for (p=0; p<32; ++p)
    {
        *(unsigned long*)page = ((source[p].offset)&0x0FFFFF);   //5 bytes
        page += 8;

        //printf("start ints %p\n", page);
        //printf("second int %p\n", page+1);
        //Not take 24 pairs of ints and squeeze them into 5bytes.
        int i;
        for (i=0; i<24; ++i)
        {
            page[0] = (unsigned char) source[p].msg_sizes[i*2]&0377;
            page[1] = (unsigned char) (source[p].msg_sizes[i*2]>>8)&0377;
            page[2] = (unsigned char) ((source[p].msg_sizes[i*2]>>16)&017) + ((source[p].msg_sizes[i*2+1]&017)<<4); 
            page[3] = (unsigned char) (source[p].msg_sizes[i*2+1]>>4)&0377;
            page[4] = (unsigned char) (source[p].msg_sizes[i*2+1]>>12)&0377;
            //printf("i*2   %d %d %d \n", source[p].msg_sizes[i*2], source[p].msg_sizes[i*2]>>8, source[p].msg_sizes[i*2]>>16);
            //printf("i*2+1 %d %d %d \n", source[p].msg_sizes[i*2+1], source[p].msg_sizes[i*2+1]>>4, source[p].msg_sizes[i*2+1]>>12);
            //printf("%d %d %d %d %d\n", page[0], page[1], page[2], page[3], page[4]);
            page += 5;
        }
    }
    //printf("Thingy %d\n", page-page_old);
    return 0;
}

int bitpack__pgread(unsigned char *page, struct index_packet *target, int little_endian)
{
    if (little_endian)
        return pgread_little_endian(page, target);
    else
        return pgread_big_endian(page, target);
}

int bitpack__pgwrite(unsigned char *page, struct index_packet *source, int little_endian)
{
    if (little_endian)
        return pgwrite_little_endian(page, source);
    else
        return pgwrite_big_endian(page, source);
}

int bitpack__print_structs(struct index_packet *packets, unsigned int npacks)
{
    printf("STRUCT DUMP >>>>>>>>>\n");
    int p;
    for (p=0; p<npacks ; ++p)
    { 
        printf("+++++++++packet number='%d'\n", p);
        bitpack__print_struct(packets+p);
    }
    printf("END STRUCT DUMP <<<<<<<<\n");
    return 0;
}

int bitpack__print_struct(struct index_packet *packet)
{
    printf("offset=%ld\n", packet->offset);
    
    int i;
    for (i=0; i<48; ++i)
    {
        printf("size(%d)=%d\n", i, packet->msg_sizes[i]);
    }
    return 0;
}
