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
#include "precompiled_basic.hxx"
#include <basic/sbx.hxx>
#include "sbcomp.hxx"
#include <com/sun/star/script/ModuleType.hpp>

struct SbiParseStack {				// "Stack" fuer Statement-Blocks
	SbiParseStack* pNext;  			// Chain
	SbiExprNode* pWithVar;			// Variable fuer WITH
	SbiToken eExitTok;				// Exit-Token
	sal_uInt32  nChain;					// JUMP-Chain
};

struct SbiStatement {
	SbiToken eTok;
	void( SbiParser::*Func )();		// Verarbeitungsroutine
	sal_Bool  bMain;					// sal_True: ausserhalb SUBs OK
	sal_Bool  bSubr;					// sal_True: in SUBs OK
};

#define	Y	sal_True
#define	N	sal_False

static SbiStatement StmntTable [] = {
{ CALL,		&SbiParser::Call,   	N, Y, }, // CALL
{ CLOSE,	&SbiParser::Close,		N, Y, }, // CLOSE
{ _CONST_,	&SbiParser::Dim, 		Y, Y, }, // CONST
{ DECLARE,	&SbiParser::Declare,	Y, N, }, // DECLARE
{ DEFBOOL,	&SbiParser::DefXXX,		Y, N, }, // DEFBOOL
{ DEFCUR,	&SbiParser::DefXXX,		Y, N, }, // DEFCUR
{ DEFDATE,	&SbiParser::DefXXX,		Y, N, }, // DEFDATE
{ DEFDBL,	&SbiParser::DefXXX,		Y, N, }, // DEFDBL
{ DEFERR,	&SbiParser::DefXXX,		Y, N, }, // DEFERR
{ DEFINT,	&SbiParser::DefXXX,		Y, N, }, // DEFINT
{ DEFLNG,	&SbiParser::DefXXX,		Y, N, }, // DEFLNG
{ DEFOBJ,	&SbiParser::DefXXX,		Y, N, }, // DEFOBJ
{ DEFSNG,	&SbiParser::DefXXX,		Y, N, }, // DEFSNG
{ DEFSTR,	&SbiParser::DefXXX,		Y, N, }, // DEFSTR
{ DEFVAR,	&SbiParser::DefXXX,		Y, N, }, // DEFVAR
{ DIM,		&SbiParser::Dim,		Y, Y, }, // DIM
{ DO,		&SbiParser::DoLoop,		N, Y, }, // DO
{ ELSE,		&SbiParser::NoIf,		N, Y, }, // ELSE
{ ELSEIF,	&SbiParser::NoIf,		N, Y, }, // ELSEIF
{ ENDIF,	&SbiParser::NoIf,		N, Y, }, // ENDIF
{ END,		&SbiParser::Stop,		N, Y, }, // END
{ ENUM,		&SbiParser::Enum,		Y, N, }, // TYPE
{ ERASE,	&SbiParser::Erase,		N, Y, }, // ERASE
{ _ERROR_,	&SbiParser::ErrorStmnt,	N, Y, }, // ERROR
{ EXIT,		&SbiParser::Exit,		N, Y, }, // EXIT
{ FOR,		&SbiParser::For,		N, Y, }, // FOR
{ FUNCTION,	&SbiParser::SubFunc,	Y, N, }, // FUNCTION
{ GOSUB,	&SbiParser::Goto,		N, Y, }, // GOSUB
{ GLOBAL,	&SbiParser::Dim,		Y, N, }, // GLOBAL
{ GOTO,		&SbiParser::Goto,		N, Y, }, // GOTO
{ IF,		&SbiParser::If,			N, Y, }, // IF
{ IMPLEMENTS, &SbiParser::Implements, Y, N, }, // IMPLEMENTS
{ INPUT,	&SbiParser::Input,		N, Y, }, // INPUT
{ LET,		&SbiParser::Assign,		N, Y, }, // LET
{ LINE,		&SbiParser::Line,		N, Y, }, // LINE, -> LINE INPUT (#i92642)
{ LINEINPUT,&SbiParser::LineInput,	N, Y, }, // LINE INPUT
{ LOOP,		&SbiParser::BadBlock,	N, Y, }, // LOOP
{ LSET,		&SbiParser::LSet,		N, Y, }, // LSET
{ NAME,		&SbiParser::Name,		N, Y, }, // NAME
{ NEXT,		&SbiParser::BadBlock,	N, Y, }, // NEXT
{ ON,		&SbiParser::On,			N, Y, }, // ON
{ OPEN,		&SbiParser::Open,		N, Y, }, // OPEN
{ OPTION,	&SbiParser::Option,    	Y, N, }, // OPTION
{ PRINT,	&SbiParser::Print,		N, Y, }, // PRINT
{ PRIVATE,	&SbiParser::Dim,  		Y, N, }, // PRIVATE
{ PROPERTY,	&SbiParser::SubFunc,	Y, N, }, // FUNCTION
{ PUBLIC,	&SbiParser::Dim,  		Y, N, }, // PUBLIC
{ REDIM,	&SbiParser::ReDim,  	N, Y, }, // DIM
{ RESUME,	&SbiParser::Resume,		N, Y, }, // RESUME
{ RETURN,	&SbiParser::Return,		N, Y, }, // RETURN
{ RSET,		&SbiParser::RSet,		N, Y, }, // RSET
{ SELECT,	&SbiParser::Select,		N, Y, }, // SELECT
{ SET,		&SbiParser::Set,		N, Y, }, // SET
{ STATIC,	&SbiParser::Static,		Y, Y, }, // STATIC
{ STOP,		&SbiParser::Stop,		N, Y, }, // STOP
{ SUB,		&SbiParser::SubFunc,	Y, N, }, // SUB
{ TYPE,		&SbiParser::Type,		Y, N, }, // TYPE
{ UNTIL,	&SbiParser::BadBlock,	N, Y, }, // UNTIL
{ WHILE,	&SbiParser::While,		N, Y, }, // WHILE
{ WEND,		&SbiParser::BadBlock,	N, Y, }, // WEND
{ WITH,		&SbiParser::With,		N, Y, }, // WITH
{ WRITE,	&SbiParser::Write,		N, Y, }, // WRITE

{ NIL, NULL, N, N }
};


