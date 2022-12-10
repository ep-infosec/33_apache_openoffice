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

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
// declare system types in sysdata.hxx
#include <svsys.h>
#include <vcl/sysdata.hxx>

#include <svdata.hxx>
#include <salframe.hxx>
#include <brdwin.hxx>
#include <window.h>

// =======================================================================

#define WORKWIN_WINDOWSTATE_FULLSCREEN          ((sal_uLong)0x00010000)
#define WORKWIN_WINDOWSTATE_ALL                 ((sal_uLong)0x00FF0000)

// =======================================================================

void WorkWindow::ImplInitWorkWindowData()
{
    mnIcon                  = 0; // Should be removed in the next top level update - now in SystemWindow

    mnPresentationFlags     = 0;
    mbPresentationMode      = sal_False;
    mbPresentationVisible   = sal_False;
    mbPresentationFull      = sal_False;
    mbFullScreenMode        = sal_False;
}

// -----------------------------------------------------------------------

void WorkWindow::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    sal_uInt16 nFrameStyle = BORDERWINDOW_STYLE_FRAME;
    if ( nStyle & WB_APP )
        nFrameStyle |= BORDERWINDOW_STYLE_APP;

    ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, pSystemParentData, nStyle, nFrameStyle );
    Window::ImplInit( pBorderWin, nStyle & (WB_3DLOOK | WB_CLIPCHILDREN | WB_DIALOGCONTROL | WB_SYSTEMFLOATWIN), NULL );
    pBorderWin->mpWindowImpl->mpClientWindow = this;
    pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
    mpWindowImpl->mpBorderWindow  = pBorderWin;
//        mpWindowImpl->mpRealParent    = pParent; // !!! Muesste eigentlich gesetzt werden, aber wegen Fehlern mit dem MenuBar erstmal nicht gesetzt !!!

    if ( nStyle & WB_APP )
    {
        ImplSVData* pSVData = ImplGetSVData();
        DBG_ASSERT( !pSVData->maWinData.mpAppWin, "WorkWindow::WorkWindow(): More than one window with style WB_APP" );
        pSVData->maWinData.mpAppWin = this;
    }

    SetActivateMode( ACTIVATE_MODE_GRABFOCUS );
}

// -----------------------------------------------------------------------

void WorkWindow::ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken )
{
    if( aSystemWorkWindowToken.hasValue() )
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
        aSystemWorkWindowToken >>= aSeq;
        SystemParentData* pData = (SystemParentData*)aSeq.getArray();
        DBG_ASSERT( aSeq.getLength() == sizeof( SystemParentData ) && pData->nSize == sizeof( SystemParentData ), "WorkWindow::WorkWindow( Window*, const Any&, WinBits ) called with invalid Any" );
        // init with style 0 as does WorkWindow::WorkWindow( SystemParentData* );
        ImplInit( pParent, 0, pData );
    }
    else
        ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( WindowType nType ) :
    SystemWindow( nType )
{
    ImplInitWorkWindowData();
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    ImplInit( pParent, nStyle, NULL );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, const ResId& rResId ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    rResId.SetRT( RSC_WORKWIN );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( Window* pParent, const ::com::sun::star::uno::Any& aSystemWorkWindowToken, WinBits nStyle ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    mbSysChild = sal_True;
    ImplInit( pParent, nStyle, aSystemWorkWindowToken );
}

// -----------------------------------------------------------------------

WorkWindow::WorkWindow( SystemParentData* pParent ) :
    SystemWindow( WINDOW_WORKWINDOW )
{
    ImplInitWorkWindowData();
    mbSysChild = sal_True;
    ImplInit( NULL, 0, pParent );
}

// -----------------------------------------------------------------------

void WorkWindow::ImplLoadRes( const ResId& rResId )
{
    SystemWindow::ImplLoadRes( rResId );

    ReadLongRes();
    if ( !(rResId.GetWinBits() & WB_HIDE) && (RSC_WORKWIN == rResId.GetRT()) )
        Show();
}

// -----------------------------------------------------------------------

WorkWindow::~WorkWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpAppWin == this )
    {
        pSVData->maWinData.mpAppWin = NULL;
        Application::Quit();
    }
}

// -----------------------------------------------------------------------

void WorkWindow::ShowFullScreenMode( sal_Bool bFullScreenMode, sal_Int32 nDisplay )
{
    if ( !mbFullScreenMode == !bFullScreenMode )
        return;

    if( (nDisplay < -1)
    || (nDisplay >= static_cast<sal_Int32>(Application::GetScreenCount()) ) )
    {
        nDisplay = GetScreenNumber();
    }

    mbFullScreenMode = bFullScreenMode != 0;
    if ( !mbSysChild )
    {
        mpWindowImpl->mpFrameWindow->mpWindowImpl->mbWaitSystemResize = sal_True;
        ImplGetFrame()->ShowFullScreen( bFullScreenMode, nDisplay );
    }
}

