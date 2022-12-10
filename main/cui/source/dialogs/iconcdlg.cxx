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
#include "precompiled_cui.hxx"

#include <sfx2/app.hxx>
#include <tools/rc.h>
#include <tools/shl.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <dialmgr.hxx>

#include "iconcdlg.hxx"

#include "helpid.hrc"
#include <cuires.hrc>
#include <unotools/viewoptions.hxx>
#include <svtools/apearcfg.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/i18nhelp.hxx>

#ifdef WNT
int __cdecl IconcDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
#if defined(OS2) && defined(ICC)
int _Optlink IconcDlgCmpUS_Impl( const void* p1, const void* p2 )
#else
int IconcDlgCmpUS_Impl( const void* p1, const void* p2 )
#endif
#endif
{
	return *(sal_uInt16*)p1 - *(sal_uInt16*)p2;
}

// some stuff for easier changes for SvtViewOptions
static const sal_Char*		pViewOptDataName = "dialog data";
#define VIEWOPT_DATANAME	::rtl::OUString::createFromAscii( pViewOptDataName )

static inline void SetViewOptUserItem( SvtViewOptions& rOpt, const String& rData )
{
	rOpt.SetUserItem( VIEWOPT_DATANAME, ::com::sun::star::uno::makeAny( ::rtl::OUString( rData ) ) );
}

static inline String GetViewOptUserItem( const SvtViewOptions& rOpt )
{
	::com::sun::star::uno::Any aAny( rOpt.GetUserItem( VIEWOPT_DATANAME ) );
	::rtl::OUString aUserData;
	aAny >>= aUserData;

	return String( aUserData );
}


//#####################################################################
//
// Class IconChoicePage
//
//#####################################################################

/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

IconChoicePage::IconChoicePage( Window *pParent, const ResId &rResId,
							    const SfxItemSet &rAttrSet )
:	TabPage					  ( pParent, rResId ),
	pSet					  ( &rAttrSet ),
	bHasExchangeSupport		  ( sal_False ),
	pDialog					  ( NULL ),
	bStandard				  ( sal_False )
{
	SetStyle ( GetStyle()  | WB_DIALOGCONTROL | WB_HIDE );
}

// -----------------------------------------------------------------------

IconChoicePage::~IconChoicePage()
{
}

/**********************************************************************
|
| Activate / Deaktivate
|
\**********************************************************************/

void IconChoicePage::ActivatePage( const SfxItemSet& )
{
}

// -----------------------------------------------------------------------

int IconChoicePage::DeactivatePage( SfxItemSet* )
{
	return LEAVE_PAGE;
}

/**********************************************************************
|
| ...
|
\**********************************************************************/

void IconChoicePage::FillUserData()
{
}

// -----------------------------------------------------------------------

sal_Bool IconChoicePage::IsReadOnly() const
{
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool IconChoicePage::QueryClose()
{
    return sal_True;
}

/**********************************************************************
|
| window-methods
|
\**********************************************************************/

void IconChoicePage::ImplInitSettings()
{
	Window* pParent = GetParent();
	if ( pParent->IsChildTransparentModeEnabled() && !IsControlBackground() )
	{
		EnableChildTransparentMode( sal_True );
		SetParentClipMode( PARENTCLIPMODE_NOCLIP );
		SetPaintTransparent( sal_True );
		SetBackground();
	}
	else
	{
		EnableChildTransparentMode( sal_False );
		SetParentClipMode( 0 );
		SetPaintTransparent( sal_False );

		if ( IsControlBackground() )
			SetBackground( GetControlBackground() );
		else
			SetBackground( pParent->GetBackground() );
	}
}

// -----------------------------------------------------------------------

void IconChoicePage::StateChanged( StateChangedType nType )
{
	Window::StateChanged( nType );

	if ( nType == STATE_CHANGE_CONTROLBACKGROUND )
	{
		ImplInitSettings();
		Invalidate();
	}
}

// -----------------------------------------------------------------------

void IconChoicePage::DataChanged( const DataChangedEvent& rDCEvt )
{
	Window::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
		 (rDCEvt.GetFlags() & SETTINGS_STYLE) )
	{
		ImplInitSettings();
		Invalidate();
	}
}

//#####################################################################
//
// Class IconChoiceDialog
//
//#####################################################################

/**********************************************************************
|
| Ctor / Dtor
|
\**********************************************************************/

IconChoiceDialog::IconChoiceDialog ( Window* pParent, const ResId &rResId,
									 const EIconChoicePos ePos,
									 const SfxItemSet *pItemSet )//, sal_Bool bEditFmt, const String *pUserButtonText = 0 )
