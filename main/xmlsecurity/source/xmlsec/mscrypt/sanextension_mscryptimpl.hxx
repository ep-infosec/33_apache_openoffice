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



#ifndef _SANEXTENSION_MSCRYPTIMPL_HXX_
#define _SANEXTENSION_MSCRYPTIMPL_HXX_

#ifdef _MSC_VER
#pragma warning(push,1)
#endif
#include "Windows.h"
#include "WinCrypt.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include "com/sun/star/uno/SecurityException.hpp"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/security/XCertificateExtension.hpp>
#include <com/sun/star/security/XSanExtension.hpp>
#include <com/sun/star/security/CertAltNameEntry.hpp>

class SanExtensionImpl : public ::cppu::WeakImplHelper1<
    ::com::sun::star::security::XSanExtension >
{
	private :
		sal_Bool m_critical ;
		::com::sun::star::uno::Sequence< sal_Int8 > m_xExtnId ;
		::com::sun::star::uno::Sequence< sal_Int8 > m_xExtnValue ;

        ::com::sun::star::uno::Sequence< com::sun::star::security::CertAltNameEntry > m_Entries;

	public :
		SanExtensionImpl() ;
		virtual ~SanExtensionImpl() ;

		//Methods from XCertificateExtension
		virtual sal_Bool SAL_CALL isCritical() throw( ::com::sun::star::uno::RuntimeException ) ;

		virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getExtensionId() throw( ::com::sun::star::uno::RuntimeException ) ;

		virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getExtensionValue() throw( ::com::sun::star::uno::RuntimeException ) ;

        //Methods from XSanExtension

		virtual ::com::sun::star::uno::Sequence< com::sun::star::security::CertAltNameEntry > SAL_CALL getAlternativeNames() throw( ::com::sun::star::uno::RuntimeException ) ;

		//Helper method
		void setCertExtn( ::com::sun::star::uno::Sequence< sal_Int8 > extnId, ::com::sun::star::uno::Sequence< sal_Int8 > extnValue, sal_Bool critical ) ;

		void setCertExtn( unsigned char* value, unsigned int vlen, unsigned char* id, unsigned int idlen, sal_Bool critical ) ;

        void extractCertExt() ;
} ;

#endif	// _CERTIFICATEEXTENSION_XMLSECIMPL_HXX_

