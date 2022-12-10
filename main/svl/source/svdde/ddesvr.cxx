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
#include "precompiled_svl.hxx"

#define UNICODE
#include "ddeimp.hxx"
#include <svl/svdde.hxx>
#include <svl/svarray.hxx>
#include <tools/debug.hxx>
#include <osl/thread.h>

//static long         hCurConv  = 0;
//static DWORD        hDdeInst  = NULL;
//static short        nInstance = 0;
//static DdeServices* pServices;

enum DdeItemType
{
	DDEITEM,
	DDEGETPUTITEM
};

struct DdeItemImpData
{
	sal_uLong nHCnv;
	sal_uInt16 nCnt;

	DdeItemImpData( sal_uLong nH ) : nHCnv( nH ), nCnt( 1 ) {}
};

SV_DECL_VARARR( DdeItemImp, DdeItemImpData, 1, 1 )
SV_IMPL_VARARR( DdeItemImp, DdeItemImpData )

// --- DdeInternat::SvrCallback() ----------------------------------

#ifdef WNT
HDDEDATA CALLBACK DdeInternal::SvrCallback(
			WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
			HDDEDATA hData, DWORD, DWORD )
#else
#if defined ( MTW ) || ( defined ( GCC ) && defined ( OS2 )) || defined( ICC )
HDDEDATA CALLBACK __EXPORT DdeInternal::SvrCallback(
			WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
			HDDEDATA hData, DWORD, DWORD )
#else
HDDEDATA CALLBACK _export DdeInternal::SvrCallback(
			WORD nCode, WORD nCbType, HCONV hConv, HSZ hText1, HSZ hText2,
			HDDEDATA hData, DWORD, DWORD )
