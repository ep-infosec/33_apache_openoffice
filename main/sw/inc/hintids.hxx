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

#ifndef _HINTIDS_HXX
#define _HINTIDS_HXX

#include <tools/solar.h>		// fuer __FAR_DATA
#include <sal/types.h>			// for sal_Unicode
#include <svx/xdef.hxx>
#include "swdllapi.h"

// fuer SwTxtHints ohne Endindex wird folgendes Zeichen eingefuegt:
//JP 24.05.00: for the new UniCode Version:
#define CH_TXTATR_BREAKWORD		((sal_Unicode)0x01)
#define CH_TXTATR_INWORD		((sal_Unicode)0x02)
#define CH_TXTATR_TAB           ((sal_Unicode)'\t')
#define CH_TXTATR_NEWLINE       ((sal_Unicode)'\n')
#define CH_TXT_ATR_INPUTFIELDSTART ((sal_Unicode)0x04)
#define CH_TXT_ATR_INPUTFIELDEND ((sal_Unicode)0x05)

#define CH_TXT_ATR_FORMELEMENT ((sal_Unicode)0x06)

#define CH_TXT_ATR_FIELDSTART ((sal_Unicode)0x07)
#define CH_TXT_ATR_FIELDEND ((sal_Unicode)0x08)
#define CH_TXT_ATR_SUBST_FIELDSTART ("[")
#define CH_TXT_ATR_SUBST_FIELDEND ("]")

/*
 * Hier kommen erst mal die enums fuer die Hints
 */

#define HINT_BEGIN	1

#define POOLATTR_BEGIN		HINT_BEGIN
#define POOLATTR_END		RES_UNKNOWNATR_END

// Bereiche fuer die Id's der einzelnen Format-Attribute
// die Which-Werte fuer die Charakter-Format Attribute
enum RES_CHRATR
{
RES_CHRATR_BEGIN = HINT_BEGIN,
	RES_CHRATR_CASEMAP = RES_CHRATR_BEGIN,	//	1
	RES_CHRATR_CHARSETCOLOR,				//	2
	RES_CHRATR_COLOR,						//	3
	RES_CHRATR_CONTOUR,						//	4
	RES_CHRATR_CROSSEDOUT,					//	5
	RES_CHRATR_ESCAPEMENT,					//	6
	RES_CHRATR_FONT,						//	7
	RES_CHRATR_FONTSIZE,					//	8
	RES_CHRATR_KERNING, 					//	9
	RES_CHRATR_LANGUAGE,					// 10
	RES_CHRATR_POSTURE, 					// 11
	RES_CHRATR_PROPORTIONALFONTSIZE,		// 12
	RES_CHRATR_SHADOWED,					// 13
	RES_CHRATR_UNDERLINE,					// 14
	RES_CHRATR_WEIGHT,						// 15
	RES_CHRATR_WORDLINEMODE,				// 16
	RES_CHRATR_AUTOKERN,					// 17
	RES_CHRATR_BLINK,						// 18
	RES_CHRATR_NOHYPHEN,					// 19
	RES_CHRATR_NOLINEBREAK, 				// 20
	RES_CHRATR_BACKGROUND,					// 21
	RES_CHRATR_CJK_FONT,					// 22
	RES_CHRATR_CJK_FONTSIZE,				// 23
	RES_CHRATR_CJK_LANGUAGE,				// 24
	RES_CHRATR_CJK_POSTURE, 				// 25
	RES_CHRATR_CJK_WEIGHT,					// 26
	RES_CHRATR_CTL_FONT,					// 27
	RES_CHRATR_CTL_FONTSIZE,				// 28
	RES_CHRATR_CTL_LANGUAGE,				// 29
	RES_CHRATR_CTL_POSTURE, 				// 30
	RES_CHRATR_CTL_WEIGHT,					// 31
	RES_CHRATR_ROTATE,						// 32
	RES_CHRATR_EMPHASIS_MARK,				// 33
	RES_CHRATR_TWO_LINES, 					// 34
	RES_CHRATR_SCALEW,						// 35
	RES_CHRATR_RELIEF,						// 36
    RES_CHRATR_HIDDEN,                      // 37
	RES_CHRATR_OVERLINE,					// 38
	RES_CHRATR_DUMMY1,						// 39
	RES_CHRATR_DUMMY2,						// 40
	RES_CHRATR_BIDIRTL,						// 41
	RES_CHRATR_IDCTHINT,					// 42
RES_CHRATR_END
};