#ifdef _MSC_VER
// 'this' : used in base member initializer list
#pragma warning( disable: 4355 )
#endif

SbiParser::SbiParser( StarBASIC* pb, SbModule* pm )
		: SbiTokenizer( pm->GetSource32(), pb ),
		  aGblStrings( this ),
		  aLclStrings( this ),
		  aGlobals( aGblStrings, SbGLOBAL ),
		  aPublics( aGblStrings, SbPUBLIC ),
		  aRtlSyms( aGblStrings, SbRTL ),
		  aGen( *pm, this, 1024 )
{
	pBasic	 = pb;
	eCurExpr = SbSYMBOL;
	eEndTok  = NIL;
	pProc    = NULL;
	pStack   = NULL;
	pWithVar = NULL;
	nBase	 = 0;
	bText	 =
	bGblDefs =
	bNewGblDefs =
	bSingleLineIf =
	bExplicit = sal_False;
	bClassModule = ( pm->GetModuleType() == com::sun::star::script::ModuleType::CLASS );
	OSL_TRACE("Parser - %s, bClassModule %d", rtl::OUStringToOString( pm->GetName(), RTL_TEXTENCODING_UTF8 ).getStr(), bClassModule );
	pPool	 = &aPublics;
	for( short i = 0; i < 26; i++ )
		eDefTypes[ i ] = SbxVARIANT;    // Kein expliziter Defaulttyp

	aPublics.SetParent( &aGlobals );
	aGlobals.SetParent( &aRtlSyms );

	// Die globale Chainkette faengt bei Adresse 0 an:
	nGblChain = aGen.Gen( _JUMP, 0 );

	rTypeArray = new SbxArray; // Array fuer Benutzerdefinierte Typen
	rEnumArray = new SbxArray; // Array for Enum types
	bVBASupportOn = pm->IsVBACompat();
	if ( bVBASupportOn )
		EnableCompatibility();

}