#endif
#endif
{
	DdeServices&    rAll = DdeService::GetServices();
	DdeService*     pService;
	DdeTopic*       pTopic;
	DdeItem*        pItem;
	DdeData*        pData;
	Conversation*   pC;

	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");

	switch( nCode )
	{
		case XTYP_WILDCONNECT:
		{
			int nTopics = 0;

#if 1
			TCHAR chTopicBuf[250];
			if( hText1 )
				DdeQueryString( pInst->hDdeInstSvr, hText1, chTopicBuf,
								sizeof(chTopicBuf)/sizeof(TCHAR), CP_WINUNICODE );

			for( pService = rAll.First();pService;pService = rAll.Next() )
			{
				if ( !hText2 || ( *pService->pName == hText2 ) )
				{
					String sTopics( pService->Topics() );
					if( sTopics.Len() )
					{
						if( hText1 )
						{
							sal_uInt16 n = 0;
							while( STRING_NOTFOUND != n )
							{
								String s( sTopics.GetToken( 0, '\t', n ));
								if( s == reinterpret_cast<const sal_Unicode*>(chTopicBuf) )
									++nTopics;
							}
						}
						else
							nTopics += sTopics.GetTokenCount( '\t' );
					}
				}
			}

#else
			for( pService = rAll.First();pService;pService = rAll.Next() )
			{
				if ( !hText2 || ( *pService->pName == hText2 ) )
				{
					for( pTopic = pService->aTopics.First(); pTopic;
						 pTopic = pService->aTopics.Next() )
					{
						if ( !hText1 || (*pTopic->pName == hText1) )
							nTopics++;
					}
				}
			}
#endif
			if( !nTopics )
				return (HDDEDATA)NULL;

			HSZPAIR* pPairs = new HSZPAIR [nTopics + 1];
			if ( !pPairs )
				return (HDDEDATA)NULL;

			HSZPAIR* q = pPairs;
			for( pService = rAll.First(); pService; pService = rAll.Next() )
			{
				if ( !hText2 || (*pService->pName == hText2 ) )
				{
#if 0
					for ( pTopic = pService->aTopics.First(); pTopic;
						  pTopic = pService->aTopics.Next() )
					{
						if ( !hText1 || (*pTopic->pName == hText1) )
						{
							q->hszSvc   = *pService->pName;
							q->hszTopic = *pTopic->pName;
							q++;
						}
					}
#else
					String sTopics( pService->Topics() );
					sal_uInt16 n = 0;
					while( STRING_NOTFOUND != n )
					{
						String s( sTopics.GetToken( 0, '\t', n ));
						s.EraseAllChars( '\n' ).EraseAllChars( '\r' );
						if( !hText1 || s == reinterpret_cast<const sal_Unicode*>(chTopicBuf) )
						{
							DdeString aDStr( pInst->hDdeInstSvr, s );
							pTopic = FindTopic( *pService, (HSZ)aDStr );
							if( pTopic )
							{
								q->hszSvc   = *pService->pName;
								q->hszTopic = *pTopic->pName;
								q++;
							}
						}
					}

#endif
				}
			}

			q->hszSvc   = NULL;
			q->hszTopic = NULL;
			HDDEDATA h = DdeCreateDataHandle(
							pInst->hDdeInstSvr, (LPBYTE) pPairs,
							sizeof(HSZPAIR) * (nTopics+1),
							0, NULL, nCbType, 0);
			delete [] pPairs;
			return h;
		}

		case XTYP_CONNECT:
			pService = FindService( hText2 );
			if ( pService)
				pTopic = FindTopic( *pService, hText1 );
			else
				pTopic = NULL;
			if ( pTopic )
				return (HDDEDATA)DDE_FACK;
			else
				return (HDDEDATA) NULL;

		case XTYP_CONNECT_CONFIRM:
			pService = FindService( hText2 );
			if ( pService )
			{
				pTopic = FindTopic( *pService, hText1 );
				if ( pTopic )
				{
					pTopic->Connect( (long) hConv );
					pC = new Conversation;
					pC->hConv = hConv;
					pC->pTopic = pTopic;
					pService->pConv->Insert( pC );
				}
			}
			return (HDDEDATA)NULL;
	}

	for ( pService = rAll.First(); pService; pService = rAll.Next() )
	{
		for( pC = pService->pConv->First(); pC;
			 pC = pService->pConv->Next() )
		{
			if ( pC->hConv == hConv )
				goto found;
		}
	}

	return (HDDEDATA) DDE_FNOTPROCESSED;

found:
	if ( nCode == XTYP_DISCONNECT)
	{
		pC->pTopic->_Disconnect( (long) hConv );
		pService->pConv->Remove( pC );
		delete pC;
		return (HDDEDATA)NULL;
	}

	sal_Bool bExec = sal_Bool(nCode == XTYP_EXECUTE);
	pTopic = pC->pTopic;
	if ( pTopic && !bExec )
		pItem = FindItem( *pTopic, hText2 );
	else
		pItem = NULL;

	if ( !bExec && !pService->HasCbFormat( nCbType ) )
		pItem = NULL;
	if ( !pItem && !bExec )
		return (HDDEDATA)DDE_FNOTPROCESSED;
	if ( pItem )
		pTopic->aItem = pItem->GetName();
	else
		pTopic->aItem.Erase();

	sal_Bool bRes = sal_False;
	pInst->hCurConvSvr = hConv;
	switch( nCode )
	{
		case XTYP_REQUEST:
		case XTYP_ADVREQ:
			{
			String aRes;          // darf erst am Ende freigegeben werden!!
			if ( pTopic->IsSystemTopic() )
			{
				if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_TOPICS) )
					aRes = pService->Topics();
				else if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_SYSITEMS) )
					aRes = pService->SysItems();
				else if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_STATUS) )
					aRes = pService->Status();
				else if ( pTopic->aItem == reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_FORMATS) )
					aRes = pService->Formats();
				else if ( pTopic->aItem ==  reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_HELP) )
					aRes = pService->GetHelp();
				else
					aRes = pService->SysTopicGet( pTopic->aItem );

				if ( aRes.Len() )
					pData = new DdeData( aRes );
				else
					pData = NULL;
			}
			else if( DDEGETPUTITEM == pItem->nType )
				pData = ((DdeGetPutItem*)pItem)->Get(
							DdeData::GetInternalFormat( nCbType ) );
			else
				pData = pTopic->Get( DdeData::GetInternalFormat( nCbType ));

			if ( pData )
				return DdeCreateDataHandle( pInst->hDdeInstSvr,
											(LPBYTE)pData->pImp->pData,
											pData->pImp->nData,
											0, hText2,
											DdeData::GetExternalFormat(
												pData->pImp->nFmt ),
											0 );
			}
			break;

		case XTYP_POKE:
			if ( !pTopic->IsSystemTopic() )
			{
				DdeData d;
				d.pImp->hData = hData;
				d.pImp->nFmt  = DdeData::GetInternalFormat( nCbType );
				d.Lock();
				if( DDEGETPUTITEM == pItem->nType )
					bRes = ((DdeGetPutItem*)pItem)->Put( &d );
				else
					bRes = pTopic->Put( &d );
			}
			pInst->hCurConvSvr = NULL;
			if ( bRes )
				return (HDDEDATA)DDE_FACK;
			else
				return (HDDEDATA) DDE_FNOTPROCESSED;

		case XTYP_ADVSTART:
			{
				// wird das Item zum erstenmal ein HotLink ?
				if( !pItem->pImpData && pTopic->StartAdviseLoop() )
				{
					// dann wurde das Item ausgewechselt
					pTopic->aItems.Remove( pItem );
					DdeItem* pTmp;
					for(  pTmp = pTopic->aItems.First(); pTmp;
									pTmp = pTopic->aItems.Next() )
						if( *pTmp->pName == hText2 )
						{
							// es wurde tatsaechlich ausgewechselt
							delete pItem;
							pItem = 0;
							break;
						}
					if( pItem )
						// es wurde doch nicht ausgewechselt, also wieder rein
						pTopic->aItems.Insert( pItem );
					else
						pItem = pTmp;
				}
				pItem->IncMonitor( (long)hConv );
				pInst->hCurConvSvr = NULL;
			}
			return (HDDEDATA)sal_True;

		case XTYP_ADVSTOP:
			pItem->DecMonitor( (long)hConv );
			if( !pItem->pImpData )
				pTopic->StopAdviseLoop();
			pInst->hCurConvSvr = NULL;
			return (HDDEDATA)sal_True;

		case XTYP_EXECUTE:
			{
				DdeData aExec;
				aExec.pImp->hData = hData;
				aExec.pImp->nFmt  = DdeData::GetInternalFormat( nCbType );
				aExec.Lock();
				String aName;

				aName = (const sal_Unicode *)aExec.pImp->pData;

				if( pTopic->IsSystemTopic() )
					bRes = pService->SysTopicExecute( &aName );
				else
					bRes = pTopic->Execute( &aName );
			}
			pInst->hCurConvSvr = NULL;
			if ( bRes )
				return (HDDEDATA)DDE_FACK;
			else
				return (HDDEDATA)DDE_FNOTPROCESSED;
	}

	return (HDDEDATA)NULL;
}

