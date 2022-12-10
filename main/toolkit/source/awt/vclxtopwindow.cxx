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
#include "precompiled_toolkit.hxx"
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/SystemDependentXWindow.hpp>

#ifdef WNT
#include <tools/prewin.h>
#include <windows.h>
#include <tools/postwin.h>
#elif defined ( OS2 )
#include <svpm.h>
#elif defined ( QUARTZ )
#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"
#endif

#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

#include <toolkit/awt/vclxtopwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <toolkit/helper/macros.hxx>

#include <vcl/wrkwin.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>

#include <tools/debug.hxx>

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Any;
using ::com::sun::star::lang::IndexOutOfBoundsException;

VCLXTopWindow_Base::VCLXTopWindow_Base( const bool _bSupportSystemWindowPeer )
    :m_bWHWND( _bSupportSystemWindowPeer )
{
}

VCLXTopWindow_Base::~VCLXTopWindow_Base()
{
}

Any VCLXTopWindow_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    ::com::sun::star::uno::Any aRet( VCLXTopWindow_XBase::queryInterface( rType ) );

    // do not expose XSystemDependentWindowPeer if we do not have a system window handle
    if ( !aRet.hasValue() && m_bWHWND )
        aRet = VCLXTopWindow_SBase::queryInterface( rType );

    return aRet;
}

Sequence< Type > VCLXTopWindow_Base::getTypes() throw(RuntimeException)
{
    Sequence< Type > aTypes( VCLXTopWindow_XBase::getTypes() );
    if ( m_bWHWND )
        aTypes = ::comphelper::concatSequences( aTypes, VCLXTopWindow_SBase::getTypes() );
    return aTypes;
}

::com::sun::star::uno::Any VCLXTopWindow_Base::getWindowHandle( const ::com::sun::star::uno::Sequence< sal_Int8 >& /*ProcessId*/, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );

	// TODO, check the process id
	::com::sun::star::uno::Any aRet;
	Window* pWindow = GetWindowImpl();
	if ( pWindow )
	{
		const SystemEnvData* pSysData = ((SystemWindow *)pWindow)->GetSystemData();
		if( pSysData )
		{
#if (defined WNT)
			if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_WIN32 )
			{
				 aRet <<= (sal_Int32)pSysData->hWnd;
			}
#elif (defined OS2)
			if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_OS2 )
			{
				 aRet <<= (sal_Int32)pSysData->hWnd;
			}
#elif (defined QUARTZ)
			if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_MAC )
			{
				 aRet <<= (sal_IntPtr)pSysData->mpNSView;
			}
#elif (defined UNX)
			if( SystemType == ::com::sun::star::lang::SystemDependent::SYSTEM_XWINDOW )
			{
				::com::sun::star::awt::SystemDependentXWindow aSD;
				aSD.DisplayPointer = sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_IntPtr >(pSysData->pDisplay));
				aSD.WindowHandle = pSysData->aWindow;
				aRet <<= aSD;
			}
#endif
		}
	}
	return aRet;
}

void VCLXTopWindow_Base::addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );

	GetTopWindowListenersImpl().addInterface( rxListener );
}

void VCLXTopWindow_Base::removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& rxListener ) throw(::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );

	GetTopWindowListenersImpl().removeInterface( rxListener );
}

void VCLXTopWindow_Base::toFront(  ) throw(::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );

	Window* pWindow = GetWindowImpl();
	if ( pWindow )
		((WorkWindow*)pWindow)->ToTop( TOTOP_RESTOREWHENMIN );
}

void VCLXTopWindow_Base::toBack(  ) throw(::com::sun::star::uno::RuntimeException)
{
#if 0 // Not possible in VCL...

	::vos::OGuard aGuard( GetMutexImpl() );

	Window* pWindow = GetWindowImpl();
	if ( pWindow )
	{
		((WorkWindow*)pWindow)->ToBack();
	}
#endif
}

void VCLXTopWindow_Base::setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& rxMenu ) throw(::com::sun::star::uno::RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );
	
	SystemWindow* pWindow = (SystemWindow*) GetWindowImpl();
	if ( pWindow )
	{
		pWindow->SetMenuBar( NULL );
		if ( rxMenu.is() )
		{
			VCLXMenu* pMenu = VCLXMenu::GetImplementation( rxMenu );
			if ( pMenu && !pMenu->IsPopupMenu() )
				pWindow->SetMenuBar( (MenuBar*) pMenu->GetMenu() );
		}
	}
	mxMenuBar = rxMenu;
}

