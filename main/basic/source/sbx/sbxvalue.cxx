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

#define _TLBIGINT_INT64
#include <tools/bigint.hxx>
#include <tools/stream.hxx>

#include <basic/sbx.hxx>
#include "sbxconv.hxx"
#include <math.h>
#include "runtime.hxx"
// AB 29.10.99 Unicode
#ifndef _USE_NO_NAMESPACE
using namespace rtl;
#endif


TYPEINIT1(SbxValue,SbxBase)

/////////////////////////// SbxINT64 /////////////////////////////////////
SbxINT64 &SbxINT64::operator -= ( const SbxINT64 &r )
{
	BigInt b( *this );
	b -= BigInt( r );
	b.INT64( this );
	return *this;
}
SbxINT64 &SbxINT64::operator += ( const SbxINT64 &r )
{
	BigInt b( *this );
	b += BigInt( r );
	b.INT64( this );
	return *this;
}
SbxINT64 &SbxINT64::operator *= ( const SbxINT64 &r )
{
	BigInt b( *this );
	b *= BigInt( r );
	b.INT64( this );
	return *this;
}
SbxINT64 &SbxINT64::operator %= ( const SbxINT64 &r )
{
	BigInt b( *this );
	b %= BigInt( r );
	b.INT64( this );
	return *this;
}
SbxINT64 &SbxINT64::operator /= ( const SbxINT64 &r )
{
	BigInt b( *this );
	b /= BigInt( r );
	b.INT64( this );
	return *this;
}
SbxINT64 &SbxINT64::operator &= ( const SbxINT64 &r )
{
	nHigh &= r.nHigh;
	nLow  &= r.nLow;
	return *this;
}
SbxINT64 &SbxINT64::operator |= ( const SbxINT64 &r )
{
	nHigh |= r.nHigh;
	nLow  |= r.nLow;
	return *this;
}
SbxINT64 &SbxINT64::operator ^= ( const SbxINT64 &r )
{
	nHigh ^= r.nHigh;
	nLow  ^= r.nLow;
	return *this;
}

SbxINT64 operator - ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a(l);
	a -= r;
	return a;
}
SbxINT64 operator + ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a(l);
	a += r;
	return a;
}
SbxINT64 operator / ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a(l);
	a /= r;
	return a;
}
SbxINT64 operator % ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a(l);
	a %= r;
	return a;
}
SbxINT64 operator * ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a(l);
	a *= r;
	return a;
}
SbxINT64 operator & ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a;
	a.nHigh = r.nHigh & l.nHigh;
	a.nLow  = r.nLow  & l.nLow;
	return a;
}
SbxINT64 operator | ( const SbxINT64 &l, const SbxINT64 &r )
{
	SbxINT64 a;
	a.nHigh = r.nHigh | l.nHigh;
	a.nLow  = r.nLow  | l.nLow;
	return a;
}
SbxINT64 operator ^ ( const SbxINT64 &r, const SbxINT64 &l )
{
	SbxINT64 a;
	a.nHigh = r.nHigh ^ l.nHigh;
	a.nLow  = r.nLow  ^ l.nLow;
	return a;
}

SbxINT64 operator - ( const SbxINT64 &r )
{
	SbxINT64 a( r );
	a.CHS();
	return a;
}
SbxINT64 operator ~ ( const SbxINT64 &r )
{
	SbxINT64 a;
	a.nHigh = ~r.nHigh;
	a.nLow  = ~r.nLow;
	return a;
}

SbxUINT64 &SbxUINT64::operator %= ( const SbxUINT64 &r )
{
	BigInt b( *this );
	b %= BigInt( r );
	b.UINT64( this );
	return *this;
}
SbxUINT64 &SbxUINT64::operator /= ( const SbxUINT64 &r )
{
	BigInt b( *this );
	b /= BigInt( r );
	b.UINT64( this );
	return *this;
}
/////////////////////////// Fehlerbehandlung /////////////////////////////

#ifdef _USED
// NOCH NACHZUBAUEN!

// Das Default-Handling setzt nur den Fehlercode.

#ifndef WNT
#if defined ( UNX )
int matherr( struct exception* p )
#else
int matherr( struct _exception* p )
#endif
{
	switch( p->type )
	{
#if defined ( UNX )
		case OVERFLOW: SbxBase::SetError( SbxERR_OVERFLOW ); break;
#else
		case _OVERFLOW: SbxBase::SetError( SbxERR_OVERFLOW ); break;
#endif
		default:		SbxBase::SetError( SbxERR_NOTIMP ); break;
	}
	return sal_True;
}
#endif

#endif // _USED


///////////////////////////// Konstruktoren //////////////////////////////

SbxValue::SbxValue() : SbxBase()
{
	aData.eType = SbxEMPTY;
}

SbxValue::SbxValue( SbxDataType t, void* p ) : SbxBase()
{
	int n = t & 0x0FFF;
	if( p )
		n |= SbxBYREF;
	if( n == SbxVARIANT )
		n = SbxEMPTY;
	else
		SetFlag( SBX_FIXED );
	if( p )
	switch( t & 0x0FFF )
	{
		case SbxINTEGER:	n |= SbxBYREF; aData.pInteger = (sal_Int16*) p; break;
		case SbxULONG64:	n |= SbxBYREF; aData.pULong64 = (SbxUINT64*) p; break;
		case SbxLONG64:
		case SbxCURRENCY:	n |= SbxBYREF; aData.pLong64 = (SbxINT64*) p; break;
		case SbxLONG:		n |= SbxBYREF; aData.pLong = (sal_Int32*) p; break;
		case SbxSINGLE:		n |= SbxBYREF; aData.pSingle = (float*) p; break;
		case SbxDATE:
		case SbxDOUBLE:		n |= SbxBYREF; aData.pDouble = (double*) p; break;
        case SbxSTRING:		n |= SbxBYREF; aData.pOUString = (::rtl::OUString*) p; break;
		case SbxERROR:
		case SbxUSHORT:
		case SbxBOOL:		n |= SbxBYREF; aData.pUShort = (sal_uInt16*) p; break;
		case SbxULONG:		n |= SbxBYREF; aData.pULong = (sal_uInt32*) p; break;
		case SbxCHAR:		n |= SbxBYREF; aData.pChar = (xub_Unicode*) p; break;
		case SbxBYTE:		n |= SbxBYREF; aData.pByte = (sal_uInt8*) p; break;
		case SbxINT:		n |= SbxBYREF; aData.pInt = (int*) p; break;
		case SbxOBJECT:
			aData.pObj = (SbxBase*) p;
			if( p )
				aData.pObj->AddRef();
			break;
		case SbxDECIMAL:
			aData.pDecimal = (SbxDecimal*) p;
			if( p )
				aData.pDecimal->addRef();
			break;
		default:
			DBG_ASSERT( sal_False, "Indication of an invalid pointer" );
			n = SbxNULL;
	}
	else
		memset( &aData, 0, sizeof( SbxValues ) );
	aData.eType = SbxDataType( n );
}

SbxValue::SbxValue( const SbxValue& r )
    : SvRefBase( r ), SbxBase( r )
{
	if( !r.CanRead() )
	{
		SetError( SbxERR_PROP_WRITEONLY );
		if( !IsFixed() )
			aData.eType = SbxNULL;
	}
	else
	{
		((SbxValue*) &r)->Broadcast( SBX_HINT_DATAWANTED );
		aData = r.aData;
		// Pointer kopieren, Referenzen inkrementieren
		switch( aData.eType )
		{
			case SbxSTRING:
				if( aData.pOUString )
					aData.pOUString = new ::rtl::OUString( *aData.pOUString );
				break;
			case SbxOBJECT:
				if( aData.pObj )
					aData.pObj->AddRef();
				break;
			case SbxDECIMAL:
				if( aData.pDecimal )
					aData.pDecimal->addRef();
				break;
			default: break;
		}
	}
}

