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

#include "fopen_ascii.h"
#include "playback.h"

#define MAX_DATA_LEN 20000

struct arg_struct
{
    char *file_path;
};

/*---
FUNCTIONS
---*/
void exit_with_error(char *err_msg);
void exit_with_error(char *err_msg)
{
    printf("%s\n", err_msg);
    exit(1);
}

int setup(char *file_path);
int setup(char *file_path)
{
    return setup_fmap_ascii(file_path);
}

int teardown();
int teardown()
{
    return close_fmap_ascii();
}

struct arg_struct handle_args(int argc, char *argv[]);
struct arg_struct handle_args(int argc, char *argv[])
{
    struct arg_struct arg_data;
    if (argc!=1)
    {
        arg_data.file_path = argv[1];
        printf("Plyabck file location '%s'\n", arg_data.file_path);
    }
    else
    {
        printf("We need one argument the playback path.  We got %d args.\n", argc);
        exit(1);
    }
    
    return arg_data;
}

static int count = 0;
static unsigned char data[MAX_DATA_LEN];
int parse_pb();
int parse_pb()
{
    /* Load the data file into memory */
    extern unsigned char data[];
    extern count;
    int amount_got = 0;
    while (1)
    {
        amount_got = getdata(data, MAX_DATA_LEN);
        if (amount_got==0)
        {
            printf("no data remaining :)\n");
            break;
        }
        //printf("DATA %s\n", data);
        /* Attempt to decode the file */
        BitBuffer buffer = {data, 0, amount_got*8};
        struct Playback result;
        if (!decodePlayback(&buffer, &result))
        {
            /* Decode failed! */
            fprintf(stderr, "Decode failed!\n");
            return 3;
        }
        count += 1;
        //printXmlPlayback(&result, 0, "playback");
        freePlayback(&result);
    }
    printf("Decoded %d messages\n", count);
    return 0;
}


int main(int argc, char* argv[])
{
    struct arg_struct arg_data;
    arg_data = handle_args(argc, argv);
    if (setup(arg_data.file_path)!=0)
        exit_with_error("Fail to setup");

    if (setup(arg_data.file_path)!=0)
        exit_with_error("Fail to setup");

    if (parse_pb()!=0)
        exit_with_error("Fail to parse_pb");

    if (teardown()!=0)
        exit_with_error("Fail to teardown");
}

