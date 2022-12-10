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
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <osl/diagnose.h>
#include "WinClipboard.hxx"
#include <com/sun/star/datatransfer/clipboard/ClipboardEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "WinClipbImpl.hxx"

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace rtl;
using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define WINCLIPBOARD_IMPL_NAME  "com.sun.star.datatransfer.clipboard.ClipboardW32"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
	Sequence< OUString > SAL_CALL WinClipboard_getSupportedServiceNames()
	{
		Sequence< OUString > aRet(1);
		aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
		return aRet;
	}
}

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------
/*XEventListener,*/
CWinClipboard::CWinClipboard( const Reference< XMultiServiceFactory >& rServiceManager, const OUString& aClipboardName ) : 
	WeakComponentImplHelper4< XClipboardEx, XFlushableClipboard, XClipboardNotifier, XServiceInfo >( m_aCbListenerMutex ),	
	m_SrvMgr( rServiceManager )
{
	m_pImpl.reset( new CWinClipbImpl( aClipboardName, this ) );
}

//========================================================================
// XClipboard
//========================================================================

//------------------------------------------------------------------------
// getContent
// to avoid unnecessary traffic we check first if there is a clipboard
// content which was set via setContent, in this case we don't need 
// to query the content from the clipboard, create a new wrapper object
// and so on, we simply return the orignial XTransferable instead of our
// DOTransferable
//------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL CWinClipboard::getContents( ) throw( RuntimeException )
{
	MutexGuard aGuard( m_aMutex );
	
	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	if ( NULL != m_pImpl.get( ) )
		return m_pImpl->getContents( );

	return Reference< XTransferable >( );
}

//------------------------------------------------------------------------
// setContent
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::setContents( const Reference< XTransferable >& xTransferable,
										  const Reference< XClipboardOwner >& xClipboardOwner ) 
										  throw( RuntimeException )
{
	MutexGuard aGuard( m_aMutex );

	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	if ( NULL != m_pImpl.get( ) )
		m_pImpl->setContents( xTransferable, xClipboardOwner );
}

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

OUString SAL_CALL CWinClipboard::getName(  ) throw( RuntimeException )
{
	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	if ( NULL != m_pImpl.get( ) )
		return m_pImpl->getName( );

	return OUString::createFromAscii( "" );
}

//========================================================================
// XFlushableClipboard
//========================================================================

void SAL_CALL CWinClipboard::flushClipboard( ) throw( RuntimeException )
{
	MutexGuard aGuard( m_aMutex );

	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	if ( NULL != m_pImpl.get( ) )
		m_pImpl->flushClipboard( );
}

//========================================================================
// XClipboardEx
//========================================================================

sal_Int8 SAL_CALL CWinClipboard::getRenderingCapabilities(  ) throw( RuntimeException )
{	
	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	if ( NULL != m_pImpl.get( ) )
		return m_pImpl->getRenderingCapabilities( );	

	return 0;
}

//========================================================================
// XClipboardNotifier 
//========================================================================

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::addClipboardListener( const Reference< XClipboardListener >& listener ) 
	throw( RuntimeException )
{
	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	// check input parameter
	if ( !listener.is( ) )
		throw IllegalArgumentException( OUString::createFromAscii( "empty reference" ), 
										static_cast< XClipboardEx* >( this ), 
										1 );

	rBHelper.aLC.addInterface( getCppuType( &listener ), listener );			
}

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener ) 
	throw( RuntimeException )
{
	if ( rBHelper.bDisposed )
		throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
								 static_cast< XClipboardEx* >( this ) );

	// check input parameter
	if ( !listener.is( ) )
		throw IllegalArgumentException( OUString::createFromAscii( "empty reference" ),
										static_cast< XClipboardEx* >( this ),
										1 );

	rBHelper.aLC.removeInterface( getCppuType( &listener ), listener );	
}

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::notifyAllClipboardListener( )
{
	if ( !rBHelper.bDisposed )
	{
		ClearableMutexGuard aGuard( rBHelper.rMutex );
		if ( !rBHelper.bDisposed )
		{
			aGuard.clear( );

			OInterfaceContainerHelper* pICHelper = rBHelper.aLC.getContainer( 
				getCppuType( ( Reference< XClipboardListener > * ) 0 ) );

			if ( pICHelper )
			{			    
			    try
			    {
				    OInterfaceIteratorHelper iter(*pICHelper);
				    Reference<XTransferable> rXTransf(m_pImpl->getContents());
				    ClipboardEvent aClipbEvent(static_cast<XClipboard*>(this), rXTransf);
			    			    
				    while(iter.hasMoreElements())
				    {
					    try
					    {
						    Reference<XClipboardListener> xCBListener(iter.next(), UNO_QUERY);
						    if (xCBListener.is())
							    xCBListener->changedContents(aClipbEvent);
					    }
					    catch(RuntimeException&)
					    {
						    OSL_ENSURE( false, "RuntimeException caught" );
					    }					
				    } 
				}
			    catch(const ::com::sun::star::lang::DisposedException&)
			    {			        
			        OSL_ENSURE(false, "Service Manager disposed");
			        
			        // no further clipboard changed notifications
			        m_pImpl->unregisterClipboardViewer();
			    }
			    
			} // end if
		} // end if
	} // end if
}

//------------------------------------------------
// overwritten base class method which will be 
// called by the base class dispose method
//------------------------------------------------

void SAL_CALL CWinClipboard::disposing()
{
	// do my own stuff
	m_pImpl->dispose( );

	// force destruction of the impl class
	m_pImpl.reset();
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CWinClipboard::getImplementationName(  ) 
	throw(RuntimeException)
{
	return OUString::createFromAscii( WINCLIPBOARD_IMPL_NAME );
}

// -------------------------------------------------
//	XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CWinClipboard::supportsService( const OUString& ServiceName ) 
	throw(RuntimeException)
{
	Sequence < OUString > SupportedServicesNames = WinClipboard_getSupportedServiceNames();

	for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
		if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
			return sal_True;

	return sal_False;
}

// -------------------------------------------------
//	XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CWinClipboard::getSupportedServiceNames(	 ) 
	throw(RuntimeException)
{
	return WinClipboard_getSupportedServiceNames();
}