SbxValue& SbxValue::operator=( const SbxValue& r )
{
	if( &r != this )
	{
		if( !CanWrite() )
			SetError( SbxERR_PROP_READONLY );
		else
		{
			// string -> byte array
			if( IsFixed() && (aData.eType == SbxOBJECT)
				&& aData.pObj && ( aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )		   	
				&& (r.aData.eType == SbxSTRING) )
			{
				::rtl::OUString aStr = r.GetString();
				SbxArray* pArr = StringToByteArray(aStr);
				PutObject(pArr);
				return *this;
			}
			// byte array -> string
			if( r.IsFixed() && (r.aData.eType == SbxOBJECT)
				&& r.aData.pObj && ( r.aData.pObj->GetType() == (SbxARRAY | SbxBYTE) )
				&& (aData.eType == SbxSTRING) )
			{
				SbxBase* pObj = r.GetObject();
				SbxArray* pArr = PTR_CAST(SbxArray, pObj);
				if( pArr )
				{
					::rtl::OUString aStr = ByteArrayToString( pArr );
					PutString(aStr);
					return *this;
				}
			}
			// Den Inhalt der Variablen auslesen
			SbxValues aNew;
			if( IsFixed() )
				// fest: dann muss der Typ stimmen
				aNew.eType = aData.eType;
			else if( r.IsFixed() )
				// Quelle fest: Typ uebernehmen
				aNew.eType = SbxDataType( r.aData.eType & 0x0FFF );
			else
				// beides Variant: dann isses egal
				aNew.eType = SbxVARIANT;
			if( r.Get( aNew ) )
				Put( aNew );
		}
	}
	return *this;
}

SbxValue::~SbxValue()
{
#ifndef C50
	Broadcast( SBX_HINT_DYING );
	SetFlag( SBX_WRITE );
	SbxValue::Clear();
#else
	// Provisorischer Fix fuer Solaris 5.0 Compiler Bug
	// bei Nutzung virtueller Vererbung. Virtuelle Calls
	// im Destruktor vermeiden. Statt Clear() zu rufen
	// moegliche Objekt-Referenzen direkt freigeben.
	if( aData.eType == SbxOBJECT )
	{
		if( aData.pObj && aData.pObj != this )
		{
			HACK(nicht bei Parent-Prop - sonst CyclicRef)
			SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
			sal_Bool bParentProp = pThisVar && 5345 ==
			( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) );
			if ( !bParentProp )
				aData.pObj->ReleaseRef();
		}
	}
	else if( aData.eType == SbxDECIMAL )
	{
		releaseDecimalPtr( aData.pDecimal );
	}
#endif
}

void SbxValue::Clear()
{
	switch( aData.eType )
	{
		case SbxNULL:
		case SbxEMPTY:
		case SbxVOID:
			break;
		case SbxSTRING:
			delete aData.pOUString; aData.pOUString = NULL;
			break;
		case SbxOBJECT:
			if( aData.pObj )
			{
				if( aData.pObj != this )
				{
					HACK(nicht bei Parent-Prop - sonst CyclicRef)
					SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
					sal_Bool bParentProp = pThisVar && 5345 ==
					( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) );
					if ( !bParentProp )
						aData.pObj->ReleaseRef();
				}
				aData.pObj = NULL;
			}
			break;
		case SbxDECIMAL:
			if( aData.eType == SbxDECIMAL )
				releaseDecimalPtr( aData.pDecimal );
			break;
		case SbxDATAOBJECT:
			aData.pData = NULL; break;
		default:
		{
			SbxValues aEmpty;
			memset( &aEmpty, 0, sizeof( SbxValues ) );
			aEmpty.eType = GetType();
			Put( aEmpty );
		}
	}
}

// Dummy

void SbxValue::Broadcast( sal_uIntPtr )
{}

//////////////////////////// Daten auslesen //////////////////////////////

// Ermitteln der "richtigen" Variablen. Falls es ein Objekt ist, wird
// entweder das Objekt selbst oder dessen Default-Property angesprochen.
// Falls die Variable eine Variable oder ein Objekt enthaelt, wird
// dieses angesprochen.

SbxValue* SbxValue::TheRealValue() const
{
	return TheRealValue( sal_True );
}

// #55226 Zusaetzliche Info transportieren
bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal );	// sbunoobj.cxx

SbxValue* SbxValue::TheRealValue( sal_Bool bObjInObjError ) const
{
	SbxValue* p = (SbxValue*) this;
	for( ;; )
	{
		SbxDataType t = SbxDataType( p->aData.eType & 0x0FFF );
		if( t == SbxOBJECT )
		{
			// Der Block enthaelt ein Objekt oder eine Variable
			SbxObject* pObj = PTR_CAST(SbxObject,p->aData.pObj);
			if( pObj )
			{
				// Hat das Objekt eine Default-Property?
				SbxVariable* pDflt = pObj->GetDfltProperty();

				// Falls dies ein Objekt ist und sich selbst enthaelt,
				// koennen wir nicht darauf zugreifen
				// #55226# Die alte Bedingung, um einen Fehler zu setzen,
				// ist nicht richtig, da z.B. eine ganz normale Variant-
				// Variable mit Objekt davon betroffen sein kann, wenn ein
				// anderer Wert zugewiesen werden soll. Daher mit Flag.
				if( bObjInObjError && !pDflt &&
					((SbxValue*) pObj)->aData.eType == SbxOBJECT &&
					((SbxValue*) pObj)->aData.pObj == pObj )
				{
					bool bSuccess = handleToStringForCOMObjects( pObj, p );
					if( !bSuccess )
					{
						SetError( SbxERR_BAD_PROP_VALUE );
						p = NULL;
					}
				}
				else if( pDflt )
					p = pDflt;
				/* ALT:
				else
					p = pDflt ? pDflt : (SbxVariable*) pObj;
				*/
				break;
			}
			// Haben wir ein Array?
			SbxArray* pArray = PTR_CAST(SbxArray,p->aData.pObj);
			if( pArray )
			{
				// Ggf. Parameter holen
				SbxArray* pPar = NULL;
				SbxVariable* pVar = PTR_CAST(SbxVariable,p);
				if( pVar )
					pPar = pVar->GetParameters();
				if( pPar )
				{
					// Haben wir ein dimensioniertes Array?
					SbxDimArray* pDimArray = PTR_CAST(SbxDimArray,p->aData.pObj);
					if( pDimArray )
						p = pDimArray->Get( pPar );
					else
						p = pArray->Get( pPar->Get( 1 )->GetInteger() );
					break;
				}
			}
			// Sonst einen SbxValue annehmen
			SbxValue* pVal = PTR_CAST(SbxValue,p->aData.pObj);
			if( pVal )
				p = pVal;
			else
				break;
		}
		else
			break;
	}
	return p;
}