// diese Attribute stehen nur im SwpAttr-Array vom TextNode
enum RES_TXTATR
{
RES_TXTATR_BEGIN = RES_CHRATR_END,

/** text attributes with start and end.
   #i105453#:
   Hints (SwTxtAttr) with the same start and end position are sorted by
   WhichId, i.e., the TXTATR constants defined here.
   The text formatting (SwAttrIter) poses some requirements on TXTATR order:
   - AUTOFMT must precede CHARFMT, so that auto style can overwrite char style.
   - INETFMT must precede CHARFMT, so that link style can overwrite char style.
     (this is actually surprising: CHARFMT hints are not split at INETFMT
      hints on insertion, but on exporting to ODF. if CHARFMT would precede
      INETFMT, then exporting and importing will effectively change precedence)

   Nesting hints (SwTxtAttrNesting) also have requirements on TXTATR order,
   to ensure proper nesting (because CJK_RUBY and INETFMT have no CH_TXTATR):
   - INETFMT should precede CJK_RUBY (for UNO API it does not matter...)
   - META and METAFIELD must precede CJK_RUBY and INETFMT
 */
RES_TXTATR_WITHEND_BEGIN = RES_TXTATR_BEGIN ,
    RES_TXTATR_REFMARK = RES_TXTATR_WITHEND_BEGIN,  // 43
    RES_TXTATR_TOXMARK,                             // 44
    RES_TXTATR_META,                                // 45
    RES_TXTATR_METAFIELD,                           // 46
    RES_TXTATR_AUTOFMT,                             // 47
    RES_TXTATR_INETFMT,                             // 48
    RES_TXTATR_CHARFMT,                             // 49
    RES_TXTATR_CJK_RUBY,                            // 50
    RES_TXTATR_UNKNOWN_CONTAINER,                   // 51
    RES_TXTATR_INPUTFIELD,                          // 52
RES_TXTATR_WITHEND_END,

// alle TextAttribute ohne ein Ende
RES_TXTATR_NOEND_BEGIN = RES_TXTATR_WITHEND_END,
    RES_TXTATR_FIELD = RES_TXTATR_NOEND_BEGIN,      // 53
    RES_TXTATR_FLYCNT,                              // 54
    RES_TXTATR_FTN,                                 // 55
    RES_TXTATR_ANNOTATION,                          // 56
    RES_TXTATR_DUMMY3,                              // 57
    RES_TXTATR_DUMMY1,                              // 58
    RES_TXTATR_DUMMY2,                              // 59
RES_TXTATR_NOEND_END,
RES_TXTATR_END = RES_TXTATR_NOEND_END
};

enum RES_PARATR
{
RES_PARATR_BEGIN = RES_TXTATR_END,
    RES_PARATR_LINESPACING = RES_PARATR_BEGIN,      // 60
	RES_PARATR_ADJUST,								// 61
	RES_PARATR_SPLIT,								// 62
	RES_PARATR_ORPHANS, 							// 63
	RES_PARATR_WIDOWS,								// 64
	RES_PARATR_TABSTOP, 							// 65
	RES_PARATR_HYPHENZONE,							// 66
	RES_PARATR_DROP,								// 67
	RES_PARATR_REGISTER,							// 68
	RES_PARATR_NUMRULE, 							// 69
	RES_PARATR_SCRIPTSPACE,							// 70
	RES_PARATR_HANGINGPUNCTUATION,					// 71
	RES_PARATR_FORBIDDEN_RULES,						// 72
	RES_PARATR_VERTALIGN,							// 73
    RES_PARATR_SNAPTOGRID,                          // 74
    RES_PARATR_CONNECT_BORDER,                      // 75
    RES_PARATR_OUTLINELEVEL,                        // 76
RES_PARATR_END
};

// --> OD 2008-02-25 #refactorlists#
// list attributes for paragraphs.
// intentionally these list attributes are not contained in paragraph styles
enum RES_PARATR_LIST
{
RES_PARATR_LIST_BEGIN = RES_PARATR_END,
    RES_PARATR_LIST_ID = RES_PARATR_LIST_BEGIN,     // 77
    RES_PARATR_LIST_LEVEL,                          // 78
    RES_PARATR_LIST_ISRESTART,                      // 79
    RES_PARATR_LIST_RESTARTVALUE,                   // 80
    RES_PARATR_LIST_ISCOUNTED,                      // 81
RES_PARATR_LIST_END
};
// <--

