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

#include "firststart.hxx"
#include "../migration/wizard.hxx"
#include <comphelper/sequenceashashmap.hxx>

using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace desktop{


const char* FirstStart::interfaces[] =
{
    "com.sun.star.task.XJob",
    NULL,
};
const char* FirstStart::implementationName = "com.sun.star.comp.desktop.FirstStart";
const char* FirstStart::serviceName = "com.sun.star.task.Job";

OUString FirstStart::GetImplementationName()
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM( implementationName));
}

Sequence< OUString > FirstStart::GetSupportedServiceNames()
{
	sal_Int32 nSize = (sizeof( interfaces ) / sizeof( const char *)) - 1;
	Sequence< OUString > aResult( nSize );

	for( sal_Int32 i = 0; i < nSize; i++ )
		aResult[i] = OUString::createFromAscii( interfaces[i] );
	return aResult;
}

Reference< XInterface >  SAL_CALL FirstStart::CreateInstance(
    const Reference< XMultiServiceFactory >& rSMgr )
{
	    static osl::Mutex aMutex;
		osl::MutexGuard guard( aMutex );
		return (XComponent*) ( new FirstStart( rSMgr ) );
}

FirstStart::FirstStart( const Reference< XMultiServiceFactory >& xFactory ) :
	m_aListeners( m_aMutex ),
	m_xServiceManager( xFactory )
{
}

FirstStart::~FirstStart()
{
}

// XComponent
void SAL_CALL FirstStart::dispose() throw ( RuntimeException )
{
    EventObject aObject;
    aObject.Source = (XComponent*)this;
    m_aListeners.disposeAndClear( aObject );
}

void SAL_CALL FirstStart::addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException )
{
    m_aListeners.addInterface( aListener );
}

void SAL_CALL FirstStart::removeEventListener( const Reference< XEventListener > & aListener ) throw ( RuntimeException )
{
    m_aListeners.removeInterface( aListener );
}

// XServiceInfo
::rtl::OUString SAL_CALL FirstStart::getImplementationName()
throw ( RuntimeException )
{
	return FirstStart::GetImplementationName();
}

sal_Bool SAL_CALL FirstStart::supportsService( const ::rtl::OUString& rServiceName )
throw ( RuntimeException )
{
	sal_Int32 nSize = sizeof( interfaces ) / sizeof( const char *);

	for( sal_Int32 i = 0; i < nSize; i++ )
		if ( rServiceName.equalsAscii( interfaces[i] ))
			return sal_True;
	return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL FirstStart::getSupportedServiceNames()
throw ( RuntimeException )
{
	return FirstStart::GetSupportedServiceNames();
}

// XJob
Any SAL_CALL FirstStart::execute(const Sequence<NamedValue>& args)
throw ( RuntimeException )
{
    static const ::rtl::OUString ARG_LICENSENEEDED( RTL_CONSTASCII_USTRINGPARAM( "LicenseNeedsAcceptance" ) );
    static const ::rtl::OUString ARG_LICENSEPATH(   RTL_CONSTASCII_USTRINGPARAM( "LicensePath" ) );

    ::comphelper::SequenceAsHashMap lArgs(args);

    sal_Bool bLicenseNeeded    = lArgs.getUnpackedValueOrDefault( ARG_LICENSENEEDED, (sal_Bool)sal_True );
    rtl::OUString aLicensePath = lArgs.getUnpackedValueOrDefault( ARG_LICENSEPATH, rtl::OUString() );

    FirstStartWizard fsw( NULL, bLicenseNeeded && ( aLicensePath.getLength() > 0 ), aLicensePath );
    return makeAny( (sal_Bool)fsw.Execute() );
}

// XJobExecutor
void SAL_CALL FirstStart::trigger(const OUString&)
throw ( RuntimeException )
{
    // trigger wizard with override, so it gets started regardless of
    // configuration
    Sequence<NamedValue> seq(1);
    seq[0] = NamedValue(
        OUString::createFromAscii("Override"),
        makeAny(sal_True));
    execute(seq);
}


} // namespace desktop