// Ist  Teil der Runtime-Library?
SbiSymDef* SbiParser::CheckRTLForSym( const String& rSym, SbxDataType eType )
{
	SbxVariable* pVar = GetBasic()->GetRtl()->Find( rSym, SbxCLASS_DONTCARE );
	SbiSymDef* pDef = NULL;
	if( pVar )
	{
		if( pVar->IsA( TYPE(SbxMethod) ) )
		{
			SbiProcDef* pProc_ = aRtlSyms.AddProc( rSym );
			SbxMethod* pMethod = (SbxMethod*) pVar;
			if ( pMethod && pMethod->IsRuntimeFunction() )
			{
				pProc_->SetType( pMethod->GetRuntimeFunctionReturnType() );
			}
			else
			{
				pProc_->SetType( pVar->GetType() );
			}
			pDef = pProc_;
		}
		else
		{
			pDef = aRtlSyms.AddSym( rSym );
			pDef->SetType( eType );
		}
	}
	return pDef;
}

// Globale Chainkette schliessen

sal_Bool SbiParser::HasGlobalCode()
{
	if( bGblDefs && nGblChain )
	{
		aGen.BackChain( nGblChain );
		aGen.Gen( _LEAVE );
		// aGen.Gen( _STOP );
		nGblChain = 0;
	}
	return bGblDefs;
}

void SbiParser::OpenBlock( SbiToken eTok, SbiExprNode* pVar )
{
	SbiParseStack* p = new SbiParseStack;
	p->eExitTok = eTok;
	p->nChain   = 0;
	p->pWithVar = pWithVar;
	p->pNext    = pStack;
	pStack      = p;
	pWithVar    = pVar;

	// #29955 for-Schleifen-Ebene pflegen
	if( eTok == FOR )
		aGen.IncForLevel();
}

void SbiParser::CloseBlock()
{
	if( pStack )
	{
		SbiParseStack* p = pStack;

		// #29955 for-Schleifen-Ebene pflegen
		if( p->eExitTok == FOR )
			aGen.DecForLevel();

		aGen.BackChain( p->nChain );
		pStack = p->pNext;
		pWithVar = p->pWithVar;
		delete p;
	}
}

// EXIT ...

void SbiParser::Exit()
{
	SbiToken eTok = Next();
	for( SbiParseStack* p = pStack; p; p = p->pNext )
	{
		SbiToken eExitTok = p->eExitTok;
		if( eTok == eExitTok || 
			(eTok == PROPERTY && (eExitTok == GET || eExitTok == LET) ) )	// #i109051
		{
			p->nChain = aGen.Gen( _JUMP, p->nChain );
			return;
		}
	}
	if( pStack )
		Error( SbERR_EXPECTED, pStack->eExitTok );
	else
		Error( SbERR_BAD_EXIT );
}

sal_Bool SbiParser::TestSymbol( sal_Bool bKwdOk )
{
	Peek();
	if( eCurTok == SYMBOL || ( bKwdOk && IsKwd( eCurTok ) ) )
	{
		Next(); return sal_True;
	}
	Error( SbERR_SYMBOL_EXPECTED );
	return sal_False;
}

// Testen auf ein bestimmtes Token

sal_Bool SbiParser::TestToken( SbiToken t )
{
	if( Peek() == t )
	{
		Next(); return sal_True;
	}
	else
	{
		Error( SbERR_EXPECTED, t );
		return sal_False;
	}
}

// Testen auf Komma oder EOLN

sal_Bool SbiParser::TestComma()
{
	SbiToken eTok = Peek();
	if( IsEoln( eTok ) )
	{
		Next();
		return sal_False;
	}
	else if( eTok != COMMA )
	{
		Error( SbERR_EXPECTED, COMMA );
		return sal_False;
	}
	Next();
	return sal_True;
}

// Testen, ob EOLN vorliegt

void SbiParser::TestEoln()
{
	if( !IsEoln( Next() ) )
	{
		Error( SbERR_EXPECTED, EOLN );
		while( !IsEoln( Next() ) ) {}
	}
}

// Parsing eines Statement-Blocks
// Das Parsing laeuft bis zum Ende-Token.

void SbiParser::StmntBlock( SbiToken eEnd )
{
	SbiToken xe = eEndTok;
	eEndTok = eEnd;
	while( !bAbort && Parse() ) {}
	eEndTok = xe;
	if( IsEof() )
	{
		Error( SbERR_BAD_BLOCK, eEnd );
		bAbort = sal_True;
	}
}

