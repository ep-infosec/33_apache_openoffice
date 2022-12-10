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
#include "precompiled_xmlsecurity.hxx"
#include <sal/config.h>
#include <rtl/uuid.h>
#include "securityenvironment_nssimpl.hxx"

#ifndef _XMLSECURITYCONTEXT_NSSIMPL_HXX_
#include "xmlsecuritycontext_nssimpl.hxx"
#endif
#include "xmlstreamio.hxx"

#include <sal/types.h>
//For reasons that escape me, this is what xmlsec does when size_t is not 4
#if SAL_TYPES_SIZEOFPOINTER != 4
#    define XMLSEC_NO_SIZE_T
#endif
#include "xmlsec/xmlsec.h"
#include "xmlsec/keysmngr.h"
#include "xmlsec/crypto.h"

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;

XMLSecurityContext_NssImpl :: XMLSecurityContext_NssImpl( const Reference< XMultiServiceFactory >& aFactory ) 
	://i39448 : m_pKeysMngr( NULL ) ,
	m_xServiceManager( aFactory ) ,
	m_nDefaultEnvIndex(-1)
	//m_xSecurityEnvironment( NULL ) 
{
	//Init xmlsec library
	if( xmlSecInit() < 0 ) {
		throw RuntimeException() ;
	}

	//Init xmlsec crypto engine library
	if( xmlSecCryptoInit() < 0 ) {
		xmlSecShutdown() ;
		throw RuntimeException() ;
	}

	//Enable external stream handlers
	if( xmlEnableStreamInputCallbacks() < 0 ) {
		xmlSecCryptoShutdown() ;
		xmlSecShutdown() ;
		throw RuntimeException() ;
	}
}

XMLSecurityContext_NssImpl :: ~XMLSecurityContext_NssImpl() {
#if 0 //i39448
	if( m_pKeysMngr != NULL ) {
		xmlSecKeysMngrDestroy( m_pKeysMngr ) ;
	}
#endif

	xmlDisableStreamInputCallbacks() ;
	xmlSecCryptoShutdown() ;
	xmlSecShutdown() ;
}

//i39448 : new methods
sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::addSecurityEnvironment(
	const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment)
	throw (::com::sun::star::security::SecurityInfrastructureException, ::com::sun::star::uno::RuntimeException)
{
	if( !aSecurityEnvironment.is() )
	{
		throw RuntimeException() ;
	}

	m_vSecurityEnvironments.push_back( aSecurityEnvironment );

	return m_vSecurityEnvironments.size() - 1 ;
}


sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::getSecurityEnvironmentNumber(  )
	throw (::com::sun::star::uno::RuntimeException)
{
	return m_vSecurityEnvironments.size();
}
	
::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
	XMLSecurityContext_NssImpl::getSecurityEnvironmentByIndex( sal_Int32 index )
	throw (::com::sun::star::uno::RuntimeException)
{
	::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment;
	
	if (index >= 0 && index < ( sal_Int32 )m_vSecurityEnvironments.size())
	{
		xSecurityEnvironment = m_vSecurityEnvironments[index];
	}
	else
		throw RuntimeException() ;
		
	return xSecurityEnvironment;
}
	
::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL 
	XMLSecurityContext_NssImpl::getSecurityEnvironment(  ) 
	throw (::com::sun::star::uno::RuntimeException)
{
	if (m_nDefaultEnvIndex >= 0 && m_nDefaultEnvIndex < ( sal_Int32 )m_vSecurityEnvironments.size())
		return getSecurityEnvironmentByIndex(m_nDefaultEnvIndex);
	else
		throw RuntimeException() ;
}

sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::getDefaultSecurityEnvironmentIndex(  )
	throw (::com::sun::star::uno::RuntimeException)
{
	return m_nDefaultEnvIndex ;
}

void SAL_CALL XMLSecurityContext_NssImpl::setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex )
	throw (::com::sun::star::uno::RuntimeException)
{
	m_nDefaultEnvIndex = nDefaultEnvIndex;
}

#if 0 //i39448 : old methods should be deleted
/* XXMLSecurityContext */
void SAL_CALL XMLSecurityContext_NssImpl :: setSecurityEnvironment( const Reference< XSecurityEnvironment >& aSecurityEnvironment ) throw( com::sun::star::security::SecurityInfrastructureException ) {
	PK11SlotInfo* slot ;
	CERTCertDBHandle* handler ;
	//xmlSecKeyPtr key ;
	//xmlSecKeyDataPtr keyData ;
	PK11SymKey* symKey ;
	SECKEYPublicKey* pubKey ;
	SECKEYPrivateKey* priKey ;
	unsigned int i ;

	if( !aSecurityEnvironment.is() )
		throw RuntimeException() ;

	m_xSecurityEnvironment = aSecurityEnvironment ;

	//Clear key manager
	if( m_pKeysMngr != NULL ) {
		xmlSecKeysMngrDestroy( m_pKeysMngr ) ;
		m_pKeysMngr = NULL ;
	}

	//Create key manager
	Reference< XUnoTunnel > xEnvTunnel( m_xSecurityEnvironment , UNO_QUERY ) ;
	if( !xEnvTunnel.is() ) {
		throw RuntimeException() ;^1
	}

	SecurityEnvironment_NssImpl* pSecEnv = ( SecurityEnvironment_NssImpl* )xEnvTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ) ;
	if( pSecEnv == NULL )
		throw RuntimeException() ;

	//todo
