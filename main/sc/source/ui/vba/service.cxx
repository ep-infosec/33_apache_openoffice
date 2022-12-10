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


#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "comphelper/servicedecl.hxx"

// =============================================================================
// component exports
// =============================================================================
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdecl = comphelper::service_decl;

// reference service helper(s)
namespace  range
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace  workbook
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace  worksheet
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace window 
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace globals
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace hyperlink 
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace application 
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace vbaeventshelper
{
extern sdecl::ServiceDecl const serviceDecl;
}
namespace textframe 
{
extern sdecl::ServiceDecl const serviceDecl;
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( 
        const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
    {
		OSL_TRACE("In component_getImplementationEnv");
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( 
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
		OSL_TRACE("In component_getFactory for %s", pImplName );
	void* pRet =  component_getFactoryHelper(
        	pImplName, pServiceManager, pRegistryKey, range::serviceDecl, workbook::serviceDecl, worksheet::serviceDecl, globals::serviceDecl, window::serviceDecl, hyperlink::serviceDecl, application::serviceDecl );
    if( !pRet )
        pRet = component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey, vbaeventshelper::serviceDecl, textframe::serviceDecl );
	OSL_TRACE("Ret is 0x%x", pRet);
	return pRet;
    }
}
