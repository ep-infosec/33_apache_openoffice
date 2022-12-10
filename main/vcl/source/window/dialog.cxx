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
#include "precompiled_vcl.hxx"
#include <tools/debug.hxx>

#include <tools/rc.h>

#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>

#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/dialog.hxx>
#include <vcl/decoview.hxx>

#ifdef DBG_UTIL
#include <vcl/msgbox.hxx>
#endif

#include <vcl/unowrap.hxx>



// =======================================================================

#ifdef DBG_UTIL

static ByteString ImplGetDialogText( Dialog* pDialog )
{
    ByteString aErrorStr( pDialog->GetText(), RTL_TEXTENCODING_UTF8 );
    if ( (pDialog->GetType() == WINDOW_MESSBOX) ||
         (pDialog->GetType() == WINDOW_INFOBOX) ||
         (pDialog->GetType() == WINDOW_WARNINGBOX) ||
         (pDialog->GetType() == WINDOW_ERRORBOX) ||
         (pDialog->GetType() == WINDOW_QUERYBOX) )
    {
        aErrorStr += ", ";
        aErrorStr += ByteString( ((MessBox*)pDialog)->GetMessText(), RTL_TEXTENCODING_UTF8 );
    }
    return aErrorStr;
}

#endif

// =======================================================================

