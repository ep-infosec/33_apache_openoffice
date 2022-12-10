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


#ifndef _ACCHEADERFOOTER_HXX
#define _ACCHEADERFOOTER_HXX
#ifndef _ACCCONTEXT_HXX
#include "acccontext.hxx"
#endif

class SwHeaderFrm;
class SwFooterFrm;

class SwAccessibleHeaderFooter : public	SwAccessibleContext
{

protected:

	virtual ~SwAccessibleHeaderFooter();

public:

    SwAccessibleHeaderFooter( SwAccessibleMap* pInitMap,
                              const SwHeaderFrm* pHdFrm );
    SwAccessibleHeaderFooter( SwAccessibleMap* pInitMap,
                              const SwFooterFrm* pFtFrm );


	//=====  XAccessibleContext  ==============================================

    ///	Return this object's description.
	virtual ::rtl::OUString SAL_CALL
    	getAccessibleDescription (void)
        throw (com::sun::star::uno::RuntimeException);

	//=====  XServiceInfo  ====================================================

    /**	Returns an identifier for the implementation of this object.
    */
	virtual ::rtl::OUString SAL_CALL
    	getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);

    /**	Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
    	supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.  In this case that is just
    	the AccessibleContext service.
    */
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    	getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

	//=====  XTypeProvider  ====================================================
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
	//=====  XAccessibleComponent  ============================================
	sal_Int32 SAL_CALL getBackground() 
		throw (::com::sun::star::uno::RuntimeException);
};


#endif

