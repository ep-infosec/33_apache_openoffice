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
#include "precompiled_sfx2.hxx"
#include <vcl/wrkwin.hxx>
#include <svl/rectitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <basic/sbstar.hxx>
#include <svl/stritem.hxx>
#include <svl/svdde.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>

#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/pathoptions.hxx>
#ifndef GCC
#endif

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include <sfx2/objsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/sfxsids.hrc>
#include "helper.hxx"
#include <sfx2/docfile.hxx>

//========================================================================

String SfxDdeServiceName_Impl( const String& sIn )
{
	ByteString sTemp( sIn, RTL_TEXTENCODING_UTF8 );
	ByteString sReturn;

	for ( sal_uInt16 n = sTemp.Len(); n; --n )
		if ( sTemp.Copy( n-1, 1 ).IsAlphaNumericAscii() )
			sReturn += sTemp.GetChar(n-1);

	return String( sReturn, RTL_TEXTENCODING_UTF8 );
}


class ImplDdeService : public DdeService
{
public:
	ImplDdeService( const String& rNm )
		: DdeService( rNm )
	{}
	virtual sal_Bool MakeTopic( const String& );

	virtual String	Topics();
//	virtual String	Formats();
//	virtual String	SysItems();
//	virtual String	Status();

	virtual sal_Bool SysTopicExecute( const String* pStr );
};

class SfxDdeTriggerTopic_Impl : public DdeTopic
{
public:
	SfxDdeTriggerTopic_Impl()
	: DdeTopic( DEFINE_CONST_UNICODE("TRIGGER") )
	{}

	virtual sal_Bool Execute( const String* );
};

class SfxDdeDocTopic_Impl : public DdeTopic
{
public:
	SfxObjectShell* pSh;
	DdeData aData;
	::com::sun::star::uno::Sequence< sal_Int8 > aSeq;

	SfxDdeDocTopic_Impl( SfxObjectShell* pShell )
		: DdeTopic( pShell->GetTitle(SFX_TITLE_FULLNAME) ), pSh( pShell )
	{}

	virtual DdeData* Get( sal_uIntPtr );
	virtual sal_Bool Put( const DdeData* );
	virtual sal_Bool Execute( const String* );
	virtual sal_Bool StartAdviseLoop();
	virtual sal_Bool MakeItem( const String& rItem );

// wird benoetigt?
//	virtual void Connect( long n );
//	virtual void Disconnect( long n );
//	virtual void StopAdviseLoop();

};


SV_DECL_PTRARR( SfxDdeDocTopics_Impl, SfxDdeDocTopic_Impl *, 4, 4 )
SV_IMPL_PTRARR( SfxDdeDocTopics_Impl, SfxDdeDocTopic_Impl *)

//========================================================================

sal_Bool SfxAppEvent_Impl( ApplicationEvent &rAppEvent,
					   const String &rCmd, const String &rEvent )

/*	[Beschreibung]

	Pr"uft, ob 'rCmd' das Event 'rEvent' ist (ohne '(') und baut
	aus diesem dann ein <ApplicationEvent> zusammen, das per
	<Application::AppEvent()> ausgef"uhrt werden kann. Ist 'rCmd' das
	angegegeben Event 'rEvent', dann wird TRUE zur"uckgegeben, sonst FALSE.


	[Beispiel]

	rCmd = "Open(\"d:\doc\doc.sdw\")"
	rEvent = "Open"
*/

{
	String aEvent( rEvent );
	aEvent += '(';
    if ( rCmd.CompareIgnoreCaseToAscii( aEvent, aEvent.Len() ) == COMPARE_EQUAL )
	{
		String aData( rCmd );
		aData.Erase( 0, aEvent.Len() );
		if ( aData.Len() > 2 )
		{
			// in das ApplicationEvent-Format wandeln
			aData.Erase( aData.Len()-1, 1 );
			for ( sal_uInt16 n = 0; n < aData.Len(); ++n )
			{
				if ( aData.GetChar(n) == 0x0022 ) // " = 22h
					for ( ; aData.GetChar(++n) != 0x0022 ; )
						/* empty loop */ ;
				else if ( aData.GetChar(n) == 0x0020 ) // SPACE = 20h
					aData.SetChar(n, '\n');
			}
			aData.EraseAllChars( 0x0022 );
			ApplicationAddress aAddr;
			rAppEvent = ApplicationEvent( String(), aAddr, ByteString( rEvent, RTL_TEXTENCODING_UTF8), aData );
			return sal_True;
		}
	}

	return sal_False;
}