: 	ModalDialog			( pParent, rResId ),
    meChoicePos     ( ePos ),
    maIconCtrl      ( this, WB_3DLOOK | WB_ICON | WB_BORDER |
                            WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME |
                            /* WB_NOSELECTION | */ WB_NODRAGSELECTION | WB_TABSTOP ),
    mnCurrentPageId ( USHRT_MAX ),

    aOKBtn          ( this, WB_DEFBUTTON ),
	aCancelBtn		( this, WB_DEFBUTTON ),
	aHelpBtn		( this ),
	aResetBtn		( this ),
    pSet            ( pItemSet ),
    pOutSet         ( NULL ),
    pExampleSet     ( NULL ),
    pRanges         ( NULL ),
    nResId          ( rResId.GetId() ),

    bHideResetBtn   ( sal_False ),
	bModal			( sal_False ),
	bInOK			( sal_False ),
	bModified		( sal_False ),
    bItemsReset     ( sal_False )
{
	// IconChoiceCtrl-Settings
	//maIconCtrl.SetBackground ( Wallpaper( Color (146, 146, 186) ) );

	maIconCtrl.SetStyle (WB_3DLOOK | WB_ICON | WB_BORDER | WB_NOCOLUMNHEADER | WB_HIGHLIGHTFRAME | /* WB_NOSELECTION | */ WB_NODRAGSELECTION | WB_TABSTOP | WB_CLIPCHILDREN );
	SetCtrlPos ( meChoicePos );
	maIconCtrl.SetClickHdl ( LINK ( this, IconChoiceDialog , ChosePageHdl_Impl ) );
	maIconCtrl.Show();
	maIconCtrl.SetChoiceWithCursor ( sal_True );
	maIconCtrl.SetSelectionMode( SINGLE_SELECTION );
	maIconCtrl.SetHelpId( HID_ICCDIALOG_CHOICECTRL );

	// ItemSet
	if ( pSet )
	{
		pExampleSet = new SfxItemSet( *pSet );
		pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
	}

	// Buttons
	aOKBtn.SetClickHdl	 ( LINK( this, IconChoiceDialog, OkHdl ) );
	aOKBtn.SetHelpId( HID_ICCDIALOG_OK_BTN );
	aCancelBtn.SetHelpId( HID_ICCDIALOG_CANCEL_BTN );
	aResetBtn.SetClickHdl( LINK( this, IconChoiceDialog, ResetHdl ) );
	aResetBtn.SetText( CUI_RESSTR(RID_SVXSTR_ICONCHOICEDLG_RESETBUT) );
	aResetBtn.SetHelpId( HID_ICCDIALOG_RESET_BTN );
	aOKBtn.Show();
	aCancelBtn.Show();
	aHelpBtn.Show();
	aResetBtn.Show();

	SetPosSizeCtrls ( sal_True );
}

// -----------------------------------------------------------------------

/*
IconChoiceDialog ::IconChoiceDialog ( SfxViewFrame *pViewFrame, Window* pParent, const ResId &rResId,
				   const SfxItemSet * = 0, sal_Bool bEditFmt = sal_False,
				   const String *pUserButtonText = 0 )
: 	meChoicePos		( PosLeft ),	// Default erst ma Links
	maIconCtrl		( this, Die_Winbits ),
	aOKBtn			( this ),
	pUserBtn		( pUserButtonText? new PushButton(this): 0 ),
	aCancelBtn		( this ),
	aHelpBtn		( this ),
	aResetBtn		( this ),
	aBaseFmtBtn		( this ),
	mnCurrentPageId	( 0 )
{
	FreeResource();
}
*/

// -----------------------------------------------------------------------

IconChoiceDialog ::~IconChoiceDialog ()
{
	sal_uLong i;

	// save configuration at INI-Manager
	// and remove pages
	SvtViewOptions aTabDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );
	aTabDlgOpt.SetWindowState( ::rtl::OUString::createFromAscii( GetWindowState((WINDOWSTATE_MASK_X | WINDOWSTATE_MASK_Y | WINDOWSTATE_MASK_STATE | WINDOWSTATE_MASK_MINIMIZED)).GetBuffer() ) );
	aTabDlgOpt.SetPageID( mnCurrentPageId );

	const sal_uLong nCount = maPageList.Count();

	for ( i = 0; i < nCount; ++i )
	{
		IconChoicePageData* pData = maPageList.GetObject(i);

		if ( pData->pPage )
		{
			pData->pPage->FillUserData();
			String aPageData(pData->pPage->GetUserData());
			if ( aPageData.Len() )
			{
				SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pData->nId ) );

				SetViewOptUserItem( aTabPageOpt, aPageData );
			}

			if ( pData->bOnDemand )
				delete (SfxItemSet*)&pData->pPage->GetItemSet();
			delete pData->pPage;
		}
		delete pData;
	}

	// remove Userdata from Icons
	for ( i=0; i<maIconCtrl.GetEntryCount(); i++)
	{
		SvxIconChoiceCtrlEntry* pEntry = maIconCtrl.GetEntry ( i );
		sal_uInt16* pUserData = (sal_uInt16*) pEntry->GetUserData();
		delete pUserData;
	}

	//
	if ( pRanges )
		delete pRanges;
	if ( pOutSet )
		delete pOutSet;
}

