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
#include "precompiled_desktop.hxx"

#include "evaluation.hxx"
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/util/Date.hpp>
#include <rtl/ustrbuf.hxx>
#include <uno/environment.h>
#include <cppuhelper/factory.hxx>
#include <unotools/configmgr.hxx>
#include <vcl/msgbox.hxx>
#include <tools/resmgr.hxx>
#include <tools/resid.hxx>
#include "../app/desktop.hrc"


using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;

namespace desktop {

static SOEvaluation*	pSOEval=0;

const char* SOEvaluation::interfaces[] =
{
    "com.sun.star.beans.XExactName",
    "com.sun.star.beans.XMaterialHolder",
    "com.sun.star.lang.XComponent",
    "com.sun.star.lang.XServiceInfo",
    NULL,
};

const char* SOEvaluation::implementationName = "com.sun.star.comp.desktop.Evaluation";
const char* SOEvaluation::serviceName = "com.sun.star.office.Evaluation";

OUString SOEvaluation::GetImplementationName()
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM( implementationName));
}

Sequence< OUString > SOEvaluation::GetSupportedServiceNames()
{
	sal_Int32 nSize = (sizeof( interfaces ) / sizeof( const char *)) - 1;
	Sequence< OUString > aResult( nSize );
	
	for( sal_Int32 i = 0; i < nSize; i++ )
		aResult[i] = OUString::createFromAscii( interfaces[i] );
	return aResult;
}

Reference< XInterface >  SAL_CALL SOEvaluation::CreateInstance( 
    const Reference< XMultiServiceFactory >& rSMgr )
{
	static osl::Mutex	aMutex;
	if ( pSOEval == 0 )
	{
		osl::MutexGuard guard( aMutex );
		if ( pSOEval == 0 )
			return (XComponent*) ( new SOEvaluation( rSMgr ) );
	}
	return (XComponent*)0;
}

SOEvaluation::SOEvaluation( const Reference< XMultiServiceFactory >& xFactory ) : 
	m_aListeners( m_aMutex ),
	m_xServiceManager( xFactory )
{
}

SOEvaluation::~SOEvaluation()
{
}

// XComponent
void SAL_CALL SOEvaluation::dispose() throw ( RuntimeException )
{
    EventObject aObject;
    aObject.Source = (XComponent*)this;
    m_aListeners.disposeAndClear( aObject );
}

void SAL_CALL SOEvaluation::addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException )
{
    m_aListeners.addInterface( aListener );
}

void SAL_CALL SOEvaluation::removeEventListener( const Reference< XEventListener > & aListener ) throw ( RuntimeException )
{
    m_aListeners.removeInterface( aListener );
}

// XExactName
rtl::OUString SAL_CALL SOEvaluation::getExactName( const rtl::OUString& rApproximateName ) throw ( RuntimeException )
{
    // get the tabreg service for an evaluation version
    // without this service office shouldn't run at all
	OUString aTitle = rApproximateName;
    OUString aEval;
    sal_Bool bExpired = sal_True;
    Reference < XMaterialHolder > xHolder( m_xServiceManager->createInstance(
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.tab.tabreg" ) ) ), UNO_QUERY );
    if ( xHolder.is() )
    {
        // get a sequence of strings for the defined locales
        // a registered version doesn't provide data
        bExpired = sal_False;
        Any aData = xHolder->getMaterial();
        Sequence < NamedValue > aSeq;
        if ( aData >>= aSeq )
        {
            // this is an evaluation version, because it provides "material"
            bExpired = sal_True;
            for (int i=0; i<aSeq.getLength(); i++ )
            {
                NamedValue& rValue = aSeq[i];
                if ( rValue.Name.equalsAscii("expired") )
                    rValue.Value >>= bExpired;
                else if (rValue.Name.equalsAscii("title") )
                    rValue.Value >>= aEval;
            }
            // append eval string to title
            aTitle += OUString::createFromAscii(" ") + aEval;
            if ( bExpired )
                throw RuntimeException();
        }
    }

	return aTitle;
}

// XMaterialHolder
Any SAL_CALL SOEvaluation::getMaterial() throw( RuntimeException )
{
	// Time bomb implementation. Return empty Any to do nothing or
	// provide a com::sun::star::util::Date with the time bomb date.
	Any a;

    // change here to force recompile 00002
#ifdef TIMEBOMB	
	// Code for extracting/providing time bomb date!
	int nDay   = TIMEBOMB % 100;
	int nMonth = ( TIMEBOMB % 10000 ) / 100;
    int nYear  = TIMEBOMB / 10000;
	com::sun::star::util::Date	aDate( nDay, nMonth, nYear );
	a <<= aDate;
#endif
	return a;
}

// XServiceInfo
::rtl::OUString SAL_CALL SOEvaluation::getImplementationName() 
throw ( RuntimeException )
{
	return SOEvaluation::GetImplementationName();
}

sal_Bool SAL_CALL SOEvaluation::supportsService( const ::rtl::OUString& rServiceName ) 
throw ( RuntimeException )
{
	sal_Int32 nSize = (sizeof( interfaces ) / sizeof( const char *))-1;
	
	for( sal_Int32 i = 0; i < nSize; i++ )
		if ( rServiceName.equalsAscii( interfaces[i] ))
			return sal_True;
	return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL SOEvaluation::getSupportedServiceNames() 
throw ( RuntimeException )
{
	return SOEvaluation::GetSupportedServiceNames();
}

}
