


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

#include "digits.h"
#include "tendigits.h"
#include "ninedigits.h"
#include "eightdigits.h"
#include "sevendigits.h"
#include "sixdigits.h"
#include "fivedigits.h"
#include "fourdigits.h"
#include "threedigits.h"
#include "twodigits.h"
#include "digit.h"
#include "variable_integer.h"























static int decodeDigits0(BitBuffer* buffer, long long* digits)
{
    long long unusedTenDigits;
    int unusedNineDigits;
    int unusedEightDigits;
    int unusedSevenDigits;
    int unusedSixDigits;
    int unusedFiveDigits;
    int unusedFourDigits;
    int unusedThreeDigits;
    int unusedTwoDigits;
    int unusedDigit;
    int digits1;
    int digits2;
    int digits3;
    int digits4;
    int digits5;
    int digits6;
    int digits7;
    int digits8;
    int digits9;
    
    BitBuffer temp;
    
    
    
    if (temp = *buffer, decodeTenDigits(&temp, &unusedTenDigits, digits))
    {
      

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeNineDigits(&temp, &unusedNineDigits, &digits1))
    {
      
    *digits = digits1;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeEightDigits(&temp, &unusedEightDigits, &digits2))
    {
      
    *digits = digits2;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeSevenDigits(&temp, &unusedSevenDigits, &digits3))
    {
      
    *digits = digits3;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeSixDigits(&temp, &unusedSixDigits, &digits4))
    {
      
    *digits = digits4;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeFiveDigits(&temp, &unusedFiveDigits, &digits5))
    {
      
    *digits = digits5;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeFourDigits(&temp, &unusedFourDigits, &digits6))
    {
      
    *digits = digits6;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeThreeDigits(&temp, &unusedThreeDigits, &digits7))
    {
      
    *digits = digits7;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeTwoDigits(&temp, &unusedTwoDigits, &digits8))
    {
      
    *digits = digits8;

        *buffer = temp;
    }
    
    
    else if (temp = *buffer, decodeDigit(&temp, &unusedDigit, &digits9))
    {
      
    *digits = digits9;

        *buffer = temp;
    }
    else
    {
        /* Decode failed, no options succeeded... */
        return 0;
    }

    return 1;
}



void freeDigits(long long* value)
{
}


int decodeDigits(BitBuffer* buffer, long long* result, long long* digits)
{
    long long digits0;
    
    if (!decodeDigits0(buffer, &digits0))
    {
        return 0;
    }
    
    long long digitsValue = digits0;
    

    *result = digitsValue;
    *digits = digitsValue;

    return 1;
}












void printXmlDigits(long long* data, unsigned int offset, const char* name)
{
    
    if (offset > 0)
    {
        printf("%*c", offset, ' ');
    }

    printf("<%s>%lli</%s>\n", name, *data, name);
}



















