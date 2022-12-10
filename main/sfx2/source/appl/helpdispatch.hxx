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


#ifndef SFX_HELPDISPATCH_HXX
#define SFX_HELPDISPATCH_HXX

#include <com/sun/star/frame/XDispatch.hpp>
#include <cppuhelper/implbase1.hxx>

#include "helpinterceptor.hxx"

class HelpDispatch_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XDispatch >
{
private:
	HelpInterceptor_Impl&		m_rInterceptor;
	::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >
								m_xRealDispatch;

public:
	HelpDispatch_Impl( HelpInterceptor_Impl& _rInterceptor,
					   const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >& _xDisp );
	~HelpDispatch_Impl();

    // XDispatch
    virtual void SAL_CALL 	dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL 	addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL 	removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl, const ::com::sun::star::util::URL& aURL ) throw(::com::sun::star::uno::RuntimeException);
};

#endif // #ifndef SFX_HELPDISPATCHER_HXX

