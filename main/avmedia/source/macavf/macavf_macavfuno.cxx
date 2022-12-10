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

#include "macavf_manager.hxx"

using namespace ::com::sun::star;

// -------------------
// - factory methods -
// -------------------

static uno::Reference< uno::XInterface > SAL_CALL create_MediaPlayer( const uno::Reference< lang::XMultiServiceFactory >& rxFact )
{
	return uno::Reference< uno::XInterface >( *new ::avmedia::macavf::Manager( rxFact ) );
}

// ------------------------------------------
// - component_getImplementationEnvironment -
// ------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
{
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// ------------------------
// - component_getFactory -
// ------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
{
	uno::Reference< lang::XSingleServiceFactory > xFactory;
	void*									pRet = 0;

	if( rtl_str_compare( pImplName, AVMEDIA_MACAVF_MANAGER_IMPLEMENTATIONNAME ) == 0 )
	{
		const ::rtl::OUString aServiceName( ::rtl::OUString::createFromAscii( AVMEDIA_MACAVF_MANAGER_SERVICENAME ) );

		xFactory = uno::Reference< lang::XSingleServiceFactory >( ::cppu::createSingleFactory(
						reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
						::rtl::OUString::createFromAscii( AVMEDIA_MACAVF_MANAGER_IMPLEMENTATIONNAME ),
						create_MediaPlayer, uno::Sequence< ::rtl::OUString >( &aServiceName, 1 ) ) );
	}

	if( xFactory.is() )
	{
		xFactory->acquire();
		pRet = xFactory.get();
	}

	return pRet;
}