enum RES_FRMATR
{
RES_FRMATR_BEGIN = RES_PARATR_LIST_END,
    RES_FILL_ORDER = RES_FRMATR_BEGIN,              // 82
    RES_FRM_SIZE,                                   // 83
    RES_PAPER_BIN,                                  // 84
    RES_LR_SPACE,                                   // 85
    RES_UL_SPACE,                                   // 86
    RES_PAGEDESC,                                   // 87
    RES_BREAK,                                      // 88
    RES_CNTNT,                                      // 89
    RES_HEADER,                                     // 90
    RES_FOOTER,                                     // 91
    RES_PRINT,                                      // 92
    RES_OPAQUE,                                     // 93
    RES_PROTECT,                                    // 94
    RES_SURROUND,                                   // 95
    RES_VERT_ORIENT,                                // 96
    RES_HORI_ORIENT,                                // 97
    RES_ANCHOR,                                     // 98
    RES_BACKGROUND,                                 // 99
    RES_BOX,                                        // 100
    RES_SHADOW,                                     // 101
    RES_FRMMACRO,                                   // 102
    RES_COL,                                        // 103
    RES_KEEP,                                       // 104
    RES_URL,                                        // 105
    RES_EDIT_IN_READONLY,                           // 106
    RES_LAYOUT_SPLIT,                               // 107
    RES_CHAIN,                                      // 108
    RES_TEXTGRID,                                   // 109
    RES_LINENUMBER  ,                               // 110
    RES_FTN_AT_TXTEND,                              // 111
    RES_END_AT_TXTEND,                              // 112
    RES_COLUMNBALANCE,                              // 113
    RES_FRAMEDIR,                                   // 114
    RES_HEADER_FOOTER_EAT_SPACING,                  // 115
    RES_ROW_SPLIT,                                  // 116
    RES_FOLLOW_TEXT_FLOW,                           // 117
    RES_COLLAPSING_BORDERS,                         // 118
    RES_WRAP_INFLUENCE_ON_OBJPOS,                   // 119
    RES_AUTO_STYLE,                                 // 120
    RES_FRMATR_STYLE_NAME,                          // 121
    RES_FRMATR_CONDITIONAL_STYLE_NAME,              // 122
RES_FRMATR_END
};

enum RES_GRFATR
{
RES_GRFATR_BEGIN = RES_FRMATR_END,
    RES_GRFATR_MIRRORGRF = RES_GRFATR_BEGIN,        // 123
    RES_GRFATR_CROPGRF,                             // 124

    RES_GRFATR_ROTATION,                            // 125
    RES_GRFATR_LUMINANCE,                           // 126
    RES_GRFATR_CONTRAST,                            // 127
    RES_GRFATR_CHANNELR,                            // 128
    RES_GRFATR_CHANNELG,                            // 129
    RES_GRFATR_CHANNELB,                            // 130
    RES_GRFATR_GAMMA,                               // 131
    RES_GRFATR_INVERT,                              // 132
    RES_GRFATR_TRANSPARENCY,                        // 133
    RES_GRFATR_DRAWMODE,                            // 134

    RES_GRFATR_DUMMY1,                              // 135
    RES_GRFATR_DUMMY2,                              // 136
    RES_GRFATR_DUMMY3,                              // 137
    RES_GRFATR_DUMMY4,                              // 138
    RES_GRFATR_DUMMY5,                              // 139
RES_GRFATR_END
};

enum RES_BOXATR
{
RES_BOXATR_BEGIN = RES_GRFATR_END,
    RES_BOXATR_FORMAT = RES_BOXATR_BEGIN,           // 140
    RES_BOXATR_FORMULA,                             // 141
    RES_BOXATR_VALUE,                               // 142
RES_BOXATR_END
};

enum RES_UNKNOWNATR
{
RES_UNKNOWNATR_BEGIN = RES_BOXATR_END,
    RES_UNKNOWNATR_CONTAINER = RES_UNKNOWNATR_BEGIN,// 143
RES_UNKNOWNATR_END
};



// ID-s fuer die Formate
enum RES_FMT
{
RES_FMT_BEGIN = RES_UNKNOWNATR_END,
	RES_CHRFMT = RES_FMT_BEGIN,                     // 144
	RES_FRMFMT,                                     // 145
	RES_FLYFRMFMT,                                  // 146
	RES_TXTFMTCOLL,                                 // 147
	RES_GRFFMTCOLL,                                 // 148
	RES_DRAWFRMFMT,                                 // 149
	RES_CONDTXTFMTCOLL,                             // 150
RES_FMT_END
};