sal_Bool SbxValue::Get( SbxValues& rRes ) const
{
	sal_Bool bRes = sal_False;
	SbxError eOld = GetError();
	if( eOld != SbxERR_OK )
		ResetError();
	if( !CanRead() )
	{
		SetError( SbxERR_PROP_WRITEONLY );
		rRes.pObj = NULL;
	}
	else
	{
		// Falls nach einem Objekt oder einem VARIANT gefragt wird, nicht
		// die wahren Werte suchen
		SbxValue* p = (SbxValue*) this;
		if( rRes.eType != SbxOBJECT && rRes.eType != SbxVARIANT )
			p = TheRealValue();
		if( p )
		{
			p->Broadcast( SBX_HINT_DATAWANTED );
			switch( rRes.eType )
			{
				case SbxEMPTY:
				case SbxVOID:
				case SbxNULL:	 break;
				case SbxVARIANT: rRes = p->aData; break;
				case SbxINTEGER: rRes.nInteger = ImpGetInteger( &p->aData ); break;
				case SbxLONG:	 rRes.nLong = ImpGetLong( &p->aData ); break;
				case SbxSALINT64:	rRes.nInt64 = ImpGetInt64( &p->aData ); break;
				case SbxSALUINT64:	rRes.uInt64 = ImpGetUInt64( &p->aData ); break;
				case SbxSINGLE:	 rRes.nSingle = ImpGetSingle( &p->aData ); break;
				case SbxDOUBLE:	 rRes.nDouble = ImpGetDouble( &p->aData ); break;
				case SbxCURRENCY:rRes.nLong64 = ImpGetCurrency( &p->aData ); break;
				case SbxDECIMAL: rRes.pDecimal = ImpGetDecimal( &p->aData ); break;
				case SbxDATE:	 rRes.nDouble = ImpGetDate( &p->aData ); break;
				case SbxBOOL:
                    rRes.nUShort = sal::static_int_cast< sal_uInt16 >(
                        ImpGetBool( &p->aData ) );
                    break;
				case SbxCHAR:	 rRes.nChar = ImpGetChar( &p->aData ); break;
				case SbxBYTE:	 rRes.nByte = ImpGetByte( &p->aData ); break;
				case SbxUSHORT:	 rRes.nUShort = ImpGetUShort( &p->aData ); break;
				case SbxULONG:	 rRes.nULong = ImpGetULong( &p->aData ); break;
				case SbxLPSTR:
				case SbxSTRING:	 p->aPic = ImpGetString( &p->aData );
								 rRes.pOUString = &p->aPic; break;
				case SbxCoreSTRING:	p->aPic = ImpGetCoreString( &p->aData );
									rRes.pOUString = &p->aPic; break;
				case SbxINT:
#if SAL_TYPES_SIZEOFINT == 2
					rRes.nInt = (int) ImpGetInteger( &p->aData );
#else
					rRes.nInt = (int) ImpGetLong( &p->aData );
#endif
					break;
				case SbxUINT:
#if SAL_TYPES_SIZEOFINT == 2
					rRes.nUInt = (int) ImpGetUShort( &p->aData );
#else
					rRes.nUInt = (int) ImpGetULong( &p->aData );
#endif
					break;
				case SbxOBJECT:
					if( p->aData.eType == SbxOBJECT )
						rRes.pObj = p->aData.pObj;
					else
					{
						SetError( SbxERR_NO_OBJECT );
						rRes.pObj = NULL;
					}
					break;
				default:
					if( p->aData.eType == rRes.eType )
						rRes = p->aData;
					else
					{
						SetError( SbxERR_CONVERSION );
						rRes.pObj = NULL;
					}
			}
		}
		else
		{
			// Objekt enthielt sich selbst
			SbxDataType eTemp = rRes.eType;
			memset( &rRes, 0, sizeof( SbxValues ) );
			rRes.eType = eTemp;
		}
	}
	if( !IsError() )
	{
		bRes = sal_True;
		if( eOld != SbxERR_OK )
			SetError( eOld );
	}
	return bRes;
}

sal_Bool SbxValue::GetNoBroadcast( SbxValues& rRes )
{
	sal_uInt16 nFlags_ = GetFlags();
	SetFlag( SBX_NO_BROADCAST );
	sal_Bool bRes = Get( rRes );
	SetFlags( nFlags_ );
	return bRes;
}

const XubString& SbxValue::GetString() const
{
	SbxValues aRes;
	aRes.eType = SbxSTRING;
	if( Get( aRes ) )
		((SbxValue*) this)->aToolString = *aRes.pOUString;
	else
		((SbxValue*) this)->aToolString.Erase();

    return aToolString;
}

const XubString& SbxValue::GetCoreString() const
{
	SbxValues aRes;
	aRes.eType = SbxCoreSTRING;
	if( Get( aRes ) )
		((SbxValue*) this)->aToolString = *aRes.pOUString;
	else
		((SbxValue*) this)->aToolString.Erase();

    return aToolString;
}

::rtl::OUString SbxValue::GetOUString() const
{
    ::rtl::OUString aResult;
    SbxValues aRes;
    aRes.eType = SbxSTRING;
    if( Get( aRes ) )
        aResult = *aRes.pOUString;

    return aResult;
}

sal_Bool SbxValue::HasObject() const
{
	ErrCode eErr = GetError();
	SbxValues aRes;
	aRes.eType = SbxOBJECT;
	Get( aRes );
	SetError( eErr );
	return 0 != aRes.pObj;
}

sal_Bool SbxValue::GetBool() const
{
	SbxValues aRes;
	aRes.eType = SbxBOOL;
	Get( aRes );
	return sal_Bool( aRes.nUShort != 0 );
}

#define GET( g, e, t, m ) \
t SbxValue::g() const { SbxValues aRes(e); Get( aRes ); return aRes.m; }

GET( GetByte,     SbxBYTE,       sal_uInt8,             nByte )
GET( GetChar,     SbxCHAR,       xub_Unicode,           nChar )
GET( GetCurrency, SbxCURRENCY,   SbxINT64,         nLong64 )
GET( GetDate,     SbxDATE,       double,           nDouble )
GET( GetData,     SbxDATAOBJECT, void*,            pData )
GET( GetDouble,   SbxDOUBLE,     double,           nDouble )
GET( GetErr,      SbxERROR,      sal_uInt16,           nUShort )
GET( GetInt,      SbxINT,        int,              nInt )
GET( GetInteger,  SbxINTEGER,    sal_Int16,            nInteger )
GET( GetLong,     SbxLONG,       sal_Int32,            nLong )
GET( GetLong64,   SbxLONG64,     SbxINT64,         nLong64 )
GET( GetObject,   SbxOBJECT,     SbxBase*,         pObj )
GET( GetSingle,   SbxSINGLE,     float,            nSingle )
GET( GetULong,    SbxULONG,      sal_uInt32,           nULong )
GET( GetULong64,  SbxULONG64,    SbxUINT64,        nULong64 )
GET( GetUShort,   SbxUSHORT,     sal_uInt16,           nUShort )
GET( GetInt64,    SbxSALINT64,   sal_Int64,        nInt64 )
GET( GetUInt64,   SbxSALUINT64,  sal_uInt64,       uInt64 )
GET( GetDecimal,  SbxDECIMAL,    SbxDecimal*,      pDecimal )


//////////////////////////// Daten schreiben /////////////////////////////

