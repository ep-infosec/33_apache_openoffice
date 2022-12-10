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



/* makefile.mk changed 20030409, LO */

#ifndef _SOCOMP_EVALUATION_HXX_
#define _SOCOMP_EVALUATION_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace desktop {

class SOEvaluation : public ::cppu::WeakImplHelper4< XExactName, XMaterialHolder, XComponent, XServiceInfo >
{
    ::osl::Mutex						m_aMutex;
    ::cppu::OInterfaceContainerHelper	m_aListeners;
    Reference< XMultiServiceFactory >	m_xServiceManager;

public:
                            SOEvaluation( const Reference < XMultiServiceFactory >& xFactory );
    virtual                 ~SOEvaluation();

    static Reference< XSingleServiceFactory >	GetSOEvaluationFactory( Reference< XMultiServiceFactory > & xSMgr );
    static ::rtl::OUString						GetImplementationName();
    static Sequence< rtl::OUString >			GetSupportedServiceNames();

	// XComponent
    virtual void SAL_CALL				dispose() throw ( RuntimeException );
    virtual void SAL_CALL				addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException );
    virtual void SAL_CALL				removeEventListener(const Reference< XEventListener > & aListener) throw ( RuntimeException );

	// XExactName
	virtual rtl::OUString SAL_CALL		getExactName( const rtl::OUString& rApproximateName ) throw ( RuntimeException );

	// XMaterialHolder
	virtual Any SAL_CALL				getMaterial() throw ( RuntimeException );

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL	getImplementationName() throw ( RuntimeException );
	virtual sal_Bool SAL_CALL			supportsService( const ::rtl::OUString& rServiceName ) throw ( RuntimeException );
	virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException );

    static const char* interfaces[];
    static const char* implementationName;
    static const char* serviceName;
    static Reference<XInterface> SAL_CALL CreateInstance(
        const Reference< XMultiServiceFactory >&);



};
}
#endif // _SOCOMP_EVALUATION_HXX_
