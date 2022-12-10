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


#ifndef _SIMPLECM_HXX
#define _SIMPLECM_HXX

#include <tools/link.hxx>
#include <tools/string.hxx>
#include <tools/stream.hxx>
#include <vos/socket.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <automation/commdefines.hxx>
#include <automation/automationdllapi.h>

// CM steht f�r CommunicationManager
#define CM_UNLIMITED_CONNECTIONS	0xffff

typedef sal_uInt16 CM_NameType;
#define CM_DOTTED	( (CM_NameType) 01 )
#define CM_FQDN		( (CM_NameType) 02 )

typedef sal_uInt16 CM_InfoType;
// nur eines dieser 3 defines darf verwendet werden
#define CM_NO_TEXT		( (CM_InfoType) 01 )
#define CM_SHORT_TEXT	( (CM_InfoType) 02 )
#define CM_VERBOSE_TEXT	( (CM_InfoType) 03 )

#define CM_OPEN			( (CM_InfoType) 0x0004 )
#define CM_CLOSE		( (CM_InfoType) 0x0008 )
#define CM_RECEIVE		( (CM_InfoType) 0x0010 )
#define CM_SEND			( (CM_InfoType) 0x0020 )
#define CM_ERROR		( (CM_InfoType) 0x0040 )
#define CM_MISC			( (CM_InfoType) 0x0080 )

#define CM_USER_1		( (CM_InfoType) 0x0100 )
#define CM_USER_2		( (CM_InfoType) 0x0200 )
#define CM_USER_3		( (CM_InfoType) 0x0400 )
#define CM_USER_4		( (CM_InfoType) 0x0800 )

#define CM_ALL			( CM_OPEN | CM_CLOSE | CM_RECEIVE | CM_SEND | CM_ERROR | CM_MISC )
#define CM_NONE			( 0 )

#define CByteString( constAsciiStr ) ByteString( RTL_CONSTASCII_STRINGPARAM ( constAsciiStr ) )

#define INFO_MSG( Short, Long, Type, CLink ) \
{ \
	if ( (Type & GetInfoType()) > 0 ) \
	{ \
		switch ( GetInfoType() & 03 ) \
		{ \
		    case CM_NO_TEXT: \
                { \
       		        ByteString aByteString; \
        	        CallInfoMsg( InfoString( aByteString, Type, CLink ) ); \
                } \
	            break; \
		    case CM_SHORT_TEXT: \
                { \
       		        ByteString aByteString( Short ); \
	                CallInfoMsg( InfoString( aByteString, Type, CLink ) ); \
                } \
	            break; \
		    case CM_VERBOSE_TEXT: \
                { \
       		        ByteString aByteString( Long ); \
	                CallInfoMsg( InfoString( aByteString, Type, CLink ) ); \
                } \
	            break; \
    		default: \
    	        break; \
		} \
	} \
}\

class CommunicationLink;

/*#undef  PRV_SV_DECL_REF_LOCK
#define PRV_SV_DECL_REF_LOCK(ClassName, Ref)	\
protected:										\
    ClassName * pObj;							\
public:											\
PRV_SV_DECL_REF_SIGNATURE(ClassName, Ref)		\
    inline               ClassName##Ref( void * pObjP ){ClassName##Ref ((ClassName *) pObjP);}			\
*/

SV_DECL_REF( CommunicationLink )

class AUTOMATION_DLLPUBLIC InfoString : public ByteString
{
public:
	InfoString( ByteString &nMsg, CM_InfoType nIT, CommunicationLink *pCL = NULL ): ByteString( nMsg ), nInfoType( nIT ), pCommLink( pCL ) {;}
	CM_InfoType GetInfoType(){ return nInfoType; }
	CommunicationLinkRef GetCommunicationLink(){ return pCommLink; }
private:
	CM_InfoType nInfoType;
	CommunicationLinkRef pCommLink;
};

class PacketHandler;
class CommunicationManager;
class SingleCommunicationManager;
class MultiCommunicationManager;
class CommunicationManagerServerAcceptThread;
class AUTOMATION_DLLPUBLIC CommunicationLink : public SvRefBase
{
protected:
	friend class CommunicationManager;
	friend class SingleCommunicationManager;
	friend class MultiCommunicationManager;
	friend class CommunicationManagerServerAcceptThread;
	// Darf nicht abger�umt werden zwischen Empfang des Streams und ende des Callbacks

protected:	// so da� nur �ber Ref gel�scht werden kann
	virtual ~CommunicationLink();
	void InvalidateManager() { pMyManager = NULL; }