// Die Hauptroutine. Durch wiederholten Aufrufs dieser Routine wird
// die Quelle geparst. Returnwert sal_False bei Ende/Fehlern.

sal_Bool SbiParser::Parse()
{
	if( bAbort ) return sal_False;

	EnableErrors();

	bErrorIsSymbol = false;
	Peek();
	bErrorIsSymbol = true;
	// Dateiende?
	if( IsEof() )
	{
		// AB #33133: Falls keine Sub angelegt wurde, muss hier
		// der globale Chain abgeschlossen werden!
		// AB #40689: Durch die neue static-Behandlung kann noch
		// ein nGblChain vorhanden sein, daher vorher abfragen
		if( bNewGblDefs && nGblChain == 0 )
			nGblChain = aGen.Gen( _JUMP, 0 );
		return sal_False;
	}

	// Leerstatement?
	if( IsEoln( eCurTok ) )
	{
		Next(); return sal_True;
	}

	if( !bSingleLineIf && MayBeLabel( sal_True ) )
	{
		// Ist ein Label
		if( !pProc )
			Error( SbERR_NOT_IN_MAIN, aSym );
		else
			pProc->GetLabels().Define( aSym );
		Next(); Peek();
		// Leerstatement?
		if( IsEoln( eCurTok ) )
		{
			Next(); return sal_True;
		}
	}

	// Ende des Parsings?
	if( eCurTok == eEndTok ||
		( bVBASupportOn	&&		// #i109075
		  (eCurTok == ENDFUNC || eCurTok == ENDPROPERTY || eCurTok == ENDSUB) &&
		  (eEndTok == ENDFUNC || eEndTok == ENDPROPERTY || eEndTok == ENDSUB) ) )
	{
		Next();
		if( eCurTok != NIL )
			aGen.Statement();
		return sal_False;
	}

	// Kommentar?
	if( eCurTok == REM )
	{
		Next(); return sal_True;
	}

	// Kommt ein Symbol, ist es entweder eine Variable( LET )
	// oder eine SUB-Prozedur( CALL ohne Klammern )
	// DOT fuer Zuweisungen im WITH-Block: .A=5
	if( eCurTok == SYMBOL || eCurTok == DOT )
	{
		if( !pProc )
			Error( SbERR_EXPECTED, SUB );
		else
		{
			// Damit Zeile & Spalte stimmen...
			Next();
			Push( eCurTok );
			aGen.Statement();
				Symbol();
		}
	}
	else
	{
		Next();

		// Hier folgen nun die Statement-Parser.

		SbiStatement* p;
		for( p = StmntTable; p->eTok != NIL; p++ )
			if( p->eTok == eCurTok )
				break;
		if( p->eTok != NIL )
		{
			if( !pProc && !p->bMain )
				Error( SbERR_NOT_IN_MAIN, eCurTok );
			else if( pProc && !p->bSubr )
				Error( SbERR_NOT_IN_SUBR, eCurTok );
			else
			{
				// globalen Chain pflegen
				// AB #41606/#40689: Durch die neue static-Behandlung kann noch
				// ein nGblChain vorhanden sein, daher vorher abfragen
				if( bNewGblDefs && nGblChain == 0 && 
					( eCurTok == SUB || eCurTok == FUNCTION || eCurTok == PROPERTY ) )
				{
					nGblChain = aGen.Gen( _JUMP, 0 );
					bNewGblDefs = sal_False;
				}
				// Statement-Opcode bitte auch am Anfang einer Sub
				if( ( p->bSubr && (eCurTok != STATIC || Peek() == SUB || Peek() == FUNCTION ) ) ||
						eCurTok == SUB || eCurTok == FUNCTION )
					aGen.Statement();
				(this->*( p->Func ) )();
				SbxError nSbxErr = SbxBase::GetError();
				if( nSbxErr )
					SbxBase::ResetError(), Error( (SbError)nSbxErr );
			}
		}
		else
			Error( SbERR_UNEXPECTED, eCurTok );
	}

	// Test auf Ende des Statements:
	// Kann auch ein ELSE sein, da vor dem ELSE kein : stehen muss!

	if( !IsEos() )
	{
		Peek();
		if( !IsEos() && eCurTok != ELSE )
		{
			// falls das Parsing abgebrochen wurde, bis zum ":" vorgehen:
			Error( SbERR_UNEXPECTED, eCurTok );
			while( !IsEos() ) Next();
		}
	}
	// Der Parser bricht am Ende ab, das naechste Token ist noch nicht
	// geholt!
	return sal_True;
}

