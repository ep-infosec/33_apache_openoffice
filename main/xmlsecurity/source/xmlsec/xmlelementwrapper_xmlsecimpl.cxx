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

#include "xmlelementwrapper_xmlsecimpl.hxx"
#include <cppuhelper/typeprovider.hxx>

namespace cssu = com::sun::star::uno;
namespace cssl = com::sun::star::lang;

#define SERVICE_NAME "com.sun.star.xml.wrapper.XMLElementWrapper"
#define IMPLEMENTATION_NAME "com.sun.star.xml.security.bridge.xmlsec.XMLElementWrapper_XmlSecImpl"

XMLElementWrapper_XmlSecImpl::XMLElementWrapper_XmlSecImpl(const xmlNodePtr pNode)
	: m_pElement( pNode )
{
}

/* XXMLElementWrapper */


/* XUnoTunnel */
cssu::Sequence< sal_Int8 > XMLElementWrapper_XmlSecImpl::getUnoTunnelImplementationId( void ) 
	throw (cssu::RuntimeException)
{
	static ::cppu::OImplementationId* pId = 0;
	if (! pId)
	{
		::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		if (! pId)
		{
			static ::cppu::OImplementationId aId;
			pId = &aId;
		}
	}
	return pId->getImplementationId();
}

sal_Int64 SAL_CALL XMLElementWrapper_XmlSecImpl::getSomething( const cssu::Sequence< sal_Int8 >& aIdentifier )
	throw (cssu::RuntimeException)
{
	if (aIdentifier.getLength() == 16 &&
		0 == rtl_compareMemory(
			getUnoTunnelImplementationId().getConstArray(),
			aIdentifier.getConstArray(),
			16 ))
	{
		return reinterpret_cast < sal_Int64 > ( this );
	}
	else
	{
		return 0;
	}
}


rtl::OUString XMLElementWrapper_XmlSecImpl_getImplementationName ()
	throw (cssu::RuntimeException)
{
	return rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( IMPLEMENTATION_NAME ) );
}

sal_Bool SAL_CALL XMLElementWrapper_XmlSecImpl_supportsService( const rtl::OUString& ServiceName ) 
	throw (cssu::RuntimeException)
{
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME ));
}

cssu::Sequence< rtl::OUString > SAL_CALL XMLElementWrapper_XmlSecImpl_getSupportedServiceNames(  ) 
	throw (cssu::RuntimeException)
{
	cssu::Sequence < rtl::OUString > aRet(1);
	rtl::OUString* pArray = aRet.getArray();
	pArray[0] =  rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME ) );
	return aRet;
}
#undef SERVICE_NAME

cssu::Reference< cssu::XInterface > SAL_CALL
	XMLElementWrapper_XmlSecImpl_createInstance(
		const cssu::Reference< cssl::XMultiServiceFactory > &)
	throw( cssu::Exception )
{
	return (cppu::OWeakObject*) new XMLElementWrapper_XmlSecImpl(NULL);
}

/* XServiceInfo */
rtl::OUString SAL_CALL XMLElementWrapper_XmlSecImpl::getImplementationName(  ) 
	throw (cssu::RuntimeException)
{
	return XMLElementWrapper_XmlSecImpl_getImplementationName();
}
sal_Bool SAL_CALL XMLElementWrapper_XmlSecImpl::supportsService( const rtl::OUString& rServiceName ) 
	throw (cssu::RuntimeException)
{
	return XMLElementWrapper_XmlSecImpl_supportsService( rServiceName );
}
cssu::Sequence< rtl::OUString > SAL_CALL XMLElementWrapper_XmlSecImpl::getSupportedServiceNames(  ) 
	throw (cssu::RuntimeException)
{
	return XMLElementWrapper_XmlSecImpl_getSupportedServiceNames();
}

xmlNodePtr XMLElementWrapper_XmlSecImpl::getNativeElement(  ) const
/****** XMLElementWrapper_XmlSecImpl/getNativeElement *************************
 *
 *   NAME
 *	getNativeElement -- Retrieves the libxml2 node wrapped by this object
 *
 *   SYNOPSIS
 *	pNode = getNativeElement();
 *
 *   FUNCTION
 *	see NAME
 *
 *   INPUTS
 *	empty
 *
 *   RESULT
 *	pNode - the libxml2 node wrapped by this object
 *
 *   HISTORY
 *	05.01.2004 -	implemented
 *
 *   AUTHOR
 *	Michael Mi
 *	Email: michael.mi@sun.com
 ******************************************************************************/
{
	return m_pElement;
}

void XMLElementWrapper_XmlSecImpl::setNativeElement(const xmlNodePtr pNode)
/****** XMLElementWrapper_XmlSecImpl/setNativeElement *************************
 *
 *   NAME
 *	setNativeElement -- Configures the libxml2 node wrapped by this object
 *
 *   SYNOPSIS
 *	setNativeElement( pNode );
 *
 *   FUNCTION
 *	see NAME
 *
 *   INPUTS
 *	pNode -	the new libxml2 node to be wrapped by this object
 *
 *   RESULT
 *	empty
 *
 *   HISTORY
 *	05.01.2004 -	implemented
 *
 *   AUTHOR
 *	Michael Mi
 *	Email: michael.mi@sun.com
 ******************************************************************************/
{
	m_pElement = pNode;
}