/**********************************************************************
|
| add new page
|
\**********************************************************************/

SvxIconChoiceCtrlEntry* IconChoiceDialog::AddTabPage( sal_uInt16 nId, const String& rIconText,
								   const Image& rChoiceIcon,
								   const Image& rChoiceIconHC,
								   CreatePage pCreateFunc /* != 0 */,
								   GetPageRanges pRangesFunc /* darf 0 sein */,
                                   sal_Bool bItemsOnDemand, sal_uLong /*nPos*/ )
{
	IconChoicePageData* pData = new IconChoicePageData ( nId, pCreateFunc,
		                                                 pRangesFunc,
														 bItemsOnDemand );
	maPageList.Insert ( pData, LIST_APPEND );

	pData->fnGetRanges = pRangesFunc;
	pData->bOnDemand = bItemsOnDemand;

	sal_uInt16 *pId = new sal_uInt16 ( nId );
	SvxIconChoiceCtrlEntry*	pEntry = maIconCtrl.InsertEntry( rIconText, rChoiceIcon, rChoiceIconHC );
	pEntry->SetUserData ( (void*) pId );
	return pEntry;
}

/**********************************************************************
|
| Paint-method
|
\**********************************************************************/

void IconChoiceDialog::Paint( const Rectangle& rRect )
{
	Dialog::Paint ( rRect );

	for ( sal_uLong i=0; i<maPageList.Count(); i++ )
	{
		IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

		if ( pData->nId == mnCurrentPageId )
		{
			ShowPageImpl ( pData );
		}
		else
		{
			HidePageImpl ( pData );
		}
	}
}

EIconChoicePos IconChoiceDialog::SetCtrlPos( const EIconChoicePos& rPos )
{
	WinBits aWinBits = maIconCtrl.GetStyle ();

	switch ( meChoicePos )
	{
		case PosLeft :
			aWinBits &= ~WB_ALIGN_TOP & ~WB_NOVSCROLL;
			aWinBits |= WB_ALIGN_LEFT | WB_NOHSCROLL;
			break;
		case PosRight :
			aWinBits &= ~WB_ALIGN_TOP & ~WB_NOVSCROLL;
			aWinBits |= WB_ALIGN_LEFT | WB_NOHSCROLL;
			break;
		case PosTop :
			aWinBits &= ~WB_ALIGN_LEFT & ~WB_NOHSCROLL;
			aWinBits |= WB_ALIGN_TOP | WB_NOVSCROLL;
			break;
		case PosBottom :
			aWinBits &= ~WB_ALIGN_LEFT & ~WB_NOHSCROLL;
			aWinBits |= WB_ALIGN_TOP | WB_NOVSCROLL;
			break;
	};
	maIconCtrl.SetStyle ( aWinBits );

	SetPosSizeCtrls();


	EIconChoicePos eOldPos = meChoicePos;
	meChoicePos = rPos;

	return eOldPos;
}

/**********************************************************************
|
| Show / Hide page or button
|
\**********************************************************************/

void IconChoiceDialog::ShowPageImpl ( IconChoicePageData* pData )
{
	if ( pData->pPage )
		pData->pPage->Show();
}

// -----------------------------------------------------------------------

void IconChoiceDialog::HidePageImpl ( IconChoicePageData* pData )
{
	if ( pData->pPage )
		pData->pPage->Hide();
}

// -----------------------------------------------------------------------

void IconChoiceDialog::ShowPage( sal_uInt16 nId )
{
    bool bInvalidate = GetCurPageId() != nId;
    SetCurPageId( nId );
	ActivatePageImpl( );
    if(bInvalidate)
        Invalidate();

	// IA2 CWS. MT: I guess we want the event now, and not in Paint()?
	IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
	if(pData)
		ShowPageImpl ( pData );
}

/**********************************************************************
|
| Resize Dialog
|
\**********************************************************************/

#define ICONCTRL_WIDTH_PIXEL	   110
#define ICONCTRL_HEIGHT_PIXEL	    75
#define MINSIZE_BUTTON_WIDTH		70
#define MINSIZE_BUTTON_HEIGHT		22

void IconChoiceDialog::Resize()
{
	Dialog::Resize ();

	if ( IsReallyVisible() )
	{
		SetPosSizeCtrls ();
	}
}

