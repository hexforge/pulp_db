#include <stdio.h>
#include "bitpack.h"

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
    printf("Blank data------------------------\n");
    hex_dump(page, PAGE_SIZE);
    
    // Make a strut of data
    unsigned long magic = 12;
    struct index_packet data[NUM_PACKS_IN_PAGE];

    int p;
    for (p=0; p<NUM_PACKS_IN_PAGE; ++p)
    {
        data[p].offset = magic;
        for (i=0; i<NUM_MSG_IN_PACKET; ++i)
        {
            data[p].msg_sizes[i] = i*(p+1);
        }
    }

    printf("Struct to write------------------------\n");
    bitpack__print_structs(data, NUM_PACKS_IN_PAGE);

    printf("Struct packed------------------------\n");
    bitpack__pgwrite(page, data, 1);
    hex_dump(page, PAGE_SIZE);

    printf("Struct unpacked------------------------\n");
    struct index_packet result[NUM_PACKS_IN_PAGE];
    bitpack__pgread(page, result, 1);
    bitpack__print_structs(result, NUM_PACKS_IN_PAGE);
    return 0;
}