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


#include <tools/stream.hxx>
#include "svl/brdcst.hxx"

#include <basic/sbx.hxx>
#include <basic/sbxbase.hxx>
#include "sbxres.hxx"
#include "sbxconv.hxx"
#include <math.h>
#include <ctype.h>

#include "com/sun/star/uno/XInterface.hpp"
using namespace com::sun::star::uno;

///////////////////////////// SbxVariable //////////////////////////////

TYPEINIT1(SbxVariable,SbxValue)
TYPEINIT1(SbxHint,SfxSimpleHint)

extern sal_uInt32 nVarCreator;			// in SBXBASE.CXX, fuer LoadData()
#ifdef DBG_UTIL
static sal_uIntPtr nVar = 0;
#endif

///////////////////////////// SbxVariableImpl ////////////////////////////

class SbxVariableImpl
{
	friend class SbxVariable;
	String						m_aDeclareClassName;
	Reference< XInterface >		m_xComListener;
	StarBASIC*					m_pComListenerParentBasic;

	SbxVariableImpl( void )
		: m_pComListenerParentBasic( NULL )
	{}
	SbxVariableImpl( const SbxVariableImpl& r )
		: m_aDeclareClassName( r.m_aDeclareClassName )
		, m_xComListener( r.m_xComListener )
		, m_pComListenerParentBasic( r.m_pComListenerParentBasic )
	{
    }
};


///////////////////////////// Konstruktoren //////////////////////////////

SbxVariable::SbxVariable() : SbxValue()
{
	mpSbxVariableImpl = NULL;
	pCst = NULL;
	pParent = NULL;
	nUserData = 0;
	nHash = 0;
#ifdef DBG_UTIL
	DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
	GetSbxData_Impl()->aVars.Insert( this, LIST_APPEND );
#endif
}

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic );

SbxVariable::SbxVariable( const SbxVariable& r )
		   : SvRefBase( r ), SbxValue( r ), mpPar( r.mpPar ), pInfo( r.pInfo )
{
	mpSbxVariableImpl = NULL;
	if( r.mpSbxVariableImpl != NULL )
	{
		mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
		if( mpSbxVariableImpl->m_xComListener.is() )
			registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
	}
	pCst = NULL;
	if( r.CanRead() )
	{
		pParent = r.pParent;
		nUserData = r.nUserData;
		maName = r.maName;
		nHash = r.nHash;
	}
	else
	{
		pParent = NULL;
		nUserData = 0;
		nHash = 0;
	}
#ifdef DBG_UTIL
	static sal_Char const aCellsStr[] = "Cells";
	if ( maName.EqualsAscii( aCellsStr ) )
		maName.AssignAscii( aCellsStr, sizeof( aCellsStr )-1 );
	DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
	GetSbxData_Impl()->aVars.Insert( this, LIST_APPEND );
#endif
}

SbxVariable::SbxVariable( SbxDataType t, void* p ) : SbxValue( t, p )
{
	mpSbxVariableImpl = NULL;
	pCst = NULL;
	pParent = NULL;
	nUserData = 0;
	nHash = 0;
#ifdef DBG_UTIL
	DbgOutf( "SbxVariable::Ctor %lx=%ld", (void*)this, ++nVar );
	GetSbxData_Impl()->aVars.Insert( this, LIST_APPEND );
#endif
}

void removeDimAsNewRecoverItem( SbxVariable* pVar );

SbxVariable::~SbxVariable()
{
#ifdef DBG_UTIL
	ByteString aBStr( (const UniString&)maName, RTL_TEXTENCODING_ASCII_US );
	DbgOutf( "SbxVariable::Dtor %lx (%s)", (void*)this, aBStr.GetBuffer() );
	static sal_Char const aCellsStr[] = "Cells";
	if ( maName.EqualsAscii( aCellsStr ) )
		maName.AssignAscii( aCellsStr, sizeof( aCellsStr )-1 );
	GetSbxData_Impl()->aVars.Remove( this );
#endif
	if( IsSet( SBX_DIM_AS_NEW ))
		removeDimAsNewRecoverItem( this );
	delete mpSbxVariableImpl;
	delete pCst;
}

////////////////////////////// Broadcasting //////////////////////////////

SfxBroadcaster& SbxVariable::GetBroadcaster()
{
	if( !pCst )
		pCst = new SfxBroadcaster;
	return *pCst;
}

// Eines Tages kann man vielleicht den Parameter 0 schleifen,
// dann entfaellt die Kopiererei...