void IconChoiceDialog::SetPosSizeCtrls ( sal_Bool bInit )
{
	const Point aCtrlOffset ( LogicToPixel( Point( CTRLS_OFFSET, CTRLS_OFFSET ), MAP_APPFONT ) );
	Size aOutSize ( GetOutputSizePixel() );

	////////////////////////////////////////
	// Button-Defaults
	//
	Size aDefaultButtonSize = LogicToPixel( Size( 50, 14 ), MAP_APPFONT );

	// Reset-Button
	Size aResetButtonSize ( bInit ? aDefaultButtonSize :
	                                aResetBtn.GetSizePixel () );

	////////////////////////////////////////
	// IconChoiceCtrl resizen & positionieren
	//
    SvtTabAppearanceCfg aCfg;
    const long nDefaultWidth = (aCfg.GetScaleFactor() * ICONCTRL_WIDTH_PIXEL) / 100;
    const long nDefaultHeight = (aCfg.GetScaleFactor() * ICONCTRL_HEIGHT_PIXEL) / 100;

    Size aNewIconCtrlSize  ( nDefaultWidth,
                             aOutSize.Height()-(2*aCtrlOffset.X()) );
	Point aIconCtrlPos;
	switch ( meChoicePos )
	{
		case PosLeft :
			aIconCtrlPos = aCtrlOffset;
            aNewIconCtrlSize = Size ( nDefaultWidth,
		                              aOutSize.Height()-(2*aCtrlOffset.X()) );
			break;
		case PosRight :
            aIconCtrlPos = Point ( aOutSize.Width() - nDefaultWidth -
				                   aCtrlOffset.X(), aCtrlOffset.X() );
            aNewIconCtrlSize = Size ( nDefaultWidth,
		                              aOutSize.Height()-(2*aCtrlOffset.X()) );
			break;
		case PosTop :
			aIconCtrlPos = aCtrlOffset;
			aNewIconCtrlSize = Size ( aOutSize.Width()-(2*aCtrlOffset.X()),
                                      nDefaultHeight );
			break;
		case PosBottom :
			aIconCtrlPos = Point ( aCtrlOffset.X(), aOutSize.Height() -
				                   aResetButtonSize.Height() - (2*aCtrlOffset.X()) -
                                   nDefaultHeight );
			aNewIconCtrlSize = Size ( aOutSize.Width()-(2*aCtrlOffset.X()),
                                      nDefaultHeight );
			break;
	};
	maIconCtrl.SetPosSizePixel ( aIconCtrlPos, aNewIconCtrlSize );
	maIconCtrl.ArrangeIcons();

	////////////////////////////////////////
	// Pages resizen & positionieren
	//
	for ( sal_uLong i=0; i<maPageList.Count(); i++ )
	{
		IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

		Point aNewPagePos;
		Size aNewPageSize;
		switch ( meChoicePos )
		{
			case PosLeft :
				aNewPagePos = Point ( aNewIconCtrlSize.Width() + (2*CTRLS_OFFSET),
			                          CTRLS_OFFSET );
				aNewPageSize = Size ( aOutSize.Width() - aNewIconCtrlSize.Width() -
							          (3*CTRLS_OFFSET),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          (3*CTRLS_OFFSET) );
				break;
			case PosRight :
				aNewPagePos = aCtrlOffset;
				aNewPageSize = Size ( aOutSize.Width() - aNewIconCtrlSize.Width() -
							          (3*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          (3*aCtrlOffset.X()) );
				break;
			case PosTop :
				aNewPagePos = Point ( aCtrlOffset.X(), aNewIconCtrlSize.Height() +
					                  (2*aCtrlOffset.X()) );
				aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          aNewIconCtrlSize.Height() - (4*aCtrlOffset.X()) );
				break;
			case PosBottom :
				aNewPagePos = aCtrlOffset;
				aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          aNewIconCtrlSize.Height() - (4*aCtrlOffset.X()) );
				break;
		};

		if ( pData->pPage )
			pData->pPage->SetPosSizePixel ( aNewPagePos, aNewPageSize );
	}

	////////////////////////////////////////
	// Buttons positionieren
	//
	sal_uLong nXOffset=0;
	if ( meChoicePos == PosRight )
		nXOffset = aNewIconCtrlSize.Width()+(2*aCtrlOffset.X());

	aResetBtn.SetPosSizePixel ( Point( aOutSize.Width() - nXOffset -
		                               aResetButtonSize.Width()-aCtrlOffset.X(),
								       aOutSize.Height()-aResetButtonSize.Height()-
									   aCtrlOffset.X() ),
							   aResetButtonSize );
	// Help-Button
	Size aHelpButtonSize ( bInit ? aDefaultButtonSize :
	                               aHelpBtn.GetSizePixel () );
	aHelpBtn.SetPosSizePixel ( Point( aOutSize.Width()-aResetButtonSize.Width()-
		                              aHelpButtonSize.Width()- nXOffset -
								      (2*aCtrlOffset.X()),
								      aOutSize.Height()-aHelpButtonSize.Height()-
									  aCtrlOffset.X() ),
							   aHelpButtonSize );
	// Cancel-Button
	Size aCancelButtonSize ( bInit ? aDefaultButtonSize :
	                                 aCancelBtn.GetSizePixel () );
	aCancelBtn.SetPosSizePixel ( Point( aOutSize.Width()-aCancelButtonSize.Width()-
									    aResetButtonSize.Width()-aHelpButtonSize.Width()-
									    (3*aCtrlOffset.X()) -  nXOffset,
								        aOutSize.Height()-aCancelButtonSize.Height()-
										aCtrlOffset.X() ),
								aCancelButtonSize );
	// OK-Button
	Size aOKButtonSize ( bInit ? aDefaultButtonSize : aOKBtn.GetSizePixel () );
	aOKBtn.SetPosSizePixel ( Point( aOutSize.Width()-aOKButtonSize.Width()-
		                            aCancelButtonSize.Width()-aResetButtonSize.Width()-
									aHelpButtonSize.Width()-(4*aCtrlOffset.X())-  nXOffset,
								    aOutSize.Height()-aOKButtonSize.Height()-aCtrlOffset.X() ),
							aOKButtonSize );

	Invalidate();
}

