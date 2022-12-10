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
#include "precompiled_automation.hxx"


#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <tools/solar.h>
#include <automation/simplecm.hxx>

#include <automation/commdefines.hxx>
#include "packethandler.hxx"
#include "tcpio.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
void debug_printf( const char *chars )
{
    static sal_Bool bPrint = (getenv("DEBUG") != NULL);
    if ( bPrint )
    {
        printf( chars );
        fflush( stdout );
    }
}
#endif

CommunicationLink::CommunicationLink( CommunicationManager *pMan )
: pMyManager(pMan)
, pServiceData(NULL)
, nServiceProtocol( 0 )
, bIsInsideCallback( sal_False )
, nTotalBytes( 0 )
, maApplication("Undefined")
#if OSL_DEBUG_LEVEL > 1
, bFlag( sal_False )
, nSomething( 0 )
#endif
{
}

CommunicationLink::~CommunicationLink()
{
#if OSL_DEBUG_LEVEL > 1
    if ( !bFlag )    // bFlag will be set if deletion is expected else we can set a breakpoint
        bFlag = sal_False;
#endif
	if ( pMyManager )
		pMyManager->DestroyingLink( this );
}

void CommunicationLink::CallInfoMsg( InfoString aMsg )
{
	if ( pMyManager )
		pMyManager->InfoMsg( aMsg );
};

CM_InfoType CommunicationLink::GetInfoType()
{
	if ( pMyManager )
		return pMyManager->GetInfoType();
	else
		return CM_NO_TEXT;
}

IMPL_LINK( CommunicationLink, ConnectionClosed, void*, EMPTYARG )
{
	if ( pMyManager )
		pMyManager->CallConnectionClosed( this );
	return 1;
}

IMPL_LINK( CommunicationLink, DataReceived, void*, EMPTYARG )
{
	if ( pMyManager )
		pMyManager->CallDataReceived( this );
	return 1;
}

sal_Bool CommunicationLink::DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol )
{
	INFO_MSG( CByteString("S :").Append( GetCommunicationPartner( CM_FQDN ) ),
		CByteString("Daten Senden:").Append( GetCommunicationPartner( CM_FQDN ) ),
		CM_SEND, this );
	sal_Bool bWasError = sal_False;

	sal_uInt32 nBuffer;
	nBuffer = pDataStream->SeekRel(0) +1;
	bWasError = pPacketHandler->TransferData( ((SvMemoryStream*)pDataStream)->GetData(), nBuffer, nProtocol ) != C_ERROR_NONE;

	if ( bWasError )
	{
		INFO_MSG( CByteString("Send Failed:").Append( GetCommunicationPartner( CM_FQDN ) ),
			CByteString( "Socket wird wegen Fehlers beim Senden geschlossen: ").Append( GetCommunicationPartner( CM_FQDN ) ),
			CM_ERROR, this );
		ShutdownCommunication();
	}
	return !bWasError;
}

sal_Bool CommunicationLink::TransferDataStream( SvStream *pDataStream, CMProtocol nProtocol )
{
	aLastAccess = DateTime();
	nTotalBytes += pDataStream->Seek( STREAM_SEEK_TO_END );
	return DoTransferDataStream( pDataStream, nProtocol );
}

void CommunicationLink::SetApplication( const ByteString& aApp )
{
	maApplication = aApp;
}


SimpleCommunicationLinkViaSocket::SimpleCommunicationLinkViaSocket( CommunicationManager *pMan, vos::OStreamSocket *pSocket )
: CommunicationLink( pMan )
, aCommunicationPartner()
, aMyName()
, pStreamSocket( pSocket )
, pReceiveStream( NULL )
, bIsRequestShutdownPending( sal_False )
{
	pTCPIO = new TCPIO( pStreamSocket );
	pPacketHandler = new PacketHandler( (ITransmiter*) pTCPIO, pTCPIO, pMyManager->IsMultiChannel() );
}

SimpleCommunicationLinkViaSocket::~SimpleCommunicationLinkViaSocket()
{
	delete pPacketHandler;
    pPacketHandler = NULL;
	delete pTCPIO;
    pTCPIO = NULL;
	delete pStreamSocket;
    pStreamSocket = NULL;
}

void SimpleCommunicationLinkViaSocket::SetStreamSocket( vos::OStreamSocket* pSocket )
{
    if ( pTCPIO )
        pTCPIO->SetStreamSocket( pSocket );
    pStreamSocket = pSocket;
}

