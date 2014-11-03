#include <stdio.h>
#include "blockpack.h"

#define HEX_LINE 20
int hex_dump(unsigned char *data, unsigned int n);
int hex_dump(unsigned char *data, unsigned int n)
{
    printf("HEX DUMP >>>>>>>>>\n");
    printf("     :  0                          9                            19\n");
    printf(".................................................................................");
    int i;
    for (i=0; i<n; ++i)
    {   
        if ((i)%HEX_LINE==0)
        {
            printf("\n%05d: ", (i/HEX_LINE)*HEX_LINE);
        }
        printf("%2x ", (int)data[i]);
    }
    printf("\nEND HEX<<<<<<<<\n");
    return 0;
}

int main()
{
    // Make a page
    unsigned char page[PAGE_SIZE];
    int i;
    for (i=0; i<PAGE_SIZE; ++i)
    {
        page[i] = 0;
    }

    // Make a strut of data
    unsigned long magic = 12;
    struct index_packet data[NUM_PACKS_IN_PAGE];

    int p;
    for (p=0; p<NUM_PACKS_IN_PAGE; ++p)
    {
        data[p].offset = magic;
        data[p].msg_sizes[0] = p+0;
        data[p].msg_sizes[1] = p+1;
        data[p].msg_sizes[2] = p+2;
        data[p].msg_sizes[3] = p+3;
    }

    printf("Blank page ------------------------\n");
    hex_dump(page, PAGE_SIZE);

    printf("Struct to write------------------------\n");
    blockpack__print_structs(data, NUM_PACKS_IN_PAGE);

    printf("Struct packed------------------------\n");
    blockpack__pgwrite(page, data, 1);
    hex_dump(page, PAGE_SIZE);

    printf("Struct unpacked------------------------\n");
    struct index_packet result[NUM_PACKS_IN_PAGE];
    blockpack__pgread(page, result, 1);
    blockpack__print_structs(result, NUM_PACKS_IN_PAGE);
    return 0;
}