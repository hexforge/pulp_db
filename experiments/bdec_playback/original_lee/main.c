/*  Copyright (C) 2010 Henry Ludemann

    This file is part of the bdec decoder library.

    The bdec decoder library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    The bdec decoder library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, see
    <http://www.gnu.org/licenses/>. */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "playback.h"

static void usage(char* program)
{
    printf("Usage: %s [options] <filename>\n", program);
    printf("Decode the playback file to xml.\n");
    printf("Options:\n");
    printf("   -h    Display this help.\n");
    printf("   -q    Quiet output (don't print xml).\n");
}

int main(int argc, char* argv[])
{
    int i;
    int should_print_xml = 1;
    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == 'h')
            {
                usage(argv[0]);
                return 0;
            }
            else if (argv[i][1] == 'q')
            {
                should_print_xml = 0;
            }
            else
            {
                fprintf(stderr, "Unknown option '%s'! See %s -h for more details.\n",
                        argv[i], argv[0]);
                return 1;
            }
        }
        else
        {
            // We've found the first argument
            break;
        }
    }

    if (i != argc - 1)
    {
        /* Bad number of arguments */
        fprintf(stderr, "Missing single filename to decode!\nRun %s -h for more details.\n", argv[0]);
        usage(argv[0]);
        return 1;
    }
    char* filename = argv[i];
    FILE* datafile = fopen(filename, "rb");
    if (datafile == 0)
    {
        /* Failed to open file */
        fprintf(stderr, "Failed to open file '%s'!\n", filename);
        return 2;
    }
    fseek(datafile, 0, SEEK_END);
    long int length = ftell(datafile);
    fseek(datafile, 0, SEEK_SET);

    /* Load the data file into memory */
    unsigned char* data = (unsigned char*)malloc(length);
    if (fread(data, length, 1, datafile) != 1 && length != 0)
    {
        fprintf(stderr, "Failed to read from file '%s'!\n", filename);
        free(data);
        fclose(datafile);
        return 2;
    }
    fclose(datafile);

    /* Attempt to decode the file */
    BitBuffer buffer = {data, 0, length * 8};
    struct Playback result;
    if (!decodePlayback(&buffer, &result))
    {
        /* Decode failed! */
        fprintf(stderr, "Decode failed!\n");
        free(data);
        return 3;
    }


    /* Print the decoded data */
    if (should_print_xml)
    {
        printXmlPlayback(&result, 0, "playback");
    }

    freePlayback(&result);
    free(data);
    return 0;
}