void SbxVariable::Broadcast( sal_uIntPtr nHintId )
{
	if( pCst && !IsSet( SBX_NO_BROADCAST ) && StaticIsEnabledBroadcasting() )
	{
		// Da die Methode von aussen aufrufbar ist, hier noch einmal
		// die Berechtigung testen
		if( nHintId & SBX_HINT_DATAWANTED )
			if( !CanRead() )
				return;
		if( nHintId & SBX_HINT_DATACHANGED )
			if( !CanWrite() )
				return;
		// Weitere Broadcasts verhindern
		SfxBroadcaster* pSave = pCst;
		pCst = NULL;
		sal_uInt16 nSaveFlags = GetFlags();
		SetFlag( SBX_READWRITE );
		if( mpPar.Is() )
			// this, als Element 0 eintragen, aber den Parent nicht umsetzen!
			mpPar->GetRef( 0 ) = this;
		pSave->Broadcast( SbxHint( nHintId, this ) );
		delete pCst; // wer weiss schon, auf welche Gedanken mancher kommt?
		pCst = pSave;
		SetFlags( nSaveFlags );
	}
}

SbxInfo* SbxVariable::GetInfo()
{
	if( !pInfo )
	{
		Broadcast( SBX_HINT_INFOWANTED );
		if( pInfo.Is() )
			SetModified( sal_True );
	}
	return pInfo;
}

void SbxVariable::SetInfo( SbxInfo* p )
{
	pInfo = p;
}

void SbxVariable::SetParameters( SbxArray* p )
{
	mpPar = p;
}


/////////////////////////// Name der Variablen ///////////////////////////

void SbxVariable::SetName( const XubString& rName )
{
	maName = rName;
	nHash = MakeHashCode( rName );
}

const XubString& SbxVariable::GetName( SbxNameType t ) const
{
	static char cSuffixes[] = "  %&!#@ $";
	if( t == SbxNAME_NONE )
		return maName;
	// Parameter-Infos anfordern (nicht fuer Objekte)
	((SbxVariable*)this)->GetInfo();
	// Nix anfuegen, wenn einfache Property (keine leeren Klammern)
	if( !pInfo
	 || ( !pInfo->aParams.Count() && GetClass() == SbxCLASS_PROPERTY ) )
		return maName;
	xub_Unicode cType = ' ';
	XubString aTmp( maName );
	// Kurzer Typ? Dann holen, evtl. ist dieser 0.
	SbxDataType et = GetType();
	if( t == SbxNAME_SHORT_TYPES )
	{
		if( et <= SbxSTRING )
			cType = cSuffixes[ et ];
		if( cType != ' ' )
			aTmp += cType;
	}
	aTmp += '(';
	for( sal_uInt16 i = 0; i < pInfo->aParams.Count(); i++ )
	{
		const SbxParamInfo* q = pInfo->aParams.GetObject( i );
		int nt = q->eType & 0x0FFF;
		if( i )
			aTmp += ',';
		if( q->nFlags & SBX_OPTIONAL )
			aTmp += String( SbxRes( STRING_OPTIONAL ) );
		if( q->eType & SbxBYREF )
			aTmp += String( SbxRes( STRING_BYREF ) );
		aTmp += q->aName;
		cType = ' ';
		// Kurzer Typ? Dann holen, evtl. ist dieser 0.
		if( t == SbxNAME_SHORT_TYPES )
		{
			if( nt <= SbxSTRING )
				cType = cSuffixes[ nt ];
		}
		if( cType != ' ' )
		{
			aTmp += cType;
			if( q->eType & SbxARRAY )
				aTmp.AppendAscii( "()" );
		}
		else
		{
			if( q->eType & SbxARRAY )
				aTmp.AppendAscii( "()" );
			// langer Typ?
			if( t != SbxNAME_SHORT )
			{
				aTmp += String( SbxRes( STRING_AS ) );
				if( nt < 32 )
					aTmp += String( SbxRes(
                        sal::static_int_cast< sal_uInt16 >( STRING_TYPES + nt ) ) );
				else
					aTmp += String( SbxRes( STRING_ANY ) );
			}
		}
	}
	aTmp += ')';
	// Langer Typ? Dann holen
	if( t == SbxNAME_LONG_TYPES && et != SbxEMPTY )
	{
		aTmp += String( SbxRes( STRING_AS ) );
		if( et < 32 )
			aTmp += String( SbxRes(
                sal::static_int_cast< sal_uInt16 >( STRING_TYPES + et ) ) );
		else
			aTmp += String( SbxRes( STRING_ANY ) );
	}
	((SbxVariable*) this)->aToolString = aTmp;
	return aToolString;
}