// --- DdeInternat::FindService() ----------------------------------

DdeService* DdeInternal::FindService( HSZ hService )
{
	DdeService*  s;
	DdeServices& rSvc = DdeService::GetServices();
	for ( s = rSvc.First(); s; s = rSvc.Next() )
	{
		if ( *s->pName == hService )
			return s;
	}

	return NULL;
}

// --- DdeInternat::FindTopic() ------------------------------------

DdeTopic* DdeInternal::FindTopic( DdeService& rService, HSZ hTopic )
{
	DdeTopic* s;
	DdeTopics& rTopics = rService.aTopics;
	int bWeiter = sal_False;
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");

	do {            // middle check loop
		for ( s = rTopics.First(); s; s = rTopics.Next() )
		{
			if ( *s->pName == hTopic )
				return s;
		}

		bWeiter = !bWeiter;
		if( !bWeiter )
			break;

		// dann befragen wir doch mal unsere Ableitung:
		TCHAR chBuf[250];
		DdeQueryString(pInst->hDdeInstSvr,hTopic,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
		bWeiter = rService.MakeTopic( reinterpret_cast<const sal_Unicode*>(chBuf) );
		// dann muessen wir noch mal suchen
	} while( bWeiter );

	return 0;
}

// --- DdeInternal::FindItem() -------------------------------------

DdeItem* DdeInternal::FindItem( DdeTopic& rTopic, HSZ hItem )
{
	DdeItem* s;
	DdeItems& rItems = rTopic.aItems;
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	int bWeiter = sal_False;

	do {            // middle check loop

		for ( s = rItems.First(); s; s = rItems.Next() )
			if ( *s->pName == hItem )
				return s;

		bWeiter = !bWeiter;
		if( !bWeiter )
			break;

		// dann befragen wir doch mal unsere Ableitung:
		TCHAR chBuf[250];
		DdeQueryString(pInst->hDdeInstSvr,hItem,chBuf,sizeof(chBuf)/sizeof(TCHAR),CP_WINUNICODE );
		bWeiter = rTopic.MakeItem( reinterpret_cast<const sal_Unicode*>(chBuf) );
		// dann muessen wir noch mal suchen
	} while( bWeiter );

	return 0;
}

// --- DdeService::DdeService() ------------------------------------

DdeService::DdeService( const String& rService )
{
	DdeInstData* pInst = ImpGetInstData();
	if( !pInst )
		pInst = ImpInitInstData();
	pInst->nRefCount++;
	pInst->nInstanceSvr++;

	if ( !pInst->hDdeInstSvr )
	{
		nStatus = sal::static_int_cast< short >(
            DdeInitialize( &pInst->hDdeInstSvr,
                           (PFNCALLBACK)DdeInternal::SvrCallback,
                           APPCLASS_STANDARD |
                           CBF_SKIP_REGISTRATIONS |
                           CBF_SKIP_UNREGISTRATIONS, 0L ) );
		pInst->pServicesSvr = new DdeServices;
	}
	else
		nStatus = DMLERR_NO_ERROR;

	pConv = new ConvList;

	if ( pInst->pServicesSvr )
		pInst->pServicesSvr->Insert( this );

	pName = new DdeString( pInst->hDdeInstSvr, rService );
	if ( nStatus == DMLERR_NO_ERROR )
		if ( !DdeNameService( pInst->hDdeInstSvr, *pName, NULL,
								DNS_REGISTER | DNS_FILTEROFF ) )
			nStatus = DMLERR_SYS_ERROR;

	AddFormat( FORMAT_STRING );
	pSysTopic = new DdeTopic( reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC) );
	pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_TOPICS) ) );
	pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_SYSITEMS) ) );
	pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_STATUS) ) );
	pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_FORMATS) ) );
	pSysTopic->AddItem( DdeItem( reinterpret_cast<const sal_Unicode*>(SZDDESYS_ITEM_HELP) ) );
	AddTopic( *pSysTopic );
}