// Innerste With-Variable liefern
SbiExprNode* SbiParser::GetWithVar()
{
	if( pWithVar )
		return pWithVar;

	// Sonst im Stack suchen
	SbiParseStack* p = pStack;
	while( p )
	{
		// LoopVar kann zur Zeit nur fuer with sein
		if( p->pWithVar )
			return p->pWithVar;
		p = p->pNext;
	}
	return NULL;
}


// Zuweisung oder Subroutine Call

void SbiParser::Symbol( const KeywordSymbolInfo* pKeywordSymbolInfo )
{
	SbiExprMode eMode = bVBASupportOn ? EXPRMODE_STANDALONE : EXPRMODE_STANDARD;
	SbiExpression aVar( this, SbSYMBOL, eMode, pKeywordSymbolInfo );

	bool bEQ = ( Peek() == EQ );
	if( !bEQ && bVBASupportOn && aVar.IsBracket() )
		Error( SbERR_EXPECTED, "=" );

	RecursiveMode eRecMode = ( bEQ ? PREVENT_CALL : FORCE_CALL );
	bool bSpecialMidHandling = false;
	SbiSymDef* pDef = aVar.GetRealVar();
	if( bEQ && pDef && pDef->GetScope() == SbRTL )
	{
		String aRtlName = pDef->GetName();
		if( aRtlName.EqualsIgnoreCaseAscii("Mid") )
		{
			SbiExprNode* pExprNode = aVar.GetExprNode();
			// SbiNodeType eNodeType;
			if( pExprNode && pExprNode->GetNodeType() == SbxVARVAL )
			{
				SbiExprList* pPar = pExprNode->GetParameters();
				short nParCount = pPar ? pPar->GetSize() : 0;
				if( nParCount == 2 || nParCount == 3 )
				{
					if( nParCount == 2 )
						pPar->addExpression( new SbiExpression( this, -1, SbxLONG ) );

					TestToken( EQ );
					pPar->addExpression( new SbiExpression( this ) );

					bSpecialMidHandling = true;
				}
			}
		}
	}
	aVar.Gen( eRecMode );
	if( !bSpecialMidHandling )
	{
		if( !bEQ )
		{
			aGen.Gen( _GET );
		}
		else
		{
			// Dann muss es eine Zuweisung sein. Was anderes gibts nicht!
			if( !aVar.IsLvalue() )
				Error( SbERR_LVALUE_EXPECTED );
			TestToken( EQ );
			SbiExpression aExpr( this );
			aExpr.Gen();
			SbiOpcode eOp = _PUT;
			// SbiSymDef* pDef = aVar.GetRealVar();
			if( pDef )
			{
				if( pDef->GetConstDef() )
					Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
				if( pDef->GetType() == SbxOBJECT )
				{
					eOp = _SET;
					if( pDef->GetTypeId() )
					{
						aGen.Gen( _SETCLASS, pDef->GetTypeId() );
						return;
					}
				}
			}
			aGen.Gen( eOp );
		}
	}
}

// Zuweisungen

void SbiParser::Assign()
{
	SbiExpression aLvalue( this, SbLVALUE );
	TestToken( EQ );
	SbiExpression aExpr( this );
	aLvalue.Gen();
	aExpr.Gen();
	sal_uInt16 nLen = 0;
	SbiSymDef* pDef = aLvalue.GetRealVar();
	{
		if( pDef->GetConstDef() )
			Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
		nLen = aLvalue.GetRealVar()->GetLen();
	}
	if( nLen )
		aGen.Gen( _PAD, nLen );
	aGen.Gen( _PUT );
}

// Zuweisungen einer Objektvariablen