// Einen simplen Hashcode erzeugen: Es werden die ersten 6 Zeichen gewertet.

sal_uInt16 SbxVariable::MakeHashCode( const XubString& rName )
{
	sal_uInt16 n = 0;
	sal_uInt16 nLen = rName.Len();
	if( nLen > 6 )
		nLen = 6;
	const xub_Unicode* p = rName.GetBuffer();
	while( nLen-- )
	{
		sal_uInt8 c = (sal_uInt8)*p;
		p++;
		// Falls wir ein Schweinezeichen haben, abbrechen!!
		if( c >= 0x80 )
			return 0;
		n = sal::static_int_cast< sal_uInt16 >( ( n << 3 ) + toupper( c ) );
	}
	return n;
}

////////////////////////////// Operatoren ////////////////////////////////

SbxVariable& SbxVariable::operator=( const SbxVariable& r )
{
	SbxValue::operator=( r );
	delete mpSbxVariableImpl;
	if( r.mpSbxVariableImpl != NULL )
    {
		mpSbxVariableImpl = new SbxVariableImpl( *r.mpSbxVariableImpl );
		if( mpSbxVariableImpl->m_xComListener.is() )
			registerComListenerVariableForBasic( this, mpSbxVariableImpl->m_pComListenerParentBasic );
    }
	else
		mpSbxVariableImpl = NULL;
	return *this;
}

//////////////////////////////// Konversion ////////////////////////////////

SbxDataType SbxVariable::GetType() const
{
	if( aData.eType == SbxOBJECT )
		return aData.pObj ? aData.pObj->GetType() : SbxOBJECT;
	else if( aData.eType == SbxVARIANT )
		return aData.pObj ? aData.pObj->GetType() : SbxVARIANT;
	else
		return aData.eType;
}

SbxClassType SbxVariable::GetClass() const
{
	return SbxCLASS_VARIABLE;
}

void SbxVariable::SetModified( sal_Bool b )
{
	if( IsSet( SBX_NO_MODIFY ) )
		return;
	SbxBase::SetModified( b );
	if( pParent && pParent != this ) //??? HotFix: Rekursion raus MM
		pParent->SetModified( b );
}

void SbxVariable::SetParent( SbxObject* p )
{
#ifdef DBG_UTIL
	// wird der Parent eines SbxObjects gesetzt?
	if ( p && ISA(SbxObject) )
	{
		// dann mu\s dieses auch Child vom neuen Parent sein
		sal_Bool bFound = sal_False;
		SbxArray *pChilds = p->GetObjects();
		if ( pChilds )
		{
			for ( sal_uInt16 nIdx = 0; !bFound && nIdx < pChilds->Count(); ++nIdx )
				bFound = ( this == pChilds->Get(nIdx) );
		}
		if ( !bFound )
		{
			String aMsg = String::CreateFromAscii( "dangling: [" );
			aMsg += GetName();
			aMsg.AppendAscii( "].SetParent([" );
			aMsg += p->GetName();
			aMsg.AppendAscii( "])" );
			ByteString aBStr( (const UniString&)aMsg, RTL_TEXTENCODING_ASCII_US );
			DbgOut( aBStr.GetBuffer(), DBG_OUT_WARNING, __FILE__, __LINE__);
		}
	}
#endif

	pParent = p;
}

SbxVariableImpl* SbxVariable::getImpl( void )
{
	if( mpSbxVariableImpl == NULL )
		mpSbxVariableImpl = new SbxVariableImpl();
	return mpSbxVariableImpl;
}

const String& SbxVariable::GetDeclareClassName( void )
{
	SbxVariableImpl* pImpl = getImpl();
	return pImpl->m_aDeclareClassName;
}

void SbxVariable::SetDeclareClassName( const String& rDeclareClassName )
{
	SbxVariableImpl* pImpl = getImpl();
	pImpl->m_aDeclareClassName = rDeclareClassName;
}

void SbxVariable::SetComListener( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xComListener,
								  StarBASIC* pParentBasic )
{
	SbxVariableImpl* pImpl = getImpl();
	pImpl->m_xComListener = xComListener;
	pImpl->m_pComListenerParentBasic = pParentBasic;
	registerComListenerVariableForBasic( this, pParentBasic );
}