// --- DdeService::~DdeService() -----------------------------------

DdeService::~DdeService()
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	if ( pInst->pServicesSvr )
		pInst->pServicesSvr->Remove( this );

	// MT: Im Auftrage des Herrn (AM) auskommentiert...
	// Grund:
	// Bei Client/Server werden die Server nicht beendet, wenn mehr
	// als einer gestartet.
	// Weil keine System-Messagequeue ?!

	delete pSysTopic;
	delete pName;

	pInst->nInstanceSvr--;
	pInst->nRefCount--;
	if ( !pInst->nInstanceSvr && pInst->hDdeInstSvr )
	{
		if( DdeUninitialize( pInst->hDdeInstSvr ) )
		{
			pInst->hDdeInstSvr = NULL;
			delete pInst->pServicesSvr;
			pInst->pServicesSvr = NULL;
			if( pInst->nRefCount == 0)
				ImpDeinitInstData();
		}
	}
	delete pConv;
}

// --- DdeService::GetName() ---------------------------------------

const String& DdeService::GetName() const
{
	return *pName;
}

// --- DdeService::GetServices() -----------------------------------

DdeServices& DdeService::GetServices()
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	return *(pInst->pServicesSvr);
}

// --- DdeService::AddTopic() --------------------------------------

void DdeService::AddTopic( const DdeTopic& rTopic )
{
	RemoveTopic( rTopic );
	aTopics.Insert( (DdeTopic*) &rTopic );
}

// --- DdeService::RemoveTopic() -----------------------------------

void DdeService::RemoveTopic( const DdeTopic& rTopic )
{
	DdeTopic* t;
	for ( t = aTopics.First(); t; t = aTopics.Next() )
	{
		if ( !DdeCmpStringHandles (*t->pName, *rTopic.pName ) )
		{
			aTopics.Remove( t );
			// JP 27.07.95: und alle Conversions loeschen !!!
			//              (sonst wird auf geloeschten Topics gearbeitet!!)
			for( sal_uLong n = pConv->Count(); n; )
			{
				Conversation* pC = pConv->GetObject( --n );
				if( pC->pTopic == &rTopic )
				{
					pConv->Remove( pC );
					delete pC;
				}
			}
			break;
		}
	}
}

// --- DdeService::HasCbFormat() -----------------------------------

sal_Bool DdeService::HasCbFormat( sal_uInt16 nFmt )
{
	return sal_Bool( aFormats.GetPos( nFmt ) != LIST_ENTRY_NOTFOUND );
}

// --- DdeService::HasFormat() -------------------------------------

