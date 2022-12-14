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
#include "precompiled_vcl.hxx"

#ifndef SOLARIS
#include <tools/prex.h>
#include <X11/XKBlib.h>
#include <tools/postx.h>
#endif

#include <unx/saldisp.hxx>
#include <X11/keysym.h>

#if !defined (SunXK_Undo)
#define SunXK_Undo		0x0000FF65	// XK_Undo
#define SunXK_Again		0x0000FF66	// XK_Redo
#define SunXK_Find		0x0000FF68	// XK_Find
#define SunXK_Stop		0x0000FF69	// XK_Cancel
#define SunXK_Props		0x1005FF70
#define SunXK_Front		0x1005FF71
#define SunXK_Copy		0x1005FF72
#define SunXK_Open		0x1005FF73
#define SunXK_Paste		0x1005FF74
#define SunXK_Cut		0x1005FF75
#endif

#ifdef SOLARIS
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/kbio.h>
#include <sys/kbd.h>
#include <stdio.h>
#include <fcntl.h>
#include <deflt.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#include <string.h>

namespace vcl_sal {

	struct KeysymNameReplacement
	{
		KeySym			aSymbol;
		const char*		pName;
	};

	struct KeyboardReplacements
	{
		const char*						pKeyboardName;
		const KeysymNameReplacement*	pReplacements;
		int								nReplacements;
	};
    
    // ====================================================================
    //
    // CAUTION CAUTION CAUTION
    // every string value in the replacements tables must be in UTF8
    // be careful with your editor !
    //
    // ====================================================================

