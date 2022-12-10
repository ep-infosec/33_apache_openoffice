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



#ifndef EVENT_TESTLISTENER_HXX
#define EVENT_TESTLISTENER_HXX

#include <sal/types.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XEventListener.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>

#include <cppuhelper/implbase3.hxx>


using ::rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM { namespace events
{

    typedef ::cppu::WeakImplHelper3
        < ::com::sun::star::xml::dom::events::XEventListener
        , ::com::sun::star::lang::XInitialization
        , ::com::sun::star::lang::XServiceInfo
        > CTestListener_Base;

    class  CTestListener
        : public CTestListener_Base
    {

    private:
        Reference< ::com::sun::star::lang::XMultiServiceFactory > m_factory;
        Reference <XEventTarget> m_target;
        OUString m_type;
        sal_Bool m_capture;
        OUString m_name;

    public:

        // static helpers for service info and component management
        static const char* aImplementationName;
	    static const char* aSupportedServiceNames[];
        static OUString _getImplementationName();
        static Sequence< OUString > _getSupportedServiceNames();
        static Reference< XInterface > _getInstance(
            const Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                rSMgr);

        CTestListener(
                const Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                    rSMgr)
            : m_factory(rSMgr){};

        virtual ~CTestListener();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw (RuntimeException);
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
            throw (RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames ()
            throw (RuntimeException);


        // XEventListener
        virtual void SAL_CALL initialize(const Sequence< Any >& args) throw (RuntimeException);

        virtual void SAL_CALL handleEvent(const Reference< XEvent >& evt) throw (RuntimeException);


    };
}}

#endif