sal_Bool SbxValue::Put( const SbxValues& rVal )
{
	sal_Bool bRes = sal_False;
	SbxError eOld = GetError();
	if( eOld != SbxERR_OK )
		ResetError();
	if( !CanWrite() )
		SetError( SbxERR_PROP_READONLY );
	else if( rVal.eType & 0xF000 )
		SetError( SbxERR_NOTIMP );
	else
	{
		// Falls nach einem Objekt gefragt wird, nicht
		// die wahren Werte suchen
		SbxValue* p = this;
		if( rVal.eType != SbxOBJECT )
			p = TheRealValue( sal_False );	// #55226 Hier keinen Fehler erlauben
		if( p )
		{
			if( !p->CanWrite() )
				SetError( SbxERR_PROP_READONLY );
			else if( p->IsFixed() || p->SetType( (SbxDataType) ( rVal.eType & 0x0FFF ) ) )
			  switch( rVal.eType & 0x0FFF )
			{
				case SbxEMPTY:
				case SbxVOID:
				case SbxNULL:		break;
				case SbxINTEGER:	ImpPutInteger( &p->aData, rVal.nInteger ); break;
				case SbxLONG:		ImpPutLong( &p->aData, rVal.nLong ); break;
				case SbxSALINT64:	ImpPutInt64( &p->aData, rVal.nInt64 ); break;
				case SbxSALUINT64:	ImpPutUInt64( &p->aData, rVal.uInt64 ); break;
				case SbxSINGLE:		ImpPutSingle( &p->aData, rVal.nSingle ); break;
				case SbxDOUBLE:		ImpPutDouble( &p->aData, rVal.nDouble ); break;
				case SbxCURRENCY:	ImpPutCurrency( &p->aData, rVal.nLong64 ); break;
				case SbxDECIMAL:	ImpPutDecimal( &p->aData, rVal.pDecimal ); break;
				case SbxDATE:		ImpPutDate( &p->aData, rVal.nDouble ); break;
				case SbxBOOL:		ImpPutBool( &p->aData, rVal.nInteger ); break;
				case SbxCHAR:		ImpPutChar( &p->aData, rVal.nChar ); break;
				case SbxBYTE:		ImpPutByte( &p->aData, rVal.nByte ); break;
				case SbxUSHORT:		ImpPutUShort( &p->aData, rVal.nUShort ); break;
				case SbxULONG:		ImpPutULong( &p->aData, rVal.nULong ); break;
				case SbxLPSTR:
				case SbxSTRING:		ImpPutString( &p->aData, rVal.pOUString ); break;
				case SbxINT:
#if SAL_TYPES_SIZEOFINT == 2
					ImpPutInteger( &p->aData, (sal_Int16) rVal.nInt );
#else
					ImpPutLong( &p->aData, (sal_Int32) rVal.nInt );
#endif
					break;
				case SbxUINT:
#if SAL_TYPES_SIZEOFINT == 2
					ImpPutUShort( &p->aData, (sal_uInt16) rVal.nUInt );
#else
					ImpPutULong( &p->aData, (sal_uInt32) rVal.nUInt );
#endif
					break;
				case SbxOBJECT:
					if( !p->IsFixed() || p->aData.eType == SbxOBJECT )
					{
						// ist schon drin
						if( p->aData.eType == SbxOBJECT && p->aData.pObj == rVal.pObj )
							break;

						// Nur den Werteteil loeschen!
						p->SbxValue::Clear();

						// eingentliche Zuweisung
						p->aData.pObj = rVal.pObj;

						// ggf. Ref-Count mitzaehlen
						if( p->aData.pObj && p->aData.pObj != p )
						{
							if ( p != this )
                            {
								DBG_ERROR( "TheRealValue" );
                            }
							HACK(nicht bei Parent-Prop - sonst CyclicRef)
							SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
							sal_Bool bParentProp = pThisVar && 5345 ==
									( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) );
							if ( !bParentProp )
								p->aData.pObj->AddRef();
						}
					}
					else
						SetError( SbxERR_CONVERSION );
					break;
				default:
					if( p->aData.eType == rVal.eType )
						p->aData = rVal;
					else
					{
						SetError( SbxERR_CONVERSION );
						if( !p->IsFixed() )
							p->aData.eType = SbxNULL;
					}
			}
			if( !IsError() )
			{
				p->SetModified( sal_True );
				p->Broadcast( SBX_HINT_DATACHANGED );
				if( eOld != SbxERR_OK )
					SetError( eOld );
				bRes = sal_True;
			}
		}
	}
	return bRes;
}

// AB, 28.3.96:
// Methode, um bei speziellen Typen eine Vorbehandlung des Strings
// durchzufuehren. Insbesondere erforderlich fuer BASIC-IDE, damit
// die Ausgaben im Watch-Fenster mit PutStringExt zurueckgeschrieben
// werden koennen, wenn Floats mit ',' als Dezimaltrenner oder BOOLs
// explizit mit "TRUE" oder "FALSE" angegeben werden.
// Implementierung in ImpConvStringExt (SBXSCAN.CXX)
sal_Bool SbxValue::PutStringExt( const ::rtl::OUString& r )
{
	// Kopieren, bei Unicode gleich konvertieren
	::rtl::OUString aStr( r );

	// Eigenen Typ bestimmen (nicht wie in Put() mit TheRealValue(),
	// Objekte werden sowieso nicht behandelt)
	SbxDataType eTargetType = SbxDataType( aData.eType & 0x0FFF );

	// Source-Value basteln
	SbxValues aRes;
	aRes.eType = SbxSTRING;

	// Nur, wenn wirklich was konvertiert wurde, Kopie nehmen,
	// sonst Original (Unicode bleibt erhalten)
	sal_Bool bRet;
	if( ImpConvStringExt( aStr, eTargetType ) )
		aRes.pOUString = (::rtl::OUString*)&aStr;
	else
		aRes.pOUString = (::rtl::OUString*)&r;

	// #34939: Bei Strings. die eine Zahl enthalten und wenn this einen
	// Num-Typ hat, Fixed-Flag setzen, damit der Typ nicht veraendert wird
	sal_uInt16 nFlags_ = GetFlags();
	if( ( eTargetType >= SbxINTEGER && eTargetType <= SbxCURRENCY ) ||
		( eTargetType >= SbxCHAR && eTargetType <= SbxUINT ) ||
		eTargetType == SbxBOOL )
	{
		SbxValue aVal;
		aVal.Put( aRes );
		if( aVal.IsNumeric() )
			SetFlag( SBX_FIXED );
	}

	Put( aRes );
	bRet = sal_Bool( !IsError() );

	// Falls das mit dem FIXED einen Error gegeben hat, zuruecksetzen
	// (UI-Aktion sollte keinen Error ergeben, sondern nur scheitern)
	if( !bRet )
		ResetError();

	SetFlags( nFlags_ );
	return bRet;
}

sal_Bool SbxValue::PutString( const xub_Unicode* p )
{
	::rtl::OUString aVal( p );
	SbxValues aRes;
	aRes.eType = SbxSTRING;
	aRes.pOUString = &aVal;
	Put( aRes );
	return sal_Bool( !IsError() );
}

sal_Bool SbxValue::PutBool( sal_Bool b )
{
	SbxValues aRes;
	aRes.eType = SbxBOOL;
	aRes.nUShort = sal::static_int_cast< sal_uInt16 >(b ? SbxTRUE : SbxFALSE);
	Put( aRes );
	return sal_Bool( !IsError() );
}

sal_Bool SbxValue::PutEmpty()
{
	sal_Bool bRet = SetType( SbxEMPTY );
		SetModified( sal_True );
	return bRet;
}

sal_Bool SbxValue::PutNull()
{
	sal_Bool bRet = SetType( SbxNULL );
	if( bRet )
		SetModified( sal_True );
	return bRet;
}


// Special decimal methods
sal_Bool SbxValue::PutDecimal( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec )
{
	SbxValue::Clear();
	aData.pDecimal = new SbxDecimal( rAutomationDec );
	aData.pDecimal->addRef();
	aData.eType = SbxDECIMAL;
	return sal_True;
}

sal_Bool SbxValue::fillAutomationDecimal
	( com::sun::star::bridge::oleautomation::Decimal& rAutomationDec )
{
	SbxDecimal* pDecimal = GetDecimal();
	if( pDecimal != NULL )
	{
		pDecimal->fillAutomationDecimal( rAutomationDec );
		return sal_True;
	}
	return sal_False;
}


sal_Bool SbxValue::PutpChar( const xub_Unicode* p )
{
	::rtl::OUString aVal( p );
	SbxValues aRes;
	aRes.eType = SbxLPSTR;
	aRes.pOUString = &aVal;
	Put( aRes );
	return sal_Bool( !IsError() );
}

sal_Bool SbxValue::PutString( const ::rtl::OUString& r )
{
	SbxValues aRes;
	aRes.eType = SbxSTRING;
	aRes.pOUString = (::rtl::OUString*) &r;
	Put( aRes );
	return sal_Bool( !IsError() );
}


#define PUT( p, e, t, m ) \
sal_Bool SbxValue::p( t n ) \
{ SbxValues aRes(e); aRes.m = n; Put( aRes ); return sal_Bool( !IsError() ); }

