


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

#include "playback.h"
#include "variable_integer.h"




















void freePlayback(struct Playback* value)
{
    unsigned int i;
    for (i = 0; i < value->count; ++i)
    {
        freePlayback_packet(&value->items[i]);
    }
    free(value->items);
}


int decodePlayback(BitBuffer* buffer, struct Playback* result)
{
    
    

    unsigned int i;
    result->items = 0;
    result->count = 0;
    while (buffer->num_bits > 0)
    {
        i = result->count;
        ++result->count;
        result->items = (struct Playback_packet*)realloc(result->items, sizeof(struct Playback_packet) * (result->count));

        

        if (!decodePlayback_packet(buffer, &result->items[i]))
        {
            unsigned int j;
            for (j=0; j< i; ++j)
            {
                freePlayback_packet(&result->items[j]);
            }
            free(result->items);
            return 0;
        }
    }



    return 1;
}












void printXmlPlayback(struct Playback* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>\n", name);
        
    unsigned int playbackCounter;
    for (playbackCounter = 0; playbackCounter < data->count; ++playbackCounter)
    {
                
        printXmlPlayback_packet(&data->items[playbackCounter], offset + 2, "playback_packet");

    }
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("</%s>\n", name);
}



