//	slot = pSecEnv->getCryptoSlot() ;
	handler = pSecEnv->getCertDb() ;

	/*-
	 * The following lines is based on the private version of xmlSec-NSS
	 * crypto engine
	 */
	m_pKeysMngr = xmlSecNssAppliedKeysMngrCreate( slot , handler ) ;
	if( m_pKeysMngr == NULL )
		throw RuntimeException() ;

	/*-
	 * Adopt symmetric key into keys manager
	 */
	for( i = 0 ; ( symKey = pSecEnv->getSymKey( i ) ) != NULL ; i ++ ) {
		if( xmlSecNssAppliedKeysMngrSymKeyLoad( m_pKeysMngr, symKey ) < 0 ) {
			throw RuntimeException() ;
		}
	}

	/*-
	 * Adopt asymmetric public key into keys manager
	 */
	for( i = 0 ; ( pubKey = pSecEnv->getPubKey( i ) ) != NULL ; i ++ ) {
		if( xmlSecNssAppliedKeysMngrPubKeyLoad( m_pKeysMngr, pubKey ) < 0 ) {
			throw RuntimeException() ;
		}
	}

	/*-
	 * Adopt asymmetric private key into keys manager
	 */
	for( i = 0 ; ( priKey = pSecEnv->getPriKey( i ) ) != NULL ; i ++ ) {
		if( xmlSecNssAppliedKeysMngrPriKeyLoad( m_pKeysMngr, priKey ) < 0 ) {
			throw RuntimeException() ;
		}
	}
}

/* XXMLSecurityContext */
Reference< XSecurityEnvironment > SAL_CALL XMLSecurityContext_NssImpl :: getSecurityEnvironment() 
	throw (RuntimeException)
{
	return	m_xSecurityEnvironment ;
}
#endif


/* XInitialization */
void SAL_CALL XMLSecurityContext_NssImpl :: initialize( const Sequence< Any >& /*aArguments*/ ) throw( Exception, RuntimeException ) {
	// TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext_NssImpl :: getImplementationName() throw( RuntimeException ) {
	return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext_NssImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
	Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
	const OUString* pArray = seqServiceNames.getConstArray() ;
	for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
		if( *( pArray + i ) == serviceName )
			return sal_True ;
	}
	return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSecurityContext_NssImpl :: getSupportedServiceNames() throw( RuntimeException ) {
	return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSecurityContext_NssImpl :: impl_getSupportedServiceNames() {
	::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
	Sequence< OUString > seqServiceNames( 1 ) ;
	seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.XMLSecurityContext" ) ;
	return seqServiceNames ;
}

OUString XMLSecurityContext_NssImpl :: impl_getImplementationName() throw( RuntimeException ) {
	return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.XMLSecurityContext_NssImpl" ) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSecurityContext_NssImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
	return Reference< XInterface >( *new XMLSecurityContext_NssImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLSecurityContext_NssImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
	//Reference< XSingleServiceFactory > xFactory ;
	//xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
	//return xFactory ;
	return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

#if 0 //not useful any longer
/* XUnoTunnel */
sal_Int64 SAL_CALL XMLSecurityContext_NssImpl :: getSomething( const Sequence< sal_Int8 >& aIdentifier ) 
throw (RuntimeException)
{
	if( aIdentifier.getLength() == 16 && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) { 
		return ( sal_Int64 )this ;
	}
	return 0 ;
}

/* XUnoTunnel extension */
const Sequence< sal_Int8>& XMLSecurityContext_NssImpl :: getUnoTunnelId() {
	static Sequence< sal_Int8 >* pSeq = 0 ;
	if( !pSeq ) {
		::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
		if( !pSeq ) {
			static Sequence< sal_Int8> aSeq( 16 ) ;
			rtl_createUuid( ( sal_uInt8* )aSeq.getArray() , 0 , sal_True ) ;
			pSeq = &aSeq ;
		}
	}
	return *pSeq ;
}

/* XUnoTunnel extension */
XMLSecurityContext_NssImpl* XMLSecurityContext_NssImpl :: getImplementation( const Reference< XInterface > xObj ) {
	Reference< XUnoTunnel > xUT( xObj , UNO_QUERY ) ;
	if( xUT.is() ) {
		return ( XMLSecurityContext_NssImpl* )xUT->getSomething( getUnoTunnelId() ) ;
	} else
		return NULL ;
}

/* Native methods */
xmlSecKeysMngrPtr XMLSecurityContext_NssImpl :: keysManager() throw( Exception, RuntimeException ) {
	return m_pKeysMngr ;
}

#endif