static sal_Bool ImplIsMnemonicCtrl( Window* pWindow )
{
    if( ! pWindow->GetSettings().GetStyleSettings().GetAutoMnemonic() )
        return sal_False;

    if ( (pWindow->GetType() == WINDOW_RADIOBUTTON) ||
         (pWindow->GetType() == WINDOW_CHECKBOX) ||
         (pWindow->GetType() == WINDOW_TRISTATEBOX) ||
         (pWindow->GetType() == WINDOW_PUSHBUTTON) )
        return sal_True;

    if ( pWindow->GetType() == WINDOW_FIXEDTEXT )
    {
        if ( pWindow->GetStyle() & (WB_INFO | WB_NOLABEL) )
            return sal_False;
        Window* pNextWindow = pWindow->GetWindow( WINDOW_NEXT );
        if ( !pNextWindow )
            return sal_False;
        pNextWindow = pNextWindow->GetWindow( WINDOW_CLIENT );
        if ( !(pNextWindow->GetStyle() & WB_TABSTOP) ||
             (pNextWindow->GetType() == WINDOW_FIXEDTEXT) ||
             (pNextWindow->GetType() == WINDOW_GROUPBOX) ||
             (pNextWindow->GetType() == WINDOW_RADIOBUTTON) ||
             (pNextWindow->GetType() == WINDOW_CHECKBOX) ||
             (pNextWindow->GetType() == WINDOW_TRISTATEBOX) ||
             (pNextWindow->GetType() == WINDOW_PUSHBUTTON) )
            return sal_False;

        return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void ImplWindowAutoMnemonic( Window* pWindow )
{
    MnemonicGenerator   aMnemonicGenerator;
    Window*                 pGetChild;
    Window*                 pChild;

    // Die schon vergebenen Mnemonics registieren
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
    }

    // Bei TabPages auch noch die Controls vom Dialog beruecksichtigen
    if ( pWindow->GetType() == WINDOW_TABPAGE )
    {
        Window* pParent = pWindow->GetParent();
        if ( pParent->GetType() == WINDOW_TABCONTROL )
            pParent = pParent->GetParent();

        if ( (pParent->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
        {
            pGetChild = pParent->GetWindow( WINDOW_FIRSTCHILD );
            while ( pGetChild )
            {
                pChild = pGetChild->ImplGetWindow();
                aMnemonicGenerator.RegisterMnemonic( pChild->GetText() );
                pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
            }
        }
    }

    // Die Mnemonics an die Controls vergeben, die noch keinen haben
    pGetChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while ( pGetChild )
    {
        pChild = pGetChild->ImplGetWindow();
        if ( ImplIsMnemonicCtrl( pChild ) )
        {
            XubString aText = pChild->GetText();
            if ( aMnemonicGenerator.CreateMnemonic( aText ) )
                pChild->SetText( aText );
        }

        pGetChild = pGetChild->GetWindow( WINDOW_NEXT );
    }
}

// =======================================================================

static PushButton* ImplGetDefaultButton( Dialog* pDialog )
{
    Window* pChild = pDialog->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->ImplIsPushButton() )
        {
            PushButton* pPushButton = (PushButton*)pChild;
            if ( pPushButton->ImplIsDefButton() )
                return pPushButton;
        }

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static PushButton* ImplGetOKButton( Dialog* pDialog )
{
    Window* pChild = pDialog->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_OKBUTTON )
            return (PushButton*)pChild;

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static PushButton* ImplGetCancelButton( Dialog* pDialog )
{
    Window* pChild = pDialog->GetWindow( WINDOW_FIRSTCHILD );
    while ( pChild )
    {
        if ( pChild->GetType() == WINDOW_CANCELBUTTON )
            return (PushButton*)pChild;

        pChild = pChild->GetWindow( WINDOW_NEXT );
    }

    return NULL;
}

// -----------------------------------------------------------------------

static void ImplMouseAutoPos( Dialog* pDialog )
{
    sal_uLong nMouseOptions = pDialog->GetSettings().GetMouseSettings().GetOptions();
    if ( nMouseOptions & MOUSE_OPTION_AUTOCENTERPOS )
    {
        Size aSize = pDialog->GetOutputSizePixel();
        pDialog->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
    else if ( nMouseOptions & MOUSE_OPTION_AUTODEFBTNPOS )
    {
        Window* pWindow = ImplGetDefaultButton( pDialog );
        if ( !pWindow )
            pWindow = ImplGetOKButton( pDialog );
        if ( !pWindow )
            pWindow = ImplGetCancelButton( pDialog );
        if ( !pWindow )
            pWindow = pDialog;
        Size aSize = pWindow->GetOutputSizePixel();
        pWindow->SetPointerPosPixel( Point( aSize.Width()/2, aSize.Height()/2 ) );
    }
}

// =======================================================================

struct DialogImpl
{
    long    mnResult;
    bool    mbStartedModal;
    Link    maEndDialogHdl;

    DialogImpl() : mnResult( -1 ), mbStartedModal( false ) {}
};

// =======================================================================

void Dialog::ImplInitDialogData()
{
    mpWindowImpl->mbDialog  = sal_True;
    mpDialogParent          = NULL;
    mpPrevExecuteDlg        = NULL;
    mbInExecute             = sal_False;
    mbOldSaveBack           = sal_False;
    mbInClose               = sal_False;
    mbModalMode             = sal_False;
    mnMousePositioned       = 0;
    mpDialogImpl            = new DialogImpl;
}

// -----------------------------------------------------------------------

void Dialog::ImplInit( Window* pParent, WinBits nStyle )
{
    sal_uInt16 nSysWinMode = Application::GetSystemWindowMode();

    if ( !(nStyle & WB_NODIALOGCONTROL) )
        nStyle |= WB_DIALOGCONTROL;
    nStyle |= WB_ROLLABLE;

    // Now, all Dialogs are per default system windows !!!
    nStyle |= WB_SYSTEMWINDOW;


    // parent is NULL: get the default Dialog parent
    if ( !pParent )
    {
        pParent = Application::GetDefDialogParent();
        if ( !pParent && !(nStyle & WB_SYSTEMWINDOW) )
            pParent = ImplGetSVData()->maWinData.mpAppWin;

        // If Parent is disabled, then we search for a modal dialog
        // in this frame
        if ( pParent && (!pParent->IsInputEnabled() || pParent->IsInModalMode()) )
        {
            ImplSVData* pSVData = ImplGetSVData();
            Dialog*     pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
            while ( pExeDlg )
            {
                // Nur wenn er sichtbar und enabled ist
                if ( pParent->ImplGetFirstOverlapWindow()->IsWindowOrChild( pExeDlg, sal_True ) &&
                     pExeDlg->IsReallyVisible() &&
                     pExeDlg->IsEnabled() && pExeDlg->IsInputEnabled() && !pExeDlg->IsInModalMode() )
                {
                    pParent = pExeDlg;
                    break;
                }

                pExeDlg = pExeDlg->mpPrevExecuteDlg;
            }
        }
    }
    // DIALOG_NO_PARENT: explicitly don't have a parent for this Dialog
    else if( pParent == DIALOG_NO_PARENT )
        pParent = NULL;

/*
    // Now, all Dialogs are per default system windows !!!
    if ( pParent && !(nSysWinMode & SYSTEMWINDOW_MODE_NOAUTOMODE) )
    {
        if ( !pParent->mpWindowImpl->mpFrameWindow->IsVisible() )
            pParent = NULL;
        else
        {
            if ( pParent->mpWindowImpl->mpFrameWindow->IsDialog() )
            {
                Size aOutSize = pParent->mpWindowImpl->mpFrameWindow->GetOutputSizePixel();
                if ( (aOutSize.Width() < 210) ||(aOutSize.Height() < 160) )
                    nStyle |= WB_SYSTEMWINDOW;
            }
        }
    }
*/

    if ( !pParent || (nStyle & WB_SYSTEMWINDOW) ||
         (pParent->mpWindowImpl->mpFrameData->mbNeedSysWindow && !(nSysWinMode & SYSTEMWINDOW_MODE_NOAUTOMODE)) ||
         (nSysWinMode & SYSTEMWINDOW_MODE_DIALOG) )
    {
        // create window with a small border ?
        if ( (nStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE)) == WB_BORDER )
        {
            ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_FRAME );
            SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
            pBorderWin->mpWindowImpl->mpClientWindow = this;
            pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
            mpWindowImpl->mpBorderWindow  = pBorderWin;
            mpWindowImpl->mpRealParent    = pParent;
        }
        else
        {
            mpWindowImpl->mbFrame         = sal_True;
            mpWindowImpl->mbOverlapWin    = sal_True;
            SystemWindow::ImplInit( pParent, (nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_CLOSEABLE | WB_STANDALONE)) | WB_CLOSEABLE, NULL );
            // Now set all style bits
            mpWindowImpl->mnStyle = nStyle;
        }
    }
    else
    {
        ImplBorderWindow* pBorderWin  = new ImplBorderWindow( pParent, nStyle, BORDERWINDOW_STYLE_OVERLAP | BORDERWINDOW_STYLE_BORDER );
        SystemWindow::ImplInit( pBorderWin, nStyle & ~WB_BORDER, NULL );
        pBorderWin->mpWindowImpl->mpClientWindow = this;
        pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
        mpWindowImpl->mpBorderWindow  = pBorderWin;
        mpWindowImpl->mpRealParent    = pParent;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );

    ImplInitSettings();
}

// -----------------------------------------------------------------------

void Dialog::ImplInitSettings()
{
    // user override
    if ( IsControlBackground() )
        SetBackground( GetControlBackground() );
    // NWF background
    else if( IsNativeControlSupported( CTRL_WINDOW_BACKGROUND, PART_BACKGROUND_DIALOG ) )
    {
        mpWindowImpl->mnNativeBackground = PART_BACKGROUND_DIALOG;
        EnableChildTransparentMode( sal_True );
    }
    // fallback to settings color
    else
        SetBackground( GetSettings().GetStyleSettings().GetDialogColor() );

}

// -----------------------------------------------------------------------

void Dialog::ImplCenterDialog()
{
    Rectangle   aDeskRect = ImplGetFrameWindow()->GetDesktopRectPixel();
    Point       aDeskPos = aDeskRect.TopLeft();
    Size        aDeskSize = aDeskRect.GetSize();
    Size        aWinSize = GetSizePixel();
    Window *pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;
    Point       aWinPos( ((aDeskSize.Width() - aWinSize.Width()) / 2) + aDeskPos.X(),
                         ((aDeskSize.Height() - aWinSize.Height()) / 2) + aDeskPos.Y() );

    // Pruefen, ob Dialogbox ausserhalb des Desks liegt
    if ( (aWinPos.X() + aWinSize.Width()) > (aDeskPos.X()+aDeskSize.Width()) )
        aWinPos.X() = aDeskPos.X()+aDeskSize.Width() - aWinSize.Width();
    if ( (aWinPos.Y()+aWinSize.Height()) > (aDeskPos.Y()+aDeskSize.Height()) )
        aWinPos.Y() = aDeskPos.Y()+aDeskSize.Height() - aWinSize.Height();
    // Linke Ecke bevorzugen, da Titelbar oben ist
    if ( aWinPos.X() < aDeskPos.X() )
        aWinPos.X() = aDeskPos.X();
    if ( aWinPos.Y() < aDeskPos.Y() )
        aWinPos.Y() = aDeskPos.Y();

    //SetPosPixel( aWinPos );
    SetPosPixel( pWindow->ScreenToOutputPixel( aWinPos ) );
}

// -----------------------------------------------------------------------

Dialog::Dialog( WindowType nType ) :
    SystemWindow( nType )
{
    ImplInitDialogData();
}

// -----------------------------------------------------------------------

Dialog::Dialog( Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_DIALOG )
{
    ImplInitDialogData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Dialog::Dialog( Window* pParent, const ResId& rResId ) :
    SystemWindow( WINDOW_DIALOG )
{
    ImplInitDialogData();
    rResId.SetRT( RSC_DIALOG );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

Dialog::~Dialog()
{
    delete mpDialogImpl;
    mpDialogImpl = NULL;
}

// -----------------------------------------------------------------------

IMPL_LINK( Dialog, ImplAsyncCloseHdl, void*, EMPTYARG )
{
    Close();
    return 0;
}

// -----------------------------------------------------------------------

long Dialog::Notify( NotifyEvent& rNEvt )
{
    // Zuerst Basisklasse rufen wegen TabSteuerung
    long nRet = SystemWindow::Notify( rNEvt );
    if ( !nRet )
    {
        if ( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            KeyCode         aKeyCode = pKEvt->GetKeyCode();
            sal_uInt16          nKeyCode = aKeyCode.GetCode();

            if ( (nKeyCode == KEY_ESCAPE) &&
                 ((GetStyle() & WB_CLOSEABLE) || ImplGetCancelButton( this ) || ImplGetOKButton( this )) )
            {
                // #i89505# for the benefit of slightly mentally challenged implementations
                // like e.g. SfxModelessDialog which destroy themselves inside Close()
                // post this Close asynchronous so we can leave our key handler before
                // we get destroyed
                PostUserEvent( LINK( this, Dialog, ImplAsyncCloseHdl ), this );
                return sal_True;
            }
        }
        else if ( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            // make sure the dialog is still modal
            // changing focus between application frames may
            // have re-enabled input for our parent
            if( mbInExecute && mbModalMode )
            {
                // do not change modal counter (pSVData->maAppData.mnModalDialog)
                SetModalInputMode( sal_False );
                SetModalInputMode( sal_True );

                // #93022# def-button might have changed after show
                if( !mnMousePositioned )
                {
                    mnMousePositioned = 1;
                    ImplMouseAutoPos( this );
                }

            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

void Dialog::StateChanged( StateChangedType nType )
{
    SystemWindow::StateChanged( nType );

    if ( nType == STATE_CHANGE_INITSHOW )
    {
        if ( GetSettings().GetStyleSettings().GetAutoMnemonic() )
            ImplWindowAutoMnemonic( this );

        //if ( IsDefaultPos() && !mpWindowImpl->mbFrame )
        //    ImplCenterDialog();
        if ( !HasChildPathFocus() || HasFocus() )
            GrabFocusToFirstControl();
        if ( !(GetStyle() & WB_CLOSEABLE) )
        {
            if ( ImplGetCancelButton( this ) || ImplGetOKButton( this ) )
            {
                if ( ImplGetBorderWindow() )
                    ((ImplBorderWindow*)ImplGetBorderWindow())->SetCloser();
            }
        }

        ImplMouseAutoPos( this );
    }
    else if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

void Dialog::DataChanged( const DataChangedEvent& rDCEvt )
{
    SystemWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        ImplInitSettings();
        Invalidate();
    }
}

// -----------------------------------------------------------------------

sal_Bool Dialog::Close()
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    ImplCallEventListeners( VCLEVENT_WINDOW_CLOSE );
    if ( aDelData.IsDelete() )
        return sal_False;
    ImplRemoveDel( &aDelData );

    if ( mpWindowImpl->mxWindowPeer.is() && IsCreatedWithToolkit() && !IsInExecute() )
        return sal_False;

    mbInClose = sal_True;

    if ( !(GetStyle() & WB_CLOSEABLE) )
    {
        sal_Bool bRet = sal_True;
        ImplAddDel( &aDelData );
        PushButton* pButton = ImplGetCancelButton( this );
        if ( pButton )
            pButton->Click();
        else
        {
            pButton = ImplGetOKButton( this );
            if ( pButton )
                pButton->Click();
            else
                bRet = sal_False;
        }
        if ( aDelData.IsDelete() )
            return sal_True;
        ImplRemoveDel( &aDelData );
        return bRet;
    }

    if ( IsInExecute() )
    {
        EndDialog( sal_False );
        mbInClose = sal_False;
        return sal_True;
    }
    else
    {
        mbInClose = sal_False;
        return SystemWindow::Close();
    }
}

// -----------------------------------------------------------------------

sal_Bool Dialog::ImplStartExecuteModal()
{
    if ( mbInExecute )
    {
#ifdef DBG_UTIL
        ByteString aErrorStr( "Dialog::StartExecuteModal() is called in Dialog::StartExecuteModal(): " );
        aErrorStr += ImplGetDialogText( this );
        DBG_ERROR( aErrorStr.GetBuffer() );
#endif
        return sal_False;
    }

    if ( Application::IsDialogCancelEnabled() )
    {
#ifdef DBG_UTIL
        ByteString aErrorStr( "Dialog::StartExecuteModal() is called in a none UI application: " );
        aErrorStr += ImplGetDialogText( this );
        DBG_ERROR( aErrorStr.GetBuffer() );
#endif
        return sal_False;
    }

#ifdef DBG_UTIL
    Window* pParent = GetParent();
    if ( pParent )
    {
        pParent = pParent->ImplGetFirstOverlapWindow();
        DBG_ASSERT( pParent->IsReallyVisible(),
                    "Dialog::StartExecuteModal() - Parent not visible" );
        DBG_ASSERT( pParent->IsInputEnabled(),
                    "Dialog::StartExecuteModal() - Parent input disabled, use another parent to ensure modality!" );
        DBG_ASSERT( ! pParent->IsInModalMode(),
                    "Dialog::StartExecuteModal() - Parent already modally disabled, use another parent to ensure modality!" );
            
    }
#endif

    ImplSVData* pSVData = ImplGetSVData();

    // Dialoge, die sich in Execute befinden, miteinander verketten
    mpPrevExecuteDlg = pSVData->maWinData.mpLastExecuteDlg;
    pSVData->maWinData.mpLastExecuteDlg = this;

    // Capture beenden, damit der Dialog bedient werden kann
    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    if ( pSVData->maWinData.mpCaptureWin )
        pSVData->maWinData.mpCaptureWin->ReleaseMouse();
    EnableInput( sal_True, sal_True );

    if ( GetParent() )
    {
        NotifyEvent aNEvt( EVENT_EXECUTEDIALOG, this );
        GetParent()->Notify( aNEvt );
    }
    mbInExecute = sal_True;
    SetModalInputMode( sal_True );
    mbOldSaveBack = IsSaveBackgroundEnabled();
    EnableSaveBackground();
    
    // FIXME: no layouting, workaround some clipping issues
    ImplAdjustNWFSizes();
    
    Show();

    pSVData->maAppData.mnModalMode++;
    return sal_True;
}

// -----------------------------------------------------------------------

void Dialog::ImplEndExecuteModal()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnModalMode--;
}

// -----------------------------------------------------------------------

short Dialog::Execute()
{
    if ( !ImplStartExecuteModal() )
        return 0;

    ImplDelData aDelData;
    ImplAddDel( &aDelData );
    
#ifdef DBG_UTIL
    ImplDelData aParentDelData;
    Window* pDialogParent = mpDialogParent;
    if( pDialogParent )
        pDialogParent->ImplAddDel( &aParentDelData );
#endif

    // Yield util EndDialog is called or dialog gets destroyed
    // (the latter should not happen, but better safe than sorry
    while ( !aDelData.IsDelete() && mbInExecute )
        Application::Yield();

    ImplEndExecuteModal();

#ifdef DBG_UTIL
    if( pDialogParent  )
    {
        if( ! aParentDelData.IsDelete() )
            pDialogParent->ImplRemoveDel( &aParentDelData );
        else
            DBG_ERROR( "Dialog::Execute() - Parent of dialog destroyed in Execute()" );
    }
#endif
    if ( !aDelData.IsDelete() )
        ImplRemoveDel( &aDelData );
#ifdef DBG_UTIL
    else
    {
        DBG_ERROR( "Dialog::Execute() - Dialog destroyed in Execute()" );
    }
#endif

    long nRet = mpDialogImpl->mnResult;
    mpDialogImpl->mnResult = -1;
    return (short)nRet;
}

// -----------------------------------------------------------------------

// virtual
void Dialog::StartExecuteModal( const Link& rEndDialogHdl )
{
    if ( !ImplStartExecuteModal() )
        return;

    mpDialogImpl->maEndDialogHdl = rEndDialogHdl;
    mpDialogImpl->mbStartedModal = true;
}

// -----------------------------------------------------------------------

sal_Bool Dialog::IsStartedModal() const
{
    return mpDialogImpl->mbStartedModal;
}

// -----------------------------------------------------------------------

void Dialog::EndDialog( long nResult )
{
    if ( mbInExecute )
    {
        SetModalInputMode( sal_False );

        // Dialog aus der Kette der Dialoge die in Execute stehen entfernen
        ImplSVData* pSVData = ImplGetSVData();
        Dialog* pExeDlg = pSVData->maWinData.mpLastExecuteDlg;
        while ( pExeDlg )
        {
            if ( pExeDlg == this )
            {
                pSVData->maWinData.mpLastExecuteDlg = mpPrevExecuteDlg;
                break;
            }
            pExeDlg = pExeDlg->mpPrevExecuteDlg;
        }
        // set focus to previous modal dialogue if it is modal for
        // the same frame parent (or NULL)
        if( mpPrevExecuteDlg )
        {
            Window* pFrameParent = ImplGetFrameWindow()->ImplGetParent();
            Window* pPrevFrameParent = mpPrevExecuteDlg->ImplGetFrameWindow()->ImplGetParent();
            if( ( !pFrameParent && !pPrevFrameParent ) ||
                ( pFrameParent && pPrevFrameParent && pFrameParent->ImplGetFrame() == pPrevFrameParent->ImplGetFrame() )
                )
            {
                mpPrevExecuteDlg->GrabFocus();
            }
        }
        mpPrevExecuteDlg = NULL;

        Hide();
        EnableSaveBackground( mbOldSaveBack );
        if ( GetParent() )
        {
            NotifyEvent aNEvt( EVENT_ENDEXECUTEDIALOG, this );
            GetParent()->Notify( aNEvt );
        }

        mpDialogImpl->mnResult = nResult;

        if ( mpDialogImpl->mbStartedModal )
        {
            ImplEndExecuteModal();
            mpDialogImpl->maEndDialogHdl.Call( this );

            mpDialogImpl->maEndDialogHdl = Link();
            mpDialogImpl->mbStartedModal = false;
            mpDialogImpl->mnResult = -1;
        }
        mbInExecute = sal_False;
    }
}

// -----------------------------------------------------------------------

long Dialog::GetResult() const
{
    return mpDialogImpl->mnResult;
}

// -----------------------------------------------------------------------

void Dialog::EndAllDialogs( Window* pParent )
{
   ImplSVData*  pSVData = ImplGetSVData();
   Dialog*      pTempModDialog;
   Dialog*      pModDialog = pSVData->maWinData.mpLastExecuteDlg;
   while ( pModDialog )
   {
     pTempModDialog = pModDialog->mpPrevExecuteDlg;
     if( !pParent || ( pParent && pParent->IsWindowOrChild( pModDialog, sal_True ) ) )
     {
        pModDialog->EndDialog( sal_False );
        pModDialog->PostUserEvent( Link() );
     }
     pModDialog = pTempModDialog;
   }
}

// -----------------------------------------------------------------------

void Dialog::SetModalInputMode( sal_Bool bModal )
{
    if ( bModal == mbModalMode )
        return;

    ImplSVData* pSVData = ImplGetSVData();
    mbModalMode = bModal;
    if ( bModal )
    {
        pSVData->maAppData.mnModalDialog++;

        // Diable the prev Modal Dialog, because our dialog must close at first,
        // before the other dialog can be closed (because the other dialog
        // is on stack since our dialog returns)
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, sal_True ) )
            mpPrevExecuteDlg->EnableInput( sal_False, sal_True, sal_True, this );

        // determine next overlap dialog parent
        Window* pParent = GetParent();
        if ( pParent )
        {
            // #103716# dialogs should always be modal to the whole frame window
            // #115933# disable the whole frame hierarchie, useful if our parent
            // is a modeless dialog
            mpDialogParent = pParent->mpWindowImpl->mpFrameWindow;
            mpDialogParent->ImplIncModalCount();
        }

    }
    else
    {
        pSVData->maAppData.mnModalDialog--;

        if ( mpDialogParent )
        {
            // #115933# re-enable the whole frame hierarchie again (see above)
            // note that code in getfocus assures that we do not accidentally enable
            // windows that were disabled before
            mpDialogParent->ImplDecModalCount();
        }

        // Enable the prev Modal Dialog
        if ( mpPrevExecuteDlg && !mpPrevExecuteDlg->IsWindowOrChild( this, sal_True ) )
        {
            mpPrevExecuteDlg->EnableInput( sal_True, sal_True, sal_True, this );
            // ensure continued modality of prev dialog
            // do not change modality counter
			
			
			// #i119994# need find the last modal dialog before reactive it
            Dialog * pPrevModalDlg = mpPrevExecuteDlg;

            while( pPrevModalDlg && !pPrevModalDlg->IsModalInputMode() )
				pPrevModalDlg = pPrevModalDlg->mpPrevExecuteDlg;

            if( pPrevModalDlg &&
            ( pPrevModalDlg == mpPrevExecuteDlg 
				|| !pPrevModalDlg->IsWindowOrChild( this, sal_True ) ) )
            {
				mpPrevExecuteDlg->SetModalInputMode( sal_False );
				mpPrevExecuteDlg->SetModalInputMode( sal_True );
            }			
        }
    }
}

// -----------------------------------------------------------------------

void Dialog::SetModalInputMode( sal_Bool bModal, sal_Bool bSubModalDialogs )
{
    if ( bSubModalDialogs )
    {
        Window* pOverlap = ImplGetFirstOverlapWindow();
        pOverlap = pOverlap->mpWindowImpl->mpFirstOverlap;
        while ( pOverlap )
        {
            if ( pOverlap->IsDialog() )
                ((Dialog*)pOverlap)->SetModalInputMode( bModal, sal_True );
            pOverlap = pOverlap->mpWindowImpl->mpNext;
        }
    }

    SetModalInputMode( bModal );
}

// -----------------------------------------------------------------------

void Dialog::GrabFocusToFirstControl()
{
    Window* pFocusControl;

    // Wenn Dialog den Focus hat, versuchen wr trotzdem
    // ein Focus-Control zu finden
    if ( HasFocus() )
        pFocusControl = NULL;
    else
    {
        // Wenn schon ein Child-Fenster mal den Focus hatte,
        // dann dieses bevorzugen
        pFocusControl = ImplGetFirstOverlapWindow()->mpWindowImpl->mpLastFocusWindow;
        // Control aus der Dialog-Steuerung suchen
        if ( pFocusControl )
            pFocusControl = ImplFindDlgCtrlWindow( pFocusControl );
    }
    // Kein Control hatte vorher den Focus, oder das Control
    // befindet sich nicht in der Tab-Steuerung, dann das erste
    // Control in der TabSteuerung den Focus geben
    if ( !pFocusControl ||
         !(pFocusControl->GetStyle() & WB_TABSTOP) ||
         !pFocusControl->IsVisible() ||
         !pFocusControl->IsEnabled() || !pFocusControl->IsInputEnabled() )
    {
        sal_uInt16 n = 0;
        pFocusControl = ImplGetDlgWindow( n, DLGWINDOW_FIRST );
    }
    if ( pFocusControl )
        pFocusControl->ImplControlFocus( GETFOCUS_INIT );
}

void Dialog::GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder, sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    ImplBorderWindow aImplWin( (Window*)this, WB_BORDER|WB_STDWORK, BORDERWINDOW_STYLE_OVERLAP );
//  aImplWin.SetText( GetText() );
//  aImplWin.SetPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
//  aImplWin.SetDisplayActive( sal_True );
//  aImplWin.InitView();
    aImplWin.GetBorder( rLeftBorder, rTopBorder, rRightBorder, rBottomBorder );
}


void Dialog::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong )
{
    Point aPos = pDev->LogicToPixel( rPos );
    Size aSize = pDev->LogicToPixel( rSize );
    
    Wallpaper aWallpaper = GetBackground();
    if ( !aWallpaper.IsBitmap() )
        ImplInitSettings();
    
    pDev->Push();
    pDev->SetMapMode();
    pDev->SetLineColor();
    
    if ( aWallpaper.IsBitmap() )
        pDev->DrawBitmapEx( aPos, aSize, aWallpaper.GetBitmap() );
    else
    {
        pDev->SetFillColor( aWallpaper.GetColor() );
        pDev->DrawRect( Rectangle( aPos, aSize ) );
    }

    if (!( GetStyle() & WB_NOBORDER ))
	{
		ImplBorderWindow aImplWin( this, WB_BORDER|WB_STDWORK, BORDERWINDOW_STYLE_OVERLAP );
		aImplWin.SetText( GetText() );
		aImplWin.SetPosSizePixel( aPos.X(), aPos.Y(), aSize.Width(), aSize.Height() );
		aImplWin.SetDisplayActive( sal_True );
		aImplWin.InitView();

		aImplWin.Draw( Rectangle( aPos, aSize ), pDev, aPos );
	}

    pDev->Pop();
}


// =======================================================================

ModelessDialog::ModelessDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_MODELESSDIALOG )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ModelessDialog::ModelessDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_MODELESSDIALOG )
{
    rResId.SetRT( RSC_MODELESSDIALOG );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// =======================================================================

ModalDialog::ModalDialog( Window* pParent, WinBits nStyle ) :
    Dialog( WINDOW_MODALDIALOG )
{
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ModalDialog::ModalDialog( Window* pParent, const ResId& rResId ) :
    Dialog( WINDOW_MODALDIALOG )
{
    rResId.SetRT( RSC_MODALDIALOG );
    ImplInit( pParent, ImplInitRes( rResId ) );
    ImplLoadRes( rResId );
}