void SbiParser::Set()
{
	SbiExpression aLvalue( this, SbLVALUE );
	SbxDataType eType = aLvalue.GetType();
	if( eType != SbxOBJECT && eType != SbxEMPTY && eType != SbxVARIANT )
		Error( SbERR_INVALID_OBJECT );
	TestToken( EQ );
	SbiSymDef* pDef = aLvalue.GetRealVar();
	if( pDef && pDef->GetConstDef() )
		Error( SbERR_DUPLICATE_DEF, pDef->GetName() );

	SbiToken eTok = Peek();
	if( eTok == NEW )
	{
		Next();
		String aStr;
		SbiSymDef* pTypeDef = new SbiSymDef( aStr );
		TypeDecl( *pTypeDef, sal_True );

		aLvalue.Gen();
		// aGen.Gen( _CLASS, pDef->GetTypeId() | 0x8000 );
		aGen.Gen( _CREATE, pDef->GetId(), pTypeDef->GetTypeId() );
		aGen.Gen( _SETCLASS, pDef->GetTypeId() );
	}
	else
	{
		SbiExpression aExpr( this );
		aLvalue.Gen();
		aExpr.Gen();
		// Its a good idea to distinguish between
		// set someting = another & 
		// someting = another  
		// ( its necessary for vba objects where set is object
		// specific and also doesn't involve processing default params )
		if( pDef->GetTypeId() )
		{
			if ( bVBASupportOn )
				aGen.Gen( _VBASETCLASS, pDef->GetTypeId() );
			else
				aGen.Gen( _SETCLASS, pDef->GetTypeId() );
		}
		else
		{
			if ( bVBASupportOn )
				aGen.Gen( _VBASET );
			else
				aGen.Gen( _SET );
		}
	}
	// aGen.Gen( _SET );
}

// JSM 07.10.95
void SbiParser::LSet()
{
	SbiExpression aLvalue( this, SbLVALUE );
	if( aLvalue.GetType() != SbxSTRING )
		Error( SbERR_INVALID_OBJECT );
	TestToken( EQ );
	SbiSymDef* pDef = aLvalue.GetRealVar();
	if( pDef && pDef->GetConstDef() )
		Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
	SbiExpression aExpr( this );
	aLvalue.Gen();
	aExpr.Gen();
	aGen.Gen( _LSET );
}

// JSM 07.10.95
void SbiParser::RSet()
{
	SbiExpression aLvalue( this, SbLVALUE );
	if( aLvalue.GetType() != SbxSTRING )
		Error( SbERR_INVALID_OBJECT );
	TestToken( EQ );
	SbiSymDef* pDef = aLvalue.GetRealVar();
	if( pDef && pDef->GetConstDef() )
		Error( SbERR_DUPLICATE_DEF, pDef->GetName() );
	SbiExpression aExpr( this );
	aLvalue.Gen();
	aExpr.Gen();
	aGen.Gen( _RSET );
}

// DEFINT, DEFLNG, DEFSNG, DEFDBL, DEFSTR und so weiter

void SbiParser::DefXXX()
{
	sal_Unicode ch1, ch2;
	SbxDataType t = SbxDataType( eCurTok - DEFINT + SbxINTEGER );

	while( !bAbort )
	{
		if( Next() != SYMBOL ) break;
		ch1 = aSym.ToUpperAscii().GetBuffer()[0];
		ch2 = 0;
		if( Peek() == MINUS )
		{
			Next();
			if( Next() != SYMBOL ) Error( SbERR_SYMBOL_EXPECTED );
			else
			{
				ch2 = aSym.ToUpperAscii().GetBuffer()[0];
				//ch2 = aSym.Upper();
				if( ch2 < ch1 ) Error( SbERR_SYNTAX ), ch2 = 0;
			}
		}
		if (!ch2) ch2 = ch1;
		ch1 -= 'A'; ch2 -= 'A';
		for (; ch1 <= ch2; ch1++) eDefTypes[ ch1 ] = t;
		if( !TestComma() ) break;
	}
}

// STOP/SYSTEM

void SbiParser::Stop()
{
	aGen.Gen( _STOP );
	Peek();		// #35694: Nur Peek(), damit EOL in Single-Line-If erkannt wird
}

// IMPLEMENTS