// die ID's fuer Messages in den Formaten
enum RES_MSG
{
RES_MSG_BEGIN = RES_FMT_END,
	RES_OBJECTDYING = RES_MSG_BEGIN,                // 151
	RES_FMT_CHG,
	RES_ATTRSET_CHG,
	RES_FRM_SIZECHG,
	RES_TXTATR_FLDCHG,
	RES_TXTATR_EMPTYCHG,
	RES_INS_CHR,
	RES_INS_TXT,
	RES_DEL_CHR,
	RES_DEL_TXT,
	RES_UPDATE_ATTR,
	RES_PAGEDESC_FTNINFO,
	RES_REFMARKFLD_UPDATE,
	RES_DOCPOS_UPDATE,
	RES_TABLEFML_UPDATE,
	RES_UPDATEDDETBL,
	RES_TBLHEADLINECHG,
	RES_AUTOFMT_DOCNODE,
	RES_REPAINT,
	RES_DOC_DTOR,
	RES_SECTION_HIDDEN,
	RES_SECTION_NOT_HIDDEN,
	RES_GRAPHIC_ARRIVED,
	RES_GRAPHIC_PIECE_ARRIVED,
	RES_HIDDENPARA_PRINT,
	RES_CONDCOLL_CONDCHG,
	RES_VIRTPAGENUM_INFO,
    // --> OD 2008-02-25 #refactorlists#
//    RES_GETNUMNODES,
    // <--
	RES_GETLOWERNUMLEVEL,
	RES_RESET_FMTWRITTEN,
	RES_REMOVE_UNO_OBJECT,
	RES_GRF_REREAD_AND_INCACHE,
	RES_SECTION_RESETHIDDENFLAG,
	RES_FINDNEARESTNODE,
	RES_CONTENT_VISIBLE,
	RES_FOOTNOTE_DELETED,
	RES_REFMARK_DELETED,
	RES_TOXMARK_DELETED,
	RES_GRAPHIC_SWAPIN,
	RES_FIELD_DELETED,
	RES_NAME_CHANGED,
    RES_TITLE_CHANGED,
    RES_DESCRIPTION_CHANGED,
    RES_UNOCURSOR_LEAVES_SECTION,
    RES_LINKED_GRAPHIC_STREAM_ARRIVED,
RES_MSG_END
};


// eine ID fuer den RTF-Reader. Die Stylesheets werden wie
// Attribute behandelt, d.H. es gibt ein StyleSheet Attribut. Um
// nicht mit anderen Which()-Werten zu kollidieren, ist der Wert
// hier mit aufgefuehrt. (Auch das Hilfesystem definiert neue
// Attribute !!)
enum RES_FLTRATTR
{
RES_FLTRATTR_BEGIN = RES_MSG_END,
	RES_FLTR_STYLESHEET = RES_FLTRATTR_BEGIN,
	RES_FLTR_BOOKMARK,
	RES_FLTR_ANCHOR,
	RES_FLTR_BORDER,
	RES_FLTR_NUMRULE,
	RES_FLTR_NUMRULE_NUM,
	RES_FLTR_SDR_ANCHOR,
	RES_FLTR_TOX,
	RES_FLTR_SECTION,
	RES_FLTR_REDLINE,
	RES_FLTR_SCRIPTTYPE,
RES_FLTRATTR_END
};

#define RES_TBX_DUMMY RES_FLTRATTR_END + 1

#define HINT_END RES_TBX_DUMMY

// Fehler-Erkennung !!
#define INVALID_HINT HINT_END
#define RES_WHICHHINT_END HINT_END


