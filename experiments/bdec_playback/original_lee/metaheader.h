

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
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
    */

#ifndef METAHEADERHEADER_GUARD
#define METAHEADERHEADER_GUARD

#include "buffer.h"
#include "digits.h"
#include "threedigits.h"
#include "twodigits.h"

#ifdef __cplusplus
extern "C" {
#endif




struct Playback_ip
{
    long long groupa;
    long long groupb;
    long long groupc;
    long long groupd;
};


struct Playback_time
{
    int hh;
    int mm;
    int ss;
    int ms;
};


struct Timestamp
{
    long long unix_time;
    long long ns;
};


struct Metaheader
{
    long long metaheader_version;
    long long payload_length;
    struct Playback_ip playback_ip;
    long long port;
    struct Playback_time playback_time;
    struct Timestamp timestamp;
};



/**
 * Decode a metaheader instance.
 *
 *   buffer -- The data to decoded.
 *   result -- The decoded structured is stored in this argument. If the data
 *      has decoded successfully, to free any allocated memory you should
 *      call freeMetaheader.
 *   return -- 0 for decode failure, non-zero for success.
 */
int decodeMetaheader( BitBuffer* buffer, struct Metaheader* result, long long* payload_length);

/**
 * Free a decoded object.
 *
 * Do not attempt to free an object that has not been decoded.
 *
 *   value -- The entry whose contents is to be released. The pointer 'value'
 *     will not be freed.
 */
void freeMetaheader(struct Metaheader* value);


/**
 * Print an xml representation of a metaheader object.
 */
void printXmlMetaheader(struct Metaheader* data, unsigned int offset, const char* name);

#ifdef __cplusplus
}
#endif

#endif

