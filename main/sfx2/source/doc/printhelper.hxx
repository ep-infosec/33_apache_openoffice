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



#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"

#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/view/XPrintJobBroadcaster.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase3.hxx>

struct  IMPL_PrintListener_DataContainer;
class SfxViewShell;
class SfxPrinter;

class SfxPrintHelper : public cppu::WeakImplHelper3
        < com::sun::star::view::XPrintable
        , com::sun::star::view::XPrintJobBroadcaster
        , com::sun::star::lang::XInitialization >
{
public:

    SfxPrintHelper() ;
    virtual ~SfxPrintHelper() ;

    void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > SAL_CALL getPrinter() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPrinter( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& seqPrinter )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL print( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& seqOptions )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

private:

    osl::Mutex m_aMutex;
    IMPL_PrintListener_DataContainer* m_pData ;
    virtual void impl_setPrinter(const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& rPrinter,SfxPrinter*& pPrinter,sal_uInt16& nChangeFlags,SfxViewShell*& pViewSh);
} ;