inline bool isATR(const sal_uInt16 nWhich)
{
    return (RES_CHRATR_BEGIN <= nWhich) && (RES_UNKNOWNATR_END > nWhich);
}
inline bool isCHRATR(const sal_uInt16 nWhich)
{
    return (RES_CHRATR_BEGIN <= nWhich) && (RES_CHRATR_END > nWhich);
}
inline bool isTXTATR_WITHEND(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_WITHEND_BEGIN <= nWhich)
        && (RES_TXTATR_WITHEND_END > nWhich);
}
inline bool isTXTATR_NOEND(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_NOEND_BEGIN <= nWhich)
        && (RES_TXTATR_NOEND_END > nWhich);
}
inline bool isTXTATR(const sal_uInt16 nWhich)
{
    return (RES_TXTATR_BEGIN <= nWhich) && (RES_TXTATR_END > nWhich);
}
inline bool isPARATR(const sal_uInt16 nWhich)
{
    return (RES_PARATR_BEGIN <= nWhich) && (RES_PARATR_END > nWhich);
}
inline bool isPARATR_LIST(const sal_uInt16 nWhich)
{
    return (RES_PARATR_LIST_BEGIN <= nWhich) && (RES_PARATR_LIST_END > nWhich); 
}
inline bool isFRMATR(const sal_uInt16 nWhich)
{
    return (RES_FRMATR_BEGIN <= nWhich) && (RES_FRMATR_END > nWhich);
}
inline bool isDrawingLayerAttribute(const sal_uInt16 nWhich) //UUUU
{
    return (XATTR_FILL_FIRST <= nWhich) && (XATTR_FILL_LAST >= nWhich);
}
inline bool isGRFATR(const sal_uInt16 nWhich)
{
    return (RES_GRFATR_BEGIN <= nWhich) && (RES_GRFATR_END > nWhich);
}
inline bool isBOXATR(const sal_uInt16 nWhich)
{
    return (RES_BOXATR_BEGIN <= nWhich) && (RES_BOXATR_END > nWhich);
}
inline bool isUNKNOWNATR(const sal_uInt16 nWhich)
{
    return (RES_UNKNOWNATR_BEGIN <= nWhich) && (RES_UNKNOWNATR_END > nWhich);
}


/*
 * hole aus der statischen Default-Attribut Tabelle ueber den Which-Wert
 * das entsprechende default Attribut.
 * Ist keines vorhanden, returnt ein 0-Pointer !!!
 *
 * Diese Funktion ist im Init.CXX implementiert. Damit die Formate darauf
 * zugreifen koennen, ist sie hier als extern deklariert.
 * Im PRODUCT ist das Teil inline.
 */
class SfxPoolItem;
struct SfxItemInfo;
typedef SfxPoolItem* SwDfltAttrTab[ POOLATTR_END - POOLATTR_BEGIN  ];

extern SwDfltAttrTab __FAR_DATA aAttrTab;
extern SfxItemInfo	 __FAR_DATA aSlotTab[];

SW_DLLPUBLIC const SfxPoolItem* GetDfltAttr( sal_uInt16 nWhich );

SW_DLLPUBLIC sal_uInt16 GetWhichOfScript( sal_uInt16 nWhich, sal_uInt16 nScript );

// return for the given TextAttribut without an end the correct character.
// This function returns
//		CH_TXTATR_BREAKWORD for Textattribut which breaks a word (default)
//		CH_TXTATR_INWORD	for Textattribut which dont breaks a word
class SwTxtAttr;
sal_Unicode GetCharOfTxtAttr( const SwTxtAttr& rAttr );

// alle Sets stehen im init.cxx

// AttrSet-Range fuer die 3 Break-Attribute
extern sal_uInt16 __FAR_DATA aBreakSetRange[];
// AttrSet-Range fuer die TxtFmtColl
extern sal_uInt16 __FAR_DATA aTxtFmtCollSetRange[];
// AttrSet-Range fuer die GrfFmtColl
extern sal_uInt16 __FAR_DATA aGrfFmtCollSetRange[];
// AttrSet-Range fuer die TextNode
SW_DLLPUBLIC extern sal_uInt16 __FAR_DATA aTxtNodeSetRange[];
// AttrSet-Range fuer die NoTxtNode
extern sal_uInt16 __FAR_DATA aNoTxtNodeSetRange[];
// AttrSet-Range fuer die SwTable
extern sal_uInt16 __FAR_DATA aTableSetRange[];
// AttrSet-Range fuer die SwTableLine
extern sal_uInt16 __FAR_DATA aTableLineSetRange[];
// AttrSet-Range fuer die SwTableBox
extern sal_uInt16 __FAR_DATA aTableBoxSetRange[];
// AttrSet-Range fuer die SwFrmFmt
SW_DLLPUBLIC extern sal_uInt16 __FAR_DATA aFrmFmtSetRange[];
// AttrSet-Range fuer die SwCharFmt
extern sal_uInt16 __FAR_DATA aCharFmtSetRange[];
// AttrSet-Range fuer die character autostyles
extern sal_uInt16 __FAR_DATA aCharAutoFmtSetRange[];
// AttrSet-Range fuer die SwPageDescFmt
extern sal_uInt16 __FAR_DATA aPgFrmFmtSetRange[];

// check if ID is InRange of AttrSet-Ids
sal_Bool IsInRange( const sal_uInt16* pRange, const sal_uInt16 nId );

#endif
