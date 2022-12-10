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



#ifndef _DTRANS_GENERIC_CLIPBOARD_HXX_
#define _DTRANS_GENERIC_CLIPBOARD_HXX_

#include <cppuhelper/compbase4.hxx>

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBAORD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>

// ------------------------------------------------------------------------

#define GENERIC_CLIPBOARD_IMPLEMENTATION_NAME "com.sun.star.comp.datatransfer.clipboard.GenericClipboard"

// ------------------------------------------------------------------------

namespace dtrans
{

    class GenericClipboard : public ::cppu::WeakComponentImplHelper4 < \
    ::com::sun::star::datatransfer::clipboard::XClipboardEx, \
    ::com::sun::star::datatransfer::clipboard::XClipboardNotifier, \
    ::com::sun::star::lang::XServiceInfo, \
    ::com::sun::star::lang::XInitialization >
    {
        ::osl::Mutex m_aMutex;
        ::rtl::OUString m_aName;

        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > m_aContents;
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;

	    sal_Bool m_bInitialized;
        virtual ~GenericClipboard();

    public:

	    GenericClipboard();    

	    /*
	     * XInitialization
	     */

	    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
		    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

	    /*
	     * XServiceInfo
	     */

	    virtual ::rtl::OUString SAL_CALL getImplementationName(	 )
		    throw(::com::sun::star::uno::RuntimeException);

	    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
		    throw(::com::sun::star::uno::RuntimeException);

	    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
		    throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboard
         */

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents() 
            throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL setContents( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans, 
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) 
            throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL getName() 
            throw(::com::sun::star::uno::RuntimeException);    

        /*
         * XClipboardEx
         */

        virtual sal_Int8 SAL_CALL getRenderingCapabilities()
            throw(::com::sun::star::uno::RuntimeException);

        /*
         * XClipboardNotifier
         */

        virtual void SAL_CALL addClipboardListener( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) 
            throw(::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL removeClipboardListener( 
            const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener ) 
            throw(::com::sun::star::uno::RuntimeException);  

    };

}

// ------------------------------------------------------------------------

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL GenericClipboard_getSupportedServiceNames();
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL GenericClipboard_createInstance( 
	const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

#endif