sal_Bool SimpleCommunicationLinkViaSocket::StopCommunication()
{
	CommunicationLinkRef rHold(this);		// avoid deleting this link before the end of the method
	if ( !IsCommunicationError() )	// Meaning that the Communication is still running
	{
#if OSL_DEBUG_LEVEL > 1
		debug_printf("Sending REQUEST_ShutdownLink\n");
#endif
		SendHandshake( CH_REQUEST_ShutdownLink );
	}
	WaitForShutdown();
	return sal_True;
}

void SimpleCommunicationLinkViaSocket::SetFinalRecieveTimeout()
{
	if ( !IsCommunicationError() )
	{
        TimeValue aTime = {30, 0};   // 30 seconds
	    pStreamSocket->setRecvTimeout( &aTime );
	}
}

sal_Bool SimpleCommunicationLinkViaSocket::IsCommunicationError()
{
	return !pStreamSocket;
}

ByteString SimpleCommunicationLinkViaSocket::GetCommunicationPartner( CM_NameType eType )
{
	if ( pStreamSocket )
	{
		switch ( eType )
		{
			case CM_DOTTED:
				{
					rtl::OUString aDotted;
					vos::OSocketAddr *pPeerAdr = new vos::OSocketAddr;
					pStreamSocket->getPeerAddr( *pPeerAdr );
					((vos::OInetSocketAddr*)pPeerAdr)->getDottedAddr( aDotted );
					delete pPeerAdr;
					return ByteString( UniString(aDotted), RTL_TEXTENCODING_UTF8 );
				}
				//break;
			case CM_FQDN:
				{
					if ( !aCommunicationPartner.Len() )
					{
						rtl::OUString aFQDN;
						pStreamSocket->getPeerHost( aFQDN );
						aCommunicationPartner = ByteString( UniString(aFQDN), RTL_TEXTENCODING_UTF8 );
					}
					return aCommunicationPartner;
				}
				//break;
		}
	}
	return CByteString( "Unknown" );
}

ByteString SimpleCommunicationLinkViaSocket::GetMyName( CM_NameType eType )
{
	if ( pStreamSocket )
	{
		switch ( eType )
		{
			case CM_DOTTED:
				{
					rtl::OUString aDotted;
					vos::OSocketAddr *pPeerAdr = new vos::OSocketAddr;
					pStreamSocket->getLocalAddr( *pPeerAdr );
					((vos::OInetSocketAddr*)pPeerAdr)->getDottedAddr( aDotted );
					delete pPeerAdr;
					return ByteString( UniString(aDotted), RTL_TEXTENCODING_UTF8 );
				}
				//break;
			case CM_FQDN:
				{
					if ( !aMyName.Len() )
					{
						rtl::OUString aFQDN;
						pStreamSocket->getLocalHost( aFQDN );
						aMyName = ByteString( UniString(aFQDN), RTL_TEXTENCODING_UTF8 );
					}
					return aMyName;
				}
				//break;
		}
	}
	return CByteString( "Error" );
}

SvStream* SimpleCommunicationLinkViaSocket::GetBestCommunicationStream()
{
	SvStream* pStream = new SvMemoryStream;
//	pStream->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
	return pStream;
}

#define READ_SOCKET( pBuffer, nLength )\
	if ( !bWasError )\
		{bWasError |= pTCPIO->ReceiveBytes( pBuffer, nLength ) != C_ERROR_NONE;}

#define READ_SOCKET_LEN( pBuffer, nLength, nTotal )\
	READ_SOCKET( pBuffer, nLength );\
	if ( !bWasError )\
		{nTotal += nLength;}

sal_Bool SimpleCommunicationLinkViaSocket::DoReceiveDataStream()
{
	sal_Bool bWasError = sal_False;
	void* pBuffer = NULL;
	comm_UINT32 nLen;
	bWasError = pPacketHandler->ReceiveData( pBuffer, nLen ) != C_ERROR_NONE;
	if ( !bWasError )
	{
		pReceiveStream = GetBestCommunicationStream();
		DBG_ASSERT( pReceiveStream->IsA() == ID_MEMORYSTREAM, "CommunicationStream is not an SvMemoryStream. Communication has to be reimplemented here!");
		if ( pReceiveStream->IsA() == ID_MEMORYSTREAM )
			((SvMemoryStream*)pReceiveStream)->SetBuffer( pBuffer, nLen, sal_True, nLen );
		DBG_ASSERT( pReceiveStream, "Datastream is NULL");
	}

	return !bWasError;
}