PUT( PutByte,     SbxBYTE,       sal_uInt8,             nByte )
PUT( PutChar,     SbxCHAR,       xub_Unicode,      nChar )
PUT( PutCurrency, SbxCURRENCY,   const SbxINT64&,  nLong64 )
PUT( PutDate,     SbxDATE,       double,           nDouble )
PUT( PutData,     SbxDATAOBJECT, void*,            pData )
PUT( PutDouble,   SbxDOUBLE,     double,           nDouble )
PUT( PutErr,      SbxERROR,      sal_uInt16,           nUShort )
PUT( PutInt,      SbxINT,        int,              nInt )
PUT( PutInteger,  SbxINTEGER,    sal_Int16,            nInteger )
PUT( PutLong,     SbxLONG,       sal_Int32,            nLong )
PUT( PutLong64,   SbxLONG64,     const SbxINT64&,  nLong64 )
PUT( PutObject,   SbxOBJECT,     SbxBase*,         pObj )
PUT( PutSingle,   SbxSINGLE,     float,            nSingle )
PUT( PutULong,    SbxULONG,      sal_uInt32,           nULong )
PUT( PutULong64,  SbxULONG64,    const SbxUINT64&, nULong64 )
PUT( PutUShort,   SbxUSHORT,     sal_uInt16,           nUShort )
PUT( PutInt64,    SbxSALINT64,   sal_Int64,        nInt64 )
PUT( PutUInt64,   SbxSALUINT64,  sal_uInt64,       uInt64 )
PUT( PutDecimal,  SbxDECIMAL,    SbxDecimal*,      pDecimal )


////////////////////////// Setzen des Datentyps ///////////////////////////

sal_Bool SbxValue::IsFixed() const
{
	return ( (GetFlags() & SBX_FIXED) | (aData.eType & SbxBYREF) ) != 0;
}

// Eine Variable ist numerisch, wenn sie EMPTY oder wirklich numerisch ist
// oder einen vollstaendig konvertierbaren String enthaelt

// #41692, fuer RTL und Basic-Core getrennt implementieren
sal_Bool SbxValue::IsNumeric() const
{
	return ImpIsNumeric( /*bOnlyIntntl*/sal_False );
}

sal_Bool SbxValue::IsNumericRTL() const
{
	return ImpIsNumeric( /*bOnlyIntntl*/sal_True );
}

sal_Bool SbxValue::ImpIsNumeric( sal_Bool bOnlyIntntl ) const
{

	if( !CanRead() )
	{
		SetError( SbxERR_PROP_WRITEONLY ); return sal_False;
	}
	// Downcast pruefen!!!
	if( this->ISA(SbxVariable) )
		((SbxVariable*)this)->Broadcast( SBX_HINT_DATAWANTED );
	SbxDataType t = GetType();
	if( t == SbxSTRING )
	{
		if( aData.pOUString )
		{
			::rtl::OUString s( *aData.pOUString );
			double n;
			SbxDataType t2;
			sal_uInt16 nLen = 0;
			if( ImpScan( s, n, t2, &nLen, /*bAllowIntntl*/sal_False, bOnlyIntntl ) == SbxERR_OK )
				return sal_Bool( nLen == s.getLength() );
		}
		return sal_False;
	}
	else
		return sal_Bool( t == SbxEMPTY
			|| ( t >= SbxINTEGER && t <= SbxCURRENCY )
			|| ( t >= SbxCHAR && t <= SbxUINT ) );
}

SbxClassType SbxValue::GetClass() const
{
	return SbxCLASS_VALUE;
}

SbxDataType SbxValue::GetType() const
{
	return SbxDataType( aData.eType & 0x0FFF );
}

SbxDataType SbxValue::GetFullType() const
{
	return aData.eType;
}

sal_Bool SbxValue::SetType( SbxDataType t )
{
	DBG_ASSERT( !( t & 0xF000 ), "Setzen von BYREF|ARRAY verboten!" );
	if( ( t == SbxEMPTY && aData.eType == SbxVOID )
	 || ( aData.eType == SbxEMPTY && t == SbxVOID ) )
		return sal_True;
	if( ( t & 0x0FFF ) == SbxVARIANT )
	{
		// Versuch, den Datentyp auf Variant zu setzen
		ResetFlag( SBX_FIXED );
		if( IsFixed() )
		{
			SetError( SbxERR_CONVERSION ); return sal_False;
		}
		t = SbxEMPTY;
	}
	if( ( t & 0x0FFF ) != ( aData.eType & 0x0FFF ) )
	{
		if( !CanWrite() || IsFixed() )
		{
			SetError( SbxERR_CONVERSION ); return sal_False;
		}
		else
		{
			// Eventuelle Objekte freigeben
			switch( aData.eType )
			{
				case SbxSTRING:
					delete aData.pOUString;
					break;
				case SbxOBJECT:
					if( aData.pObj && aData.pObj != this )
					{
						HACK(nicht bei Parent-Prop - sonst CyclicRef)
						SbxVariable *pThisVar = PTR_CAST(SbxVariable, this);
						sal_uInt16 nSlotId = pThisVar
									? ( (sal_Int16) ( pThisVar->GetUserData() & 0xFFFF ) )
									: 0;
						DBG_ASSERT( nSlotId != 5345 || pThisVar->GetName() == UniString::CreateFromAscii( "Parent" ),
									"SID_PARENTOBJECT heisst nicht 'Parent'" );
						sal_Bool bParentProp = 5345 == nSlotId;
						if ( !bParentProp )
							aData.pObj->ReleaseRef();
					}
					break;
				default: break;
			}
			// Das klappt immer, da auch die Float-Repraesentationen 0 sind.
			memset( &aData, 0, sizeof( SbxValues ) );
			aData.eType = t;
		}
	}
	return sal_True;
}

sal_Bool SbxValue::Convert( SbxDataType eTo )
{
	eTo = SbxDataType( eTo & 0x0FFF );
	if( ( aData.eType & 0x0FFF ) == eTo )
		return sal_True;
	if( !CanWrite() )
		return sal_False;
	if( eTo == SbxVARIANT )
	{
		// Versuch, den Datentyp auf Variant zu setzen
		ResetFlag( SBX_FIXED );
		if( IsFixed() )
		{
			SetError( SbxERR_CONVERSION ); return sal_False;
		}
		else
			return sal_True;
	}
	// Convert from Null geht niemals. Einmal Null, immer Null!
	if( aData.eType == SbxNULL )
	{
		SetError( SbxERR_CONVERSION ); return sal_False;
	}

	// Konversion der Daten:
	SbxValues aNew;
	aNew.eType = eTo;
	if( Get( aNew ) )
	{
		// Der Datentyp konnte konvertiert werden. Bei Fixed-Elementen
		// ist hier Ende, da die Daten nicht uebernommen zu werden brauchen
		if( !IsFixed() )
		{
			SetType( eTo );
			Put( aNew );
			SetModified( sal_True );
		}
		Broadcast( SBX_HINT_CONVERTED );
		return sal_True;
	}
	else
		return sal_False;
}
////////////////////////////////// Rechnen /////////////////////////////////