	static const struct KeysymNameReplacement aImplReplacements_English[] =
	{
		{ XK_Control_L, "Ctrl" },
		{ XK_Control_R, "Ctrl" },
		{ XK_Escape, "Esc" }, 
		{ XK_space, "Space" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Turkish[] =
	{
		{ XK_Control_L, "Ctrl" },
		{ XK_Control_R, "Ctrl" },
		{ XK_Right, "Sa??" },
		{ XK_Left, "Sol" },
		{ XK_Up, "Yukar??" },
		{ XK_Down, "A??a????" },
		{ XK_space, "Bo??luk" } 
	};

	static const struct KeysymNameReplacement aImplReplacements_Russian[] =
	{
		{ XK_Right, "????????????" },
		{ XK_Left, "??????????" },
		{ XK_Up, "??????????" },
		{ XK_Down, "????????" },
		{ XK_space, "????????????" } 
	};

	static const struct KeysymNameReplacement aImplReplacements_German[] =
	{
		{ XK_Control_L, "Strg" },
		{ XK_Control_R, "Strg" },
		{ XK_Shift_L, "Umschalt" },
		{ XK_Shift_R, "Umschalt" },
		{ XK_Alt_L, "Alt" },
		{ XK_Alt_R, "Alt Gr" },
		{ XK_Page_Up, "Bild auf" },
		{ XK_Page_Down, "Bild ab" },
		{ XK_End, "Ende" },
		{ XK_Home, "Pos 1" },
		{ XK_Insert, "Einfg" },
		{ XK_Delete, "Entf" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "Rechts" },
		{ XK_Left, "Links" },
		{ XK_Up, "Oben" },
		{ XK_Down, "Unten" },
		{ XK_BackSpace, "R??ckschritt" },
		{ XK_Return, "Eingabe" },
		{ XK_slash, "Schr??gstrich" },
		{ XK_space, "Leertaste" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Wiederholen" },
        { SunXK_Props, "Eigenschaften" },
        { SunXK_Undo,  "Zur??cknehmen" },
        { SunXK_Front, "Vordergrund" },
        { SunXK_Copy,  "Kopieren" },
        { SunXK_Open,  "??ffnen" },
        { SunXK_Paste, "Einsetzen" },
        { SunXK_Find,  "Suchen" },
        { SunXK_Cut,   "Ausschneiden" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_French[] =
	{
		{ XK_Shift_L, "Maj" },
		{ XK_Shift_R, "Maj" },
		{ XK_Page_Up, "Pg. Pr??c" },
		{ XK_Page_Down, "Pg. Suiv" },
		{ XK_End, "Fin" },
		{ XK_Home, "Origine" },
		{ XK_Insert, "Ins??rer" },
		{ XK_Delete, "Suppr" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "Droite" },
		{ XK_Left, "Gauche" },
		{ XK_Up, "Haut" },
		{ XK_Down, "Bas" },
		{ XK_BackSpace, "Ret. Arr" },
		{ XK_Return, "Retour" },
        { XK_space, "Espace" },
		{ XK_KP_Enter, "Entr??e" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Encore" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Annuler" },
        { SunXK_Front, "Devant" },
        { SunXK_Copy,  "Copy" },
        { SunXK_Open,  "Ouvrir" },
        { SunXK_Paste, "Coller" },
        { SunXK_Find,  "Cher." },
        { SunXK_Cut,   "Couper" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Italian[] =
	{
		{ XK_Shift_L, "Maiusc" },
		{ XK_Shift_R, "Maiusc" },
		{ XK_Page_Up, "PgSu" },
		{ XK_Page_Down, "PgGiu" },
		{ XK_End, "Fine" },
		{ XK_Insert, "Ins" },
		{ XK_Delete, "Canc" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "A destra" },
		{ XK_Left, "A sinistra" },
		{ XK_Up, "Sposta verso l'alto" },
		{ XK_Down, "Sposta verso il basso" },
		{ XK_BackSpace, "Backspace" },
		{ XK_Return, "Invio" },
		{ XK_space, "Spazio" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Ancora" },
        { SunXK_Props, "Propriet??" },
        { SunXK_Undo,  "Annulla" },
        { SunXK_Front, "Davanti" },
        { SunXK_Copy,  "Copia" },
        { SunXK_Open,  "Apri" },
        { SunXK_Paste, "Incolla" },
        { SunXK_Find,  "Trova" },
        { SunXK_Cut,   "Taglia" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Dutch[] =
	{
		{ XK_Page_Up, "PageUp" },
		{ XK_Page_Down, "PageDown" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "Rechts" },
		{ XK_Left, "Links" },
		{ XK_Up, "Boven" },
		{ XK_Down, "Onder" },
		{ XK_BackSpace, "Backspace" },
		{ XK_Return, "Return" },
		{ XK_space, "Spatiebalk" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Again" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Undo" },
        { SunXK_Front, "Front" },
        { SunXK_Copy,  "Copy" },
        { SunXK_Open,  "Open" },
        { SunXK_Paste, "Paste" },
        { SunXK_Find,  "Find" },
        { SunXK_Cut,   "Cut" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Norwegian[] =
	{
		{ XK_Shift_L, "Skift" },
		{ XK_Shift_R, "Skift" },
		{ XK_Page_Up, "PageUp" },
		{ XK_Page_Down, "PageDown" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "Hyre" },
		{ XK_Left, "Venstre" },
		{ XK_Up, "Opp" },
		{ XK_Down, "Ned" },
		{ XK_BackSpace, "Tilbake" },
		{ XK_Return, "Enter" },
        { SunXK_Stop,  "Avbryt" },
        { SunXK_Again, "Gjenta" },
        { SunXK_Props, "Egenskaper" },
        { SunXK_Undo,  "Angre" },
        { SunXK_Front, "Front" },
        { SunXK_Copy,  "Kopi" },
        { SunXK_Open,  "??pne" },
        { SunXK_Paste, "Lim" },
        { SunXK_Find,  "S??k" },
        { SunXK_Cut,   "Klipp" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Swedish[] =
	{
		{ XK_Shift_L, "Skift" },
		{ XK_Shift_R, "Skift" },
		{ XK_Page_Up, "PageUp" },
		{ XK_Page_Down, "PageDown" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "H??ger" },
		{ XK_Left, "V??nster" },
		{ XK_Up, "Up" },
		{ XK_Down, "Ned" },
		{ XK_BackSpace, "Backsteg" },
		{ XK_Return, "Retur" },
		{ XK_space, "Blank" },
        { SunXK_Stop,  "Avbryt" },
        { SunXK_Again, "Upprepa" },
        { SunXK_Props, "Egenskaper" },
        { SunXK_Undo,  "??ngra" },
        { SunXK_Front, "Fram" },
        { SunXK_Copy,  "Kopiera" },
        { SunXK_Open,  "??ppna" },
        { SunXK_Paste, "Klistra in" },
        { SunXK_Find,  "S??k" },
        { SunXK_Cut,   "Klipp ut" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Portuguese[] =
	{
		{ XK_Page_Up, "PageUp" },
		{ XK_Page_Down, "PageDown" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "Direita" },
		{ XK_Left, "Esquerda" },
		{ XK_Up, "Acima" },
		{ XK_Down, "Abaixo" },
		{ XK_BackSpace, "Backspace" },
		{ XK_Return, "Enter" },
		{ XK_slash, "Barra" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Again" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Undo" },
        { SunXK_Front, "Front" },
        { SunXK_Copy,  "Copy" },
        { SunXK_Open,  "Open" },
        { SunXK_Paste, "Paste" },
        { SunXK_Find,  "Find" },
        { SunXK_Cut,   "Cut" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeysymNameReplacement aImplReplacements_Spanish[] =
	{
		{ XK_Shift_L, "May??s" },
		{ XK_Shift_R, "May??s" },
		{ XK_Page_Up, "ReP??g" },
		{ XK_Page_Down, "AvP??g" },
		{ XK_End, "Fin" },
		{ XK_Home, "Inicio" },
		{ XK_Delete, "Supr" },
		{ XK_Escape, "Esc" },
		{ XK_Right, "Hacia la derecha" },
		{ XK_Left, "Hacia la izquierda" },
		{ XK_Up, "Hacia arriba" },
		{ XK_Down, "Hacia abajo" },
		{ XK_BackSpace, "Ret" },
		{ XK_Return, "Entrada" },
		{ XK_space, "Espacio" },
		{ XK_KP_Enter, "Intro" },
        { SunXK_Stop,  "Stop" },
        { SunXK_Again, "Repetir" },
        { SunXK_Props, "Props" },
        { SunXK_Undo,  "Anular" },
        { SunXK_Front, "Delante" },
        { SunXK_Copy,  "Copiar" },
        { SunXK_Open,  "Abrir" },
        { SunXK_Paste, "Pegar" },
        { SunXK_Find,  "Buscar" },
        { SunXK_Cut,   "Cortar" },
        { XK_minus, "-" },
        { XK_plus, "+" }
	};

	static const struct KeyboardReplacements aKeyboards[] =
	{
#ifdef SOLARIS
		{ "Germany5", aImplReplacements_German, sizeof(aImplReplacements_German)/sizeof(aImplReplacements_German[0]) },
		{ "Germany4", aImplReplacements_German, sizeof(aImplReplacements_German)/sizeof(aImplReplacements_German[0]) },
		{ "France5", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]) },
		{ "France6", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]) },
		{ "France_x86", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]) },
		{ "Italy5", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]) },
		{ "Italy5-Hobo", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]) },
		{ "Italy4", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]) },
		{ "Italy6", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]) },
		{ "Italy_x86", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]) },
		{ "Netherland4", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		{ "Netherland5", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		{ "Netherland5-Hobo", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		{ "Netherland6", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		{ "Netherland_x86", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		{ "Norway5", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]) },
		{ "Norway5-Hobo", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]) },
		{ "Norway4", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]) },
		{ "Norway6", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]) },
		{ "Norway_x86", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]) },
		{ "Portugal5", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]) },
		{ "Portugal5-Hobo", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]) },
		{ "Portugal4", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]) },
		{ "Portugal6", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]) },
		{ "Portugal_x86", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]) },
		{ "Spain5", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
		{ "Spain5-Hobo", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
		{ "Spain4", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
		{ "Spain6", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
		{ "Spain_x86", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
		{ "Sweden5", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]) },
		{ "Sweden5-Hobo", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]) },
		{ "Sweden4", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]) },
		{ "Sweden6", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]) },
		{ "Sweden_x86", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]) },
