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

#define TRANSLITERATION_Separator_ja_JP
#include <transliteration_Ignore.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

OneToOneMappingTable_t ignoreSeparatorTable[] = {       
    MAKE_PAIR( 0x0021, 0xFFFF ),  // EXCLAMATION MARK
    MAKE_PAIR( 0x0023, 0xFFFF ),  // NUMBER SIGN
    MAKE_PAIR( 0x0024, 0xFFFF ),  // DOLLAR SIGN
    MAKE_PAIR( 0x0025, 0xFFFF ),  // PERCENT SIGN
    MAKE_PAIR( 0x0026, 0xFFFF ),  // AMPERSAND
    MAKE_PAIR( 0x002A, 0xFFFF ),  // ASTERISK
    MAKE_PAIR( 0x002B, 0xFFFF ),  // PLUS SIGN
    MAKE_PAIR( 0x002C, 0xFFFF ),  // COMMA
    MAKE_PAIR( 0x002D, 0xFFFF ),  // HYPHEN-MINUS
    MAKE_PAIR( 0x002E, 0xFFFF ),  // FULL STOP
    MAKE_PAIR( 0x002F, 0xFFFF ),  // SOLIDUS
    MAKE_PAIR( 0x003A, 0xFFFF ),  // COLON
    MAKE_PAIR( 0x003B, 0xFFFF ),  // SEMICOLON
    MAKE_PAIR( 0x003C, 0xFFFF ),  // LESS-THAN SIGN
    MAKE_PAIR( 0x003D, 0xFFFF ),  // EQUALS SIGN
    MAKE_PAIR( 0x003E, 0xFFFF ),  // GREATER-THAN SIGN
    MAKE_PAIR( 0x005C, 0xFFFF ),  // REVERSE SOLIDUS
    MAKE_PAIR( 0x005F, 0xFFFF ),  // LOW LINE
    MAKE_PAIR( 0x007B, 0xFFFF ),  // LEFT CURLY BRACKET
    MAKE_PAIR( 0x007C, 0xFFFF ),  // VERTICAL LINE
    MAKE_PAIR( 0x007D, 0xFFFF ),  // RIGHT CURLY BRACKET
    MAKE_PAIR( 0x007E, 0xFFFF ),  // TILDE
    MAKE_PAIR( 0x00A5, 0xFFFF ),  // YEN SIGN
    MAKE_PAIR( 0x3001, 0xFFFF ),  // IDEOGRAPHIC COMMA
    MAKE_PAIR( 0x3002, 0xFFFF ),  // IDEOGRAPHIC FULL STOP
    MAKE_PAIR( 0x3008, 0xFFFF ),  // LEFT ANGLE BRACKET
    MAKE_PAIR( 0x3009, 0xFFFF ),  // RIGHT ANGLE BRACKET
    MAKE_PAIR( 0x300A, 0xFFFF ),  // LEFT DOUBLE ANGLE BRACKET
    MAKE_PAIR( 0x300B, 0xFFFF ),  // RIGHT DOUBLE ANGLE BRACKET
    MAKE_PAIR( 0x300C, 0xFFFF ),  // LEFT CORNER BRACKET
    MAKE_PAIR( 0x300D, 0xFFFF ),  // RIGHT CORNER BRACKET
    MAKE_PAIR( 0x300E, 0xFFFF ),  // LEFT WHITE CORNER BRACKET
    MAKE_PAIR( 0x300F, 0xFFFF ),  // RIGHT WHITE CORNER BRACKET
    MAKE_PAIR( 0x3010, 0xFFFF ),  // LEFT BLACK LENTICULAR BRACKET
    MAKE_PAIR( 0x3011, 0xFFFF ),  // RIGHT BLACK LENTICULAR BRACKET
    MAKE_PAIR( 0x3014, 0xFFFF ),  // LEFT TORTOISE SHELL BRACKET
    MAKE_PAIR( 0x3015, 0xFFFF ),  // RIGHT TORTOISE SHELL BRACKET
    MAKE_PAIR( 0x3016, 0xFFFF ),  // LEFT WHITE LENTICULAR BLACKET
    MAKE_PAIR( 0x3017, 0xFFFF ),  // RIGHT WHITE LENTICULAR BLACKET
    MAKE_PAIR( 0x3018, 0xFFFF ),  // LEFT WHITETORTOISE SHELL BLACKET
    MAKE_PAIR( 0x3019, 0xFFFF ),  // RIGHT WHITETORTOISE SHELL BLACKET
    MAKE_PAIR( 0x301A, 0xFFFF ),  // LEFT WHITE SQUARE BRACKET
    MAKE_PAIR( 0x301B, 0xFFFF ),  // RIGHT WHITE SQUARE BRACKET
    MAKE_PAIR( 0x301C, 0xFFFF ),  // WAVE DASH
    MAKE_PAIR( 0x301D, 0xFFFF ),  // REVERSED DOUBLE PRIME
    MAKE_PAIR( 0x301E, 0xFFFF ),  // DOUBLE PRIME QUOTATION MARK
    MAKE_PAIR( 0x301F, 0xFFFF ),  // LOW DOUBLE PRIME QUOTATION MARK
    MAKE_PAIR( 0x3030, 0xFFFF ),  // WAVY DASH 
    MAKE_PAIR( 0x30FB, 0xFFFF ),  // KATAKANA MIDDLE DOT
    MAKE_PAIR( 0x30FC, 0xFFFF ),  // KATAKANA-HIRAHANA PROLONGED SOUND MARK
    MAKE_PAIR( 0xFF01, 0xFFFF ),  // FULLWIDTH EXCLAMATION MARK
    MAKE_PAIR( 0xFF03, 0xFFFF ),  // FULLWIDTH NUMBER SIGN
    MAKE_PAIR( 0xFF04, 0xFFFF ),  // FULLWIDTH DOLLAR SIGN
    MAKE_PAIR( 0xFF05, 0xFFFF ),  // FULLWIDTH PERCENT SIGN
    MAKE_PAIR( 0xFF06, 0xFFFF ),  // FULLWIDTH AMPERSAND
    MAKE_PAIR( 0xFF0A, 0xFFFF ),  // FULLWIDTH ASTERISK
    MAKE_PAIR( 0xFF0B, 0xFFFF ),  // FULLWIDTH PLUS SIGN
    MAKE_PAIR( 0xFF0C, 0xFFFF ),  // FULLWIDTH COMMA
    MAKE_PAIR( 0xFF0D, 0xFFFF ),  // FULLWIDTH HYPHEN-MINUS
    MAKE_PAIR( 0xFF0E, 0xFFFF ),  // FULLWIDTH FULL STOP
    MAKE_PAIR( 0xFF0F, 0xFFFF ),  // FULLWIDTH SOLIDUS
    MAKE_PAIR( 0xFF1A, 0xFFFF ),  // FULLWIDTH COLON
    MAKE_PAIR( 0xFF1B, 0xFFFF ),  // FULLWIDTH SEMICOLON
    MAKE_PAIR( 0xFF1C, 0xFFFF ),  // FULLWIDTH LESS-THAN SIGN
    MAKE_PAIR( 0xFF1D, 0xFFFF ),  // FULLWIDTH EQUALS SIGN
    MAKE_PAIR( 0xFF1E, 0xFFFF ),  // FULLWIDTH GREATER-THAN SIGN
    MAKE_PAIR( 0xFF3C, 0xFFFF ),  // FULLWIDTH REVERSE SOLIDUS
    MAKE_PAIR( 0xFF3F, 0xFFFF ),  // FULLWIDTH LOW LINE
    MAKE_PAIR( 0xFF5B, 0xFFFF ),  // FULLWIDTH LEFT CURLY BRACKET
    MAKE_PAIR( 0xFF5C, 0xFFFF ),  // FULLWIDTH VERTICAL LINE
    MAKE_PAIR( 0xFF5D, 0xFFFF ),  // FULLWIDTH RIGHT CURLY BRACKET
    MAKE_PAIR( 0xFF5E, 0xFFFF ),  // FULLWIDTH TILDE
    MAKE_PAIR( 0xFFE5, 0xFFFF ),  // FULLWIDTH YEN SIGN
};

ignoreSeparator_ja_JP::ignoreSeparator_ja_JP()
{
        static oneToOneMapping _table(ignoreSeparatorTable, sizeof(ignoreSeparatorTable));
        func = (TransFunc) 0;
        table = &_table;
        map = 0;
        transliterationName = "ignoreSeparator_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreSeparator_ja_JP";
}

} } } }