sal_Bool SbxValue::Compute( SbxOperator eOp, const SbxValue& rOp )
{
	bool bVBAInterop =  SbiRuntime::isVBAEnabled(); 

	SbxDataType eThisType = GetType();
	SbxDataType eOpType = rOp.GetType();
	SbxError eOld = GetError();
	if( eOld != SbxERR_OK )
		ResetError();
	if( !CanWrite() )
		SetError( SbxERR_PROP_READONLY );
	else if( !rOp.CanRead() )
		SetError( SbxERR_PROP_WRITEONLY );
	// Sonderregel 1: Ist ein Operand Null, ist das Ergebnis Null
	else if( eThisType == SbxNULL || eOpType == SbxNULL )
		SetType( SbxNULL );
	// Sonderregel 2: Ist ein Operand Empty, ist das Ergebnis der 2. Operand
	else if( eThisType == SbxEMPTY 
	&& !bVBAInterop 
	)
		*this = rOp;
	// 13.2.96: Nicht schon vor Get auf SbxEMPTY pruefen
	else
	{
		SbxValues aL, aR;
		bool bDecimal = false;
		if( bVBAInterop && ( ( eThisType == SbxSTRING && eOpType != SbxSTRING ) || 
			 ( eThisType != SbxSTRING && eOpType == SbxSTRING ) ) &&
			 ( eOp == SbxMUL || eOp == SbxDIV || eOp == SbxPLUS || eOp == SbxMINUS ) )
		{
			goto Lbl_OpIsDouble;
		}
		else if( eThisType == SbxSTRING || eOp == SbxCAT || ( bVBAInterop && ( eOpType == SbxSTRING ) && (  eOp == SbxPLUS ) ) )
		{
			if( eOp == SbxCAT || eOp == SbxPLUS )
			{
				// AB 5.11.1999, OUString beruecksichtigen
				aL.eType = aR.eType = SbxSTRING;
				rOp.Get( aR );
				// AB 8.12.1999, #70399: Hier wieder GetType() rufen, Get() kann Typ aendern!
				if( rOp.GetType() == SbxEMPTY )
					goto Lbl_OpIsEmpty;
				Get( aL );

				// #30576: Erstmal testen, ob Wandlung geklappt hat
				if( aL.pOUString != NULL && aR.pOUString != NULL )
				{
					*aL.pOUString += *aR.pOUString;
				}
				// Nicht einmal Left OK?
				else if( aL.pOUString == NULL )
				{
					aL.pOUString = new ::rtl::OUString();
				}
				Put( aL );
			}
			else
				SetError( SbxERR_CONVERSION );
		}
		else if( eOpType == SbxSTRING && rOp.IsFixed() )
		{	// Numerisch: rechts darf kein String stehen
			SetError( SbxERR_CONVERSION );
		}
		else if( ( eOp >= SbxIDIV && eOp <= SbxNOT ) || eOp == SbxMOD )
		{
			if( GetType() == eOpType )
			{
				if( GetType() == SbxULONG64
					|| GetType() == SbxLONG64
					|| GetType() == SbxCURRENCY
					|| GetType() == SbxULONG )
					aL.eType = aR.eType = GetType();
//				else if( GetType() == SbxDouble || GetType() == SbxSingle )
//					aL.eType = aR.eType = SbxLONG64;
				else
					aL.eType = aR.eType = SbxLONG;
			}
			else if( GetType() == SbxCURRENCY || eOpType == SbxCURRENCY
					 || GetType() == SbxULONG64 || eOpType == SbxULONG64
					 || GetType() == SbxLONG64 || eOpType == SbxLONG64 )
				aL.eType = aR.eType = SbxLONG64;
//			else if( GetType() == SbxDouble || rOP.GetType() == SbxDouble
//			         || GetType() == SbxSingle || rOP.GetType() == SbxSingle )
//				aL.eType = aR.eType = SbxLONG64;
			else
				aL.eType = aR.eType = SbxLONG;

			if( rOp.Get( aR ) )
			{
				if( rOp.GetType() == SbxEMPTY )
				{
					if ( !bVBAInterop || ( bVBAInterop && ( eOp != SbxNOT  ) ) )
						goto Lbl_OpIsEmpty;
				}
				if( Get( aL ) ) switch( eOp )
				{
					case SbxIDIV:
						if( aL.eType == SbxCURRENCY )
							aL.eType = SbxLONG64;
						if( aL.eType == SbxLONG64 )
							if( !aR.nLong64 ) SetError( SbxERR_ZERODIV );
							else aL.nLong64 /= aR.nLong64;
						else if( aL.eType == SbxULONG64 )
							if( !aR.nULong64 ) SetError( SbxERR_ZERODIV );
							else aL.nULong64 /= aR.nULong64;
						else if( aL.eType == SbxLONG )
							if( !aR.nLong ) SetError( SbxERR_ZERODIV );
							else aL.nLong /= aR.nLong;
						else
							if( !aR.nULong ) SetError( SbxERR_ZERODIV );
							else aL.nULong /= aR.nULong;
						break;
					case SbxMOD:
						if( aL.eType == SbxCURRENCY )
							aL.eType = SbxLONG64;
						if( aL.eType == SbxLONG64 )
							if( !aR.nLong64 ) SetError( SbxERR_ZERODIV );
							else aL.nLong64 %= aR.nLong64;
						else if( aL.eType == SbxULONG64 )
							if( !aR.nULong64 ) SetError( SbxERR_ZERODIV );
							else aL.nULong64 %= aR.nULong64;
						else if( aL.eType == SbxLONG )
							if( !aR.nLong ) SetError( SbxERR_ZERODIV );
							else aL.nLong %= aR.nLong;
						else
							if( !aR.nULong ) SetError( SbxERR_ZERODIV );
							else aL.nULong %= aR.nULong;
						break;
					case SbxAND:
						if( aL.eType != SbxLONG && aL.eType != SbxULONG )
							aL.nLong64 &= aR.nLong64;
						else
							aL.nLong &= aR.nLong;
						break;
					case SbxOR:
						if( aL.eType != SbxLONG && aL.eType != SbxULONG )
							aL.nLong64 |= aR.nLong64;
						else
							aL.nLong |= aR.nLong;
						break;
					case SbxXOR:
						if( aL.eType != SbxLONG && aL.eType != SbxULONG )
							aL.nLong64 ^= aR.nLong64;
						else
							aL.nLong ^= aR.nLong;
						break;
					case SbxEQV:
						if( aL.eType != SbxLONG && aL.eType != SbxULONG )
							aL.nLong64 = (aL.nLong64 & aR.nLong64) | (~aL.nLong64 & ~aR.nLong64);
						else
							aL.nLong = (aL.nLong & aR.nLong) | (~aL.nLong & ~aR.nLong);
						break;
					case SbxIMP:
						if( aL.eType != SbxLONG && aL.eType != SbxULONG )
							aL.nLong64 = ~aL.nLong64 | aR.nLong64;
						else
							aL.nLong = ~aL.nLong | aR.nLong;
						break;
					case SbxNOT:
						if( aL.eType != SbxLONG && aL.eType != SbxULONG )
							aL.nLong64 = ~aL.nLong64;
						else
							aL.nLong = ~aL.nLong;
						break;
					default: break;
				}
			}
		}
		else if( ( GetType() == SbxDECIMAL || rOp.GetType() == SbxDECIMAL ) && 
			     ( eOp == SbxMUL || eOp == SbxDIV || eOp == SbxPLUS || eOp == SbxMINUS || eOp == SbxNEG ) )
		{
			aL.eType = aR.eType = SbxDECIMAL;
			bDecimal = true;
			if( rOp.Get( aR ) )
			{
				if( rOp.GetType() == SbxEMPTY )
				{
					releaseDecimalPtr( aL.pDecimal );
					goto Lbl_OpIsEmpty;
				}
				if( Get( aL ) ) 
				{
					if( aL.pDecimal && aR.pDecimal )
					{
						bool bOk = true;
						switch( eOp )
						{
							case SbxMUL:
								bOk = ( *(aL.pDecimal) *= *(aR.pDecimal) );
								break;
							case SbxDIV:
								if( aR.pDecimal->isZero() )
									SetError( SbxERR_ZERODIV );
								else
									bOk = ( *(aL.pDecimal) /= *(aR.pDecimal) );
								break;
							case SbxPLUS:
								bOk = ( *(aL.pDecimal) += *(aR.pDecimal) );
								break;
							case SbxMINUS:
								bOk = ( *(aL.pDecimal) -= *(aR.pDecimal) );
								break;
							case SbxNEG:
								bOk = ( aL.pDecimal->neg() );
								break;
							default:
								SetError( SbxERR_NOTIMP );
						}
						if( !bOk )
							SetError( SbxERR_OVERFLOW );
					}
					else
					{
						SetError( SbxERR_CONVERSION );
					}
				}
			}
		}
		else if( GetType() == SbxCURRENCY || rOp.GetType() == SbxCURRENCY )
		{
			aL.eType = SbxCURRENCY;
			aR.eType = SbxCURRENCY;

			if( rOp.Get( aR ) )
			{
				static BigInt n10K( 10000 );

				if( rOp.GetType() == SbxEMPTY )
					goto Lbl_OpIsEmpty;

				if( Get( aL ) ) switch( eOp )
				{
					case SbxMUL:
					{
						// #i20704 Implement directly
						BigInt b1( aL.nLong64 );
						BigInt b2( aR.nLong64 );
						b1 *= b2;
						b1 /= n10K;
						double d = double( b1 ) / 10000.0;
						if( d > SbxMAXCURR || d < SbxMINCURR )
							SetError( SbxERR_OVERFLOW );
						else
							b1.INT64( &aL.nLong64 );
						break;
					}
					case SbxDIV:
						if( !aR.nLong64 )
						{
							SetError( SbxERR_ZERODIV );
						}
						else 
						{
							// #i20704 Implement directly
							BigInt b1( aL.nLong64 );
							BigInt b2( aR.nLong64 );
							b1 *= n10K;
							b1 /= b2;
							double d = double( b1 ) / 10000.0;
							if( d > SbxMAXCURR || d < SbxMINCURR )
								SetError( SbxERR_OVERFLOW );
							else
								b1.INT64( &aL.nLong64 );
						}
						break;
					case SbxPLUS:
						aL.nLong64 += aR.nLong64; break;
					case SbxMINUS:
						aL.nLong64 -= aR.nLong64; break;
					case SbxNEG:
						aL.nLong64 = -aL.nLong64; break;
					default:
						SetError( SbxERR_NOTIMP );
				}
			}
		}
		else
Lbl_OpIsDouble:			
		{	// Andere Operatoren
			aL.eType = aR.eType = SbxDOUBLE;
			if( rOp.Get( aR ) )
			{
				if( rOp.GetType() == SbxEMPTY )
				{
					if ( !bVBAInterop || ( bVBAInterop && ( eOp != SbxNEG ) ) )
						goto Lbl_OpIsEmpty;
				}
				if( Get( aL ) )
				{
					switch( eOp )
					{
						case SbxEXP:
							aL.nDouble = pow( aL.nDouble, aR.nDouble );
							break;
						case SbxMUL:
							aL.nDouble *= aR.nDouble; break;
						case SbxDIV:
							if( !aR.nDouble ) SetError( SbxERR_ZERODIV );
							else aL.nDouble /= aR.nDouble; break;
						case SbxPLUS:
							aL.nDouble += aR.nDouble; break;
						case SbxMINUS:
							aL.nDouble -= aR.nDouble; break;
						case SbxNEG:
							aL.nDouble = -aL.nDouble; break;
						default:
							SetError( SbxERR_NOTIMP );
					}

					// #45465 Date braucht bei + eine Spezial-Behandlung
					if( eOp == SbxPLUS && (GetType() == SbxDATE || rOp.GetType() == SbxDATE ) )
						aL.eType = SbxDATE;
				}
			}

		}
		if( !IsError() )
			Put( aL );
		if( bDecimal )
		{
			releaseDecimalPtr( aL.pDecimal );
			releaseDecimalPtr( aR.pDecimal );
		}
	}
Lbl_OpIsEmpty:

	sal_Bool bRes = sal_Bool( !IsError() );
	if( bRes && eOld != SbxERR_OK )
		SetError( eOld );
	return bRes;
}