void IconChoiceDialog::SetPosSizePages ( sal_uInt16 nId )
{
	const Point aCtrlOffset ( LogicToPixel( Point( CTRLS_OFFSET, CTRLS_OFFSET ), MAP_APPFONT ) );
	IconChoicePageData* pData = GetPageData ( nId );

	if ( pData->pPage )
	{
		Size aOutSize ( GetOutputSizePixel() );
		Size aIconCtrlSize ( maIconCtrl.GetSizePixel() );

		Point aNewPagePos;
		Size aNewPageSize;
		switch ( meChoicePos )
		{
			case PosLeft :
				aNewPagePos = Point ( aIconCtrlSize.Width() + (2*aCtrlOffset.X()),
			                          aCtrlOffset.X() );
				aNewPageSize = Size ( aOutSize.Width() - maIconCtrl.GetSizePixel().Width() -
							          (3*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          (3*aCtrlOffset.X()) );
				break;
			case PosRight :
				aNewPagePos = aCtrlOffset;
				aNewPageSize = Size ( aOutSize.Width() - maIconCtrl.GetSizePixel().Width() -
							          (3*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          (3*aCtrlOffset.X()) );
				break;
			case PosTop :
				aNewPagePos = Point ( aCtrlOffset.X(), aIconCtrlSize.Height() +
					                  (2*aCtrlOffset.X()) );
				aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          maIconCtrl.GetSizePixel().Height() - (4*aCtrlOffset.X()) );
				break;
			case PosBottom :
				aNewPagePos = aCtrlOffset;
				aNewPageSize = Size ( aOutSize.Width() - (2*aCtrlOffset.X()),
							          aOutSize.Height() - aOKBtn.GetSizePixel().Height() -
							          maIconCtrl.GetSizePixel().Height() - (4*aCtrlOffset.X()) );
				break;
		};

		pData->pPage->SetPosSizePixel ( aNewPagePos, aNewPageSize );
	}
}

/**********************************************************************
|
| select a page
|
\**********************************************************************/

IMPL_LINK ( IconChoiceDialog , ChosePageHdl_Impl, void *, EMPTYARG )
{
	sal_uLong nPos;

	SvxIconChoiceCtrlEntry *pEntry = maIconCtrl.GetSelectedEntry ( nPos );
	if ( !pEntry )
		pEntry = maIconCtrl.GetCursor( );

	sal_uInt16 *pId = (sal_uInt16*)pEntry->GetUserData ();

	if( *pId != mnCurrentPageId )
	{
		IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
		if ( pData->pPage )
			DeActivatePageImpl();

		SetCurPageId ( *pId );

		ActivatePageImpl();
        Invalidate();
	}

	return 0L;
}

/**********************************************************************
|
| Button-handler
|
\**********************************************************************/

IMPL_LINK( IconChoiceDialog, OkHdl, Button *, EMPTYARG )
{
	bInOK = sal_True;

	if ( OK_Impl() )
	{
		if ( bModal )
			EndDialog( Ok() );
		else
		{
			Ok();
			Close();
		}
	}
	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( IconChoiceDialog, ResetHdl, Button *, EMPTYARG )
{
	ResetPageImpl ();

	IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
	DBG_ASSERT( pData, "Id nicht bekannt" );

	if ( pData->bOnDemand )
	{
		// CSet auf AIS hat hier Probleme, daher getrennt
        const SfxItemSet* _pSet = &( pData->pPage->GetItemSet() );
        pData->pPage->Reset( *(SfxItemSet*)_pSet );
	}
	else
        pData->pPage->Reset( *pSet );


	return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( IconChoiceDialog, CancelHdl, Button*, EMPTYARG )
{
	Close();

	return 0;
}

/**********************************************************************
|
| call page
|
\**********************************************************************/

void IconChoiceDialog::ActivatePageImpl ()
{
	DBG_ASSERT( maPageList.Count(), "keine Pages angemeldet" );
	IconChoicePageData* pData = GetPageData ( mnCurrentPageId );
	DBG_ASSERT( pData, "Id nicht bekannt" );
	if ( pData )
	{
		if ( !pData->pPage )
		{
			const SfxItemSet* pTmpSet = 0;

			if ( pSet )
			{
				if ( bItemsReset && pSet->GetParent() )
					pTmpSet = pSet->GetParent();
				else
					pTmpSet = pSet;
			}

			if ( pTmpSet && !pData->bOnDemand )
				pData->pPage = (pData->fnCreatePage)( this, *pTmpSet );
			else
				pData->pPage = (pData->fnCreatePage)( this, *CreateInputItemSet( mnCurrentPageId ) );

			SvtViewOptions aTabPageOpt( E_TABPAGE, String::CreateFromInt32( pData->nId ) );
			pData->pPage->SetUserData( GetViewOptUserItem( aTabPageOpt ) );
			SetPosSizePages ( pData->nId );

			if ( pData->bOnDemand )
				pData->pPage->Reset( (SfxItemSet &)pData->pPage->GetItemSet() );
			else
				pData->pPage->Reset( *pSet );

            PageCreated( mnCurrentPageId, *pData->pPage );
		}
		else if ( pData->bRefresh )
		{
			pData->pPage->Reset( *pSet );
		}

		pData->bRefresh = sal_False;

		if ( pExampleSet )
			pData->pPage->ActivatePage( *pExampleSet );
	}

	SetHelpId( pData->pPage->GetHelpId() );

	sal_Bool bReadOnly = pData->pPage->IsReadOnly();
	if ( bReadOnly || bHideResetBtn )
		aResetBtn.Hide();
	else
		aResetBtn.Show();

}

// -----------------------------------------------------------------------

sal_Bool IconChoiceDialog::DeActivatePageImpl ()
{
	IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

	int nRet = IconChoicePage::LEAVE_PAGE;

	if ( pData )
	{
		IconChoicePage * pPage = pData->pPage;

		if ( !pExampleSet && pPage->HasExchangeSupport() && pSet )
			pExampleSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );

		if ( pSet )
		{
			SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

			if ( pPage->HasExchangeSupport() )
				nRet = pPage->DeactivatePage( &aTmp );

			if ( ( IconChoicePage::LEAVE_PAGE & nRet ) == IconChoicePage::LEAVE_PAGE &&
				 aTmp.Count() )
			{
				pExampleSet->Put( aTmp );
				pOutSet->Put( aTmp );
			}
		}
		else
		{
			if ( pPage->HasExchangeSupport() ) //!!!
			{
				if ( !pExampleSet )
				{
					SfxItemPool* pPool = pPage->GetItemSet().GetPool();
					pExampleSet =
						new SfxItemSet( *pPool, GetInputRanges( *pPool ) );
				}
				nRet = pPage->DeactivatePage( pExampleSet );
			}
			else
				nRet = pPage->DeactivatePage( NULL );
		}

		if ( nRet & IconChoicePage::REFRESH_SET )
		{
			pSet = GetRefreshedSet();
			DBG_ASSERT( pSet, "GetRefreshedSet() liefert NULL" );
			// alle Pages als neu zu initialsieren flaggen
			const sal_uLong nCount = maPageList.Count();

			for ( sal_uInt16 i = 0; i < nCount; ++i )
			{
				IconChoicePageData* pObj = (IconChoicePageData*)maPageList.GetObject(i);

				if ( pObj->pPage != pPage ) // eigene Page nicht mehr refreshen
					pObj->bRefresh = sal_True;
				else
					pObj->bRefresh = sal_False;
			}
		}
	}

	if ( nRet & IconChoicePage::LEAVE_PAGE )
		return sal_True;
	else
		return sal_False;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::ResetPageImpl ()
{
	IconChoicePageData *pData = GetPageData ( mnCurrentPageId );

	DBG_ASSERT( pData, "Id nicht bekannt" );

	if ( pData->bOnDemand )
	{
		// CSet auf AIS hat hier Probleme, daher getrennt
        const SfxItemSet* _pSet = &pData->pPage->GetItemSet();
        pData->pPage->Reset( *(SfxItemSet*)_pSet );
	}
	else
        pData->pPage->Reset( *pSet );
}

/**********************************************************************
|
| handling itemsets
|
\**********************************************************************/

const sal_uInt16* IconChoiceDialog::GetInputRanges( const SfxItemPool& rPool )
{
	if ( pSet )
	{
		DBG_ERRORFILE( "Set bereits vorhanden!" );
		return pSet->GetRanges();
	}

	if ( pRanges )
		return pRanges;
	SvUShorts aUS( 16, 16 );
	sal_uLong nCount = maPageList.Count();

	sal_uInt16 i;
	for ( i = 0; i < nCount; ++i )
	{
		IconChoicePageData* pData = maPageList.GetObject (i);

		if ( pData->fnGetRanges )
		{
			const sal_uInt16* pTmpRanges = (pData->fnGetRanges)();
			const sal_uInt16* pIter = pTmpRanges;

			sal_uInt16 nLen;
			for( nLen = 0; *pIter; ++nLen, ++pIter )
				;
			aUS.Insert( pTmpRanges, nLen, aUS.Count() );
		}
	}

	// remove double Id's
#ifndef TF_POOLABLE
	if ( rPool.HasMap() )
#endif
	{
		nCount = aUS.Count();

		for ( i = 0; i < nCount; ++i )
			aUS[i] = rPool.GetWhich( aUS[i] );
	}

	// sortieren
	if ( aUS.Count() > 1 )
	{
#if defined __SUNPRO_CC
#pragma disable_warn
#endif
		qsort( (void*)aUS.GetData(), aUS.Count(), sizeof(sal_uInt16), IconcDlgCmpUS_Impl );
#if defined __SUNPRO_CC
#pragma enable_warn
#endif
	}

	pRanges = new sal_uInt16[aUS.Count() + 1];
	memcpy(pRanges, aUS.GetData(), sizeof(sal_uInt16) * aUS.Count());
	pRanges[aUS.Count()] = 0;

	return pRanges;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::SetInputSet( const SfxItemSet* pInSet )
{
	FASTBOOL bSet = ( pSet != NULL );

	pSet = pInSet;

	if ( !bSet && !pExampleSet && !pOutSet )
	{
		pExampleSet = new SfxItemSet( *pSet );
		pOutSet = new SfxItemSet( *pSet->GetPool(), pSet->GetRanges() );
	}
}

// -----------------------------------------------------------------------

void IconChoiceDialog::PageCreated( sal_uInt16 /*nId*/, IconChoicePage& /*rPage*/ )
{
    // not interested in
}

// -----------------------------------------------------------------------

SfxItemSet* IconChoiceDialog::CreateInputItemSet( sal_uInt16 )
{
	DBG_WARNINGFILE( "CreateInputItemSet nicht implementiert" );

	return 0;
}

/**********************************************************************
|
| start dialog
|
\**********************************************************************/

short IconChoiceDialog::Execute()
{
	if ( !maPageList.Count() )
		return RET_CANCEL;

	Start_Impl();

	return Dialog::Execute();
}

// -----------------------------------------------------------------------

void IconChoiceDialog::Start( sal_Bool bShow )
{

	aCancelBtn.SetClickHdl( LINK( this, IconChoiceDialog, CancelHdl ) );
	bModal = sal_False;

	Start_Impl();

	if ( bShow )
		Window::Show();

}

// -----------------------------------------------------------------------

sal_Bool IconChoiceDialog::QueryClose()
{
    sal_Bool bRet = sal_True;
    const sal_uLong nCount = maPageList.Count();
    for ( sal_uLong i = 0; i < nCount; ++i )
    {
        IconChoicePageData* pData = maPageList.GetObject(i);
        if ( pData->pPage && !pData->pPage->QueryClose() )
        {
            bRet = sal_False;
            break;
        }
    }
    return bRet;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::Start_Impl()
{
	Point aPos;
	sal_uInt16 nActPage;

	if ( mnCurrentPageId == 0 || mnCurrentPageId == USHRT_MAX )
		nActPage = maPageList.GetObject(0)->nId;//First()->nId;
	else
		nActPage = mnCurrentPageId;

	// Konfiguration vorhanden?
	SvtViewOptions aTabDlgOpt( E_TABDIALOG, String::CreateFromInt32( nResId ) );

	if ( aTabDlgOpt.Exists() )
	{
		// ggf. Position aus Konfig
		SetWindowState( ByteString( aTabDlgOpt.GetWindowState().getStr(), RTL_TEXTENCODING_ASCII_US ) );

		// initiale TabPage aus Programm/Hilfe/Konfig
		nActPage = (sal_uInt16)aTabDlgOpt.GetPageID();

		if ( USHRT_MAX != mnCurrentPageId )
			nActPage = mnCurrentPageId;

		if ( GetPageData ( nActPage ) == NULL )
			nActPage = ( (IconChoicePageData*)maPageList.GetObject(0) )->nId;
	}
	else if ( USHRT_MAX != mnCurrentPageId && GetPageData ( mnCurrentPageId ) != NULL )
		nActPage = mnCurrentPageId;

	mnCurrentPageId = nActPage;

	FocusOnIcon( mnCurrentPageId );

	ActivatePageImpl();
}

// -----------------------------------------------------------------------

const SfxItemSet* IconChoiceDialog::GetRefreshedSet()
{
	DBG_ERRORFILE( "GetRefreshedSet nicht implementiert" );
	return 0;
}

/**********************************************************************
|
| tool-methods
|
\**********************************************************************/

IconChoicePageData* IconChoiceDialog::GetPageData ( sal_uInt16 nId )
{
	IconChoicePageData *pRet = NULL;
	sal_Bool bFound = sal_False;

	for ( sal_uLong i=0; i<maPageList.Count() && !bFound; i++ )
	{
		IconChoicePageData* pData = (IconChoicePageData*)maPageList.GetObject ( i );

		if ( pData->nId == nId )
		{
			pRet = pData;
		}
	}

	return pRet;
}

/**********************************************************************
|
| OK-Status
|
\**********************************************************************/

sal_Bool IconChoiceDialog::OK_Impl()
{
	IconChoicePage* pPage = GetPageData ( mnCurrentPageId )->pPage;

	bool bEnd = !pPage;
	if ( pPage )
	{
		int nRet = IconChoicePage::LEAVE_PAGE;
		if ( pSet )
		{
			SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );
			sal_Bool bRet = sal_False;

			if ( pPage->HasExchangeSupport() )
				nRet = pPage->DeactivatePage( &aTmp );

			if ( ( IconChoicePage::LEAVE_PAGE & nRet ) == IconChoicePage::LEAVE_PAGE
				 && aTmp.Count() )
			{
				pExampleSet->Put( aTmp );
				pOutSet->Put( aTmp );
			}
			else if ( bRet )
				bModified |= sal_True;
		}
		else
			nRet = pPage->DeactivatePage( NULL );
		bEnd = nRet;
	}

	return bEnd;
}

// -----------------------------------------------------------------------

short IconChoiceDialog::Ok()
{
	bInOK = sal_True;

	if ( !pOutSet )
	{
		if ( !pExampleSet && pSet )
			pOutSet = pSet->Clone( sal_False );	// ohne Items
		else if ( pExampleSet )
			pOutSet = new SfxItemSet( *pExampleSet );
	}
    sal_Bool _bModified = sal_False;

	const sal_uLong nCount = maPageList.Count();

	for ( sal_uInt16 i = 0; i < nCount; ++i )
	{
		IconChoicePageData* pData = GetPageData ( i );

		IconChoicePage* pPage = pData->pPage;

		if ( pPage )
		{
			if ( pData->bOnDemand )
			{
				SfxItemSet& rSet = (SfxItemSet&)pPage->GetItemSet();
				rSet.ClearItem();
                _bModified |= pPage->FillItemSet( rSet );
			}
			else if ( pSet && !pPage->HasExchangeSupport() )
			{
				SfxItemSet aTmp( *pSet->GetPool(), pSet->GetRanges() );

				if ( pPage->FillItemSet( aTmp ) )
				{
                    _bModified |= sal_True;
					pExampleSet->Put( aTmp );
					pOutSet->Put( aTmp );
				}
			}
		}
	}

    if ( _bModified || ( pOutSet && pOutSet->Count() > 0 ) )
        _bModified |= sal_True;

    return _bModified ? RET_OK : RET_CANCEL;
}

// -----------------------------------------------------------------------

void IconChoiceDialog::FocusOnIcon( sal_uInt16 nId )
{
	// set focus to icon for the current visible page
	for ( sal_uInt16 i=0; i<maIconCtrl.GetEntryCount(); i++)
	{
		SvxIconChoiceCtrlEntry* pEntry = maIconCtrl.GetEntry ( i );
		sal_uInt16* pUserData = (sal_uInt16*) pEntry->GetUserData();

		if ( pUserData && *pUserData == nId )
		{
			maIconCtrl.SetCursor( pEntry );
			break;
		}
	}
}