void SbxVariable::ClearComListener( void )
{
	SbxVariableImpl* pImpl = getImpl();
	pImpl->m_xComListener.clear();
}


////////////////////////////// Laden/Speichern /////////////////////////////

sal_Bool SbxVariable::LoadData( SvStream& rStrm, sal_uInt16 nVer )
{
	sal_uInt16 nType;
	sal_uInt8 cMark;
	rStrm >> cMark;
	if( cMark == 0xFF )
	{
		if( !SbxValue::LoadData( rStrm, nVer ) )
			return sal_False;
		rStrm.ReadByteString( maName, RTL_TEXTENCODING_ASCII_US );
		sal_uInt32 nTemp;
		rStrm >> nTemp;
		nUserData = nTemp;
	}
	else
	{
		rStrm.SeekRel( -1L );
		rStrm >> nType;
		rStrm.ReadByteString( maName, RTL_TEXTENCODING_ASCII_US );
		sal_uInt32 nTemp;
		rStrm >> nTemp;
		nUserData = nTemp;
		// Korrektur: Alte Methoden haben statt SbxNULL jetzt SbxEMPTY
		if( nType == SbxNULL && GetClass() == SbxCLASS_METHOD )
			nType = SbxEMPTY;
		SbxValues aTmp;
        String aTmpString;
		::rtl::OUString aVal;
		aTmp.eType = aData.eType = (SbxDataType) nType;
		aTmp.pOUString = &aVal;
		switch( nType )
		{
			case SbxBOOL:
			case SbxERROR:
			case SbxINTEGER:
				rStrm >> aTmp.nInteger; break;
			case SbxLONG:
				rStrm >> aTmp.nLong; break;
			case SbxSINGLE:
			{
				// Floats als ASCII
				rStrm.ReadByteString( aTmpString, RTL_TEXTENCODING_ASCII_US );
				double d;
				SbxDataType t;
				if( ImpScan( aTmpString, d, t, NULL ) != SbxERR_OK || t == SbxDOUBLE )
				{
					aTmp.nSingle = 0;
					return sal_False;
				}
				aTmp.nSingle = (float) d;
				break;
			}
			case SbxDATE:
			case SbxDOUBLE:
			{
				// Floats als ASCII
				rStrm.ReadByteString( aTmpString, RTL_TEXTENCODING_ASCII_US );
				SbxDataType t;
				if( ImpScan( aTmpString, aTmp.nDouble, t, NULL ) != SbxERR_OK )
				{
					aTmp.nDouble = 0;
					return sal_False;
				}
				break;
			}
			case SbxSTRING:
				rStrm.ReadByteString( aTmpString, RTL_TEXTENCODING_ASCII_US );
                aVal = aTmpString;
				break;
			case SbxEMPTY:
			case SbxNULL:
				break;
			default:
				aData.eType = SbxNULL;
				DBG_ASSERT( sal_False, "Unsupported data type loaded" );
				return sal_False;
		}
		// Wert putten
		if( nType != SbxNULL && nType != SbxEMPTY && !Put( aTmp ) )
			return sal_False;
	}
	rStrm >> cMark;
	// cMark ist auch eine Versionsnummer!
	// 1: initial version
	// 2: mit nUserData
	if( cMark )
	{
		if( cMark > 2 )
			return sal_False;
		pInfo = new SbxInfo;
		pInfo->LoadData( rStrm, (sal_uInt16) cMark );
	}
	// Privatdaten nur laden, wenn es eine SbxVariable ist
	if( GetClass() == SbxCLASS_VARIABLE && !LoadPrivateData( rStrm, nVer ) )
		return sal_False;
	((SbxVariable*) this)->Broadcast( SBX_HINT_DATACHANGED );
	nHash =  MakeHashCode( maName );
	SetModified( sal_True );
	return sal_True;
}