// Die Vergleichs-Routine liefert sal_True oder sal_False.

sal_Bool SbxValue::Compare( SbxOperator eOp, const SbxValue& rOp ) const
{
	bool bVBAInterop =  SbiRuntime::isVBAEnabled(); 

	sal_Bool bRes = sal_False;
	SbxError eOld = GetError();
	if( eOld != SbxERR_OK )
		ResetError();
	if( !CanRead() || !rOp.CanRead() )
		SetError( SbxERR_PROP_WRITEONLY );
	else if( GetType() == SbxNULL && rOp.GetType() == SbxNULL && !bVBAInterop )
    {
		bRes = sal_True;
    }
	else if( GetType() == SbxEMPTY && rOp.GetType() == SbxEMPTY )
		bRes = !bVBAInterop ? sal_True : ( eOp == SbxEQ ? sal_True : sal_False );
	// Sonderregel 1: Ist ein Operand Null, ist das Ergebnis FALSE
	else if( GetType() == SbxNULL || rOp.GetType() == SbxNULL )
		bRes = sal_False;
	// Sonderregel 2: Wenn beide Variant sind und einer ist numerisch,
	// und der andere ein String, ist num < str
	else if( !IsFixed() && !rOp.IsFixed()
	 && ( rOp.GetType() == SbxSTRING && GetType() != SbxSTRING && IsNumeric() ) && !bVBAInterop 
	)
		bRes = sal_Bool( eOp == SbxLT || eOp == SbxLE || eOp == SbxNE );
	else if( !IsFixed() && !rOp.IsFixed()
	 && ( GetType() == SbxSTRING && rOp.GetType() != SbxSTRING && rOp.IsNumeric() ) 
&& !bVBAInterop 
	)
		bRes = sal_Bool( eOp == SbxGT || eOp == SbxGE || eOp == SbxNE );
	else
	{
		SbxValues aL, aR;
		// Wenn einer der Operanden ein String ist,
		// findet ein Stringvergleich statt
		if( GetType() == SbxSTRING || rOp.GetType() == SbxSTRING )
		{
			aL.eType = aR.eType = SbxSTRING;
			if( Get( aL ) && rOp.Get( aR ) ) switch( eOp )
			{
				case SbxEQ:
					bRes = sal_Bool( *aL.pOUString == *aR.pOUString ); break;
				case SbxNE:
					bRes = sal_Bool( *aL.pOUString != *aR.pOUString ); break;
				case SbxLT:
					bRes = sal_Bool( *aL.pOUString <  *aR.pOUString ); break;
				case SbxGT:
					bRes = sal_Bool( *aL.pOUString >  *aR.pOUString ); break;
				case SbxLE:
					bRes = sal_Bool( *aL.pOUString <= *aR.pOUString ); break;
				case SbxGE:
					bRes = sal_Bool( *aL.pOUString >= *aR.pOUString ); break;
				default:
					SetError( SbxERR_NOTIMP );
			}
		}
		// AB 19.12.95: Wenn SbxSINGLE beteiligt, auf SINGLE konvertieren,
		//				sonst gibt es numerische Fehler
		else if( GetType() == SbxSINGLE || rOp.GetType() == SbxSINGLE )
		{
			aL.eType = aR.eType = SbxSINGLE;
			if( Get( aL ) && rOp.Get( aR ) )
			  switch( eOp )
			{
				case SbxEQ:
					bRes = sal_Bool( aL.nSingle == aR.nSingle ); break;
				case SbxNE:
					bRes = sal_Bool( aL.nSingle != aR.nSingle ); break;
				case SbxLT:
					bRes = sal_Bool( aL.nSingle <  aR.nSingle ); break;
				case SbxGT:
					bRes = sal_Bool( aL.nSingle >  aR.nSingle ); break;
				case SbxLE:
					bRes = sal_Bool( aL.nSingle <= aR.nSingle ); break;
				case SbxGE:
					bRes = sal_Bool( aL.nSingle >= aR.nSingle ); break;
				default:
					SetError( SbxERR_NOTIMP );
			}
		}
		else if( GetType() == SbxDECIMAL && rOp.GetType() == SbxDECIMAL )
		{
			aL.eType = aR.eType = SbxDECIMAL;
			Get( aL );
			rOp.Get( aR );
			if( aL.pDecimal && aR.pDecimal )
			{
				SbxDecimal::CmpResult eRes = compare( *aL.pDecimal, *aR.pDecimal );
				switch( eOp )
				{
					case SbxEQ:
						bRes = sal_Bool( eRes == SbxDecimal::EQ ); break;
					case SbxNE:
						bRes = sal_Bool( eRes != SbxDecimal::EQ ); break;
					case SbxLT:
						bRes = sal_Bool( eRes == SbxDecimal::LT ); break;
					case SbxGT:
						bRes = sal_Bool( eRes == SbxDecimal::GT ); break;
					case SbxLE:
						bRes = sal_Bool( eRes != SbxDecimal::GT ); break;
					case SbxGE:
						bRes = sal_Bool( eRes != SbxDecimal::LT ); break;
					default:
						SetError( SbxERR_NOTIMP );
				}
			}
			else
			{
				SetError( SbxERR_CONVERSION );
			}
			releaseDecimalPtr( aL.pDecimal );
			releaseDecimalPtr( aR.pDecimal );
		}
		// Alles andere auf SbxDOUBLE-Basis vergleichen
		else
		{
			aL.eType = aR.eType = SbxDOUBLE;
			//if( Get( aL ) && rOp.Get( aR ) )
			bool bGetL = Get( aL );
			bool bGetR = rOp.Get( aR );
			if( bGetL && bGetR )
			  switch( eOp )
			{
				case SbxEQ:
					bRes = sal_Bool( aL.nDouble == aR.nDouble ); break;
				case SbxNE:
					bRes = sal_Bool( aL.nDouble != aR.nDouble ); break;
				case SbxLT:
					bRes = sal_Bool( aL.nDouble <  aR.nDouble ); break;
				case SbxGT:
					bRes = sal_Bool( aL.nDouble >  aR.nDouble ); break;
				case SbxLE:
					bRes = sal_Bool( aL.nDouble <= aR.nDouble ); break;
				case SbxGE:
					bRes = sal_Bool( aL.nDouble >= aR.nDouble ); break;
				default:
					SetError( SbxERR_NOTIMP );
			}
			// at least one value was got
			// if this is VBA then a conversion error for one
			// side will yield a false result of an equality test
			else if ( bGetR || bGetL )
			{
				if ( bVBAInterop && eOp == SbxEQ && GetError() == SbxERR_CONVERSION )
				{
					ResetError();
					bRes = sal_False;
				}
			}
		}
	}
	if( eOld != SbxERR_OK )
		SetError( eOld );
	return bRes;
}

