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


#ifndef __SHUTDOWNICON_HXX__
#define __SHUTDOWNICON_HXX__

#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#ifndef _RTL_STRING_HXX
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#include <osl/mutex.hxx>
#include <osl/module.hxx>
#include <sfx2/sfxuno.hxx>
#include <cppuhelper/compbase4.hxx>
#include <sfx2/dllapi.h>
#include <tools/link.hxx>

struct AsyncDesktopTerminationData;
class ResMgr;
namespace sfx2
{
    class FileDialogHelper;
}

typedef ::cppu::WeakComponentImplHelper4<
	::com::sun::star::lang::XInitialization,
	::com::sun::star::frame::XTerminateListener,
	::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XFastPropertySet > ShutdownIconServiceBase;

#if defined(USE_APP_SHORTCUTS)
#define WRITER_URL      "private:factory/swriter"
#define CALC_URL        "private:factory/scalc"
#define IMPRESS_URL     "private:factory/simpress"
#define IMPRESS_WIZARD_URL     "private:factory/simpress?slot=6686"
#define DRAW_URL        "private:factory/sdraw"
#define MATH_URL        "private:factory/smath"
#define BASE_URL        "private:factory/sdatabase?Interactive"
#define STARTMODULE_URL ".uno:ShowStartModule"
#endif

class SFX2_DLLPUBLIC ShutdownIcon :	public ShutdownIconServiceBase
{
        ::osl::Mutex            m_aMutex;
        bool                    m_bVeto;
        bool                    m_bListenForTermination;
        bool                    m_bSystemDialogs;
        ResMgr*                 m_pResMgr;
        sfx2::FileDialogHelper* m_pFileDlg;
		::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;

        static ShutdownIcon *pShutdownIcon; // one instance

		oslGenericFunction m_pInitSystray;
		oslGenericFunction m_pDeInitSystray;
		::osl::Module  *m_pPlugin;

		bool m_bInitialized;
		void initSystray();
		void deInitSystray();

		static bool LoadModule( osl::Module **pModule,
								oslGenericFunction *pInit,
								oslGenericFunction *pDeInit );
		static void EnterModalMode();
		static void LeaveModalMode();
		static rtl::OUString getShortcutName();

		friend class SfxNotificationListener_Impl;

	public:
		ShutdownIcon( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > aSMgr );

		virtual ~ShutdownIcon();

        DECL_STATIC_LINK( ShutdownIcon, AsyncDesktopTermination, AsyncDesktopTerminationData* );

        SFX_DECL_XSERVICEINFO

		static ShutdownIcon* getInstance();
		static ShutdownIcon* createInstance();

        static void terminateDesktop();
		static void addTerminateListener();

        static void FileOpen();
        static void OpenURL( const ::rtl::OUString& aURL, const ::rtl::OUString& rTarget, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& =
			::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >( 0 ) );
        static void FromTemplate();

        static void SetAutostart( bool bActivate );
        static bool GetAutostart();
		static bool bModalMode;

		void init() throw( ::com::sun::star::uno::Exception );

		static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >
					GetWrapperFactory( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xSMgr );
		static ::rtl::OUString  GetImplementationName_static();

        ::rtl::OUString GetResString( int id );
        ::rtl::OUString GetUrlDescription( const ::rtl::OUString& aUrl );

        void SetVeto( bool bVeto )  { m_bVeto = bVeto;}
        bool GetVeto()              { return m_bVeto; }

        void                    StartFileDialog();
        sfx2::FileDialogHelper* GetFileDialog() const { return m_pFileDlg; }
        static long DialogClosedHdl_Impl( ShutdownIcon*, sfx2::FileDialogHelper* );

        static bool IsQuickstarterInstalled();

		// Component Helper - force override
		virtual void SAL_CALL disposing();

		// XEventListener
		virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
			throw(::com::sun::star::uno::RuntimeException);

		// XTerminateListener
		virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
			throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
		virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
			throw(::com::sun::star::uno::RuntimeException);

		// XInitialization
		virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
			throw( ::com::sun::star::uno::Exception );

        // XFastPropertySet
        virtual void SAL_CALL setFastPropertyValue(       ::sal_Int32                  nHandle,
                                                    const ::com::sun::star::uno::Any& aValue )
            throw (::com::sun::star::beans::UnknownPropertyException,
                    ::com::sun::star::beans::PropertyVetoException,
                    ::com::sun::star::lang::IllegalArgumentException,
                    ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue( ::sal_Int32 nHandle )
            throw (::com::sun::star::beans::UnknownPropertyException,
                    ::com::sun::star::lang::WrappedTargetException,
                    ::com::sun::star::uno::RuntimeException);

		::com::sun::star::uno::Reference< ::com::sun::star::frame::XDesktop > m_xDesktop;

#ifdef WNT
		static void EnableAutostartW32( const rtl::OUString &aShortcutName );
		static rtl::OUString GetAutostartFolderNameW32();
#endif
#ifdef OS2
	        static void SetAutostartOs2( bool bActivate );
	        static bool GetAutostartOs2( );
#endif
};

extern "C" {
#  ifdef WNT
	// builtin win32 systray
	void win32_init_sys_tray();
	void win32_shutdown_sys_tray();
#  elif defined QUARTZ
    void aqua_init_systray();
    void aqua_shutdown_systray();
#  endif
	// external plugin systray impl.
	void plugin_init_sys_tray();
	void plugin_shutdown_sys_tray();
}

#endif