// -----------------------------------------------------------------------

void WorkWindow::StartPresentationMode( sal_Bool bPresentation, sal_uInt16 nFlags, sal_Int32 nDisplay )
{
    if ( !bPresentation == !mbPresentationMode )
        return;

    if ( bPresentation )
    {
        mbPresentationMode      = sal_True;
        mbPresentationVisible   = IsVisible();
        mbPresentationFull      = mbFullScreenMode;
        mnPresentationFlags     = nFlags;

        if ( !(mnPresentationFlags & PRESENTATION_NOFULLSCREEN) )
            ShowFullScreenMode( sal_True, nDisplay );
        if ( !mbSysChild )
        {
            if ( mnPresentationFlags & PRESENTATION_HIDEALLAPPS )
                mpWindowImpl->mpFrame->SetAlwaysOnTop( sal_True );
            if ( !(mnPresentationFlags & PRESENTATION_NOAUTOSHOW) )
                ToTop();
            mpWindowImpl->mpFrame->StartPresentation( sal_True );
        }

        if ( !(mnPresentationFlags & PRESENTATION_NOAUTOSHOW) )
            Show();
    }
    else
    {
        Show( mbPresentationVisible );
        if ( !mbSysChild )
        {
            mpWindowImpl->mpFrame->StartPresentation( sal_False );
            if ( mnPresentationFlags & PRESENTATION_HIDEALLAPPS )
                mpWindowImpl->mpFrame->SetAlwaysOnTop( sal_False );
        }
        ShowFullScreenMode( mbPresentationFull, nDisplay );

        mbPresentationMode      = sal_False;
        mbPresentationVisible   = sal_False;
        mbPresentationFull      = sal_False;
        mnPresentationFlags     = 0;
    }
}

// -----------------------------------------------------------------------

sal_Bool WorkWindow::IsMinimized() const
{
    //return mpWindowImpl->mpFrameData->mbMinimized;
	SalFrameState aState;
	mpWindowImpl->mpFrame->GetWindowState(&aState);
	return (( aState.mnState & SAL_FRAMESTATE_MINIMIZED ) != 0);
}

// -----------------------------------------------------------------------

sal_Bool WorkWindow::SetPluginParent( SystemParentData* pParent )
{
    DBG_ASSERT( ! mbPresentationMode && ! mbFullScreenMode, "SetPluginParent in fullscreen or presentation mode !" );

    bool bWasDnd = Window::ImplStopDnd();

    sal_Bool bShown = IsVisible();
    Show( sal_False );
    sal_Bool bRet = mpWindowImpl->mpFrame->SetPluginParent( pParent );
    Show( bShown );

    if( bWasDnd )
        Window::ImplStartDnd();

    return bRet;
}

void WorkWindow::ImplSetFrameState( sal_uLong aFrameState )
{
    SalFrameState   aState;
    aState.mnMask   = SAL_FRAMESTATE_MASK_STATE;
    aState.mnState  = aFrameState;
    mpWindowImpl->mpFrame->SetWindowState( &aState );
}


void WorkWindow::Minimize()
{
    ImplSetFrameState( SAL_FRAMESTATE_MINIMIZED );
}

void WorkWindow::Restore()
{
    ImplSetFrameState( SAL_FRAMESTATE_NORMAL );
}

sal_Bool WorkWindow::Close()
{
    sal_Bool bCanClose = SystemWindow::Close();

    // Ist es das Applikationsfenster, dann beende die Applikation
    if ( bCanClose && ( ImplGetSVData()->maWinData.mpAppWin == this ) )
        GetpApp()->Quit();
    
    return bCanClose;
}

void WorkWindow::Maximize( sal_Bool bMaximize )
{
    ImplSetFrameState( bMaximize ? SAL_FRAMESTATE_MAXIMIZED : SAL_FRAMESTATE_NORMAL );
}

sal_Bool WorkWindow::IsMaximized() const
{
    sal_Bool bRet = sal_False;

    SalFrameState aState;
    if( mpWindowImpl->mpFrame->GetWindowState( &aState ) )
    {
        if( aState.mnState & (SAL_FRAMESTATE_MAXIMIZED			|
                              SAL_FRAMESTATE_MAXIMIZED_HORZ		|
                              SAL_FRAMESTATE_MAXIMIZED_VERT ) )
            bRet = sal_True;
    }
    return bRet;
}