//-------------------------------------------------------------------------

long SfxApplication::DdeExecute
(
	const String&	rCmd		// in unserer BASIC-Syntax formuliert
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxApplication-Subklasse gerichtete DDE-Kommandos
	zu empfangen.

	Die Basisimplementierung versteht die API-Funktionalit"at der
	betreffenden SfxApplication-Subklasse in BASIC-Syntax. R"uckgabewerte
	k"onnen dabei leider nicht "ubertragen werden.
*/

{
	// Print oder Open-Event?
	ApplicationEvent aAppEvent;
	if ( SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Print") ) ||
		 SfxAppEvent_Impl( aAppEvent, rCmd, DEFINE_CONST_UNICODE("Open") ) )
		GetpApp()->AppEvent( aAppEvent );
	else
	{
		// alle anderen per BASIC
		StarBASIC* pBasic = GetBasic();
        ENSURE_OR_RETURN( pBasic, "where's my basic?", 0 );
		SbxVariable* pRet = pBasic->Execute( rCmd );
		if( !pRet )
		{
			SbxBase::ResetError();
			return 0;
		}
	}
	return 1;
}

//--------------------------------------------------------------------

long SfxApplication::DdeGetData
(
	const String&,				// das anzusprechende Item
	const String&,				// in: Format
	::com::sun::star::uno::Any& // out: angeforderte Daten
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxApplication-Subklasse gerichtete DDE-Daten-Anforderungen
	zu empfangen.

	Die Basisimplementierung liefert keine Daten und gibt 0 zur"uck.
*/

{
	return 0;
}

//--------------------------------------------------------------------

long SfxApplication::DdeSetData
(
	const String&,                    // das anzusprechende Item
	const String&,				      // in: Format
	const ::com::sun::star::uno::Any& // out: angeforderte Daten
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxApplication-Subklasse gerichtete DDE-Daten
	zu empfangen.

	Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/

{
	return 0;
}

//--------------------------------------------------------------------

::sfx2::SvLinkSource* SfxApplication::DdeCreateLinkSource
(
	const String&	   // das zu erzeugende Item
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seiner SfxApplication-Subklasse einen DDE-Hotlink einzurichten

	Die Basisimplementierung erzeugt keinen und liefert 0 zur"uck.
*/

{
	return 0;
}

//========================================================================

long SfxObjectShell::DdeExecute
(
	const String&	rCmd		// in unserer BASIC-Syntax formuliert
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxObjectShell-Subklasse gerichtete DDE-Kommandos
	zu empfangen.

	Die Basisimplementierung f"uhrt nichts aus und liefert 0 zur"uck.
*/

{
	StarBASIC* pBasic = GetBasic();
	DBG_ASSERT( pBasic, "Wo ist mein Basic???" ) ;
	SbxVariable* pRet = pBasic->Execute( rCmd );
	if( !pRet )
	{
		SbxBase::ResetError();
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------------

long SfxObjectShell::DdeGetData
(
	const String&,				// das anzusprechende Item
	const String&,				// in: Format
	::com::sun::star::uno::Any& // out: angeforderte Daten
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxObjectShell-Subklasse gerichtete DDE-Daten-Anforderungen
	zu empfangen.

	Die Basisimplementierung liefert keine Daten und gibt 0 zur"uck.
*/

{
	return 0;
}

//--------------------------------------------------------------------

long SfxObjectShell::DdeSetData
(
	const String&,					  // das anzusprechende Item
	const String&,					  // in: Format
	const ::com::sun::star::uno::Any& // out: angeforderte Daten
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxObjectShell-Subklasse gerichtete DDE-Daten
	zu empfangen.

	Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/

{
	return 0;
}

//--------------------------------------------------------------------
::sfx2::SvLinkSource* SfxObjectShell::DdeCreateLinkSource
(
	const String&	   // das zu erzeugende Item
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seiner SfxObjectShell-Subklasse einen DDE-Hotlink einzurichten

	Die Basisimplementierung erzeugt keinen und liefert 0 zur"uck.
*/

{
	return 0;
}

//========================================================================

long SfxViewFrame::DdeExecute
(
	const String&	rCmd		// in unserer BASIC-Syntax formuliert
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxViewFrame-Subklasse gerichtete DDE-Kommandos
	zu empfangen.

	Die Basisimplementierung versteht die API-Funktionalit"at des
	betreffenden SfxViewFrame, der darin dargestellten SfxViewShell und
	der betreffenden SfxObjectShell-Subklasse in BASIC-Syntax.
	R"uckgabewerte k"onnen dabei leider nicht "ubertragen werden.
*/

{
	if ( GetObjectShell() )
		return GetObjectShell()->DdeExecute( rCmd );

	return 0;
}

//--------------------------------------------------------------------

long SfxViewFrame::DdeGetData
(
	const String&,				// das anzusprechende Item
	const String&,				// in: Format
	::com::sun::star::uno::Any& // out: angeforderte Daten
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxViewFrame-Subklasse gerichtete DDE-Daten-Anforderungen
	zu empfangen.

	Die Basisimplementierung liefert keine Daten und gibt 0 zur"uck.
*/

{
	return 0;
}

//--------------------------------------------------------------------

long SfxViewFrame::DdeSetData
(
	const String& ,						// das anzusprechende Item
	const String& ,					    // in: Format
	const ::com::sun::star::uno::Any&   // out: angeforderte Daten
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seine SfxViewFrame-Subklasse gerichtete DDE-Daten
	zu empfangen.

	Die Basisimplementierung nimmt keine Daten entgegen und liefert 0 zur"uck.
*/

{
	return 0;
}

//--------------------------------------------------------------------

::sfx2::SvLinkSource* SfxViewFrame::DdeCreateLinkSource
(
	const String& // das zu erzeugende Item
)

/*	[Beschreibung]

	Diese Methode kann vom Applikationsentwickler "uberladen werden,
	um an seiner SfxViewFrame-Subklasse einen DDE-Hotlink einzurichten

	Die Basisimplementierung erzeugt keinen und liefert 0 zur"uck.
*/

{
	return 0;
}

//========================================================================

sal_Bool SfxApplication::InitializeDde()
{
	DBG_ASSERT( !pAppData_Impl->pDdeService,
				"Dde kann nicht mehrfach initialisiert werden" );

	pAppData_Impl->pDdeService = new ImplDdeService( Application::GetAppName() );
	int nError = pAppData_Impl->pDdeService->GetError();
	if( !nError )
	{
		pAppData_Impl->pDocTopics = new SfxDdeDocTopics_Impl;

		// wir wollen auf jedenfall RTF unterstuetzen!
		pAppData_Impl->pDdeService->AddFormat( FORMAT_RTF );

		// Config-Pfad als Topic wegen Mehrfachstart
        INetURLObject aOfficeLockFile( SvtPathOptions().GetUserConfigPath() );
		aOfficeLockFile.insertName( DEFINE_CONST_UNICODE( "soffice.lck" ) );
        String aService( SfxDdeServiceName_Impl(
					aOfficeLockFile.GetMainURL(INetURLObject::DECODE_TO_IURI) ) );
		aService.ToUpperAscii();
		pAppData_Impl->pDdeService2 = new ImplDdeService( aService );
		pAppData_Impl->pTriggerTopic = new SfxDdeTriggerTopic_Impl;
		pAppData_Impl->pDdeService2->AddTopic( *pAppData_Impl->pTriggerTopic );
	}
	return !nError;
}

void SfxAppData_Impl::DeInitDDE()
{
    DELETEZ( pTriggerTopic );
    DELETEZ( pDdeService2 );
    DELETEZ( pDocTopics );
    DELETEZ( pDdeService );
}

//--------------------------------------------------------------------

void SfxApplication::AddDdeTopic( SfxObjectShell* pSh )
{
	DBG_ASSERT( pAppData_Impl->pDocTopics, "es gibt gar keinen Dde-Service" );
	//OV: Im Serverbetrieb ist DDE abgeklemmt!
	if( !pAppData_Impl->pDocTopics )
		return;

	// doppeltes Eintragen verhindern
	String sShellNm;
	sal_Bool bFnd = sal_False;
	for( sal_uInt16 n = pAppData_Impl->pDocTopics->Count(); n; )
		if( (*pAppData_Impl->pDocTopics)[ --n ]->pSh == pSh )
		{
			// JP 18.03.96 - Bug 26470
			//	falls das Document unbenannt wurde, ist trotzdem ein
			//	neues Topics anzulegen!
			if( !bFnd )
			{
				bFnd = sal_True;
				(sShellNm = pSh->GetTitle(SFX_TITLE_FULLNAME)).ToLowerAscii();
			}
			String sNm( (*pAppData_Impl->pDocTopics)[ n ]->GetName() );
			if( sShellNm == sNm.ToLowerAscii() )
				return ;
		}

	const SfxDdeDocTopic_Impl* pTopic = new SfxDdeDocTopic_Impl( pSh );
	pAppData_Impl->pDocTopics->Insert( pTopic,
									   pAppData_Impl->pDocTopics->Count() );
	pAppData_Impl->pDdeService->AddTopic( *pTopic );
}

void SfxApplication::RemoveDdeTopic( SfxObjectShell* pSh )
{
	DBG_ASSERT( pAppData_Impl->pDocTopics, "es gibt gar keinen Dde-Service" );
	//OV: Im Serverbetrieb ist DDE abgeklemmt!
	if( !pAppData_Impl->pDocTopics )
		return;

	SfxDdeDocTopic_Impl* pTopic;
	for( sal_uInt16 n = pAppData_Impl->pDocTopics->Count(); n; )
		if( ( pTopic = (*pAppData_Impl->pDocTopics)[ --n ])->pSh == pSh )
		{
			pAppData_Impl->pDdeService->RemoveTopic( *pTopic );
			pAppData_Impl->pDocTopics->DeleteAndDestroy( n );
		}
}

const DdeService* SfxApplication::GetDdeService() const
{
	return pAppData_Impl->pDdeService;
}

DdeService* SfxApplication::GetDdeService()
{
	return pAppData_Impl->pDdeService;
}

//--------------------------------------------------------------------

sal_Bool ImplDdeService::MakeTopic( const String& rNm )
{
	// Workaround gegen Event nach unserem Main() unter OS/2
	// passierte wenn man beim Beenden aus dem OffMgr die App neu startet
	if ( !Application::IsInExecute() )
		return sal_False;

	// das Topic rNm wird gesucht, haben wir es ?
	// erstmal nur ueber die ObjectShells laufen und die mit dem
	// Namen heraussuchen:
	sal_Bool bRet = sal_False;
	String sNm( rNm );
	sNm.ToLowerAscii();
	TypeId aType( TYPE(SfxObjectShell) );
	SfxObjectShell* pShell = SfxObjectShell::GetFirst( &aType );
	while( pShell )
	{
		String sTmp( pShell->GetTitle(SFX_TITLE_FULLNAME) );
		sTmp.ToLowerAscii();
		if( sTmp == sNm )		// die wollen wir haben
		{
			SFX_APP()->AddDdeTopic( pShell );
			bRet = sal_True;
			break;
		}
		pShell = SfxObjectShell::GetNext( *pShell, &aType );
	}

	if( !bRet )
	{
        INetURLObject aWorkPath( SvtPathOptions().GetWorkPath() );
		INetURLObject aFile;
		if ( aWorkPath.GetNewAbsURL( rNm, &aFile ) &&
			 SfxContentHelper::IsDocument( aFile.GetMainURL( INetURLObject::NO_DECODE ) ) )
		{
			// File vorhanden

			// dann versuche die Datei zu laden:
			SfxStringItem aName( SID_FILE_NAME, aFile.GetMainURL( INetURLObject::NO_DECODE ) );
			SfxBoolItem aNewView(SID_OPEN_NEW_VIEW, sal_True);

			SfxBoolItem aSilent(SID_SILENT, sal_True);
            SfxDispatcher* pDispatcher = SFX_APP()->GetDispatcher_Impl();
            const SfxPoolItem* pRet = pDispatcher->Execute( SID_OPENDOC,
					SFX_CALLMODE_SYNCHRON,
					&aName, &aNewView,
					&aSilent, 0L );

			if( pRet && pRet->ISA( SfxViewFrameItem ) &&
				((SfxViewFrameItem*)pRet)->GetFrame() &&
				0 != ( pShell = ((SfxViewFrameItem*)pRet)
					->GetFrame()->GetObjectShell() ) )
			{
				SFX_APP()->AddDdeTopic( pShell );
				bRet = sal_True;
			}
		}
	}
	return bRet;
}

String ImplDdeService::Topics()
{
	String sRet;
	if( GetSysTopic() )
		sRet += GetSysTopic()->GetName();

	TypeId aType( TYPE(SfxObjectShell) );
	SfxObjectShell* pShell = SfxObjectShell::GetFirst( &aType );
	while( pShell )
	{
		if( SfxViewFrame::GetFirst( pShell ) )
		{
			if( sRet.Len() )
				sRet += '\t';
			sRet += pShell->GetTitle(SFX_TITLE_FULLNAME);
		}
		pShell = SfxObjectShell::GetNext( *pShell, &aType );
	}
	if( sRet.Len() )
		sRet += DEFINE_CONST_UNICODE("\r\n");
	return sRet;
}

sal_Bool ImplDdeService::SysTopicExecute( const String* pStr )
{
	return (sal_Bool)SFX_APP()->DdeExecute( *pStr );
}

//--------------------------------------------------------------------

sal_Bool SfxDdeTriggerTopic_Impl::Execute( const String* )
{
	return sal_True;
}

//--------------------------------------------------------------------
DdeData* SfxDdeDocTopic_Impl::Get( sal_uIntPtr nFormat )
{
	String sMimeType( SotExchange::GetFormatMimeType( nFormat ));
	::com::sun::star::uno::Any aValue;
	long nRet = pSh->DdeGetData( GetCurItem(), sMimeType, aValue );
	if( nRet && aValue.hasValue() && ( aValue >>= aSeq ) )
	{
		aData = DdeData( aSeq.getConstArray(), aSeq.getLength(), nFormat );
		return &aData;
	}
	aSeq.realloc( 0 );
	return 0;
}

sal_Bool SfxDdeDocTopic_Impl::Put( const DdeData* pData )
{
	aSeq = ::com::sun::star::uno::Sequence< sal_Int8 >(
							(sal_Int8*)(const void*)*pData, (long)*pData );
	sal_Bool bRet;
	if( aSeq.getLength() )
	{
		::com::sun::star::uno::Any aValue;
		aValue <<= aSeq;
		String sMimeType( SotExchange::GetFormatMimeType( pData->GetFormat() ));
		bRet = 0 != pSh->DdeSetData( GetCurItem(), sMimeType, aValue );
	}
	else
		bRet = sal_False;
	return bRet;
}

sal_Bool SfxDdeDocTopic_Impl::Execute( const String* pStr )
{
	long nRet = pStr ? pSh->DdeExecute( *pStr ) : 0;
	return 0 != nRet;
}

sal_Bool SfxDdeDocTopic_Impl::MakeItem( const String& rItem )
{
	AddItem( DdeItem( rItem ) );
	return sal_True;
}

sal_Bool SfxDdeDocTopic_Impl::StartAdviseLoop()
{
	sal_Bool bRet = sal_False;
	::sfx2::SvLinkSource* pNewObj = pSh->DdeCreateLinkSource( GetCurItem() );
	if( pNewObj )
	{
		// dann richten wir auch einen entsprechenden SvBaseLink ein
		String sNm, sTmp( Application::GetAppName() );
		::sfx2::MakeLnkName( sNm, &sTmp, pSh->GetTitle(SFX_TITLE_FULLNAME), GetCurItem() );
        new ::sfx2::SvBaseLink( sNm, OBJECT_DDE_EXTERN, pNewObj );
		bRet = sal_True;
	}
	return bRet;
}