	PacketHandler* pPacketHandler;

public:
	CommunicationLink( CommunicationManager *pMan );

	virtual sal_Bool StopCommunication()=0;
	virtual sal_Bool IsCommunicationError()=0;
	CommunicationManager* GetCommunicationManager(){ return pMyManager; }

//	Der Name oder die IP-Adresse oder sonstwas um den Communikationspartner zu identifizieren
	virtual ByteString GetCommunicationPartner( CM_NameType eType )=0;

//	Der Name oder die IP-Adresse oder sonstwas um den Communikationspartner zu identifizieren
	virtual ByteString GetMyName( CM_NameType eType )=0;

//	Liefert einen neuen Stream zum Versenden von Daten.
	virtual SvStream* GetBestCommunicationStream()=0;

	/** will call virtual function DoTransferDataStream to do actual work
		Purpos is to allow housekeeping
	**/
	sal_Bool TransferDataStream( SvStream *pDataStream, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );

	// Liefert die ID, die vom Sender angegeben wurde.
	// Dadurch lassen sich virtuelle Kommunikationen �ber einen physikalischen Link realisiren.
	// Da die Kommunikation zu �lteren Versionen kompatibel bleiben mu�, mu� der Empf�nger raten,
	// die neue oder die alte verwendet wird, da sich der Kopf eines Auftrages dann �ndert.
	sal_uInt16 GetProtocol(){ return nServiceProtocol; }

	// Der Stream wird hier �bergeben. Der Aufrufer ist f�r dessen L�schung zust�ndig
	// Die Methode MUSS gerufen werden, da sonst keine weiteren Daten empfangen werden.
	SvStream* GetServiceData(){ SvStream *pTemp = pServiceData; pServiceData = NULL; return pTemp; }

	/// Erm�glicht das Ausl�sen des n�chsten Callbacks. Wird auch Implizit gerufen.
	void FinishCallback(){ bIsInsideCallback = sal_False; }

	/// Syncrones Empfangen der Daten. Nur f�r Kommandozeile, sonst leer implementiert
	virtual sal_Bool ReceiveDataStream(){ return sal_False; }

	/// Statistics
	DateTime GetStart() { return aStart; }
	sal_uLong GetTotalBytes() { return nTotalBytes; }
	DateTime GetLastAccess() { return aLastAccess; }
	const ByteString& GetApplication() { return maApplication; }
	virtual void SetApplication( const ByteString& aApp );

protected:
	void CallInfoMsg( InfoString aMsg );
	CM_InfoType GetInfoType();
	CommunicationManager *pMyManager;
// Diese Methoden werden im Main Kontext gerufen und an den Manager weitergereicht.
	virtual DECL_LINK( ConnectionClosed, void* = NULL );
    virtual DECL_LINK( DataReceived, void* = NULL );

	virtual sal_Bool DoTransferDataStream( SvStream *pDataStream, CMProtocol nProtocol = CM_PROTOCOL_OLDSTYLE );

	SvStream *pServiceData;
	sal_uInt16 nServiceProtocol;
	sal_uInt16 nServiceHeaderType;

	/// Verhindert das vorzeitige Ausl�sen des n�chsten Callbacks.
	void StartCallback(){ bIsInsideCallback = sal_True; }
	sal_Bool bIsInsideCallback;

	virtual sal_Bool SendHandshake( HandshakeType aHandshakeType, SvStream* pData = NULL)=0;

	virtual sal_Bool ShutdownCommunication() = 0;	/// Really stop the Communication

	/// Statistics
	DateTime aStart;
	sal_uLong nTotalBytes;
	DateTime aLastAccess;

private:
    ByteString maApplication;

#if OSL_DEBUG_LEVEL > 1
public:
    // misc (debuging) purposes
    sal_Bool bFlag;
    sal_uLong nSomething;
#endif

};

SV_IMPL_REF( CommunicationLink );