//--------------------------------------------------------------------
::sal_Bool SAL_CALL VCLXTopWindow_Base::getIsMaximized() throw (RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );
	
	const WorkWindow* pWindow = dynamic_cast< const WorkWindow* >( GetWindowImpl() );
	if ( !pWindow )
        return sal_False;

    return pWindow->IsMaximized();
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTopWindow_Base::setIsMaximized( ::sal_Bool _ismaximized ) throw (RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );
	
	WorkWindow* pWindow = dynamic_cast< WorkWindow* >( GetWindowImpl() );
	if ( !pWindow )
        return;

    pWindow->Maximize( _ismaximized );
}

//--------------------------------------------------------------------
::sal_Bool SAL_CALL VCLXTopWindow_Base::getIsMinimized() throw (RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );
	
	const WorkWindow* pWindow = dynamic_cast< const WorkWindow* >( GetWindowImpl() );
	if ( !pWindow )
        return sal_False;

    return pWindow->IsMinimized();
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTopWindow_Base::setIsMinimized( ::sal_Bool _isMinimized ) throw (RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );
	
	WorkWindow* pWindow = dynamic_cast< WorkWindow* >( GetWindowImpl() );
	if ( !pWindow )
        return;

    _isMinimized ? pWindow->Minimize() : pWindow->Restore();
}

//--------------------------------------------------------------------
::sal_Int32 SAL_CALL VCLXTopWindow_Base::getDisplay() throw (RuntimeException)
{
	::vos::OGuard aGuard( GetMutexImpl() );
	
	const SystemWindow* pWindow = dynamic_cast< const SystemWindow* >( GetWindowImpl() );
	if ( !pWindow )
        return 0;

    return pWindow->GetScreenNumber();
}

//--------------------------------------------------------------------
void SAL_CALL VCLXTopWindow_Base::setDisplay( ::sal_Int32 _display ) throw (RuntimeException, IndexOutOfBoundsException)
{
	::vos::OGuard aGuard( GetMutexImpl() );

    if ( ( _display < 0 ) || ( _display >= (sal_Int32)Application::GetScreenCount() ) )
        throw IndexOutOfBoundsException();
	
	SystemWindow* pWindow = dynamic_cast< SystemWindow* >( GetWindowImpl() );
	if ( !pWindow )
        return;

    pWindow->SetScreenNumber( _display );
}

//	----------------------------------------------------
//	class VCLXTopWindow
//	----------------------------------------------------

void VCLXTopWindow::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    VCLXContainer::ImplGetPropertyIds( rIds );
}

VCLXTopWindow::VCLXTopWindow(bool bWHWND)
    : VCLXTopWindow_Base( bWHWND )
{
}

VCLXTopWindow::~VCLXTopWindow()
{
}

vos::IMutex& VCLXTopWindow::GetMutexImpl()
{
    return VCLXContainer::GetMutex();
}

Window* VCLXTopWindow::GetWindowImpl()
{
    return VCLXContainer::GetWindow();
}

::cppu::OInterfaceContainerHelper& VCLXTopWindow::GetTopWindowListenersImpl()
{
    return GetTopWindowListeners();
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXTopWindow::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet( VCLXTopWindow_Base::queryInterface( rType ) );

    if ( !aRet.hasValue() )
        aRet = VCLXContainer::queryInterface( rType );

    return aRet;
}

::com::sun::star::uno::Sequence< sal_Int8 > VCLXTopWindow::getImplementationId() throw(::com::sun::star::uno::RuntimeException)
{
    static ::cppu::OImplementationId* pId = NULL;
    static ::cppu::OImplementationId* pIdWithHandle = NULL;
    if ( isSystemDependentWindowPeer() )
    {
        if( !pIdWithHandle )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pIdWithHandle )
            {
                static ::cppu::OImplementationId idWithHandle( sal_False );
                pIdWithHandle = &idWithHandle;
            }
        }

        return (*pIdWithHandle).getImplementationId();
    }
    else
    {
        if( !pId )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
            if( !pId )
            {
                static ::cppu::OImplementationId id( sal_False );
                pId = &id;
            }
        }

        return (*pId).getImplementationId();
    }
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > VCLXTopWindow::getTypes() throw(::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences( VCLXTopWindow_Base::getTypes(), VCLXContainer::getTypes() );
}
