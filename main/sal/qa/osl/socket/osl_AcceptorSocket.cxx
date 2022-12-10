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
#include "precompiled_sal.hxx"

/**  test coder preface:
	1. the BSD socket function will meet "unresolved external symbol error" on Windows platform
	if you are not including ws2_32.lib in makefile.mk,  the including format will be like this:

	.IF "$(GUI)" == "WNT"
	SHL1STDLIBS +=	$(SOLARLIBDIR)$/cppunit.lib
	SHL1STDLIBS +=  ws2_32.lib
	.ENDIF

	likewise on Solaris platform.
	.IF "$(GUI)" == "UNX"
	SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
	SHL1STDLIBS += -lsocket -ldl -lnsl
	.ENDIF

	2. since the Socket implementation of osl is only IPv4 oriented, our test are mainly focus on IPv4
	category.

	3. some fragment of Socket source implementation are lack of comment so it is hard for testers
	guess what the exact functionality or usage of a member.  Hope the Socket section's comment
	will be added.

	4. following functions are declared but not implemented:
	inline sal_Bool SAL_CALL operator== (const SocketAddr & Addr) const;
 */

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#include "gtest/gtest.h"

#include "osl_Socket_Const.h"
#include "sockethelper.hxx"

using namespace osl;
using namespace rtl;

#define IP_PORT_FTP     21
#define IP_PORT_MYPORT9 8897
#define IP_PORT_MYPORT4 8885
#define IP_PORT_MYPORT3 8884

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

// just used to test socket::close() when accepting
class AcceptorThread : public Thread
{
	::osl::AcceptorSocket asAcceptorSocket;
	::rtl::OUString aHostIP;
	sal_Bool bOK;
protected:
	void SAL_CALL run( )
	{
		::osl::SocketAddr saLocalSocketAddr( aHostIP, IP_PORT_MYPORT9 );
		::osl::StreamSocket ssStreamConnection;

		asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1 ); //integer not sal_Bool : sal_True);
		sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
		if  ( sal_True != bOK1 )
		{
			printf("# AcceptorSocket bind address failed.\n" ) ;
			return;
		}
		sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
		if  ( sal_True != bOK2 )
		{
			printf("# AcceptorSocket listen address failed.\n" ) ;
			return;
		}

		asAcceptorSocket.enableNonBlockingMode( sal_False );

		oslSocketResult eResult = asAcceptorSocket.acceptConnection( ssStreamConnection );
		if (eResult != osl_Socket_Ok )
		{
			bOK = sal_True;
			printf("AcceptorThread: acceptConnection failed! \n");
		}
	}
public:
	AcceptorThread(::osl::AcceptorSocket & asSocket, ::rtl::OUString const& aBindIP )
		: asAcceptorSocket( asSocket ), aHostIP( aBindIP )
	{
		bOK = sal_False;
	}

	sal_Bool isOK() { return bOK; }

	~AcceptorThread( )
	{
		if ( isRunning( ) )
		{
			asAcceptorSocket.shutdown();
			printf("# error: Acceptor thread not terminated.\n" );
		}
	}
};

namespace osl_AcceptorSocket
{

	/** testing the methods:
		inline AcceptorSocket(oslAddrFamily Family = osl_Socket_FamilyInet,
							  oslProtocol	Protocol = osl_Socket_ProtocolIp,
							  oslSocketType	Type = osl_Socket_TypeStream);
	*/

	class ctors : public ::testing::Test
	{
	public:
	}; // class ctors

    TEST_F(ctors, ctors_001)
    {
        /// Socket constructor.
        ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );

        ASSERT_TRUE(osl_Socket_TypeStream ==  asSocket.getType( )) << "test for ctors_001 constructor function: check if the acceptor socket was created successfully.";
    }