class CommonSocketFunctions;
class AUTOMATION_DLLPUBLIC CommunicationManager
{
    friend class CommunicationLink;
	friend class CommonSocketFunctions;
public:
	CommunicationManager( sal_Bool bUseMultiChannel = sal_False );
	virtual ~CommunicationManager();

	virtual sal_Bool StartCommunication()=0;
	virtual sal_Bool StartCommunication( String aApp, String aParams );
	virtual sal_Bool StartCommunication( ByteString aHost, sal_uLong nPort );
	virtual sal_Bool StopCommunication()=0;		// H�lt alle CommunicationLinks an
	virtual sal_Bool IsCommunicationRunning() { return bIsCommunicationRunning; }
//	virtual sal_Bool IsCommunicationError();

//	Der Name oder die IP-Adresse oder sonstwas um den Communikationspartner zu identifizieren
	virtual ByteString GetMyName( CM_NameType eType );

	virtual sal_Bool IsLinkValid( CommunicationLink* pCL )=0;	// Notwendig f�r call im Destruktor

	virtual sal_uInt16 GetCommunicationLinkCount()=0;
	virtual CommunicationLinkRef GetCommunicationLink( sal_uInt16 nNr )=0;

	// Liefert den letzten neuen Link oder NULL wenn dieser schon wieder geschlossen ist.
	CommunicationLinkRef GetLastNewLink() { return xLastNewLink; }

	void SetConnectionOpenedHdl( Link lConnectionOpened ){ mlConnectionOpened = lConnectionOpened; }
	void SetConnectionClosedHdl( Link lConnectionClosed ){ mlConnectionClosed = lConnectionClosed; }
	void SetDataReceivedHdl( Link lDataReceived ){ mlDataReceived = lDataReceived; }
	void SetInfoMsgHdl( Link lInfoMsg ){ mlInfoMsg = lInfoMsg; }

	void SetInfoType( CM_InfoType nIT ){ nInfoType = nIT; }
	CM_InfoType GetInfoType(){ return nInfoType; }

	sal_Bool IsMultiChannel(){ return bIsMultiChannel; }
	void SetApplication( const ByteString& aApp, sal_Bool bRunningLinks = sal_False );
	const ByteString& GetApplication() { return maApplication; }

protected:
	// Diese Methoden werden innerhalb gerufen. Sie erledigen eventuelles Housekeeping
	// und rufen dann die entsprechende Methode
	virtual void CallConnectionOpened( CommunicationLink* pCL );
	virtual void CallConnectionClosed( CommunicationLink* pCL );
	void CallDataReceived( CommunicationLink* pCL );
	void CallInfoMsg( InfoString aMsg );

	CM_InfoType nInfoType;

	// 	Diese Routinen rufen den Link oder sind �berladen
	virtual void ConnectionOpened( CommunicationLink* pCL ){ mlConnectionOpened.Call( pCL ); }
	virtual void ConnectionClosed( CommunicationLink* pCL ){ mlConnectionClosed.Call( pCL ); }
	virtual void DataReceived( CommunicationLink* pCL ){ mlDataReceived.Call( pCL ); }
	virtual void InfoMsg( InfoString aMsg ){ mlInfoMsg.Call( &aMsg ); }

	sal_Bool bIsCommunicationRunning;

	virtual void DestroyingLink( CommunicationLink *pCL )=0;	// Link tr�gt sich im Destruktor aus

private:
	ByteString maApplication;
	Link mlConnectionOpened;
	Link mlConnectionClosed;
	Link mlDataReceived;
	Link mlInfoMsg;
	CommunicationLinkRef xLastNewLink;

	sal_Bool bIsMultiChannel;
};

class AUTOMATION_DLLPUBLIC SingleCommunicationManager : public CommunicationManager
{
public:
	SingleCommunicationManager( sal_Bool bUseMultiChannel = sal_False );
	virtual ~SingleCommunicationManager();
	virtual sal_Bool StopCommunication();		// H�lt alle CommunicationLinks an
	virtual sal_Bool IsLinkValid( CommunicationLink* pCL );
	virtual sal_uInt16 GetCommunicationLinkCount();
	virtual CommunicationLinkRef GetCommunicationLink( sal_uInt16 nNr );

protected:
	virtual void CallConnectionOpened( CommunicationLink* pCL );
	virtual void CallConnectionClosed( CommunicationLink* pCL );
	CommunicationLinkRef xActiveLink;
	CommunicationLink *pInactiveLink;
	virtual void DestroyingLink( CommunicationLink *pCL );	// Link tr�gt sich im Destruktor aus
};

