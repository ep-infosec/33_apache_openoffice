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
#include "xmlencryption_nssimpl.hxx"

#ifndef _XMLDOCUMENTWRAPPER_XMLSECIMPL_HXX_
#include "xmldocumentwrapper_xmlsecimpl.hxx"
#endif

#ifndef _XMLELEMENTWRAPPER_XMLSECIMPL_HXX_
#include "xmlelementwrapper_xmlsecimpl.hxx"
#endif

#ifndef _SECURITYENVIRONMENT_NSSIMPL_HXX_
#include "securityenvironment_nssimpl.hxx"
#endif
#include "errorcallback.hxx"

#include <sal/types.h>
//For reasons that escape me, this is what xmlsec does when size_t is not 4
#if SAL_TYPES_SIZEOFPOINTER != 4
#    define XMLSEC_NO_SIZE_T
#endif
#include "xmlsec/xmlsec.h"
#include "xmlsec/xmltree.h"
#include "xmlsec/xmlenc.h"
#include "xmlsec/crypto.h"

#ifdef UNX
#define stricmp strcasecmp
#endif

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::wrapper::XXMLDocumentWrapper ;
using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLEncryption ;
using ::com::sun::star::xml::crypto::XXMLEncryptionTemplate ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;
using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XMLEncryptionException ;

XMLEncryption_NssImpl :: XMLEncryption_NssImpl( const Reference< XMultiServiceFactory >& aFactory ) : m_xServiceManager( aFactory ) {
}

