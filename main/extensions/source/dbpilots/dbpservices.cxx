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
#include "componentmodule.hxx"

//---------------------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL createRegistryInfo_OGroupBoxWizard();
extern "C" void SAL_CALL createRegistryInfo_OListComboWizard();
extern "C" void SAL_CALL createRegistryInfo_OGridWizard();

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL dbp_initializeModule()
{
	static sal_Bool s_bInit = sal_False;
	if (!s_bInit)
	{
		createRegistryInfo_OGroupBoxWizard();
		createRegistryInfo_OListComboWizard();
		createRegistryInfo_OGridWizard();
		::dbp::OModule::setResourceFilePrefix("dbp");
		s_bInit = sal_True;
	}
}

//---------------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
				const sal_Char	**ppEnvTypeName,
				uno_Environment	** /*ppEnv*/
			)
{
	dbp_initializeModule();
	*ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(
					const sal_Char* pImplementationName,
					void* pServiceManager,
					void* /*pRegistryKey*/)
{
	Reference< XInterface > xRet;
	if (pServiceManager && pImplementationName)
	{
		xRet = ::dbp::OModule::getComponentFactory(
			::rtl::OUString::createFromAscii(pImplementationName),
			static_cast< XMultiServiceFactory* >(pServiceManager));
	}

	if (xRet.is())
		xRet->acquire();
	return xRet.get();
};


