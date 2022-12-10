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
#include "precompiled_extensions.hxx"
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <uno/mapping.hxx>
#include "scanner.hxx"

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace com::sun::star::registry;

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment( const sal_Char** ppEnvTypeName, uno_Environment** /*ppEnv*/ )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /*pRegistryKey*/ )
{
	REF( ::com::sun::star::lang::XSingleServiceFactory ) xFactory;
	void*												 pRet = 0;
	
	if( ::rtl::OUString::createFromAscii( pImplName ) == ScannerManager::getImplementationName_Static() )
	{
		xFactory = REF( ::com::sun::star::lang::XSingleServiceFactory )( ::cppu::createSingleFactory(
						static_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ),
						ScannerManager::getImplementationName_Static(),
						ScannerManager_CreateInstance, 
						ScannerManager::getSupportedServiceNames_Static() ) );
	}

	if( xFactory.is() )
	{
		xFactory->acquire();
		pRet = xFactory.get();
	}
	
	return pRet;
}