///////////////////////////// Lesen/Schreiben ////////////////////////////

sal_Bool SbxValue::LoadData( SvStream& r, sal_uInt16 )
{
	SbxValue::Clear();
	sal_uInt16 nType;
	r >> nType;
	aData.eType = SbxDataType( nType );
	switch( nType )
	{
		case SbxBOOL:
		case SbxINTEGER:
			r >> aData.nInteger; break;
		case SbxLONG:
			r >> aData.nLong; break;
		case SbxSINGLE:
		{
			// Floats als ASCII
			XubString aVal;
			r.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
			double d;
			SbxDataType t;
			if( ImpScan( aVal, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
			{
				aData.nSingle = 0.0F;
				return sal_False;
			}
			aData.nSingle = (float) d;
			break;
		}
		case SbxDATE:
		case SbxDOUBLE:
		{
			// Floats als ASCII
			XubString aVal;
			r.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
			SbxDataType t;
			if( ImpScan( aVal, aData.nDouble, t, NULL ) != SbxERR_OK )
			{
				aData.nDouble = 0.0;
				return sal_False;
			}
			break;
		}
		case SbxULONG64:
		{
			r >> aData.nULong64.nHigh >> aData.nULong64.nLow;
			break;
		}
		case SbxLONG64:
		case SbxCURRENCY:
		{
			r >> aData.nLong64.nHigh >> aData.nLong64.nLow;
			break;
		}
		case SbxSTRING:
		{
			XubString aVal;
			r.ReadByteString( aVal, RTL_TEXTENCODING_ASCII_US );
			if( aVal.Len() )
				aData.pOUString = new ::rtl::OUString( aVal );
			else
				aData.pOUString = NULL; // JSM 22.09.1995
			break;
		}
		case SbxERROR:
		case SbxUSHORT:
			r >> aData.nUShort; break;
		case SbxOBJECT:
		{
			sal_uInt8 nMode;
			r >> nMode;
			switch( nMode )
			{
				case 0:
					aData.pObj = NULL;
					break;
				case 1:
					aData.pObj = SbxBase::Load( r );
					return sal_Bool( aData.pObj != NULL );
				case 2:
					aData.pObj = this;
					break;
			}
			break;
		}
		case SbxCHAR:
		{
			char c;
			r >> c;
			aData.nChar = c;
			break;
		}
		case SbxBYTE:
			r >> aData.nByte; break;
		case SbxULONG:
			r >> aData.nULong; break;
		case SbxINT:
		{
			sal_uInt8 n;
			r >> n;
			// Passt der Int auf diesem System?
			if( n > SAL_TYPES_SIZEOFINT )
				r >> aData.nLong, aData.eType = SbxLONG;
			else
				r >> aData.nInt;
			break;
		}
		case SbxUINT:
		{
			sal_uInt8 n;
			r >> n;
			// Passt der UInt auf diesem System?
			if( n > SAL_TYPES_SIZEOFINT )
				r >> aData.nULong, aData.eType = SbxULONG;
			else
				r >> (sal_uInt32&)aData.nUInt;
			break;
		}
		case SbxEMPTY:
		case SbxNULL:
		case SbxVOID:
			break;
		case SbxDATAOBJECT:
			r >> aData.nLong;
			break;
		// #78919 For backwards compatibility
		case SbxWSTRING:
		case SbxWCHAR:
			break;
		default:
			memset (&aData,0,sizeof(aData));
			ResetFlag(SBX_FIXED);
			aData.eType = SbxNULL;
			DBG_ASSERT( sal_False, "Unsupported data type loaded" );
			return sal_False;
	}
	return sal_True;
}

sal_Bool SbxValue::StoreData( SvStream& r ) const
{
	sal_uInt16 nType = sal::static_int_cast< sal_uInt16 >(aData.eType);
	r << nType;
	switch( nType & 0x0FFF )
	{
		case SbxBOOL:
		case SbxINTEGER:
			r << aData.nInteger; break;
		case SbxLONG:
			r << aData.nLong; break;
		case SbxDATE:
			// #49935: Als double speichern, sonst Fehler beim Einlesen
			((SbxValue*)this)->aData.eType = (SbxDataType)( ( nType & 0xF000 ) | SbxDOUBLE );
			r.WriteByteString( GetCoreString(), RTL_TEXTENCODING_ASCII_US );
			((SbxValue*)this)->aData.eType = (SbxDataType)nType;
			break;
		case SbxSINGLE:
		case SbxDOUBLE:
			r.WriteByteString( GetCoreString(), RTL_TEXTENCODING_ASCII_US );
			break;
		case SbxULONG64:
		{
			r << aData.nULong64.nHigh << aData.nULong64.nLow;
			break;
		}
		case SbxLONG64:
		case SbxCURRENCY:
		{
			r << aData.nLong64.nHigh << aData.nLong64.nLow;
			break;
		}
		case SbxSTRING:
			if( aData.pOUString )
			{
				r.WriteByteString( *aData.pOUString, RTL_TEXTENCODING_ASCII_US );
			}
			else
			{
				String aEmpty;
				r.WriteByteString( aEmpty, RTL_TEXTENCODING_ASCII_US );
			}
			break;
		case SbxERROR:
		case SbxUSHORT:
			r << aData.nUShort; break;
		case SbxOBJECT:
			// sich selbst als Objektptr speichern geht nicht!
			if( aData.pObj )
			{
				if( PTR_CAST(SbxValue,aData.pObj) != this )
				{
					r << (sal_uInt8) 1;
					return aData.pObj->Store( r );
				}
				else
					r << (sal_uInt8) 2;
			}
			else
				r << (sal_uInt8) 0;
			break;
		case SbxCHAR:
		{
			char c = sal::static_int_cast< char >(aData.nChar); 
			r << c;
			break;
		}
		case SbxBYTE:
			r << aData.nByte; break;
		case SbxULONG:
			r << aData.nULong; break;
		case SbxINT:
		{
			sal_uInt8 n = SAL_TYPES_SIZEOFINT;
			r << n << (sal_Int32)aData.nInt;
			break;
		}
		case SbxUINT:
		{
			sal_uInt8 n = SAL_TYPES_SIZEOFINT;
			r << n << (sal_uInt32)aData.nUInt;
			break;
		}
		case SbxEMPTY:
		case SbxNULL:
		case SbxVOID:
			break;
		case SbxDATAOBJECT:
			r << aData.nLong;
			break;
		// #78919 For backwards compatibility
		case SbxWSTRING:
		case SbxWCHAR:
			break;
		default:
			DBG_ASSERT( sal_False, "Save an unsupported data type" );
			return sal_False;
	}
	return sal_True;
}