XMLEncryption_NssImpl :: ~XMLEncryption_NssImpl() {
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate >
SAL_CALL XMLEncryption_NssImpl :: encrypt(
	const Reference< XXMLEncryptionTemplate >& aTemplate ,
	const Reference< XSecurityEnvironment >& aEnvironment
) throw( com::sun::star::xml::crypto::XMLEncryptionException, 
		 com::sun::star::uno::SecurityException )
{
	xmlSecKeysMngrPtr pMngr = NULL ;
	xmlSecEncCtxPtr pEncCtx = NULL ;
	xmlNodePtr pEncryptedData = NULL ;
	xmlNodePtr pContent = NULL ;

	if( !aTemplate.is() )
		throw RuntimeException() ;

	if( !aEnvironment.is() )
		throw RuntimeException() ;

	//Get Keys Manager
	Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY ) ;
	if( !xSecTunnel.is() ) {
		 throw RuntimeException() ;
	}

#if 0
	XMLSecurityContext_NssImpl* pSecCtxt = ( XMLSecurityContext_NssImpl* )xSecTunnel->getSomething( XMLSecurityContext_NssImpl::getUnoTunnelId() ) ;
	if( pSecCtxt == NULL )
		throw RuntimeException() ;
#endif

	SecurityEnvironment_NssImpl* pSecEnv =
        reinterpret_cast<SecurityEnvironment_NssImpl*>(
            sal::static_int_cast<sal_uIntPtr>(xSecTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() ))) ;
	if( pSecEnv == NULL )
		throw RuntimeException() ;
		
	//Get the encryption template
	Reference< XXMLElementWrapper > xTemplate = aTemplate->getTemplate() ;
	if( !xTemplate.is() ) {
		throw RuntimeException() ;
	}

	Reference< XUnoTunnel > xTplTunnel( xTemplate , UNO_QUERY ) ;
	if( !xTplTunnel.is() ) {
		throw RuntimeException() ;
	}

	XMLElementWrapper_XmlSecImpl* pTemplate =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xTplTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
	if( pTemplate == NULL ) {
		throw RuntimeException() ;
	}

	//MM : Get the element to be encrypted
	Reference< XXMLElementWrapper > xTarget = aTemplate->getTarget() ;
	if( !xTarget.is() ) {
		throw XMLEncryptionException() ;
	}

	Reference< XUnoTunnel > xTgtTunnel( xTarget , UNO_QUERY ) ;
	if( !xTgtTunnel.is() ) {
		throw XMLEncryptionException() ;
	}

	XMLElementWrapper_XmlSecImpl* pTarget =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xTgtTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
	if( pTarget == NULL ) {
		throw RuntimeException() ;
	}

	pContent = pTarget->getNativeElement() ;
	//MM : end

	if( pContent == NULL ) {
		throw XMLEncryptionException() ;
	}
	
	/* MM : remove the following 2 lines
	xmlUnlinkNode(pContent);
	xmlAddNextSibling(pEncryptedData, pContent);
	*/

	//remember the position of the element to be signed
	sal_Bool isParentRef = sal_True;
	xmlNodePtr pParent = pEncryptedData->parent;
	xmlNodePtr referenceNode;

	if (pEncryptedData == pParent->children)
	{
		referenceNode = pParent;
	}
	else
	{
		referenceNode = pEncryptedData->prev;
		isParentRef = sal_False;
	}

 	setErrorRecorder( );

	pMngr = pSecEnv->createKeysManager() ; //i39448
	if( !pMngr ) {
		throw RuntimeException() ;
	}

	//Create Encryption context
	pEncCtx = xmlSecEncCtxCreate( pMngr ) ;
	if( pEncCtx == NULL )
	{
		pSecEnv->destroyKeysManager( pMngr ) ; //i39448
		//throw XMLEncryptionException() ;
		clearErrorRecorder();
		return aTemplate;
	}

	pEncryptedData = pTemplate->getNativeElement() ;

	//Find the element to be encrypted. 
	/* MM : remove the old method to get the target element
	//This element is wrapped in the CipherValue sub-element.
	xmlNodePtr pCipherData = pEncryptedData->children;
	while (pCipherData != NULL && stricmp((const char *)(pCipherData->name), "CipherData"))
	{
		pCipherData = pCipherData->next;
	}

	if( pCipherData == NULL ) {
		xmlSecEncCtxDestroy( pEncCtx ) ;
		throw XMLEncryptionException() ;
	}

	xmlNodePtr pCipherValue = pCipherData->children;
	while (pCipherValue != NULL && stricmp((const char *)(pCipherValue->name), "CipherValue"))
	{
		pCipherValue = pCipherValue->next;
	}

	if( pCipherValue == NULL ) {
		xmlSecEncCtxDestroy( pEncCtx ) ;
		throw XMLEncryptionException() ;
	}

	pContent = pCipherValue->children;
	*/

	//Encrypt the template
	if( xmlSecEncCtxXmlEncrypt( pEncCtx , pEncryptedData , pContent ) < 0 ) 
	{
		xmlSecEncCtxDestroy( pEncCtx ) ;
		pSecEnv->destroyKeysManager( pMngr ) ; //i39448
		
		//throw XMLEncryptionException() ;
		clearErrorRecorder();
		return aTemplate;
	}

	xmlSecEncCtxDestroy( pEncCtx ) ;
	pSecEnv->destroyKeysManager( pMngr ) ; //i39448

	//get the new EncryptedData element
	if (isParentRef)
	{
		pTemplate->setNativeElement(referenceNode->children) ;
	}
	else
	{
		pTemplate->setNativeElement(referenceNode->next);
	}

	return aTemplate ;
}

