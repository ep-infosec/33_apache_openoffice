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



#ifndef FORMULA_MODULE_HELPER_RPT_HXX_
#define FORMULA_MODULE_HELPER_RPT_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <tools/resmgr.hxx>
#include "formula/formuladllapi.h"
//.........................................................................
namespace formula
{
//.........................................................................
    //=========================================================================
    //= OModule
    //=========================================================================
    class OModuleImpl;
    class FORMULA_DLLPUBLIC OModule
    {
	    friend class OModuleClient;

    private:
	    OModule();
		    // not implemented. OModule is a static class

    protected:
	    static ::osl::Mutex	s_aMutex;		/// access safety
	    static sal_Int32	s_nClients;		/// number of registered clients
	    static OModuleImpl*	s_pImpl;		/// impl class. lives as long as at least one client for the module is registered

    public:
	    /// get the vcl res manager of the module
	    static ResMgr*	getResManager();
    protected:
	    /// register a client for the module
	    static void	registerClient();
	    /// revoke a client for the module
	    static void	revokeClient();

    private:
	    /** ensure that the impl class exists
		    @precond m_aMutex is guarded when this method gets called
	    */
	    static void ensureImpl();
    };

    //=========================================================================
    //= OModuleClient
    //=========================================================================
    /** base class for objects which uses any global module-specific ressources 
    */
    class FORMULA_DLLPUBLIC OModuleClient
    {
    public:
	    OModuleClient()		{ OModule::registerClient(); }
	    ~OModuleClient()	{ OModule::revokeClient(); }
    };

    //=========================================================================
    //= ModuleRes
    //=========================================================================
    /** specialized ResId, using the ressource manager provided by the global module
    */
    class FORMULA_DLLPUBLIC ModuleRes : public ::ResId
    {
    public:
	    ModuleRes(sal_uInt16 _nId) : ResId(_nId, *OModule::getResManager()) { }
    };
//.........................................................................
}	// namespace formula
//.........................................................................

#endif // FORMULA_MODULE_HELPER_RPT_HXX_