#endif
		{ "U.S. English", aImplReplacements_English, sizeof(aImplReplacements_English)/sizeof(aImplReplacements_English[0]) },
		{ "United Kingdom", aImplReplacements_English, sizeof(aImplReplacements_English)/sizeof(aImplReplacements_English[0]) },
        // Germany, German
		{ "German", aImplReplacements_German, sizeof(aImplReplacements_German)/sizeof(aImplReplacements_German[0]) },
		{ "France", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]) },
		{ "French", aImplReplacements_French, sizeof(aImplReplacements_French)/sizeof(aImplReplacements_French[0]) },
        // Italy, Italian
		{ "Ital", aImplReplacements_Italian, sizeof(aImplReplacements_Italian)/sizeof(aImplReplacements_Italian[0]) },
        // Norway, Norwegian
		{ "Norw", aImplReplacements_Norwegian, sizeof(aImplReplacements_Norwegian)/sizeof(aImplReplacements_Norwegian[0]) },
        // Portugal, Portuguese
		{ "Portu", aImplReplacements_Portuguese, sizeof(aImplReplacements_Portuguese)/sizeof(aImplReplacements_Portuguese[0]) },
		{ "Spain", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
		{ "Spanish", aImplReplacements_Spanish, sizeof(aImplReplacements_Spanish)/sizeof(aImplReplacements_Spanish[0]) },
        // Sweden, Swedish
		{ "Swed", aImplReplacements_Swedish, sizeof(aImplReplacements_Swedish)/sizeof(aImplReplacements_Swedish[0]) },
		{ "Netherland", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		{ "Dutch", aImplReplacements_Dutch, sizeof(aImplReplacements_Dutch)/sizeof(aImplReplacements_Dutch[0]) },
		// Turkish, Turkey
		{ "Turk", aImplReplacements_Turkish, sizeof(aImplReplacements_Turkish)/sizeof(aImplReplacements_Turkish[0]) },
		// Russian, Russia
		{ "Russia", aImplReplacements_Russian, sizeof(aImplReplacements_Russian)/sizeof(aImplReplacements_Russian[0]) },
		{ "English", aImplReplacements_English, sizeof(aImplReplacements_English)/sizeof(aImplReplacements_English[0]) }
	};

	String getKeysymReplacementName( const char* pKeyboard, KeySym nSymbol )
	{
		for( unsigned int n = 0; n < sizeof(aKeyboards)/sizeof(aKeyboards[0]); n++ )
		{
			if( ! strncasecmp( pKeyboard, aKeyboards[n].pKeyboardName, strlen( aKeyboards[n].pKeyboardName ) ) )
			{
				const struct KeysymNameReplacement* pRepl = aKeyboards[n].pReplacements;
				for( int m = aKeyboards[n].nReplacements ; m ; )
				{
					if( nSymbol == pRepl[--m].aSymbol )
						return String( pRepl[m].pName, RTL_TEXTENCODING_UTF8 );
				}
			}
		}
        // try english fallbacks
        const struct KeysymNameReplacement* pRepl = aImplReplacements_English;
        for( int m = sizeof(aImplReplacements_English)/sizeof(aImplReplacements_English[0]) ; m ; )
        {
            if( nSymbol == pRepl[--m].aSymbol )
                return String( pRepl[m].pName, RTL_TEXTENCODING_UTF8 );
        }
		return String();
	}

}