sal_Bool DdeService::HasFormat( sal_uLong nFmt )
{
	return HasCbFormat( (sal_uInt16)DdeData::GetExternalFormat( nFmt ));
}

// --- DdeService::AddFormat() -------------------------------------

void DdeService::AddFormat( sal_uLong nFmt )
{
	nFmt = DdeData::GetExternalFormat( nFmt );
	aFormats.Remove( nFmt );
	aFormats.Insert( nFmt );
}

// --- DdeService::RemoveFormat() ----------------------------------

void DdeService::RemoveFormat( sal_uLong nFmt )
{
	aFormats.Remove( DdeData::GetExternalFormat( nFmt ) );
}

// --- DdeTopic::DdeTopic() ----------------------------------------

DdeTopic::DdeTopic( const String& rName )
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	pName = new DdeString( pInst->hDdeInstSvr, rName );
}

// --- DdeTopic::~DdeTopic() ---------------------------------------

DdeTopic::~DdeTopic()
{
	DdeItem* t;
	while( ( t = aItems.First() ) != NULL )
	{
		aItems.Remove( t );
		t->pMyTopic = 0;
		delete t;
	}
	delete pName;
}

// --- DdeTopic::GetName() -----------------------------------------

const String& DdeTopic::GetName() const
{
	return *pName;
}

// --- DdeTopic::IsSystemTopic() -----------------------------------

sal_Bool DdeTopic::IsSystemTopic()
{
	return sal_Bool (GetName() == reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC));
}

// --- DdeTopic::AddItem() -----------------------------------------

DdeItem* DdeTopic::AddItem( const DdeItem& r )
{
	DdeItem* s;
	if( DDEGETPUTITEM == r.nType )
		s = new DdeGetPutItem( r );
	else
		s = new DdeItem( r );
	if ( s )
	{
		aItems.Insert( s );
		s->pMyTopic = this;
	}
	return s;
}

// --- DdeTopic::InsertItem() -----------------------------------------

void DdeTopic::InsertItem( DdeItem* pNew )
{
	if( pNew )
	{
		aItems.Insert( pNew );
		pNew->pMyTopic = this;
	}
}

// --- DdeTopic::RemoveItem() --------------------------------------

void DdeTopic::RemoveItem( const DdeItem& r )
{
	DdeItem* s;
	for ( s = aItems.First(); s; s = aItems.Next() )
	{
		if ( !DdeCmpStringHandles (*s->pName, *r.pName ) )
			break;
	}

	if ( s )
	{
		aItems.Remove( s );
		s->pMyTopic = 0;
		delete s;
	}
}

// --- DdeTopic::NotifyClient() ------------------------------------

void DdeTopic::NotifyClient( const String& rItem )
{
	DdeItem* pItem;
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	for ( pItem = aItems.First(); pItem; pItem = aItems.Next() )
	{
		if ( pItem->GetName() == rItem )
		{
			if ( pItem->pImpData )
				DdePostAdvise( pInst->hDdeInstSvr, *pName, *pItem->pName );
		}
		break;
	}
}

// --- DdeTopic::Connect() -----------------------------------------

void __EXPORT DdeTopic::Connect( long nId )
{
	aConnectLink.Call( (void*)nId );
}

// --- DdeTopic::Disconnect() --------------------------------------

void __EXPORT DdeTopic::Disconnect( long nId )
{
	aDisconnectLink.Call( (void*)nId );
}

// --- DdeTopic::_Disconnect() --------------------------------------

void __EXPORT DdeTopic::_Disconnect( long nId )
{
	for( DdeItem* pItem = aItems.First(); pItem; pItem = aItems.Next() )
		pItem->DecMonitor( nId );

	Disconnect( nId );
}

// --- DdeTopic::Get() ---------------------------------------------

DdeData* __EXPORT DdeTopic::Get( sal_uLong nFmt )
{
	if ( aGetLink.IsSet() )
		return (DdeData*)aGetLink.Call( (void*)nFmt );
	else
		return NULL;
}

// --- DdeTopic::Put() ---------------------------------------------

sal_Bool __EXPORT DdeTopic::Put( const DdeData* r )
{
	if ( aPutLink.IsSet() )
		return (sal_Bool)aPutLink.Call( (void*) r );
	else
		return sal_False;
}

// --- DdeTopic::Execute() -----------------------------------------

sal_Bool __EXPORT DdeTopic::Execute( const String* r )
{
	if ( aExecLink.IsSet() )
		return (sal_Bool)aExecLink.Call( (void*)r );
	else
		return sal_False;
}

