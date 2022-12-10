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
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIELEMENT_EDITTOOLBARCONTROLLER_HXX
#include "uielement/edittoolbarcontroller.hxx"
#endif

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_TOOLBAR_HXX_
#include "uielement/toolbar.hxx"
#endif

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/status/ItemStatus.hpp>
#include <com/sun/star/frame/status/ItemState.hpp>
#include <com/sun/star/frame/status/Visibility.hpp>
#include <com/sun/star/frame/XControlNotificationListener.hpp>

//_________________________________________________________________________________________________________________
//	other includes
//_________________________________________________________________________________________________________________
#include <svtools/toolboxcontroller.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace framework
{

// ------------------------------------------------------------------

// Wrapper class to notify controller about events from edit.
// Unfortunaltly the events are notifed through virtual methods instead
// of Listeners.

class EditControl : public Edit
{
    public:
        EditControl( Window* pParent, WinBits nStyle, IEditListener* pEditListener );
        virtual ~EditControl();

        virtual void Modify();
        virtual void KeyInput( const ::KeyEvent& rKEvt );
	    virtual void GetFocus();
	    virtual void LoseFocus();
        virtual long PreNotify( NotifyEvent& rNEvt );

    private:
        IEditListener* m_pEditListener;
};

EditControl::EditControl( Window* pParent, WinBits nStyle, IEditListener* pEditListener ) :
    Edit( pParent, nStyle )
    , m_pEditListener( pEditListener )
{
}

EditControl::~EditControl()
{
    m_pEditListener = 0;
}

void EditControl::Modify()
{
    Edit::Modify();
    if ( m_pEditListener )
        m_pEditListener->Modify();
}

void EditControl::KeyInput( const ::KeyEvent& rKEvt )
{
    Edit::KeyInput( rKEvt );
    if ( m_pEditListener )
        m_pEditListener->KeyInput( rKEvt );
}

void EditControl::GetFocus()
{
    Edit::GetFocus();
    if ( m_pEditListener )
        m_pEditListener->GetFocus();
}

void EditControl::LoseFocus()
{
    Edit::LoseFocus();
    if ( m_pEditListener )
        m_pEditListener->LoseFocus();
}

long EditControl::PreNotify( NotifyEvent& rNEvt )
{
    long nRet( 0 );
    if ( m_pEditListener )
        nRet = m_pEditListener->PreNotify( rNEvt );
    if ( nRet == 0 )
        nRet = Edit::PreNotify( rNEvt );

    return nRet;
}

// ------------------------------------------------------------------

EditToolbarController::EditToolbarController(
    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >&               rFrame,
    ToolBox*                                 pToolbar,
    sal_uInt16                                   nID,
    sal_Int32                                nWidth,
    const ::rtl::OUString&                          aCommand ) :
    ComplexToolbarController( rServiceManager, rFrame, pToolbar, nID, aCommand )
    ,   m_pEditControl( 0 )
{
    m_pEditControl = new EditControl( m_pToolbar, WB_BORDER, this );
    if ( nWidth == 0 )
        nWidth = 100;

    // Calculate height of the edit field according to the application font height
    sal_Int32 nHeight = getFontSizePixel( m_pEditControl ) + 6 + 1;

    m_pEditControl->SetSizePixel( ::Size( nWidth, nHeight ));
    m_pToolbar->SetItemWindow( m_nID, m_pEditControl );
}

// ------------------------------------------------------------------

EditToolbarController::~EditToolbarController()
{
}

// ------------------------------------------------------------------

void SAL_CALL EditToolbarController::dispose()
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    m_pToolbar->SetItemWindow( m_nID, 0 );
    delete m_pEditControl;

    ComplexToolbarController::dispose();

    m_pEditControl = 0;
}

// ------------------------------------------------------------------
Sequence<PropertyValue> EditToolbarController::getExecuteArgs(sal_Int16 KeyModifier) const
{
    Sequence<PropertyValue> aArgs( 2 );
    ::rtl::OUString aSelectedText = m_pEditControl->GetText();

    // Add key modifier to argument list
    aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
    aArgs[0].Value <<= KeyModifier;
    aArgs[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ));
    aArgs[1].Value <<= aSelectedText;
    return aArgs;
}

// ------------------------------------------------------------------

void EditToolbarController::Modify()
{
    notifyTextChanged( m_pEditControl->GetText() );
}

void EditToolbarController::KeyInput( const ::KeyEvent& /*rKEvt*/ )
{
}

void EditToolbarController::GetFocus()
{
    notifyFocusGet();
}

void EditToolbarController::LoseFocus()
{
    notifyFocusLost();
}

long EditToolbarController::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const ::KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
        const KeyCode& rKeyCode = pKeyEvent->GetKeyCode();
        if(( rKeyCode.GetModifier() | rKeyCode.GetCode()) == KEY_RETURN )
        {
            // Call execute only with non-empty text
            if ( m_pEditControl->GetText().Len() > 0 )
                execute( rKeyCode.GetModifier() );
            return 1;
        }
    }

    return 0;
}

// --------------------------------------------------------

void EditToolbarController::executeControlCommand( const ::com::sun::star::frame::ControlCommand& rControlCommand )
{
    if ( rControlCommand.Command.equalsAsciiL( "SetText", 7 ))
    {
        for ( sal_Int32 i = 0; i < rControlCommand.Arguments.getLength(); i++ )
        {
            if ( rControlCommand.Arguments[i].Name.equalsAsciiL( "Text", 4 ))
            {
                rtl::OUString aText;
                rControlCommand.Arguments[i].Value >>= aText;
                m_pEditControl->SetText( aText );

                // send notification
                notifyTextChanged( aText );
                break;
            }
        }
    }
}

} // namespace