#ifdef SOLARIS
typedef struct {
	int 		n_layout;
	const char* p_description;
} keyboard_layout;

static const keyboard_layout type0_layout[] =
{
	{ 0, "US4" },
	{ -1, NULL }
};

static const keyboard_layout type3_layout[] =
{
	{ 0, "US3" },
	{ -1, NULL }
};

static const keyboard_layout type4_layout[] =
{
	{ 0,  "US4" },
	{ 1,  "US4" },
	{ 2,  "FranceBelg4" },
	{ 3,  "Canada4" },
	{ 4,  "Denmark4" },
	{ 5,  "Germany4" },
	{ 6,  "Italy4" },
	{ 7,  "Netherland4" },
	{ 8,  "Norway4" },
	{ 9,  "Portugal4" },
	{ 10, "SpainLatAm4" },
	{ 11, "SwedenFin4" },
	{ 12, "Switzer_Fr4" },
	{ 13, "Switzer_Ge4" },
	{ 14, "UK4" },
	{ 16, "Korea4" },
	{ 17, "Taiwan4" },
	{ 19, "US101A_PC" },
	{ 19, "US101A_Sun" },
	{ 32, "Japan4" },
	{ 33, "US5" },
	{ 34, "US_UNIX5" },
	{ 35, "France5" },
	{ 36, "Denmark5" },
	{ 37, "Germany5" },
	{ 38, "Italy5" },
	{ 39, "Netherland5" },
	{ 40, "Norway5" },
	{ 41, "Portugal5" },
	{ 42, "Spain5" },
	{ 43, "Sweden5" },
	{ 44, "Switzer_Fr5" },
	{ 45, "Switzer_Ge5" },
	{ 46, "UK5" },
	{ 47, "Korea5" },
	{ 48, "Taiwan5" },
	{ 49, "Japan5" },
	{ 50, "Canada_Fr5" },
	{ 51, "Hungary5" },
	{ 52, "Poland5" },
	{ 53, "Czech5" },
	{ 54, "Russia5" },
	{ 55, "Latvia5" },
	{ 56, "Turkey5" },
	{ 57, "Greece5" },
	{ 58, "Estonia5" },
	{ 59, "Lithuania5" },
	{ 63, "Canada_Fr5_TBITS5" },
	{ 80, "US5_Hobo" },
	{ 81, "US_UNIX5_Hobo" },
	{ 82, "France5_Hobo" },
	{ 83, "Denmark5_Hobo" },
	{ 84, "Germany5_Hobo" },
	{ 85, "Italy5_Hobo" },
	{ 86, "Netherland5_Hobo" },
	{ 87, "Norway5_Hobo" },
	{ 88, "Portugal5_Hobo" },
	{ 89, "Spain5_Hobo" },
	{ 90, "Sweden5_Hobo" },
	{ 91, "Switzer_Fr5_Hobo" },
	{ 92, "Switzer_Ge5_Hobo" },
	{ 93, "UK5_Hobo" },
	{ 94, "Korea5_Hobo" },
	{ 95, "Taiwan5_Hobo" },
	{ 96, "Japan5_Hobo" },
	{ 97, "Canada_Fr5_Hobo" },
	{ -1, NULL }
};

