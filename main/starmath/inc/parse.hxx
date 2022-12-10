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


#ifndef PARSE_HXX
#define PARSE_HXX


#include <vcl/svapp.hxx>
#include <tools/stack.hxx>
#include <tools/list.hxx>
#include <tools/string.hxx>

#include <set>
#include <stack>
#include <list>

#include "types.hxx"

class SmNode;
class SmDocShell;

//////////////////////////////////////////////////////////////////////

// TokenGroups
#define TGOPER			0x00000001
#define TGRELATION		0x00000002
#define TGSUM			0x00000004
#define TGPRODUCT		0x00000008
#define TGUNOPER		0x00000010
#define TGPOWER 		0x00000020
#define TGATTRIBUT		0x00000040
#define TGALIGN 		0x00000080
#define TGFUNCTION		0x00000100
#define TGBLANK 		0x00000200
#define TGLBRACES		0x00000400
#define TGRBRACES		0x00000800
#define TGCOLOR 		0x00001000
#define TGFONT			0x00002000
#define TGSTANDALONE	0x00004000
#define TGDISCARDED		0x00008000
#define TGLIMIT			0x00010000
#define TGFONTATTR		0x00020000


enum SmTokenType
{
	TEND,			TLGROUP,		TRGROUP,		TLPARENT,		TRPARENT,
	TLBRACKET,		TRBRACKET,		TPLUS,			TMINUS,			TMULTIPLY,
	TDIVIDEBY,		TASSIGN,		TPOUND,			TSPECIAL,		TSLASH,
	TBACKSLASH,		TBLANK,			TSBLANK,		TRSUB,			TRSUP,
	TCSUB,			TCSUP,			TLSUB,			TLSUP,			TGT,
	TLT,			TAND,			TOR,			TINTERSECT,		TUNION,
	TNEWLINE,		TBINOM,			TFROM,			TTO,			TINT,
	TSUM,			TOPER,			TABS,			TSQRT,			TFACT,
	TNROOT,			TOVER,			TTIMES,			TGE,			TLE,
	TGG,			TLL,			TDOTSAXIS,		TDOTSLOW,		TDOTSVERT,
	TDOTSDIAG,		TDOTSUP,		TDOTSDOWN,		TACUTE,			TBAR,
	TBREVE,			TCHECK,			TCIRCLE,		TDOT,			TDDOT,
	TDDDOT,			TGRAVE,			THAT,			TTILDE,			TVEC,
	TUNDERLINE,		TOVERLINE,		TOVERSTRIKE,	TITALIC,		TNITALIC,
	TBOLD,			TNBOLD,			TPHANTOM,		TFONT,			TSIZE,
	TCOLOR,			TALIGNL,		TALIGNC,		TALIGNR,		TLEFT,
	TRIGHT,			TLANGLE,		TLBRACE,		TLLINE,			TLDLINE,
	TLCEIL,			TLFLOOR,		TNONE,			TMLINE,			TRANGLE,
	TRBRACE,		TRLINE,			TRDLINE,		TRCEIL,			TRFLOOR,
	TSIN,			TCOS,			TTAN,			TCOT,			TFUNC,
	TSTACK,			TMATRIX,		TMATFORM,		TDPOUND,		TPLACE,
	TTEXT,			TNUMBER,		TCHARACTER,		TIDENT,			TNEQ,
	TEQUIV,			TDEF,			TPROP,			TSIM,			TSIMEQ,
	TAPPROX,		TPARALLEL,		TORTHO,			TIN,			TNOTIN,
	TSUBSET,		TSUBSETEQ,		TSUPSET,		TSUPSETEQ,		TPLUSMINUS,
	TMINUSPLUS,		TOPLUS,			TOMINUS,		TDIV,			TOTIMES,
	TODIVIDE,		TTRANSL,		TTRANSR,		TIINT,			TIIINT,
	TLINT,			TLLINT,			TLLLINT,		TPROD,			TCOPROD,
	TFORALL,		TEXISTS,		TLIM,			TNABLA,			TTOWARD,
	TSINH,			TCOSH,			TTANH,			TCOTH,			TASIN,
	TACOS,			TATAN,			TLN,			TLOG,			TUOPER,
	TBOPER,			TBLACK,			TWHITE,			TRED,			TGREEN,
	TBLUE,			TCYAN,			TMAGENTA,		TYELLOW,        TSILVER,
    TGRAY,          TMAROON,        TPURPLE,        TLIME,          TOLIVE,
    TNAVY,          TTEAL,          TAQUA,          TFUCHSIA,		TFIXED,
	TSANS,			TSERIF,			TPOINT,			TASINH,			TACOSH,
	TATANH,			TACOTH,			TACOT,			TEXP,			TCDOT,
	TODOT,			TLESLANT,		TGESLANT,		TNSUBSET,		TNSUPSET,
	TNSUBSETEQ,		TNSUPSETEQ,		TPARTIAL,		TNEG,			TNI,
	TBACKEPSILON,	TALEPH,			TIM,			TRE,			TWP,
	TEMPTYSET,		TINFINITY,		TESCAPE,		TLIMSUP,		TLIMINF,
	TNDIVIDES,		TDRARROW,		TDLARROW,		TDLRARROW,		TUNDERBRACE,
	TOVERBRACE,		TCIRC,			TTOP,			THBAR,			TLAMBDABAR,
	TLEFTARROW,		TRIGHTARROW,	TUPARROW,		TDOWNARROW,		TDIVIDES,
	TNDIBVIDES,		TSETN,			TSETZ,			TSETQ,			TSETR,
	TSETC,			TWIDEVEC,		TWIDETILDE,		TWIDEHAT,		TWIDESLASH,
	TWIDEBACKSLASH, TLDBRACKET,		TRDBRACKET,     TNOSPACE,
	TUNKNOWN,		TDEBUG
};


