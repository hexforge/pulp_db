#include <stdio.h>
#include "hex.h"

int hex__dump(char *data, unsigned int n)
{
    printf("HEX DUMP >>>>>>>>>\n");
    printf("     :  0                          9                            19\n");
    printf(".................................................................................");
    unsigned int i;
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