#if 0  /* OBSOLETE */
	class operator_assign : public ::testing::Test
	{
	public:
	}; // class operator_assign

    TEST_F(assign, assign_001)
    {
#if defined(LINUX)
        ::osl::AcceptorSocket asSocket( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        ::osl::AcceptorSocket asSocketAssign( osl_Socket_FamilyInet, osl_Socket_ProtocolIp, osl_Socket_TypeStream );
        asSocket.setOption( osl_Socket_OptionReuseAddr, 1);
        ::osl::SocketAddr saSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT4 );
        asSocket.bind( saSocketAddr );

        AcceptorThread myAcceptorThread( asSocketAssign, rtl::OUString::createFromAscii("127.0.0.1") );
        myAcceptorThread.create();

        thread_sleep( 1 );
        //when accepting, assign another socket to the socket, the thread will not be closed, so is blocking
        asSocketAssign = asSocket;

        printf("#asSocketAssign port number is %d\n", asSocketAssign.getLocalPort() );

        asSocketAssign.shutdown();
        myAcceptorThread.join();

        ASSERT_TRUE(myAcceptorThread.isOK() == sal_True) << "test for close when is accepting: the socket will quit accepting status.";


#endif /* LINUX */
    }
#endif /* OBSOLETE */

	/** testing the method:
		inline sal_Bool SAL_CALL listen(sal_Int32 MaxPendingConnections= -1);
		inline oslSocketResult SAL_CALL acceptConnection( StreamSocket& Connection);
		inline oslSocketResult SAL_CALL acceptConnection( StreamSocket&	Connection, SocketAddr & PeerAddr);
	*/

	class listen_accept : public ::testing::Test
	{
	public:
		TimeValue *pTimeout;
		::osl::AcceptorSocket asAcceptorSocket;
		::osl::ConnectorSocket csConnectorSocket;


		// initialization
		void SetUp( )
		{
			pTimeout  = ( TimeValue* )malloc( sizeof( TimeValue ) );
			pTimeout->Seconds = 3;
			pTimeout->Nanosec = 0;
			asAcceptorSocket.setOption( osl_Socket_OptionReuseAddr, 1);
		//	sHandle = osl_createSocket( osl_Socket_FamilyInet, osl_Socket_TypeStream, osl_Socket_ProtocolIp );
		}

		void TearDown( )
		{
			free( pTimeout );
		//	sHandle = NULL;
			asAcceptorSocket.close( );
			csConnectorSocket.close( );
		}
	}; // class listen_accept

    TEST_F(listen_accept, listen_accept_001)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT3 );
        ::osl::SocketAddr saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT3 );
        ::osl::StreamSocket ssConnection;

        /// launch server socket
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";
        asAcceptorSocket.enableNonBlockingMode( sal_True );

        /// launch client socket
        csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

        oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection); /// waiting for incoming connection...

        ASSERT_TRUE( ( osl_Socket_Ok == eResult ) )
            << "test for listen_accept function: try to create a connection with remote host, using listen and accept.";
    }

    TEST_F(listen_accept, listen_accept_002)
    {
        ::osl::SocketAddr saLocalSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT4 );
        ::osl::SocketAddr saTargetSocketAddr( rtl::OUString::createFromAscii("127.0.0.1"), IP_PORT_MYPORT4 );
        ::osl::SocketAddr saPeerSocketAddr( rtl::OUString::createFromAscii("129.158.217.202"), IP_PORT_FTP );
        ::osl::StreamSocket ssConnection;

        /// launch server socket
        sal_Bool bOK1 = asAcceptorSocket.bind( saLocalSocketAddr );
        ASSERT_TRUE(sal_True == bOK1) << "AcceptorSocket bind address failed.";
        sal_Bool bOK2 = asAcceptorSocket.listen( 1 );
        ASSERT_TRUE(sal_True == bOK2) << "AcceptorSocket listen failed.";
        asAcceptorSocket.enableNonBlockingMode( sal_True );

        /// launch client socket
        csConnectorSocket.connect( saTargetSocketAddr, pTimeout );   /// connecting to server...

        oslSocketResult eResult = asAcceptorSocket.acceptConnection(ssConnection, saPeerSocketAddr); /// waiting for incoming connection...

        ASSERT_TRUE(
                                ( sal_True == bOK2 ) &&
                                ( osl_Socket_Ok == eResult ) &&
                                ( sal_True == compareSocketAddr( saPeerSocketAddr, saLocalSocketAddr ) ) )
            << "test for listen_accept function: try to create a connection with remote host, using listen and accept, accept with peer address.";
    }

} // namespace osl_AcceptorSocket

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
