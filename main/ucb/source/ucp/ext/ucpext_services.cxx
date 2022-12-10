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



#include "precompiled_ext.hxx"

#include "ucpext_provider.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/implementationentry.hxx>

//......................................................................................................................
namespace ucb { namespace ucp { namespace ext
{
//......................................................................................................................

	/** === begin UNO using === **/
	using ::com::sun::star::uno::Reference;
	using ::com::sun::star::uno::XInterface;
	using ::com::sun::star::uno::UNO_QUERY;
	using ::com::sun::star::uno::UNO_QUERY_THROW;
	using ::com::sun::star::uno::UNO_SET_THROW;
	using ::com::sun::star::uno::Exception;
	using ::com::sun::star::uno::RuntimeException;
	using ::com::sun::star::uno::Any;
	using ::com::sun::star::uno::makeAny;
	using ::com::sun::star::uno::Sequence;
	using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XSingleComponentFactory;
	/** === end UNO using === **/

	//==================================================================================================================
    //= descriptors for the services implemented in this component
    //==================================================================================================================
    static struct ::cppu::ImplementationEntry s_aServiceEntries[] =
    {
        {
            ContentProvider::Create,
            ContentProvider::getImplementationName_static,
            ContentProvider::getSupportedServiceNames_static,
            ::cppu::createOneInstanceComponentFactory, NULL, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

//......................................................................................................................
} } }   // namespace ucb::ucp::ext
//......................................................................................................................

extern "C"
{
    //------------------------------------------------------------------------------------------------------------------
    SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    //------------------------------------------------------------------------------------------------------------------
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , ::ucb::ucp::ext::s_aServiceEntries );
    }
}