void SimpleCommunicationLinkViaSocket::SetApplication( const ByteString& aApp )
{
	CommunicationLink::SetApplication( aApp );
	SvStream* pData = GetBestCommunicationStream();
	*pData << aApp;
	SendHandshake( CH_SetApplication, pData );
	delete pData;
}

void SimpleCommunicationLinkViaSocket::SetNewPacketAsCurrent()
{
	pServiceData = pReceiveStream;
	nServiceProtocol = pPacketHandler->GetReceiveProtocol();
	nServiceHeaderType = pPacketHandler->GetReceiveHeaderType();
}

sal_Bool SimpleCommunicationLinkViaSocket::SendHandshake( HandshakeType aHandshakeType, SvStream* pData )
{
	sal_Bool bWasError;

	if ( pData )
	{
		sal_uInt32 nBuffer;
		nBuffer = pData->Seek( STREAM_SEEK_TO_END );
		bWasError = !pPacketHandler->SendHandshake( aHandshakeType, ((SvMemoryStream*)pData)->GetData(), nBuffer );
	}
	else
		bWasError = !pPacketHandler->SendHandshake( aHandshakeType );


	if ( bWasError )
	{
		INFO_MSG( CByteString("Send Failed:").Append( GetCommunicationPartner( CM_FQDN ) ),
			CByteString( "Socket wird wegen Fehlers beim Senden geschlossen: ").Append( GetCommunicationPartner( CM_FQDN ) ),
			CM_ERROR, this );
		ShutdownCommunication();
	}
	else
	{	// set new status
		switch ( aHandshakeType )
		{
			case CH_REQUEST_HandshakeAlive:
				break;
			case CH_RESPONSE_HandshakeAlive:
				break;
			case CH_REQUEST_ShutdownLink:
				bIsRequestShutdownPending = sal_True;
				break;
			case CH_ShutdownLink:
				break;
			case CH_SUPPORT_OPTIONS:
				break;
			case CH_SetApplication:
				break;
			default:
				DBG_ERROR("Unknown HandshakeType");
		}
	}
	return !bWasError;
}

SimpleCommunicationLinkViaSocketWithReceiveCallbacks::SimpleCommunicationLinkViaSocketWithReceiveCallbacks( CommunicationManager *pMan, vos::OStreamSocket *pSocket )
: SimpleCommunicationLinkViaSocket( pMan, pSocket )
{
}

SimpleCommunicationLinkViaSocketWithReceiveCallbacks::~SimpleCommunicationLinkViaSocketWithReceiveCallbacks()
{
	if ( pMyManager && pMyManager->IsLinkValid( this ) && !bIsRequestShutdownPending )
		StopCommunication();
}

void SimpleCommunicationLinkViaSocketWithReceiveCallbacks::WaitForShutdown()
{
	CommunicationLinkRef rHold(this);		// avoid deleting this link before the end of the method
	SetFinalRecieveTimeout();
    while ( pMyManager && !IsCommunicationError() )
		ReceiveDataStream();
}

sal_Bool SimpleCommunicationLinkViaSocketWithReceiveCallbacks::ReceiveDataStream()
{
	if ( DoReceiveDataStream() )
	{
		SetNewPacketAsCurrent();
		StartCallback();
		DataReceived();
		return sal_True;
	}
	else
	{
		StartCallback();
		ShutdownCommunication();
		return sal_False;
	}
}

sal_Bool SimpleCommunicationLinkViaSocketWithReceiveCallbacks::ShutdownCommunication()
{
    if ( GetStreamSocket() )
		GetStreamSocket()->shutdown();

	if ( GetStreamSocket() )
		GetStreamSocket()->close();

    vos::OStreamSocket *pTempSocket = GetStreamSocket();
    SetStreamSocket( NULL );
    delete pTempSocket;

	ConnectionClosed();

	return sal_True;
}



CommunicationManager::CommunicationManager( sal_Bool bUseMultiChannel )
: nInfoType( CM_NONE )
, bIsCommunicationRunning( sal_False )
, maApplication("Unknown")
, bIsMultiChannel( bUseMultiChannel )
{
}

CommunicationManager::~CommunicationManager()
{
	xLastNewLink.Clear();
}

sal_Bool CommunicationManager::StartCommunication( String aApp, String aParams )
{
    (void) aApp; /* avoid warning about unused parameter */ 
    (void) aParams; /* avoid warning about unused parameter */ 
    return sal_False;
}

