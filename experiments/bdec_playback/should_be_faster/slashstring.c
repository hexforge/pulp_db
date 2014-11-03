


/*  Copyright (C) 2008-2010 Henry Ludemann
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
    <http://www.gnu.org/licenses/>.
  
 This file incorporates work covered by the following copyright and  
 permission notice:  
  
    Copyright (c) 2010, PRESENSE Technologies GmbH
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of the PRESENSE Technologies GmbH nor the
          names of its contributors may be used to endorse or promote products
          derived from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL PRESENSE Technologies GmbH BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "slashstring.h"
#include "variable_integer.h"






















static void freeNull(unsigned char* value)
{

}


static int decodeNull(BitBuffer* buffer, int* shouldEnd)
{
    
    unsigned int nullExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - nullExpectedLength;
    if (nullExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = nullExpectedLength;
    
    
    unsigned char nullValue;
      
      
    
    nullValue = decode_integer(buffer, 8);
    
    if (nullValue != 47)
    {
        freeNull(&nullValue);
        return 0;
    }

    freeNull(&nullValue);

    *shouldEnd = 1;
    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeC(Text* value)
{
    free(value->buffer);
}


static int decodeC(BitBuffer* buffer, Text* result)
{
    
    unsigned int cExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - cExpectedLength;
    if (cExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = cExpectedLength;
    
    
    Text cValue;
    unsigned int i;
    cValue.length = 8 / 8;
    cValue.buffer = (char*)malloc(cValue.length + 1);
    cValue.buffer[cValue.length] = 0;
    for (i = 0; i < cValue.length; ++i)
    {
        cValue.buffer[i] = decode_integer(buffer, 8);
    }
    

    (*result) = cValue;

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        freeC(result);
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}



static void freeEntry(struct Entry* value)
{
    switch(value->option)
    {
    case NULL_:
        break;
    case C:
        
          
        freeC(&value->value.c);
        break;
    }
}


static int decodeEntry(BitBuffer* buffer, struct Entry* result, int* shouldEnd)
{
    
    memset(result, 0, sizeof(struct Entry));
    BitBuffer temp;
    
    
    
    if (temp = *buffer, decodeNull(&temp, shouldEnd))
    {
      

        result->option = NULL_;
        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeC(&temp, &result->value.c))
    {
      

        result->option = C;
        *buffer = temp;
    }
    else
    {
        /* Decode failed, no options succeeded... */
        return 0;
    }

    return 1;
}



void freeSlashstring(struct Slashstring* value)
{
    unsigned int i;
    for (i = 0; i < value->count; ++i)
    {
        freeEntry(&value->items[i]);
    }
    free(value->items);
}


int decodeSlashstring(BitBuffer* buffer, struct Slashstring* result)
{
    int shouldEnd;
    
    
    shouldEnd = 0;

    unsigned int i;
    result->items = 0;
    result->count = 0;
    while (!shouldEnd)
    {
        i = result->count;
        ++result->count;
        result->items = (struct Entry*)realloc(result->items, sizeof(struct Entry) * (result->count));

        

        if (shouldEnd || !decodeEntry(buffer, &result->items[i], &shouldEnd))
        {
            unsigned int j;
            for (j=0; j< i; ++j)
            {
                freeEntry(&result->items[j]);
            }
            free(result->items);
            return 0;
        }
    }

    if (!shouldEnd)
    {
        // The data finished without receiving an 'end sequence'!
        freeSlashstring(result);
        return 0;
    }


    return 1;
}














static void printXmlNull(unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s />\n", name);
}




static void printXmlC(Text* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>", name);
    print_escaped_string(data);
    printf("</%s>\n", name);
}



static void printXmlEntry(struct Entry* data, unsigned int offset, const char* name)
{
    switch(data->option)
    {
    case NULL_:
          
            
                
        printXmlNull(offset + 0, "null");

        break;
    case C:
          
            
                
        printXmlC(&data->value.c, offset + 0, "c");

        break;
    }
}



void printXmlSlashstring(struct Slashstring* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    unsigned int slashstringCounter;
    for (slashstringCounter = 0; slashstringCounter < data->count; ++slashstringCounter)
    {
                
        printXmlEntry(&data->items[slashstringCounter], offset + 2, "entry");

    }
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}



