// --- DdeTopic::GetConvId() ---------------------------------------

sal_uIntPtr DdeTopic::GetConvId()
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	return (sal_uIntPtr)pInst->hCurConvSvr;
}

// --- DdeTopic::StartAdviseLoop() ---------------------------------

sal_Bool DdeTopic::StartAdviseLoop()
{
	return sal_False;
}

// --- DdeTopic::StopAdviseLoop() ----------------------------------

sal_Bool DdeTopic::StopAdviseLoop()
{
	return sal_False;
}

// --- DdeItem::DdeItem() ------------------------------------------

DdeItem::DdeItem( const sal_Unicode* p )
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	pName = new DdeString( pInst->hDdeInstSvr, p );
	nType = DDEITEM;
	pMyTopic = 0;
	pImpData = 0;
}

// --- DdeItem::DdeItem() ------------------------------------------

DdeItem::DdeItem( const String& r)
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	pName = new DdeString( pInst->hDdeInstSvr, r );
	nType = DDEITEM;
	pMyTopic = 0;
	pImpData = 0;
}

// --- DdeItem::DdeItem() ------------------------------------------

DdeItem::DdeItem( const DdeItem& r)
{
	DdeInstData* pInst = ImpGetInstData();
	DBG_ASSERT(pInst,"SVDDE:No instance data");
	pName = new DdeString( pInst->hDdeInstSvr, *r.pName );
	nType = DDEITEM;
	pMyTopic = 0;
	pImpData = 0;
}

// --- DdeItem::~DdeItem() -----------------------------------------

DdeItem::~DdeItem()
{
	if( pMyTopic )
		pMyTopic->aItems.Remove( this );
	delete pName;
	delete pImpData;
}

// --- DdeItem::GetName() ------------------------------------------

const String& DdeItem::GetName() const
{
	return *pName;
}

// --- DdeItem::NotifyClient() ------------------------------------------

void DdeItem::NotifyClient()
{
	if( pMyTopic && pImpData )
	{
		DdeInstData* pInst = ImpGetInstData();
		DBG_ASSERT(pInst,"SVDDE:No instance data");
		DdePostAdvise( pInst->hDdeInstSvr, *pMyTopic->pName, *pName );
	}
}

// --- DdeItem::IncMonitor() ------------------------------------------

void DdeItem::IncMonitor( sal_uLong nHCnv )
{
	if( !pImpData )
	{
		pImpData = new DdeItemImp;
		if( DDEGETPUTITEM == nType )
			((DdeGetPutItem*)this)->AdviseLoop( sal_True );
	}
	else
	{
		for( sal_uInt16 n = pImpData->Count(); n; )
			if( (*pImpData)[ --n ].nHCnv == nHCnv )
			{
				++(*pImpData)[ n ].nHCnv;
				return ;
			}
	}

	pImpData->Insert( DdeItemImpData( nHCnv ), pImpData->Count() );
}

// --- DdeItem::DecMonitor() ------------------------------------------

void DdeItem::DecMonitor( sal_uLong nHCnv )
{
	if( pImpData )
	{
		DdeItemImpData* pData = (DdeItemImpData*)pImpData->GetData();
		for( sal_uInt16 n = pImpData->Count(); n; --n, ++pData )
			if( pData->nHCnv == nHCnv )
			{
				if( !pData->nCnt || !--pData->nCnt )
				{
					if( 1 < pImpData->Count() )
						pImpData->Remove( pImpData->Count() - n );
					else
					{
						delete pImpData, pImpData = 0;
						if( DDEGETPUTITEM == nType )
							((DdeGetPutItem*)this)->AdviseLoop( sal_False );
					}
				}
				return ;
			}
	}
}

// --- DdeItem::GetLinks() ------------------------------------------

short DdeItem::GetLinks()
{
	short nCnt = 0;
	if( pImpData )
		for( sal_uInt16 n = pImpData->Count(); n; )
			nCnt = nCnt + (*pImpData)[ --n ].nCnt;
	return nCnt;
}

// --- DdeGetPutItem::DdeGetPutItem() ------------------------------

DdeGetPutItem::DdeGetPutItem( const sal_Unicode* p )
	: DdeItem( p )
{
	nType = DDEGETPUTITEM;
}

// --- DdeGetPutItem::DdeGetPutItem() ------------------------------

