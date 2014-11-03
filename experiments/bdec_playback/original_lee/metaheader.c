


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
    
      
      
    if (metaheader_startValue.length != 19 ||
            memcmp(metaheader_startValue.buffer, "\052M\052E\052T\052A\052S\052T\052A\052R\052T\052", 19) != 0)
    {
        freeMetaheader_start(&metaheader_startValue);
        return 0;
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




static void freeSep1(Text* value)
{
    free(value->buffer);
}


static int decodeSep1(BitBuffer* buffer)
{
    
    unsigned int sep1ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep1ExpectedLength;
    if (sep1ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep1ExpectedLength;
    
    
    Text sep1Value;
    unsigned int i;
    sep1Value.length = 8 / 8;
    sep1Value.buffer = (char*)malloc(sep1Value.length + 1);
    sep1Value.buffer[sep1Value.length] = 0;
    for (i = 0; i < sep1Value.length; ++i)
    {
        sep1Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep1Value.length != 1 ||
            memcmp(sep1Value.buffer, "\057", 1) != 0)
    {
        freeSep1(&sep1Value);
        return 0;
    }

    freeSep1(&sep1Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeSep2(Text* value)
{
    free(value->buffer);
}


static int decodeSep2(BitBuffer* buffer)
{
    
    unsigned int sep2ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep2ExpectedLength;
    if (sep2ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep2ExpectedLength;
    
    
    Text sep2Value;
    unsigned int i;
    sep2Value.length = 8 / 8;
    sep2Value.buffer = (char*)malloc(sep2Value.length + 1);
    sep2Value.buffer[sep2Value.length] = 0;
    for (i = 0; i < sep2Value.length; ++i)
    {
        sep2Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep2Value.length != 1 ||
            memcmp(sep2Value.buffer, "\057", 1) != 0)
    {
        freeSep2(&sep2Value);
        return 0;
    }

    freeSep2(&sep2Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
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
    
      
      
    if (sep3Value.length != 1 ||
            memcmp(sep3Value.buffer, "\057", 1) != 0)
    {
        freeSep3(&sep3Value);
        return 0;
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





static void freeDot10(Text* value)
{
    free(value->buffer);
}


static int decodeDot10(BitBuffer* buffer)
{
    
    unsigned int dot1ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot1ExpectedLength;
    if (dot1ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot1ExpectedLength;
    
    
    Text dot1Value;
    unsigned int i;
    dot1Value.length = 8 / 8;
    dot1Value.buffer = (char*)malloc(dot1Value.length + 1);
    dot1Value.buffer[dot1Value.length] = 0;
    for (i = 0; i < dot1Value.length; ++i)
    {
        dot1Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot1Value.length != 1 ||
            memcmp(dot1Value.buffer, "\056", 1) != 0)
    {
        freeDot10(&dot1Value);
        return 0;
    }

    freeDot10(&dot1Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeDot20(Text* value)
{
    free(value->buffer);
}


static int decodeDot20(BitBuffer* buffer)
{
    
    unsigned int dot2ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot2ExpectedLength;
    if (dot2ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot2ExpectedLength;
    
    
    Text dot2Value;
    unsigned int i;
    dot2Value.length = 8 / 8;
    dot2Value.buffer = (char*)malloc(dot2Value.length + 1);
    dot2Value.buffer[dot2Value.length] = 0;
    for (i = 0; i < dot2Value.length; ++i)
    {
        dot2Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot2Value.length != 1 ||
            memcmp(dot2Value.buffer, "\056", 1) != 0)
    {
        freeDot20(&dot2Value);
        return 0;
    }

    freeDot20(&dot2Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeDot30(Text* value)
{
    free(value->buffer);
}


static int decodeDot30(BitBuffer* buffer)
{
    
    unsigned int dot3ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot3ExpectedLength;
    if (dot3ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot3ExpectedLength;
    
    
    Text dot3Value;
    unsigned int i;
    dot3Value.length = 8 / 8;
    dot3Value.buffer = (char*)malloc(dot3Value.length + 1);
    dot3Value.buffer[dot3Value.length] = 0;
    for (i = 0; i < dot3Value.length; ++i)
    {
        dot3Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot3Value.length != 1 ||
            memcmp(dot3Value.buffer, "\056", 1) != 0)
    {
        freeDot30(&dot3Value);
        return 0;
    }

    freeDot30(&dot3Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}



static void freePlayback_ip(struct Playback_ip* value)
{
    freeDigits(&value->groupa);
    freeDigits(&value->groupb);
    freeDigits(&value->groupc);
    freeDigits(&value->groupd);
}


static int decodePlayback_ip(BitBuffer* buffer, struct Playback_ip* result)
{
    long long unusedDigits;
    
    if (!decodeDigits(buffer, &result->groupa, &unusedDigits))
    {
        return 0;
    }
    if (!decodeDot10(buffer))
    {
        freeDigits(&result->groupa);
        return 0;
    }
    if (!decodeDigits(buffer, &result->groupb, &unusedDigits))
    {
        freeDigits(&result->groupa);
        return 0;
    }
    if (!decodeDot20(buffer))
    {
        freeDigits(&result->groupa);
        freeDigits(&result->groupb);
        return 0;
    }
    if (!decodeDigits(buffer, &result->groupc, &unusedDigits))
    {
        freeDigits(&result->groupa);
        freeDigits(&result->groupb);
        return 0;
    }
    if (!decodeDot30(buffer))
    {
        freeDigits(&result->groupa);
        freeDigits(&result->groupb);
        freeDigits(&result->groupc);
        return 0;
    }
    if (!decodeDigits(buffer, &result->groupd, &unusedDigits))
    {
        freeDigits(&result->groupa);
        freeDigits(&result->groupb);
        freeDigits(&result->groupc);
        return 0;
    }

    return 1;
}




static void freeSep4(Text* value)
{
    free(value->buffer);
}


static int decodeSep4(BitBuffer* buffer)
{
    
    unsigned int sep4ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep4ExpectedLength;
    if (sep4ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep4ExpectedLength;
    
    
    Text sep4Value;
    unsigned int i;
    sep4Value.length = 8 / 8;
    sep4Value.buffer = (char*)malloc(sep4Value.length + 1);
    sep4Value.buffer[sep4Value.length] = 0;
    for (i = 0; i < sep4Value.length; ++i)
    {
        sep4Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep4Value.length != 1 ||
            memcmp(sep4Value.buffer, "\057", 1) != 0)
    {
        freeSep4(&sep4Value);
        return 0;
    }

    freeSep4(&sep4Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeSep5(Text* value)
{
    free(value->buffer);
}


static int decodeSep5(BitBuffer* buffer)
{
    
    unsigned int sep5ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep5ExpectedLength;
    if (sep5ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep5ExpectedLength;
    
    
    Text sep5Value;
    unsigned int i;
    sep5Value.length = 8 / 8;
    sep5Value.buffer = (char*)malloc(sep5Value.length + 1);
    sep5Value.buffer[sep5Value.length] = 0;
    for (i = 0; i < sep5Value.length; ++i)
    {
        sep5Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep5Value.length != 1 ||
            memcmp(sep5Value.buffer, "\057", 1) != 0)
    {
        freeSep5(&sep5Value);
        return 0;
    }

    freeSep5(&sep5Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}





static void freeDot11(Text* value)
{
    free(value->buffer);
}


static int decodeDot11(BitBuffer* buffer)
{
    
    unsigned int dot1ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot1ExpectedLength;
    if (dot1ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot1ExpectedLength;
    
    
    Text dot1Value;
    unsigned int i;
    dot1Value.length = 8 / 8;
    dot1Value.buffer = (char*)malloc(dot1Value.length + 1);
    dot1Value.buffer[dot1Value.length] = 0;
    for (i = 0; i < dot1Value.length; ++i)
    {
        dot1Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot1Value.length != 1 ||
            memcmp(dot1Value.buffer, "\072", 1) != 0)
    {
        freeDot11(&dot1Value);
        return 0;
    }

    freeDot11(&dot1Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeDot21(Text* value)
{
    free(value->buffer);
}


static int decodeDot21(BitBuffer* buffer)
{
    
    unsigned int dot2ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot2ExpectedLength;
    if (dot2ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot2ExpectedLength;
    
    
    Text dot2Value;
    unsigned int i;
    dot2Value.length = 8 / 8;
    dot2Value.buffer = (char*)malloc(dot2Value.length + 1);
    dot2Value.buffer[dot2Value.length] = 0;
    for (i = 0; i < dot2Value.length; ++i)
    {
        dot2Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot2Value.length != 1 ||
            memcmp(dot2Value.buffer, "\072", 1) != 0)
    {
        freeDot21(&dot2Value);
        return 0;
    }

    freeDot21(&dot2Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeDot31(Text* value)
{
    free(value->buffer);
}


static int decodeDot31(BitBuffer* buffer)
{
    
    unsigned int dot3ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot3ExpectedLength;
    if (dot3ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot3ExpectedLength;
    
    
    Text dot3Value;
    unsigned int i;
    dot3Value.length = 8 / 8;
    dot3Value.buffer = (char*)malloc(dot3Value.length + 1);
    dot3Value.buffer[dot3Value.length] = 0;
    for (i = 0; i < dot3Value.length; ++i)
    {
        dot3Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot3Value.length != 1 ||
            memcmp(dot3Value.buffer, "\056", 1) != 0)
    {
        freeDot31(&dot3Value);
        return 0;
    }

    freeDot31(&dot3Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}



static void freePlayback_time(struct Playback_time* value)
{
    freeTwoDigits(&value->hh);
    freeTwoDigits(&value->mm);
    freeTwoDigits(&value->ss);
    freeThreeDigits(&value->ms);
}


static int decodePlayback_time(BitBuffer* buffer, struct Playback_time* result)
{
    int unusedThreeDigits;
    int unusedTwoDigits;
    
    if (!decodeTwoDigits(buffer, &result->hh, &unusedTwoDigits))
    {
        return 0;
    }
    if (!decodeDot11(buffer))
    {
        freeTwoDigits(&result->hh);
        return 0;
    }
    if (!decodeTwoDigits(buffer, &result->mm, &unusedTwoDigits))
    {
        freeTwoDigits(&result->hh);
        return 0;
    }
    if (!decodeDot21(buffer))
    {
        freeTwoDigits(&result->hh);
        freeTwoDigits(&result->mm);
        return 0;
    }
    if (!decodeTwoDigits(buffer, &result->ss, &unusedTwoDigits))
    {
        freeTwoDigits(&result->hh);
        freeTwoDigits(&result->mm);
        return 0;
    }
    if (!decodeDot31(buffer))
    {
        freeTwoDigits(&result->hh);
        freeTwoDigits(&result->mm);
        freeTwoDigits(&result->ss);
        return 0;
    }
    if (!decodeThreeDigits(buffer, &result->ms, &unusedThreeDigits))
    {
        freeTwoDigits(&result->hh);
        freeTwoDigits(&result->mm);
        freeTwoDigits(&result->ss);
        return 0;
    }

    return 1;
}




static void freeSep6(Text* value)
{
    free(value->buffer);
}


static int decodeSep6(BitBuffer* buffer)
{
    
    unsigned int sep6ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep6ExpectedLength;
    if (sep6ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep6ExpectedLength;
    
    
    Text sep6Value;
    unsigned int i;
    sep6Value.length = 8 / 8;
    sep6Value.buffer = (char*)malloc(sep6Value.length + 1);
    sep6Value.buffer[sep6Value.length] = 0;
    for (i = 0; i < sep6Value.length; ++i)
    {
        sep6Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep6Value.length != 1 ||
            memcmp(sep6Value.buffer, "\057", 1) != 0)
    {
        freeSep6(&sep6Value);
        return 0;
    }

    freeSep6(&sep6Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}





static void freeDot12(Text* value)
{
    free(value->buffer);
}


static int decodeDot12(BitBuffer* buffer)
{
    
    unsigned int dot1ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - dot1ExpectedLength;
    if (dot1ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = dot1ExpectedLength;
    
    
    Text dot1Value;
    unsigned int i;
    dot1Value.length = 8 / 8;
    dot1Value.buffer = (char*)malloc(dot1Value.length + 1);
    dot1Value.buffer[dot1Value.length] = 0;
    for (i = 0; i < dot1Value.length; ++i)
    {
        dot1Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (dot1Value.length != 1 ||
            memcmp(dot1Value.buffer, "\056", 1) != 0)
    {
        freeDot12(&dot1Value);
        return 0;
    }

    freeDot12(&dot1Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}



static void freeTimestamp(struct Timestamp* value)
{
    freeDigits(&value->unix_time);
    freeDigits(&value->ns);
}


static int decodeTimestamp(BitBuffer* buffer, struct Timestamp* result)
{
    long long unusedDigits;
    
    if (!decodeDigits(buffer, &result->unix_time, &unusedDigits))
    {
        return 0;
    }
    if (!decodeDot12(buffer))
    {
        freeDigits(&result->unix_time);
        return 0;
    }
    if (!decodeDigits(buffer, &result->ns, &unusedDigits))
    {
        freeDigits(&result->unix_time);
        return 0;
    }

    return 1;
}




static void freeSep7(Text* value)
{
    free(value->buffer);
}


static int decodeSep7(BitBuffer* buffer)
{
    
    unsigned int sep7ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep7ExpectedLength;
    if (sep7ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep7ExpectedLength;
    
    
    Text sep7Value;
    unsigned int i;
    sep7Value.length = 8 / 8;
    sep7Value.buffer = (char*)malloc(sep7Value.length + 1);
    sep7Value.buffer[sep7Value.length] = 0;
    for (i = 0; i < sep7Value.length; ++i)
    {
        sep7Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep7Value.length != 1 ||
            memcmp(sep7Value.buffer, "\057", 1) != 0)
    {
        freeSep7(&sep7Value);
        return 0;
    }

    freeSep7(&sep7Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
        return 0;
    }
    buffer->num_bits = unusedNumberOfBits;
    return 1;
}




static void freeSep8(Text* value)
{
    free(value->buffer);
}


static int decodeSep8(BitBuffer* buffer)
{
    
    unsigned int sep8ExpectedLength = 8;
    unsigned int unusedNumberOfBits = buffer->num_bits - sep8ExpectedLength;
    if (sep8ExpectedLength > buffer->num_bits)
    {
        /* Not enough data */
        return 0;
    }
    buffer->num_bits = sep8ExpectedLength;
    
    
    Text sep8Value;
    unsigned int i;
    sep8Value.length = 8 / 8;
    sep8Value.buffer = (char*)malloc(sep8Value.length + 1);
    sep8Value.buffer[sep8Value.length] = 0;
    for (i = 0; i < sep8Value.length; ++i)
    {
        sep8Value.buffer[i] = decode_integer(buffer, 8);
    }
    
      
      
    if (sep8Value.length != 1 ||
            memcmp(sep8Value.buffer, "\057", 1) != 0)
    {
        freeSep8(&sep8Value);
        return 0;
    }

    freeSep8(&sep8Value);

    if (buffer->num_bits != 0)
    {
        /* The entry didn't use all of the data */
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
    
      
      
    if (metaheader_endValue.length != 15 ||
            memcmp(metaheader_endValue.buffer, "\052M\052E\052T\052A\052E\052N\052D\052", 15) != 0)
    {
        freeMetaheader_end(&metaheader_endValue);
        return 0;
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
    freeDigits(&value->metaheader_version);
    freeDigits(&value->payload_length);
    freePlayback_ip(&value->playback_ip);
    freeDigits(&value->port);
    freePlayback_time(&value->playback_time);
    freeTimestamp(&value->timestamp);
}


int decodeMetaheader(BitBuffer* buffer, struct Metaheader* result, long long* payload_length)
{
    long long unusedDigits;
    
    if (!decodeMetaheader_start(buffer))
    {
        return 0;
    }
    if (!decodeSep1(buffer))
    {
        return 0;
    }
    if (!decodeDigits(buffer, &result->metaheader_version, &unusedDigits))
    {
        return 0;
    }
    if (!decodeSep2(buffer))
    {
        freeDigits(&result->metaheader_version);
        return 0;
    }
    if (!decodeDigits(buffer, &result->payload_length, payload_length))
    {
        freeDigits(&result->metaheader_version);
        return 0;
    }
    if (!decodeSep3(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        return 0;
    }
    if (!decodePlayback_ip(buffer, &result->playback_ip))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        return 0;
    }
    if (!decodeSep4(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        return 0;
    }
    if (!decodeDigits(buffer, &result->port, &unusedDigits))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        return 0;
    }
    if (!decodeSep5(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        return 0;
    }
    if (!decodePlayback_time(buffer, &result->playback_time))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        return 0;
    }
    if (!decodeSep6(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        freePlayback_time(&result->playback_time);
        return 0;
    }
    if (!decodeTimestamp(buffer, &result->timestamp))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        freePlayback_time(&result->playback_time);
        return 0;
    }
    if (!decodeSep7(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        freePlayback_time(&result->playback_time);
        freeTimestamp(&result->timestamp);
        return 0;
    }
    if (!decodeSep8(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        freePlayback_time(&result->playback_time);
        freeTimestamp(&result->timestamp);
        return 0;
    }
    if (!decodeMetaheader_end(buffer))
    {
        freeDigits(&result->metaheader_version);
        freeDigits(&result->payload_length);
        freePlayback_ip(&result->playback_ip);
        freeDigits(&result->port);
        freePlayback_time(&result->playback_time);
        freeTimestamp(&result->timestamp);
        return 0;
    }

    return 1;
}













static void printXmlPlayback_ip(struct Playback_ip* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    printXmlDigits(&data->groupa, offset + 2, "groupA");

        
    printXmlDigits(&data->groupb, offset + 2, "groupB");

        
    printXmlDigits(&data->groupc, offset + 2, "groupC");

        
    printXmlDigits(&data->groupd, offset + 2, "groupD");

    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}




static void printXmlPlayback_time(struct Playback_time* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    printXmlTwoDigits(&data->hh, offset + 2, "hh");

        
    printXmlTwoDigits(&data->mm, offset + 2, "mm");

        
    printXmlTwoDigits(&data->ss, offset + 2, "ss");

        
    printXmlThreeDigits(&data->ms, offset + 2, "ms");

    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}




static void printXmlTimestamp(struct Timestamp* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    printXmlDigits(&data->unix_time, offset + 2, "unix_time");

        
    printXmlDigits(&data->ns, offset + 2, "ns");

    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}



void printXmlMetaheader(struct Metaheader* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    printXmlDigits(&data->metaheader_version, offset + 2, "metaheader_version");

        
    printXmlDigits(&data->payload_length, offset + 2, "payload_length");

        
    printXmlPlayback_ip(&data->playback_ip, offset + 2, "playback_ip");

        
    printXmlDigits(&data->port, offset + 2, "port");

        
    printXmlPlayback_time(&data->playback_time, offset + 2, "playback_time");

        
    printXmlTimestamp(&data->timestamp, offset + 2, "timestamp");

    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}



