static const keyboard_layout type101_layout[] =
{
	{  0, "US101A_x86" },
	{  1, "US101A_x86" },
	{ 34, "J3100_x86" },
	{ 35, "France_x86" },
	{ 36, "Denmark_x86" },
	{ 37, "Germany_x86" },
	{ 38, "Italy_x86" },
	{ 39, "Netherland_x86" },
	{ 40, "Norway_x86" },
	{ 41, "Portugal_x86" },
	{ 42, "Spain_x86" },
	{ 43, "Sweden_x86" },
	{ 44, "Switzer_Fr_x86" },
	{ 45, "Switzer_Ge_x86" },
	{ 46, "UK_x86" },
	{ 47, "Korea_x86" },
	{ 48, "Taiwan_x86" },
	{ 49, "Japan_x86" },
	{ 50, "Canada_Fr2_x86" },
	{ 51, "Hungary_x86" },
	{ 52, "Poland_x86" },
	{ 53, "Czech_x86" },
	{ 54, "Russia_x86" },
	{ 55, "Latvia_x86" },
	{ 56, "Turkey_x86" },
	{ 57, "Greece_x86" },
	{ 59, "Lithuania_x86" },
	{ 1001, "MS_US101A_x86" },
	{ -1, NULL }
};

static const keyboard_layout type6_layout[] =
{
	{ 0,  "US6" },
	{ 6,  "Denmark6" },
	{ 7,  "Finnish6" },
	{ 8,  "France6" },
	{ 9,  "Germany6" },
	{ 14, "Italy6" },
	{ 15, "Japan6" },
	{ 16, "Korea6" },
	{ 18, "Netherland6" },
	{ 19, "Norway6" },
	{ 22, "Portugal6" },
	{ 25, "Spain6" },
	{ 26, "Sweden6" },
	{ 27, "Switzer_Fr6" },
	{ 28, "Switzer_Ge6" },
	{ 30, "Taiwan6" },
	{ 32, "UK6" },
	{ 33, "US6" },
	{ -1, NULL }
};
#endif