DdeGetPutItem::DdeGetPutItem( const String& rStr )
	: DdeItem( rStr )
{
	nType = DDEGETPUTITEM;
}

// --- DdeGetPutItem::DdeGetPutItem() ------------------------------

DdeGetPutItem::DdeGetPutItem( const DdeItem& rItem )
	: DdeItem( rItem )
{
	nType = DDEGETPUTITEM;
}


// --- DdeGetPutData::Get() ----------------------------------------

DdeData* DdeGetPutItem::Get( sal_uLong )
{
	return 0;
}

// --- DdeGetPutData::Put() ----------------------------------------

sal_Bool DdeGetPutItem::Put( const DdeData* )
{
	return sal_False;
}

// --- DdeGetPutData::AdviseLoop() ---------------------------------

void DdeGetPutItem::AdviseLoop( sal_Bool )
{
}


// --- DdeService::SysItems() --------------------------------------

String DdeService::SysItems()
{
	String s;
	DdeTopic* t;
	for ( t = aTopics.First(); t; t = aTopics.Next() )
	{
		if ( t->GetName() == reinterpret_cast<const sal_Unicode*>(SZDDESYS_TOPIC) )
		{
			short n = 0;
			DdeItem* pi;
			for ( pi = t->aItems.First(); pi; pi = t->aItems.Next(), n++ )
			{
				if ( n )
					s += '\t';
				s += pi->GetName();
			}
			s += String::CreateFromAscii("\r\n");
		}
	}

	return s;
}

// --- DdeService::Topics() ----------------------------------------

String DdeService::Topics()
{
	String      s;
	DdeTopic*   t;
	short       n = 0;

	for ( t = aTopics.First(); t; t = aTopics.Next(), n++ )
	{
		if ( n )
			s += '\t';
		s += t->GetName();
	}
	s += String::CreateFromAscii("\r\n");

	return s;
}

// --- DdeService::Formats() ---------------------------------------

String DdeService::Formats()
{
	String      s;
	long        f;
	TCHAR       buf[128];
	LPCTSTR		p;
	short       n = 0;

	for ( f = aFormats.First(); f; f = aFormats.Next(), n++ )
	{
		if ( n )
			s += '\t';
		p = buf;

		switch( (sal_uInt16)f )
		{
			case CF_TEXT:
				p = reinterpret_cast<LPCTSTR>(String::CreateFromAscii("TEXT").GetBuffer());
				break;
			case CF_BITMAP:
				p = reinterpret_cast<LPCTSTR>(String::CreateFromAscii("BITMAP").GetBuffer());
				break;
#ifdef OS2
			case CF_DSPTEXT:
				p = String::CreateFromAscii("TEXT").GetBuffer();
				break;
			case CF_DSPBITMAP:
				p = String::CreateFromAscii("BITMAP").GetBuffer();
				break;
			case CF_METAFILE:
				p = String::CreateFromAscii("METAFILE").GetBuffer();
				break;
			case CF_DSPMETAFILE:
				p = String::CreateFromAscii("METAFILE").GetBuffer();
				break;
			case CF_PALETTE:
				p = String::CreateFromAscii("PALETTE").GetBuffer();
				break;
			default:
				p= String::CreateFromAscii("PRIVATE").GetBuffer();
#else
			default:
				GetClipboardFormatName( (UINT)f, buf, sizeof(buf) / sizeof(TCHAR) );
#endif
		}
		s += String( reinterpret_cast<const sal_Unicode*>(p) );
	}
	s += String::CreateFromAscii("\r\n");

	return s;
}

// --- DdeService::Status() ----------------------------------------

String DdeService::Status()
{
	return IsBusy() ? String::CreateFromAscii("Busy\r\n") : String::CreateFromAscii("Ready\r\n");
}

// --- DdeService::IsBusy() ----------------------------------------

sal_Bool __EXPORT DdeService::IsBusy()
{
	return sal_False;
}

// --- DdeService::GetHelp() ----------------------------------------

String __EXPORT DdeService::GetHelp()
{
	return String();
}

sal_Bool DdeTopic::MakeItem( const String& )
{
	return sal_False;
}

sal_Bool DdeService::MakeTopic( const String& )
{
	return sal_False;
}

String DdeService::SysTopicGet( const String& )
{
	return String();
}

sal_Bool DdeService::SysTopicExecute( const String* )
{
	return sal_False;
}

