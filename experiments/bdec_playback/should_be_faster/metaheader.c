


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

#include "metaheader.h"
#include "variable_integer.h"





















static void freeMetaheader_start(Text* value)
{
    free(value->buffer);
}


static int decodeMetaheader_start(BitBuffer* buffer)
{
    
    unsigned int metaheader_startExpectedLength = (19 * 8);
    unsigned int unusedNumberOfBits = buffer->num_bits - metaheader_startExpectedLength;
    if (metaheader_startExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = metaheader_startExpectedLength;
    
    
    Text metaheader_startValue;
    unsigned int i;
    metaheader_startValue.length = (19 * 8) / 8;
    metaheader_startValue.buffer = (char*)malloc(metaheader_startValue.length + 1);
    metaheader_startValue.buffer[metaheader_startValue.length] = 0;
    for (i = 0; i < metaheader_startValue.length; ++i)
    {
        metaheader_startValue.buffer[i] = decode_integer(buffer, 8);
    }
    

    freeMetaheader_start(&metaheader_startValue);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeIrrelevent0(Text* value)
{
    free(value->buffer);
}


static int decodeIrrelevent0(BitBuffer* buffer, Text* result)
{
    
    unsigned int irreleventExpectedLength = 24;
    unsigned int unusedNumberOfBits = buffer->num_bits - irreleventExpectedLength;
    if (irreleventExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = irreleventExpectedLength;
    
    
    Text irreleventValue;
    unsigned int i;
    irreleventValue.length = 24 / 8;
    irreleventValue.buffer = (char*)malloc(irreleventValue.length + 1);
    irreleventValue.buffer[irreleventValue.length] = 0;
    for (i = 0; i < irreleventValue.length; ++i)
    {
        irreleventValue.buffer[i] = decode_integer(buffer, 8);
    }
    

    (*result) = irreleventValue;

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        freeIrrelevent0(result);
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeSep3(Text* value)
{
    free(value->buffer);
}


static int decodeSep3(BitBuffer* buffer)
{
    
    unsigned int sep3ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep3ExpectedLength;
    if (sep3ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep3ExpectedLength;
    
    
    Text sep3Value;
    unsigned int i;
    sep3Value.length = 8 / 8;
    sep3Value.buffer = (char*)malloc(sep3Value.length + 1);
    sep3Value.buffer[sep3Value.length] = 0;
    for (i = 0; i < sep3Value.length; ++i)
    {
        sep3Value.buffer[i] = decode_integer(buffer, 8);
    }
    

    freeSep3(&sep3Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeIrrelevent1(Text* value)
{
    free(value->buffer);
}


static int decodeIrrelevent1(BitBuffer* buffer, Text* result)
{
    
    unsigned int irreleventExpectedLength = 112;
    unsigned int unusedNumberOfBits = buffer->num_bits - irreleventExpectedLength;
    if (irreleventExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = irreleventExpectedLength;
    
    
    Text irreleventValue;
    unsigned int i;
    irreleventValue.length = 112 / 8;
    irreleventValue.buffer = (char*)malloc(irreleventValue.length + 1);
    irreleventValue.buffer[irreleventValue.length] = 0;
    for (i = 0; i < irreleventValue.length; ++i)
    {
        irreleventValue.buffer[i] = decode_integer(buffer, 8);
    }
    

    (*result) = irreleventValue;

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        freeIrrelevent1(result);
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeIrrelevent2(Text* value)
{
    free(value->buffer);
}


static int decodeIrrelevent2(BitBuffer* buffer, Text* result)
{
    
    unsigned int irreleventExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - irreleventExpectedLength;
    if (irreleventExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = irreleventExpectedLength;
    
    
    Text irreleventValue;
    unsigned int i;
    irreleventValue.length = 8 / 8;
    irreleventValue.buffer = (char*)malloc(irreleventValue.length + 1);
    irreleventValue.buffer[irreleventValue.length] = 0;
    for (i = 0; i < irreleventValue.length; ++i)
    {
        irreleventValue.buffer[i] = decode_integer(buffer, 8);
    }
    

    (*result) = irreleventValue;

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        freeIrrelevent2(result);
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeMetaheader_end(Text* value)
{
    free(value->buffer);
}


static int decodeMetaheader_end(BitBuffer* buffer)
{
    
    unsigned int metaheader_endExpectedLength = (15 * 8);
    unsigned int unusedNumberOfBits = buffer->num_bits - metaheader_endExpectedLength;
    if (metaheader_endExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = metaheader_endExpectedLength;
    
    
    Text metaheader_endValue;
    unsigned int i;
    metaheader_endValue.length = (15 * 8) / 8;
    metaheader_endValue.buffer = (char*)malloc(metaheader_endValue.length + 1);
    metaheader_endValue.buffer[metaheader_endValue.length] = 0;
    for (i = 0; i < metaheader_endValue.length; ++i)
    {
        metaheader_endValue.buffer[i] = decode_integer(buffer, 8);
    }
    

    freeMetaheader_end(&metaheader_endValue);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}



void freeMetaheader(struct Metaheader* value)
{
    freeIrrelevent0(&value->irrelevent0);
    freeDigits(&value->payload_length);
    freeSlashstring(&value->playback_ip);
    freeSlashstring(&value->port);
    freeIrrelevent1(&value->irrelevent1);
    freeSlashstring(&value->timestamp);
    freeIrrelevent2(&value->irrelevent2);
}


int decodeMetaheader(BitBuffer* buffer, struct Metaheader* result, long long* payload_length)
{
    
    if (!decodeMetaheader_start(buffer))
    {
        return 0;
    }
    if (!decodeIrrelevent0(buffer, &result->irrelevent0))
    {
        return 0;
    }
    if (!decodeDigits(buffer, &result->payload_length, payload_length))
    {
        freeIrrelevent0(&result->irrelevent0);
        return 0;
    }
    if (!decodeSep3(buffer))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        return 0;
    }
    if (!decodeSlashstring(buffer, &result->playback_ip))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        return 0;
    }
    if (!decodeSlashstring(buffer, &result->port))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        freeSlashstring(&result->playback_ip);
        return 0;
    }
    if (!decodeIrrelevent1(buffer, &result->irrelevent1))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        freeSlashstring(&result->playback_ip);
        freeSlashstring(&result->port);
        return 0;
    }
    if (!decodeSlashstring(buffer, &result->timestamp))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        freeSlashstring(&result->playback_ip);
        freeSlashstring(&result->port);
        freeIrrelevent1(&result->irrelevent1);
        return 0;
    }
    if (!decodeIrrelevent2(buffer, &result->irrelevent2))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        freeSlashstring(&result->playback_ip);
        freeSlashstring(&result->port);
        freeIrrelevent1(&result->irrelevent1);
        freeSlashstring(&result->timestamp);
        return 0;
    }
    if (!decodeMetaheader_end(buffer))
    {
        freeIrrelevent0(&result->irrelevent0);
        freeDigits(&result->payload_length);
        freeSlashstring(&result->playback_ip);
        freeSlashstring(&result->port);
        freeIrrelevent1(&result->irrelevent1);
        freeSlashstring(&result->timestamp);
        freeIrrelevent2(&result->irrelevent2);
        return 0;
    }

    return 1;
}













static void printXmlIrrelevent0(Text* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>", name);
    print_escaped_string(data);
    printf("</%s>\n", name);
}




static void printXmlIrrelevent1(Text* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>", name);
    print_escaped_string(data);
    printf("</%s>\n", name);
}




static void printXmlIrrelevent2(Text* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>", name);
    print_escaped_string(data);
    printf("</%s>\n", name);
}



void printXmlMetaheader(struct Metaheader* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    printXmlIrrelevent0(&data->irrelevent0, offset + 2, "irrelevent");

        
    printXmlDigits(&data->payload_length, offset + 2, "payload_length");

        
    printXmlSlashstring(&data->playback_ip, offset + 2, "playback_ip");

        
    printXmlSlashstring(&data->port, offset + 2, "port");

        
    printXmlIrrelevent1(&data->irrelevent1, offset + 2, "irrelevent");

        
    printXmlSlashstring(&data->timestamp, offset + 2, "timestamp");

        
    printXmlIrrelevent2(&data->irrelevent2, offset + 2, "irrelevent");

    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}



