sal_Bool CommunicationManager::StartCommunication( ByteString aHost, sal_uLong nPort )
{
    (void) aHost; /* avoid warning about unused parameter */ 
    (void) nPort; /* avoid warning about unused parameter */ 
    return sal_False;
}

ByteString CommunicationManager::GetMyName( CM_NameType )
{
	rtl::OUString aHostname;
	vos::OSocketAddr::getLocalHostname( aHostname );
	return ByteString( UniString(aHostname), RTL_TEXTENCODING_UTF8 );
}

void CommunicationManager::CallConnectionOpened( CommunicationLink* pCL )
{
	pCL->StartCallback();		// Sollte bereits vor dem Aufruf gerufen werden
	pCL->aStart = DateTime();
	pCL->aLastAccess = pCL->aStart;
	bIsCommunicationRunning = sal_True;
	pCL->SetApplication( GetApplication() );

	xLastNewLink = pCL;

	INFO_MSG( CByteString("C+:").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
		CByteString("Verbindung aufgebaut: ").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
		CM_OPEN, pCL );
	ConnectionOpened( pCL );
	pCL->FinishCallback();
}

void CommunicationManager::CallConnectionClosed( CommunicationLink* pCL )
{
	pCL->StartCallback();		// Sollte bereits vor dem Aufruf gerufen werden
	pCL->aLastAccess = DateTime();

	INFO_MSG( CByteString("C-:").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
		CByteString("Verbindung abgebrochen: ").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
		CM_CLOSE, pCL );
	ConnectionClosed( pCL );

	if ( xLastNewLink == pCL )
		xLastNewLink.Clear();

	pCL->FinishCallback();
//	delete pCL;
}

void CommunicationManager::CallDataReceived( CommunicationLink* pCL )
{
	pCL->StartCallback();		// Sollte bereits vor dem Aufruf gerufen werden
	pCL->aLastAccess = DateTime();
	CommunicationLinkRef rHold(pCL);	// H�lt den Zeiger bis zum Ende des calls

	// should be impossible but happens for mysterious reasons
	if ( !pCL->pServiceData )
	{
		DBG_ERROR( "Datastream is NULL" );
		pCL->FinishCallback();
		return;
	}


	if ( CH_Handshake == pCL->nServiceHeaderType )
	{
		SvStream *pData = pCL->GetServiceData();
		sal_uInt16 nType;
		pData->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );	// Unfortulately it is written this way :((
		*pData >> nType;
		pData->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
		switch ( nType )
		{
			case CH_REQUEST_HandshakeAlive:
				{
					pCL->SendHandshake( CH_RESPONSE_HandshakeAlive );
				}
				break;
			case CH_REQUEST_ShutdownLink:
				{
#if OSL_DEBUG_LEVEL > 1
					debug_printf("Sending ShutdownLink\n");
#endif
					pCL->SendHandshake( CH_ShutdownLink );
				}
				break;
			case CH_ShutdownLink:
				{
#if OSL_DEBUG_LEVEL > 1
					debug_printf("Executing ShutdownLink\n");
#endif
					pCL->ShutdownCommunication();
				}
				break;
			case CH_SetApplication:
				{
					ByteString aApplication;
					*pData >> aApplication;
					pCL->CommunicationLink::SetApplication( aApplication );
#if OSL_DEBUG_LEVEL > 1
					debug_printf( "Setting Application to " );
					debug_printf( aApplication.GetBuffer() );
					debug_printf( "\n" );
#endif
				}
				break;

#if OSL_DEBUG_LEVEL > 1
			default:
				{
					debug_printf("Unknown Handshake received\n");
				}
#endif
		}
		delete pData;
	}
	else
	{
		if ( pCL->pServiceData )
		{
			pCL->nTotalBytes += pCL->pServiceData->Seek( STREAM_SEEK_TO_END );
			pCL->pServiceData->Seek( STREAM_SEEK_TO_BEGIN );
		}

		INFO_MSG( CByteString("D :").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
			CByteString("Daten Empfangen:").Append( pCL->GetCommunicationPartner( CM_FQDN ) ),
			CM_RECEIVE, pCL );
		DataReceived( pCL );
	}
	delete pCL->GetServiceData();
	pCL->FinishCallback();
}

void CommunicationManager::CallInfoMsg( InfoString aMsg )
{
	// Hier wird es wohl kein Housekeeping geben
	InfoMsg( aMsg );
}