struct SmToken
{
    
    String          aText;      // token text
    SmTokenType     eType;      // token info
    sal_Unicode		cMathChar;
    
    // parse-help info
    sal_uLong       nGroup;
    sal_uInt16      nLevel;
    
    // token position
    sal_uInt16      nRow;
    xub_StrLen      nCol;

	SmToken();
};


enum SmParseError
{
	PE_NONE,					PE_UNEXPECTED_END_OF_INPUT,
	PE_UNEXPECTED_CHAR,			PE_UNEXPECTED_TOKEN,
	PE_FUNC_EXPECTED,			PE_UNOPER_EXPECTED,
	PE_BINOPER_EXPECTED,		PE_SYMBOL_EXPECTED,
	PE_IDENTIFIER_EXPECTED,		PE_POUND_EXPECTED,
	PE_COLOR_EXPECTED,			PE_LGROUP_EXPECTED,
	PE_RGROUP_EXPECTED,			PE_LBRACE_EXPECTED,
	PE_RBRACE_EXPECTED,			PE_PARENT_MISMATCH,
	PE_RIGHT_EXPECTED,			PE_FONT_EXPECTED,
	PE_SIZE_EXPECTED,			PE_DOUBLE_ALIGN,
	PE_DOUBLE_SUBSUPSCRIPT
};


struct SmErrorDesc
{
	SmParseError  Type;
	SmNode		 *pNode;
	String		  Text;
};


DECLARE_STACK(SmNodeStack,  SmNode *)
DECLARE_LIST(SmErrDescList, SmErrorDesc *)

/**************************************************************************/

// defines possible conversions of the formula text from the format of
// one release to the one of another.
enum SmConvert
{
    CONVERT_NONE,
    CONVERT_40_TO_50,
    CONVERT_50_TO_60,
    CONVERT_60_TO_50
};


class SmParser
{
    String          m_aBufferString;
    SmToken         m_aCurToken;
    SmNodeStack     m_aNodeStack;
    SmErrDescList   m_aErrDescList;
    int             m_nCurError;
    LanguageType    m_nLang;
    xub_StrLen      m_nBufferIndex,
                    m_nTokenIndex;
    sal_uInt16          m_Row,
                    m_nColOff;
    SmConvert       m_eConversion;
    sal_Bool        m_bImportSymNames,
                    m_bExportSymNames;

    // map of used symbols (used to reduce file size by exporting only actually used symbols)
    std::set< rtl::OUString >   m_aUsedSymbols;

	// declare copy-constructor and assignment-operator private
	SmParser(const SmParser &);
	SmParser & operator = (const SmParser &);

protected:
#if OSL_DEBUG_LEVEL
	sal_Bool			IsDelimiter( const String &rTxt, xub_StrLen nPos );
#endif
	void			NextToken();
    xub_StrLen      GetTokenIndex() const   { return m_nTokenIndex; }
	void 			Insert(const String &rText, sal_uInt16 nPos);
    void            Replace( sal_uInt16 nPos, sal_uInt16 nLen, const String &rText );

	inline sal_Bool		TokenInGroup(sal_uLong nGroup);

	// grammar
	void	Table();
	void	Line();
	void	Expression();
	void	Relation();
	void	Sum();
	void	Product();
	void	SubSup(sal_uLong nActiveGroup);
	void	OpSubSup();
	void 	Power();
	void	Blank();
	void	Term();
	void	Escape();
	void	Operator();
	void	Oper();
	void	UnOper();
	void	Align();
	void	FontAttribut();
	void	Attribut();
	void	Font();
	void	FontSize();
	void	Color();
	void	Brace();
	void	Bracebody(sal_Bool bIsLeftRight);
	void	Function();
	void	Binom();
	void	Stack();
	void	Matrix();
	void	Special();
	void	GlyphSpecial();
	// end of grammar
    
    LanguageType    GetLanguage() const { return m_nLang; }
    void            SetLanguage( LanguageType nNewLang ) { m_nLang = nNewLang; }

	void	Error(SmParseError Error);

    void    ClearUsedSymbols()                              { m_aUsedSymbols.clear(); }
    void    AddToUsedSymbols( const String &rSymbolName )   { m_aUsedSymbols.insert( rSymbolName ); }

public:
                 SmParser();

	SmNode		*Parse(const String &rBuffer);

    const String & GetText() const { return m_aBufferString; };

    SmConvert    GetConversion() const              { return m_eConversion; }
    void         SetConversion(SmConvert eConv)     { m_eConversion = eConv; }

    sal_Bool     IsImportSymbolNames() const        { return m_bImportSymNames; }
    void         SetImportSymbolNames(sal_Bool bVal)    { m_bImportSymNames = bVal; }
    sal_Bool     IsExportSymbolNames() const        { return m_bExportSymNames; }
    void         SetExportSymbolNames(sal_Bool bVal)    { m_bExportSymNames = bVal; }

	sal_uInt16		 AddError(SmParseError Type, SmNode *pNode);

	const SmErrorDesc *	NextError();
	const SmErrorDesc *	PrevError();
    const SmErrorDesc * GetError(sal_uInt16 i = 0xFFFF);

    bool    IsUsedSymbol( const String &rSymbolName ) const { return m_aUsedSymbols.find( rSymbolName ) != m_aUsedSymbols.end(); }
    std::set< rtl::OUString >   GetUsedSymbols() const      { return m_aUsedSymbols; }
};


inline sal_Bool SmParser::TokenInGroup(sal_uLong nGroup)
{
    return (m_aCurToken.nGroup & nGroup) ? sal_True : sal_False;
}


#endif