class AUTOMATION_DLLPUBLIC ICommunicationManagerClient
{
	friend class CommonSocketFunctions;
protected:
	virtual sal_Bool RetryConnect() { return sal_False; }	// Kann dann eventuell die Applikation starten
};

class TCPIO;
class AUTOMATION_DLLPUBLIC SimpleCommunicationLinkViaSocket : public CommunicationLink
{
public:
	virtual sal_Bool IsCommunicationError();
	virtual sal_Bool StopCommunication();

	virtual ByteString GetCommunicationPartner( CM_NameType eType );
	virtual ByteString GetMyName( CM_NameType eType );
	virtual SvStream* GetBestCommunicationStream();
	virtual void SetApplication( const ByteString& aApp );

private:
	ByteString aCommunicationPartner;
	ByteString aMyName;

	TCPIO* pTCPIO;
	vos::OStreamSocket *pStreamSocket;

protected:
	SimpleCommunicationLinkViaSocket( CommunicationManager *pMan, vos::OStreamSocket *pSocket );
	virtual ~SimpleCommunicationLinkViaSocket();

    vos::OStreamSocket* GetStreamSocket() { return pStreamSocket; }
    void SetStreamSocket( vos::OStreamSocket* pSocket );

	SvStream *pReceiveStream;
	sal_Bool DoReceiveDataStream();				/// Receive DataPacket from Socket
	virtual sal_Bool SendHandshake( HandshakeType aHandshakeType, SvStream* pData = NULL);
	void SetFinalRecieveTimeout();
	sal_Bool bIsRequestShutdownPending;
	virtual void WaitForShutdown()=0;
	void SetNewPacketAsCurrent();
};

class AUTOMATION_DLLPUBLIC SimpleCommunicationLinkViaSocketWithReceiveCallbacks : public SimpleCommunicationLinkViaSocket
{
public:
	SimpleCommunicationLinkViaSocketWithReceiveCallbacks( CommunicationManager *pMan, vos::OStreamSocket *pSocket );
	~SimpleCommunicationLinkViaSocketWithReceiveCallbacks();
	virtual sal_Bool ReceiveDataStream();
protected:
	virtual sal_Bool ShutdownCommunication();	/// Really stop the Communication
	virtual void WaitForShutdown();
};

class AUTOMATION_DLLPUBLIC CommonSocketFunctions
{
public:
	sal_Bool DoStartCommunication( CommunicationManager *pCM, ICommunicationManagerClient *pCMC, ByteString aHost, sal_uLong nPort );
protected:
	virtual CommunicationLink *CreateCommunicationLink( CommunicationManager *pCM, vos::OConnectorSocket *pCS )=0;
};

class AUTOMATION_DLLPUBLIC SingleCommunicationManagerClientViaSocket : public SingleCommunicationManager, public ICommunicationManagerClient, CommonSocketFunctions
{
public:
    using CommunicationManager::StartCommunication;

	SingleCommunicationManagerClientViaSocket( ByteString aHost, sal_uLong nPort, sal_Bool bUseMultiChannel = sal_False );
	SingleCommunicationManagerClientViaSocket( sal_Bool bUseMultiChannel = sal_False );
	virtual sal_Bool StartCommunication(){ return DoStartCommunication( this, (ICommunicationManagerClient*) this, aHostToTalk, nPortToTalk );}
	virtual sal_Bool StartCommunication( ByteString aHost, sal_uLong nPort ){ return DoStartCommunication( this, (ICommunicationManagerClient*) this, aHost, nPort );}
private:
	ByteString aHostToTalk;
	sal_uLong nPortToTalk;
protected:
	virtual CommunicationLink *CreateCommunicationLink( CommunicationManager *pCM, vos::OConnectorSocket *pCS ){ return new SimpleCommunicationLinkViaSocketWithReceiveCallbacks( pCM, pCS ); }
};

#endif
