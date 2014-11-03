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
#include "hex.h"
/*---
DATA
---*/


/*---
FUNCTIONS
---*/
static int decode_big_endian(char *source, struct bitpack__clump *target);
int decode_big_endian(char *source, struct bitpack__clump *target)
{
    printf("Not implemented");
    return -1;
}

static int encode_big_endian(char *target, struct bitpack__clump *source);
int encode_big_endian(char *target, struct bitpack__clump *source)
{
    printf("Not implemented");
    return -1;
}

static unsigned int size;
static long long offset;
static int decode_little_endian(char *source, struct bitpack__clump *target);
int decode_little_endian(char *source, struct bitpack__clump *target)
{
    //hex__dump(source, 128);
    extern long long offset;
    offset = (*(long long*)source);
    source += 8;
    int i;
    for (i=0; i<24; ++i)
    {
        size = (*(unsigned int*)source)&0x0FFFFF;
        if (size==0)
        {
            return (i*2);
        }

        target->offsets[i*2] = offset;
        target->msg_sizes[i*2] = size;
        offset += size;
        
        size = ((*(unsigned int*)(source+2))&0x0FFFFF0)>>4;
        if (size==0)
        {
            return (i*2+1);
        }

        target->offsets[i*2+1] = offset;
        target->msg_sizes[i*2+1] = size;
        offset += size;

        source += 5;
    }
    return 24*2;
}

static int encode_little_endian(char *target, struct bitpack__clump *source);
int encode_little_endian(char *target, struct bitpack__clump *source)
{
    (*(long long*)target) = ((source->offsets[0])&0x0FFFFFFFFFF);   //5 bytes
    
    target += 8;

    //printf("start ints %p\n", target);
    //printf("second int %p\n", target+1);
    //Not take 24 pairs of ints and squeeze them into 5bytes.
    int i;
    for (i=0; i<24; ++i)
    {
        //*(unsigned int*)target = source.msg_sizes[i*2]&0x0FFFFF;
        //target[2] = (char) (source.msg_sizes[i*2+1]&017)<<4; 
        //target[3] = (char) (source.msg_sizes[i*2+1]>>4)&0377;
        //target[4] = (char) (source.msg_sizes[i*2+1]>>12)&0377;

        target[0] = (char) source->msg_sizes[i*2]&0377;
        target[1] = (char) (source->msg_sizes[i*2]>>8)&0377;
        target[2] = (char) ((source->msg_sizes[i*2]>>16)&017) + ((source->msg_sizes[i*2+1]&017)<<4); 
        target[3] = (char) (source->msg_sizes[i*2+1]>>4)&0377;
        target[4] = (char) (source->msg_sizes[i*2+1]>>12)&0377;
        
        //printf("i*2   %d %d %d \n", source->msg_sizes[i*2], source->msg_sizes[i*2]>>8, source->msg_sizes[i*2]>>16);
        //printf("i*2+1 %d %d %d \n", source->msg_sizes[i*2+1], source->msg_sizes[i*2+1]>>4, source->msg_sizes[i*2+1]>>12);
        //printf("%d %d %d %d %d\n", target[0], target[1], target[2], target[3], target[4]);
        target += 5;
    }
    //printf("Thingy %d\n", page-page_old);
    return 0;
}

int bitpack__print_struct(struct bitpack__clump *clump)
{
    int i;
    for (i=0; i<48; ++i)
    {   
        printf("offset(%d)=%lld\n", i, clump->offsets[i]);
        printf("size(%d)=%d\n", i, clump->msg_sizes[i]);
    }
    return 0;
}

int bitpack__decode(char *source, struct bitpack__clump *target, int little_endian)
{
    if (little_endian)
        return decode_little_endian(source, target);
    else
        return decode_big_endian(source, target);
}

int bitpack__encode(char *target, struct bitpack__clump *source, int little_endian)
{
    if (little_endian)
        return encode_little_endian(target, source);
    else
        return encode_big_endian(target, source);
}