void SbiParser::Implements()
{
	if( !bClassModule )
	{
		Error( SbERR_UNEXPECTED, IMPLEMENTS );
		return;
	}

	Peek();
	if( eCurTok != SYMBOL )
	{
		Error( SbERR_SYMBOL_EXPECTED );
		return;
	}

	String aImplementedIface = aSym;
	Next();
	if( Peek() == DOT )
	{
		String aDotStr( '.' );
		while( Peek() == DOT )
		{
			aImplementedIface += aDotStr;
			Next();
            SbiToken ePeekTok = Peek();
			if( ePeekTok == SYMBOL || IsKwd( ePeekTok ) )
			{
				Next();
				aImplementedIface += aSym;
			}
			else
			{
				Next();
				Error( SbERR_SYMBOL_EXPECTED );
				break;
			}
		}
	}
	aIfaceVector.push_back( aImplementedIface );
}

void SbiParser::EnableCompatibility()
{
	if( !bCompatible )
		AddConstants();
	bCompatible = sal_True; 
}

// OPTION

void SbiParser::Option()
{
	switch( Next() )
	{
		case BASIC_EXPLICIT:
			bExplicit = sal_True; break;
		case BASE:
			if( Next() == NUMBER )
			{
				if( nVal == 0 || nVal == 1 )
				{
					nBase = (short) nVal;
					break;
				}
			}
			Error( SbERR_EXPECTED, "0/1" );
			break;
		case PRIVATE:
		{
			String aString = SbiTokenizer::Symbol(Next());
			if( !aString.EqualsIgnoreCaseAscii("Module") )
				Error( SbERR_EXPECTED, "Module" );
			break;
		}
		case COMPARE:
		{
			SbiToken eTok = Next();
			if( eTok == BINARY )
				bText = sal_False;
			else if( eTok == SYMBOL && GetSym().EqualsIgnoreCaseAscii("text") )
				bText = sal_True;
			else
				Error( SbERR_EXPECTED, "Text/Binary" );
			break;
		}
		case COMPATIBLE:
			EnableCompatibility();
			break;

		case CLASSMODULE:
			bClassModule = sal_True; 
			aGen.GetModule().SetModuleType( com::sun::star::script::ModuleType::CLASS );
			break;
		case VBASUPPORT:
			if( Next() == NUMBER )
			{
				if ( nVal == 1 || nVal == 0 )
				{
					bVBASupportOn = ( nVal == 1 );
					if ( bVBASupportOn )
						EnableCompatibility();
					// if the module setting is different
					// reset it to what the Option tells us
					if ( bVBASupportOn != aGen.GetModule().IsVBACompat() )
						aGen.GetModule().SetVBACompat( bVBASupportOn );
					break;
				}
			}
			Error( SbERR_EXPECTED, "0/1" );
			break;
		default:
			Error( SbERR_BAD_OPTION, eCurTok );
	}
}

void addStringConst( SbiSymPool& rPool, const char* pSym, const String& rStr )
{
	SbiConstDef* pConst = new SbiConstDef( String::CreateFromAscii( pSym ) );
	pConst->SetType( SbxSTRING );
	pConst->Set( rStr );
	rPool.Add( pConst );
}

inline void addStringConst( SbiSymPool& rPool, const char* pSym, const char* pStr )
{
	addStringConst( rPool, pSym, String::CreateFromAscii( pStr ) );
}

void SbiParser::AddConstants( void )
{
	// #113063 Create constant RTL symbols
	addStringConst( aPublics, "vbCr", "\x0D" );
	addStringConst( aPublics, "vbCrLf", "\x0D\x0A" );
	addStringConst( aPublics, "vbFormFeed", "\x0C" );
	addStringConst( aPublics, "vbLf", "\x0A" );
#if defined(UNX)
	addStringConst( aPublics, "vbNewLine", "\x0A" );
#else
	addStringConst( aPublics, "vbNewLine", "\x0D\x0A" );
#endif
	addStringConst( aPublics, "vbNullString", "" );
	addStringConst( aPublics, "vbTab", "\x09" );
	addStringConst( aPublics, "vbVerticalTab", "\x0B" );

	// Force length 1 and make char 0 afterwards
	String aNullCharStr( String::CreateFromAscii( " " ) );
	aNullCharStr.SetChar( 0, 0 );
	addStringConst( aPublics, "vbNullChar", aNullCharStr );
}

// ERROR n

void SbiParser::ErrorStmnt()
{
	SbiExpression aPar( this );
	aPar.Gen();
	aGen.Gen( _ERROR );
}