/* XXMLEncryption */
Reference< XXMLEncryptionTemplate >
SAL_CALL XMLEncryption_NssImpl :: decrypt(
	const Reference< XXMLEncryptionTemplate >& aTemplate ,
	const Reference< XXMLSecurityContext >& aSecurityCtx
) throw( com::sun::star::xml::crypto::XMLEncryptionException , 
		 com::sun::star::uno::SecurityException) {
	xmlSecKeysMngrPtr pMngr = NULL ;
	xmlSecEncCtxPtr pEncCtx = NULL ;
	xmlNodePtr pEncryptedData = NULL ;

	if( !aTemplate.is() )
		throw RuntimeException() ;

	if( !aSecurityCtx.is() )
		throw RuntimeException() ;

	//Get the encryption template
	Reference< XXMLElementWrapper > xTemplate = aTemplate->getTemplate() ;
	if( !xTemplate.is() ) {
		throw RuntimeException() ;
	}

	Reference< XUnoTunnel > xTplTunnel( xTemplate , UNO_QUERY ) ;
	if( !xTplTunnel.is() ) {
		throw RuntimeException() ;
	}

	XMLElementWrapper_XmlSecImpl* pTemplate =
        reinterpret_cast<XMLElementWrapper_XmlSecImpl*>(
            sal::static_int_cast<sal_uIntPtr>(
                xTplTunnel->getSomething( XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId() )));
	if( pTemplate == NULL ) {
		throw RuntimeException() ;
	}

	pEncryptedData = pTemplate->getNativeElement() ;

	//remember the position of the element to be signed
	sal_Bool isParentRef = sal_True;
	xmlNodePtr pParent = pEncryptedData->parent;
	xmlNodePtr referenceNode;

	if (pEncryptedData == pParent->children)
	{
		referenceNode = pParent;
	}
	else
	{
		referenceNode = pEncryptedData->prev;
		isParentRef = sal_False;
	}

 	setErrorRecorder( );
		
	sal_Int32 nSecurityEnvironment = aSecurityCtx->getSecurityEnvironmentNumber();
	sal_Int32 i;
	
	for (i=0; i<nSecurityEnvironment; ++i)
	{
		Reference< XSecurityEnvironment > aEnvironment = aSecurityCtx->getSecurityEnvironmentByIndex(i);
		
		//Get Keys Manager
		Reference< XUnoTunnel > xSecTunnel( aEnvironment , UNO_QUERY ) ;
		if( !aEnvironment.is() ) {
			 throw RuntimeException() ;
		}
	
		SecurityEnvironment_NssImpl* pSecEnv =
            reinterpret_cast<SecurityEnvironment_NssImpl*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xSecTunnel->getSomething( SecurityEnvironment_NssImpl::getUnoTunnelId() )));
		if( pSecEnv == NULL )
			throw RuntimeException() ;
			
		pMngr = pSecEnv->createKeysManager() ; //i39448
		if( !pMngr ) {
			throw RuntimeException() ;
		}
			
		//Create Encryption context
		pEncCtx = xmlSecEncCtxCreate( pMngr ) ;
		if( pEncCtx == NULL )
		{
			pSecEnv->destroyKeysManager( pMngr ) ; //i39448
			//throw XMLEncryptionException() ;
			clearErrorRecorder();
			return aTemplate;
		}

		//Decrypt the template
		if(!( xmlSecEncCtxDecrypt( pEncCtx , pEncryptedData ) < 0 || pEncCtx->result == NULL ))
		{
			//The decryption succeeds

			//Destroy the encryption context
			xmlSecEncCtxDestroy( pEncCtx ) ;
			pSecEnv->destroyKeysManager( pMngr ) ; //i39448
		
			//get the decrypted element
			XMLElementWrapper_XmlSecImpl * ret = new XMLElementWrapper_XmlSecImpl(isParentRef?
				(referenceNode->children):(referenceNode->next));
		
			//return ret;
			aTemplate->setTemplate(ret);
			break;
		}
		else
		{
			//The decryption fails, continue with the next security environment
			xmlSecEncCtxDestroy( pEncCtx ) ;
			pSecEnv->destroyKeysManager( pMngr ) ; //i39448
		}
	}

	clearErrorRecorder();
	return aTemplate;
}

/* XInitialization */
void SAL_CALL XMLEncryption_NssImpl :: initialize( const Sequence< Any >& /*aArguments*/ ) throw( Exception, RuntimeException ) {
	// TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLEncryption_NssImpl :: getImplementationName() throw( RuntimeException ) {
	return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryption_NssImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
	Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
	const OUString* pArray = seqServiceNames.getConstArray() ;
	for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
		if( *( pArray + i ) == serviceName )
			return sal_True ;
	}
	return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryption_NssImpl :: getSupportedServiceNames() throw( RuntimeException ) {
	return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryption_NssImpl :: impl_getSupportedServiceNames() {
	::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
	Sequence< OUString > seqServiceNames( 1 ) ;
	seqServiceNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.xml.crypto.XMLEncryption" ) ;
	return seqServiceNames ;
}

OUString XMLEncryption_NssImpl :: impl_getImplementationName() throw( RuntimeException ) {
	return OUString::createFromAscii( "com.sun.star.xml.security.bridge.xmlsec.XMLEncryption_NssImpl" ) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryption_NssImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
	return Reference< XInterface >( *new XMLEncryption_NssImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLEncryption_NssImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
	//Reference< XSingleServiceFactory > xFactory ;
	//xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
	//return xFactory ;
	return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

