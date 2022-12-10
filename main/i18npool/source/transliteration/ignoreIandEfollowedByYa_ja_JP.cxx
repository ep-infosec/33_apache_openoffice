/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <i18nutil/oneToOneMapping.hxx>
#define TRANSLITERATION_IandEfollowedByYa_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t IandE[] = {      
    MAKE_PAIR( 0x30A3, 0x0000 ),  // KATAKANA LETTER SMALL I
    MAKE_PAIR( 0x30A4, 0x0000 ),  // KATAKANA LETTER I
    MAKE_PAIR( 0x30A7, 0x0000 ),  // KATAKANA LETTER SMALL E
    MAKE_PAIR( 0x30A8, 0x0000 ),  // KATAKANA LETTER E
    MAKE_PAIR( 0x30AD, 0x0000 ),  // KATAKANA LETTER KI
    MAKE_PAIR( 0x30AE, 0x0000 ),  // KATAKANA LETTER GI
    MAKE_PAIR( 0x30B1, 0x0000 ),  // KATAKANA LETTER KE
    MAKE_PAIR( 0x30B2, 0x0000 ),  // KATAKANA LETTER GE
    MAKE_PAIR( 0x30B7, 0x0000 ),  // KATAKANA LETTER SI
    MAKE_PAIR( 0x30B8, 0x0000 ),  // KATAKANA LETTER ZI
    MAKE_PAIR( 0x30BB, 0x0000 ),  // KATAKANA LETTER SE
    MAKE_PAIR( 0x30BC, 0x0000 ),  // KATAKANA LETTER ZE
    MAKE_PAIR( 0x30C1, 0x0000 ),  // KATAKANA LETTER TI
    MAKE_PAIR( 0x30C2, 0x0000 ),  // KATAKANA LETTER DI
    MAKE_PAIR( 0x30C6, 0x0000 ),  // KATAKANA LETTER TE
    MAKE_PAIR( 0x30C7, 0x0000 ),  // KATAKANA LETTER DE
    MAKE_PAIR( 0x30CB, 0x0000 ),  // KATAKANA LETTER NI
    MAKE_PAIR( 0x30CD, 0x0000 ),  // KATAKANA LETTER NE
    MAKE_PAIR( 0x30D2, 0x0000 ),  // KATAKANA LETTER HI
    MAKE_PAIR( 0x30D3, 0x0000 ),  // KATAKANA LETTER BI
    MAKE_PAIR( 0x30D4, 0x0000 ),  // KATAKANA LETTER PI
    MAKE_PAIR( 0x30D8, 0x0000 ),  // KATAKANA LETTER HE
    MAKE_PAIR( 0x30D9, 0x0000 ),  // KATAKANA LETTER BE
    MAKE_PAIR( 0x30DA, 0x0000 ),  // KATAKANA LETTER PE
    MAKE_PAIR( 0x30DF, 0x0000 ),  // KATAKANA LETTER MI
    MAKE_PAIR( 0x30E1, 0x0000 ),  // KATAKANA LETTER ME
    MAKE_PAIR( 0x30EA, 0x0000 ),  // KATAKANA LETTER RI
    MAKE_PAIR( 0x30EC, 0x0000 ),  // KATAKANA LETTER RE
    MAKE_PAIR( 0x30F0, 0x0000 ),  // KATAKANA LETTER WI
    MAKE_PAIR( 0x30F1, 0x0000 ),  // KATAKANA LETTER WE
    MAKE_PAIR( 0x30F6, 0x0000 ),  // KATAKANA LETTER SMALL KE
    MAKE_PAIR( 0x30F8, 0x0000 ),  // KATAKANA LETTER VI
    MAKE_PAIR( 0x30F9, 0x0000 )   // KATAKANA LETTER VE
};




OUString SAL_CALL 
ignoreIandEfollowedByYa_ja_JP::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // Create a string buffer which can hold nCount + 1 characters.
    // The reference count is 0 now.
    rtl_uString * newStr = x_rtl_uString_new_WithLength( nCount ); // defined in x_rtl_ustring.h  
    sal_Unicode * dst = newStr->buffer;
    const sal_Unicode * src = inStr.getStr() + startPos;

    sal_Int32 *p = 0;
	sal_Int32 position = 0;
    if (useOffset) {
        // Allocate nCount length to offset argument.
        offset.realloc( nCount );
        p = offset.getArray();
        position = startPos;
    }

    // 
    sal_Unicode previousChar = *src ++;
    sal_Unicode currentChar;

    // One to one mapping
    oneToOneMapping aTable(IandE, sizeof(IandE));

    // Translation
    while (-- nCount > 0) {
        currentChar = *src ++;

        // the character listed in above table + YA --> the character + A
        if (currentChar == 0x30E3 ||   // KATAKANA LETTER SMALL YA
                currentChar == 0x30E4) {   // KATAKANA LETTER YA
            if (aTable[ previousChar ] != previousChar) {
                if (useOffset) {
                    *p ++ = position++;
                    *p ++ = position++;
                }
                *dst ++ = previousChar;
                *dst ++ = 0x30A2;          // KATAKANA LETTER A
                previousChar = *src ++;
                nCount --;
                continue;
            }
        }

        if (useOffset)
            *p ++ = position++;
        *dst ++ = previousChar;
        previousChar = currentChar;
    }

    if (nCount == 0) {
        if (useOffset)
            *p = position;
        *dst ++ = previousChar;
    }

    *dst = (sal_Unicode) 0;

    newStr->length = sal_Int32(dst - newStr->buffer);
    if (useOffset)
        offset.realloc(newStr->length);
    return OUString( newStr, SAL_NO_ACQUIRE ); // take over ownership of <newStr>
}

} } } }
