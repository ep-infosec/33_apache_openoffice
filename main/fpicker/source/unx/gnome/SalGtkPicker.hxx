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



#ifndef _SALGTKFPICKER_HXX_
#define _SALGTKFPICKER_HXX_

//_____________________________________________________________________________
//	includes of other projects
//_____________________________________________________________________________

#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <com/sun/star/awt/XTopWindowListener.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

//----------------------------------------------------------
// class declaration		
//----------------------------------------------------------

class SalGtkPicker
{
	public:
		SalGtkPicker(const ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>& xServiceMgr);
		virtual ~SalGtkPicker();
	protected:
		osl::Mutex m_rbHelperMtx;
		GtkWidget  *m_pDialog;
	protected:
		virtual void SAL_CALL implsetTitle( const ::rtl::OUString& aTitle ) 
			throw( ::com::sun::star::uno::RuntimeException );

		virtual void SAL_CALL implsetDisplayDirectory( const rtl::OUString& rDirectory )
			throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

		virtual rtl::OUString SAL_CALL implgetDisplayDirectory(  )
			throw( com::sun::star::uno::RuntimeException );
		static rtl::OUString uritounicode(const gchar *pIn);
		static rtl::OString unicodetouri(const rtl::OUString &rURL);
};

class GdkThreadLock
{
public:
    GdkThreadLock() { gdk_threads_enter(); }
    ~GdkThreadLock() { gdk_threads_leave(); }
};

//Run the Gtk Dialog. Watch for any "new windows" created while we're
//executing and consider that a CANCEL event to avoid e.g. "file cannot be opened"
//modal dialogs and this one getting locked if some other API call causes this
//to happen while we're opened waiting for user input, e.g. 
//https://bugzilla.redhat.com/show_bug.cgi?id=441108
class RunDialog :
    public cppu::WeakComponentImplHelper1< ::com::sun::star::awt::XTopWindowListener >
{
private:
    osl::Mutex maLock;
    GtkWidget *mpDialog;
    GdkWindow *mpCreatedParent;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XExtendedToolkit>  mxToolkit;
public:

    // XTopWindowListener
    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& )
        throw(::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowOpened( const ::com::sun::star::lang::EventObject& e )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowClosing( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowClosed( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowMinimized( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowNormalized( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowActivated( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
    virtual void SAL_CALL windowDeactivated( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException) {}
public:
    RunDialog(GtkWidget *pDialog, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XExtendedToolkit > &rToolkit);
    gint run();
    void cancel();
    ~RunDialog();
};

#endif
