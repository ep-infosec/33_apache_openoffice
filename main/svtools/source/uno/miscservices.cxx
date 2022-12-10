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
#include "precompiled_svtools.hxx"
#include "sal/types.h"
#include "rtl/ustring.hxx"
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>
#include <uno/mapping.hxx>
#include "provider.hxx"
#include "renderer.hxx"
#include "unowizard.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include "comphelper/servicedecl.hxx"

#include "cppuhelper/implementationentry.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::lang;
using namespace unographic;

using rtl::OUString;

namespace sdecl = comphelper::service_decl;

namespace unographic {
extern sdecl::ServiceDecl const serviceDecl;
}

// -------------------------------------------------------------------------------------

// for CreateInstance functions implemented elsewhere
#define DECLARE_CREATEINSTANCE( ImplName ) \
	Reference< XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< XMultiServiceFactory >& );

// for CreateInstance functions implemented elsewhere, while the function is within a namespace
#define DECLARE_CREATEINSTANCE_NAMESPACE( nmspe, ImplName ) \
	namespace nmspe {	\
		Reference< XInterface > SAL_CALL ImplName##_CreateInstance( const Reference< XMultiServiceFactory >& );	\
	}

namespace
{
    static struct ::cppu::ImplementationEntry s_aServiceEntries[] =
    {
        {
            ::svt::uno::Wizard::Create,
            ::svt::uno::Wizard::getImplementationName_static,
            ::svt::uno::Wizard::getSupportedServiceNames_static,
            ::cppu::createSingleComponentFactory, NULL, 0 
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

// -------------------------------------------------------------------------------------

DECLARE_CREATEINSTANCE_NAMESPACE( svt, OAddressBookSourceDialogUno )
DECLARE_CREATEINSTANCE( SvFilterOptionsDialog )
DECLARE_CREATEINSTANCE_NAMESPACE( unographic, GraphicProvider )
DECLARE_CREATEINSTANCE_NAMESPACE( unographic, GraphicRendererVCL )

// -------------------------------------------------------------------------------------
extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
	const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */)
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
	const sal_Char * pImplementationName, void * _pServiceManager, void * pRegistryKey)
{
	void * pResult = 0;
	if ( _pServiceManager )
	{
		Reference< XSingleServiceFactory > xFactory;
		if (rtl_str_compare (
				pImplementationName, "com.sun.star.comp.svtools.OAddressBookSourceDialogUno") == 0)
		{
			Sequence< OUString > aServiceNames(1);
			aServiceNames.getArray()[0] =
				OUString::createFromAscii( "com.sun.star.ui.AddressBookSourceDialog" );

            xFactory = ::cppu::createSingleFactory (
				reinterpret_cast< XMultiServiceFactory* >( _pServiceManager ),
				OUString::createFromAscii( pImplementationName ),
				svt::OAddressBookSourceDialogUno_CreateInstance,
				aServiceNames);
		}
		else if (rtl_str_compare (
					 pImplementationName, "com.sun.star.svtools.SvFilterOptionsDialog") == 0)
		{
			Sequence< OUString > aServiceNames(1);
			aServiceNames.getArray()[0] =
				OUString::createFromAscii( "com.sun.star.ui.dialogs.FilterOptionsDialog" );

            xFactory = ::cppu::createSingleFactory (
				reinterpret_cast< XMultiServiceFactory* >( _pServiceManager ),
				OUString::createFromAscii( pImplementationName ),
				SvFilterOptionsDialog_CreateInstance,
				aServiceNames);
		}
		else if( 0 == GraphicProvider::getImplementationName_Static().compareToAscii( pImplementationName ) )
		{
			xFactory =  ::cppu::createOneInstanceFactory( 
				reinterpret_cast< lang::XMultiServiceFactory * >( _pServiceManager ),
				GraphicProvider::getImplementationName_Static(),
				GraphicProvider_CreateInstance,
				GraphicProvider::getSupportedServiceNames_Static() );
		}
		else if( 0 == GraphicRendererVCL::getImplementationName_Static().compareToAscii( pImplementationName ) )
		{
			xFactory = ::cppu::createOneInstanceFactory( 
				reinterpret_cast< lang::XMultiServiceFactory * >( _pServiceManager ),
				GraphicRendererVCL::getImplementationName_Static(),
				GraphicRendererVCL_CreateInstance,
				GraphicRendererVCL::getSupportedServiceNames_Static() );
		}
		else 
		{
		    pResult =  component_getFactoryHelper( pImplementationName, reinterpret_cast< lang::XMultiServiceFactory * >( _pServiceManager ),reinterpret_cast< registry::XRegistryKey* >( pRegistryKey ), serviceDecl );
            if ( !pResult )
                pResult = ::cppu::component_getFactoryHelper( pImplementationName, _pServiceManager, pRegistryKey, s_aServiceEntries );
		}

		if ( xFactory.is() )
		{
			xFactory->acquire();
			pResult = xFactory.get();
		}
	}
	return pResult;
}

}	// "C"