void CommunicationManager::SetApplication( const ByteString& aApp, sal_Bool bRunningLinks )
{
	maApplication = aApp;
	if ( bRunningLinks )
	{
		sal_uInt16 i;
		for ( i = 0 ; i < GetCommunicationLinkCount() ; i++ )
			GetCommunicationLink( i )->SetApplication( aApp );
	}
}



SingleCommunicationManager::SingleCommunicationManager( sal_Bool bUseMultiChannel )
: CommunicationManager( bUseMultiChannel )
{
	xActiveLink = NULL;
	pInactiveLink = NULL;
}

SingleCommunicationManager::~SingleCommunicationManager()
{
	StopCommunication();
	if ( pInactiveLink )
		pInactiveLink->InvalidateManager();
}

sal_Bool SingleCommunicationManager::StopCommunication()
{
	if ( xActiveLink.Is() )
	{
		sal_Bool bSuccess = xActiveLink->StopCommunication();
		if ( pInactiveLink )
			pInactiveLink->InvalidateManager();
		pInactiveLink = xActiveLink;
		xActiveLink.Clear();
		return bSuccess;
	}
	return sal_True;
}

sal_Bool SingleCommunicationManager::IsLinkValid( CommunicationLink* pCL )
{
	return &xActiveLink == pCL;
}

sal_uInt16 SingleCommunicationManager::GetCommunicationLinkCount()
{
	return IsCommunicationRunning()?1:0;
}

CommunicationLinkRef SingleCommunicationManager::GetCommunicationLink( sal_uInt16 )
{
	return xActiveLink;
}

void SingleCommunicationManager::CallConnectionOpened( CommunicationLink* pCL )
{
	DBG_ASSERT( !xActiveLink.Is(), "Es ist bereits ein CommunicationLink aktiv");
	if ( xActiveLink.Is() )
	{
		if ( pInactiveLink )
			pInactiveLink->InvalidateManager();
		pInactiveLink = xActiveLink;
		xActiveLink->StopCommunication();	// Den alten Link brutal abw�rgen
	}
	xActiveLink = pCL;
	CommunicationManager::CallConnectionOpened( pCL );
}

void SingleCommunicationManager::CallConnectionClosed( CommunicationLink* pCL )
{
	CommunicationManager::CallConnectionClosed( pCL );

	DBG_ASSERT( pCL == xActiveLink, "SingleCommunicationManager::CallConnectionClosed mit fremdem Link");
	if ( pInactiveLink )
		pInactiveLink->InvalidateManager();
	pInactiveLink = xActiveLink;
	xActiveLink.Clear();
	bIsCommunicationRunning = sal_False;
}

void SingleCommunicationManager::DestroyingLink( CommunicationLink *pCL )
{
	pInactiveLink = NULL;
	pCL->InvalidateManager();
}


SingleCommunicationManagerClientViaSocket::SingleCommunicationManagerClientViaSocket( ByteString aHost, sal_uLong nPort, sal_Bool bUseMultiChannel )
: SingleCommunicationManager( bUseMultiChannel )
, aHostToTalk( aHost )
, nPortToTalk( nPort )
{
}


SingleCommunicationManagerClientViaSocket::SingleCommunicationManagerClientViaSocket( sal_Bool bUseMultiChannel )
: SingleCommunicationManager( bUseMultiChannel )
, aHostToTalk()
, nPortToTalk( 0 )
{
}


sal_Bool CommonSocketFunctions::DoStartCommunication( CommunicationManager *pCM, ICommunicationManagerClient *pCMC, ByteString aHost, sal_uLong nPort )
{
	vos::OInetSocketAddr Addr;
	vos::OConnectorSocket *pConnSocket;

	Addr.setAddr( rtl::OUString( UniString( aHost, RTL_TEXTENCODING_UTF8 ) ) );
	Addr.setPort( nPort );

	TimeValue aTV;
	aTV.Seconds = 10;		// Warte 10 Sekunden
	aTV.Nanosec = 0;
	do
	{
		pConnSocket = new vos::OConnectorSocket();
		pConnSocket->setTcpNoDelay( 1 );
		if ( pConnSocket->connect( Addr, &aTV ) == vos::ISocketTypes::TResult_Ok )
		{
			pConnSocket->setTcpNoDelay( 1 );

			pCM->CallConnectionOpened( CreateCommunicationLink( pCM, pConnSocket ) );
			return sal_True;
		}
		else
			delete pConnSocket;

	} while ( pCMC->RetryConnect() );

	return sal_False;
}