sal_Bool SbxVariable::StoreData( SvStream& rStrm ) const
{
	rStrm << (sal_uInt8) 0xFF;		// Marker
	sal_Bool bValStore;
	if( this->IsA( TYPE(SbxMethod) ) )
	{
		// #50200 Verhindern, dass Objekte, die zur Laufzeit als Return-Wert
		// in der Methode als Value gespeichert sind, mit gespeichert werden
		SbxVariable* pThis = (SbxVariable*)this;
		sal_uInt16 nSaveFlags = GetFlags();
		pThis->SetFlag( SBX_WRITE );
		pThis->SbxValue::Clear();
		pThis->SetFlags( nSaveFlags );

		// Damit die Methode in keinem Fall ausgefuehrt wird!
		// CAST, um const zu umgehen!
		pThis->SetFlag( SBX_NO_BROADCAST );
		bValStore = SbxValue::StoreData( rStrm );
		pThis->ResetFlag( SBX_NO_BROADCAST );
	}
	else
		bValStore = SbxValue::StoreData( rStrm );
	if( !bValStore )
		return sal_False;
	// if( !SbxValue::StoreData( rStrm ) )
		// return sal_False;
	rStrm.WriteByteString( maName, RTL_TEXTENCODING_ASCII_US );
	rStrm << (sal_uInt32)nUserData;
	if( pInfo.Is() )
	{
		rStrm << (sal_uInt8) 2;		// Version 2: mit UserData!
		pInfo->StoreData( rStrm );
	}
	else
		rStrm << (sal_uInt8) 0;
	// Privatdaten nur speichern, wenn es eine SbxVariable ist
	if( GetClass() == SbxCLASS_VARIABLE )
		return StorePrivateData( rStrm );
	else
		return sal_True;
}

////////////////////////////// SbxInfo ///////////////////////////////////

SbxInfo::SbxInfo() : aHelpFile(), nHelpId( 0 ), aParams()
{}

SbxInfo::SbxInfo( const String& r, sal_uInt32 n )
	   : aHelpFile( r ), nHelpId( n ), aParams()
{}

////////////////////////////// SbxAlias //////////////////////////////////

SbxAlias::SbxAlias( const XubString& rName, SbxVariable* p )
		: SbxVariable(), xAlias( p )
{
	SetName( rName );
	SetFlags( p->GetFlags() );
	SetFlag( SBX_DONTSTORE );
	aData.eType = p->GetType();
	StartListening( p->GetBroadcaster() );
}

SbxAlias::SbxAlias( const SbxAlias& r )
		: SvRefBase( r ), SbxVariable( r ),
          SfxListener( r ), xAlias( r.xAlias )
{}

SbxAlias& SbxAlias::operator=( const SbxAlias& r )
{
	xAlias = r.xAlias;
	return *this;
}

SbxAlias::~SbxAlias()
{
	if( xAlias.Is() )
		EndListening( xAlias->GetBroadcaster() );
}

void SbxAlias::Broadcast( sal_uIntPtr nHt )
{
	if( xAlias.Is() && StaticIsEnabledBroadcasting() )
	{
		xAlias->SetParameters( GetParameters() );
		if( nHt == SBX_HINT_DATAWANTED )
			SbxVariable::operator=( *xAlias );
		else if( nHt == SBX_HINT_DATACHANGED || nHt == SBX_HINT_CONVERTED )
			*xAlias = *this;
		else if( nHt == SBX_HINT_INFOWANTED )
		{
			xAlias->Broadcast( nHt );
			pInfo = xAlias->GetInfo();
		}
	}
}

void SbxAlias::SFX_NOTIFY( SfxBroadcaster&, const TypeId&,
						   const SfxHint& rHint, const TypeId& )
{
	const SbxHint* p = PTR_CAST(SbxHint,&rHint);
	if( p && p->GetId() == SBX_HINT_DYING )
	{
		xAlias.Clear();
		// Alias loeschen?
		if( pParent )
			pParent->Remove( this );
	}
}

void SbxVariable::Dump( SvStream& rStrm, sal_Bool bFill )
{
	ByteString aBNameStr( (const UniString&)GetName( SbxNAME_SHORT_TYPES ), RTL_TEXTENCODING_ASCII_US );
	rStrm << "Variable( "
          << ByteString::CreateFromInt64( (sal_uIntPtr) this ).GetBuffer() << "=="
		  << aBNameStr.GetBuffer();
	ByteString aBParentNameStr( (const UniString&)GetParent()->GetName(), RTL_TEXTENCODING_ASCII_US );
	if ( GetParent() )
		rStrm << " in parent '" << aBParentNameStr.GetBuffer() << "'";
	else
		rStrm << " no parent";
	rStrm << " ) ";

	// bei Object-Vars auch das Object ausgeben
	if ( GetValues_Impl().eType == SbxOBJECT &&
			GetValues_Impl().pObj &&
			GetValues_Impl().pObj != this &&
			GetValues_Impl().pObj != GetParent() )
	{
		rStrm << " contains ";
		((SbxObject*) GetValues_Impl().pObj)->Dump( rStrm, bFill );
	}
	else
		rStrm << endl;
}

