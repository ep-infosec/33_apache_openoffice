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
#define TRANSLITERATION_smallToLarge_ja_JP
#include <transliteration_OneToOne.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

// ftp://ftp.unicode.org/Public/3.0-Update/UnicodeData-3.0.0.txt
// ftp://ftp.unicode.org/Public/3.0-Update/UnicodeData-3.0.0.html
// http://charts.unicode.org/Web/U3040.html Hiragana (U+3040..U+309F)
// http://charts.unicode.org/Web/U30A0.html Katakana (U+30A0..U+30FF)
// http://charts.unicode.org/Web/UFF00.html 

OneToOneMappingTable_t small2large[] = {        
    MAKE_PAIR( 0x3041, 0x3042 ),  // HIRAGANA LETTER SMALL A --> HIRAGANA LETTER A
    MAKE_PAIR( 0x3043, 0x3044 ),  // HIRAGANA LETTER SMALL I --> HIRAGANA LETTER I
    MAKE_PAIR( 0x3045, 0x3046 ),  // HIRAGANA LETTER SMALL U --> HIRAGANA LETTER U
    MAKE_PAIR( 0x3047, 0x3048 ),  // HIRAGANA LETTER SMALL E --> HIRAGANA LETTER E
    MAKE_PAIR( 0x3049, 0x304A ),  // HIRAGANA LETTER SMALL O --> HIRAGANA LETTER O
    MAKE_PAIR( 0x3063, 0x3064 ),  // HIRAGANA LETTER SMALL TU --> HIRAGANA LETTER TU
    MAKE_PAIR( 0x3083, 0x3084 ),  // HIRAGANA LETTER SMALL YA --> HIRAGANA LETTER YA
    MAKE_PAIR( 0x3085, 0x3086 ),  // HIRAGANA LETTER SMALL YU --> HIRAGANA LETTER YU
    MAKE_PAIR( 0x3087, 0x3088 ),  // HIRAGANA LETTER SMALL YO --> HIRAGANA LETTER YO
    MAKE_PAIR( 0x308E, 0x308F ),  // HIRAGANA LETTER SMALL WA --> HIRAGANA LETTER WA
    MAKE_PAIR( 0x30A1, 0x30A2 ),  // KATAKANA LETTER SMALL A --> KATAKANA LETTER A
    MAKE_PAIR( 0x30A3, 0x30A4 ),  // KATAKANA LETTER SMALL I --> KATAKANA LETTER I
    MAKE_PAIR( 0x30A5, 0x30A6 ),  // KATAKANA LETTER SMALL U --> KATAKANA LETTER U
    MAKE_PAIR( 0x30A7, 0x30A8 ),  // KATAKANA LETTER SMALL E --> KATAKANA LETTER E
    MAKE_PAIR( 0x30A9, 0x30AA ),  // KATAKANA LETTER SMALL O --> KATAKANA LETTER O
    MAKE_PAIR( 0x30C3, 0x30C4 ),  // KATAKANA LETTER SMALL TU --> KATAKANA LETTER TU
    MAKE_PAIR( 0x30E3, 0x30E4 ),  // KATAKANA LETTER SMALL YA --> KATAKANA LETTER YA
    MAKE_PAIR( 0x30E5, 0x30E6 ),  // KATAKANA LETTER SMALL YU --> KATAKANA LETTER YU
    MAKE_PAIR( 0x30E7, 0x30E8 ),  // KATAKANA LETTER SMALL YO --> KATAKANA LETTER YO
    MAKE_PAIR( 0x30EE, 0x30EF ),  // KATAKANA LETTER SMALL WA --> KATAKANA LETTER WA
    MAKE_PAIR( 0x30F5, 0x30AB ),  // KATAKANA LETTER SMALL KA --> KATAKANA LETTER KA
    MAKE_PAIR( 0x30F6, 0x30B1 ),  // KATAKANA LETTER SMALL KE --> KATAKANA LETTER KE
    MAKE_PAIR( 0xFF67, 0xFF71 ),  // HALFWIDTH KATAKANA LETTER SMALL A --> HALFWIDTH KATAKANA LETTER A
    MAKE_PAIR( 0xFF68, 0xFF72 ),  // HALFWIDTH KATAKANA LETTER SMALL I --> HALFWIDTH KATAKANA LETTER I
    MAKE_PAIR( 0xFF69, 0xFF73 ),  // HALFWIDTH KATAKANA LETTER SMALL U --> HALFWIDTH KATAKANA LETTER U
    MAKE_PAIR( 0xFF6A, 0xFF74 ),  // HALFWIDTH KATAKANA LETTER SMALL E --> HALFWIDTH KATAKANA LETTER E
    MAKE_PAIR( 0xFF6B, 0xFF75 ),  // HALFWIDTH KATAKANA LETTER SMALL O --> HALFWIDTH KATAKANA LETTER O
    MAKE_PAIR( 0xFF6C, 0xFF94 ),  // HALFWIDTH KATAKANA LETTER SMALL YA --> HALFWIDTH KATAKANA LETTER YA
    MAKE_PAIR( 0xFF6D, 0xFF95 ),  // HALFWIDTH KATAKANA LETTER SMALL YU --> HALFWIDTH KATAKANA LETTER YU
    MAKE_PAIR( 0xFF6E, 0xFF96 ),  // HALFWIDTH KATAKANA LETTER SMALL YO --> HALFWIDTH KATAKANA LETTER YO
    MAKE_PAIR( 0xFF6F, 0xFF82 )   // HALFWIDTH KATAKANA LETTER SMALL TU --> HALFWIDTH KATAKANA LETTER TU
};

smallToLarge_ja_JP::smallToLarge_ja_JP()
{
    static oneToOneMapping _table(small2large, sizeof(small2large));
    func = (TransFunc) 0;
    table = &_table;
    transliterationName = "smallToLarge_ja_JP";
    implementationName = "com.sun.star.i18n.Transliteration.smallToLarge_ja_JP";
}

} } } }