#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

const char* SalDisplay::GetKeyboardName( sal_Bool bRefresh )
{
	if( bRefresh || ! m_aKeyboardName.Len() )
	{
#ifdef SOLARIS
		if( IsLocal() )
		{
			int kbd = open( "/dev/kbd", O_RDONLY );
			if( kbd >= 0 )
			{
				int kbd_type = 0;
				if( ! ioctl( kbd, KIOCTYPE, &kbd_type ) )
				{
					int kbd_layout = 0;
					if( ! ioctl( kbd, KIOCLAYOUT, &kbd_layout ) )
					{
						const keyboard_layout *p_layout = NULL;
						switch( kbd_type )
						{
							case KB_KLUNK: p_layout = type0_layout;   break;
							case KB_SUN3:  p_layout = type3_layout;   break;
							case KB_SUN4:  p_layout = type4_layout;   break;
							case KB_USB:   p_layout = type6_layout;   break;
							case KB_PC:    p_layout = type101_layout; break;
						}

						if( p_layout )
						{
							while( p_layout->n_layout != -1 )
							{
								if ( p_layout->n_layout == kbd_layout )
								{
									m_aKeyboardName = p_layout->p_description;
									break;
								}
								p_layout++;
							}
						}
					}
				}
				close(kbd);
			}
		}
#else
		int opcode, event, error;
		int major = XkbMajorVersion, minor = XkbMinorVersion;
		if( XkbQueryExtension( GetDisplay(), &opcode, &event,&error, &major, &minor ) )
		{
			XkbDescPtr pXkbDesc = NULL;
			// try X keyboard extension
			if( (pXkbDesc = XkbGetKeyboard( GetDisplay(), XkbAllComponentsMask, XkbUseCoreKbd )) )
			{
                const char* pAtom = NULL;
                if( pXkbDesc->names->groups[0] )
                {
                    pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->groups[0] );
                    m_aKeyboardName = pAtom;
                    XFree( (void*)pAtom );
                }
                else
                    m_aKeyboardName = "<unknown keyboard>";
#if OSL_DEBUG_LEVEL > 1
#define PRINT_ATOM( x ) { if( pXkbDesc->names->x ) { pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->x ); fprintf( stderr, "%s: %s\n", #x, pAtom ); XFree( (void*)pAtom ); } else fprintf( stderr, "%s: <nil>\n", #x ); }

				PRINT_ATOM( keycodes );
				PRINT_ATOM( geometry );
				PRINT_ATOM( symbols );
				PRINT_ATOM( types );
				PRINT_ATOM( compat );
				PRINT_ATOM( phys_symbols );

#define PRINT_ATOM_2( x ) { if( pXkbDesc->names->x[i] ) { pAtom = XGetAtomName( GetDisplay(), pXkbDesc->names->x[i] ); fprintf( stderr, "%s[%d]: %s\n", #x, i, pAtom ); XFree( (void*)pAtom ); } else fprintf( stderr, "%s[%d]: <nil>\n", #x, i ); }
				int i;
				for( i = 0; i < XkbNumVirtualMods; i++ )
					PRINT_ATOM_2( vmods );
				for( i = 0; i < XkbNumIndicators; i++ )
					PRINT_ATOM_2( indicators );
				for( i = 0; i < XkbNumKbdGroups; i++ )
					PRINT_ATOM_2( groups );
#endif
				XkbFreeKeyboard( pXkbDesc, XkbAllComponentsMask, True );
			}
		}
#endif
		if( ! m_aKeyboardName.Len() )
            m_aKeyboardName = "<unknown keyboard>";
	}
	return m_aKeyboardName.GetBuffer();
}
