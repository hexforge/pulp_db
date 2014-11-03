


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

#include "playback_packet.h"
#include "variable_integer.h"





















static void freePayload(BitBuffer* value)
{
    free(value->buffer);
}


static int decodePayload(BitBuffer* buffer, BitBuffer* result, long long metaheaderpayload_length)
{
    
    unsigned int payloadExpectedLength = (metaheaderpayload_length * 8);
    unsigned int unusedNumberOfBits = buffer->num_bits - payloadExpectedLength;
    if (payloadExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = payloadExpectedLength;
    
    
    BitBuffer payloadValue;
    payloadValue.start_bit = buffer->start_bit;
    payloadValue.num_bits = (metaheaderpayload_length * 8);
    unsigned int numBytes = (buffer->start_bit + buffer->num_bits + 7) / 8;
    payloadValue.buffer = (unsigned char*)malloc(numBytes);
    memcpy(payloadValue.buffer, buffer->buffer, numBytes);
    buffer->start_bit += payloadValue.num_bits;
    buffer->buffer += buffer->start_bit / 8;
    buffer->start_bit %= 8;
    buffer->num_bits -= payloadValue.num_bits;

    

    (*result) = payloadValue;

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        freePayload(result);
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}






static int decode_hidden(BitBuffer* buffer)
{
    
    unsigned int expectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - expectedLength;
    if (expectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = expectedLength;
    
    
    int value;
      
      
    
    value = decode_integer(buffer, 8);
    
    if (value != 10)
    {
        return 0;
    }


    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}



void freePlayback_packet(struct Playback_packet* value)
{
    freeMetaheader(&value->metaheader);
    freePayload(&value->payload);
}


int decodePlayback_packet(BitBuffer* buffer, struct Playback_packet* result)
{
    long long metaheaderpayload_length;
    
    if (!decodeMetaheader(buffer, &result->metaheader, &metaheaderpayload_length))
    {
        return 0;
    }
    if (!decodePayload(buffer, &result->payload, metaheaderpayload_length))
    {
        freeMetaheader(&result->metaheader);
        return 0;
    }
    if (!decode_hidden(buffer))
    {
        freeMetaheader(&result->metaheader);
        freePayload(&result->payload);
        return 0;
    }

    return 1;
}













static void printXmlPayload(BitBuffer* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>", name);
        
        
    BitBuffer copyOfPayload = *data;
    unsigned int payloadWhitespaceCounter = copyOfPayload.num_bits > 8 ? copyOfPayload.num_bits % 8 : 8;
    for (; copyOfPayload.num_bits != 0; --payloadWhitespaceCounter)
    {
        if (payloadWhitespaceCounter == 0)
        {
            putchar(' ');
            payloadWhitespaceCounter = 8;
        }
        printf("%u", decode_integer(&copyOfPayload, 1));
    }
    printf("</%s>\n", name);
}



void printXmlPlayback_packet(struct Playback_packet* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    printXmlMetaheader(&data->metaheader, offset + 2, "metaheader");

        
    printXmlPayload(&data->payload, offset + 2, "payload");

    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}



















