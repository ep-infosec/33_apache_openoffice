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

#include <tools/ref.hxx>
#include <tools/debug.hxx>
#include <tools/rc.h>

#include <svdata.hxx>

#include <vcl/button.hxx>
#include <vcl/btndlg.hxx>



// =======================================================================

struct ImplBtnDlgItem
{
	sal_uInt16				mnId;
	sal_Bool				mbOwnButton;
	sal_Bool				mbDummyAlign;
	long				mnSepSize;
	PushButton* 		mpPushButton;
};

DECLARE_LIST( ImplBtnDlgItemList, ImplBtnDlgItem* )

// =======================================================================

void ButtonDialog::ImplInitButtonDialogData()
{
	mpItemList				= new ImplBtnDlgItemList( 8, 8 );
	mnButtonSize			= 0;
	mnCurButtonId			= 0;
	mnFocusButtonId 		= BUTTONDIALOG_BUTTON_NOTFOUND;
	mbFormat				= sal_True;
}

// -----------------------------------------------------------------------

ButtonDialog::ButtonDialog( WindowType nType ) :
	Dialog( nType )
{
	ImplInitButtonDialogData();
}

// -----------------------------------------------------------------------

ButtonDialog::ButtonDialog( Window* pParent, WinBits nStyle ) :
	Dialog( WINDOW_BUTTONDIALOG )
{
	ImplInitButtonDialogData();
	ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

ButtonDialog::ButtonDialog( Window* pParent, const ResId& rResId ) :
	Dialog( WINDOW_BUTTONDIALOG )
{
	ImplInitButtonDialogData();
	rResId.SetRT( RSC_DIALOG ); 	// !!!!!!!!!! RSC_BUTTONDIALOG !!!!!!!!
	ImplInit( pParent, ImplInitRes( rResId ) );
	ImplLoadRes( rResId );
}

// -----------------------------------------------------------------------

ButtonDialog::~ButtonDialog()
{
	ImplBtnDlgItem* pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mpPushButton && pItem->mbOwnButton )
			delete pItem->mpPushButton;
		delete pItem;
		pItem = mpItemList->Next();
	}

	delete mpItemList;
}

// -----------------------------------------------------------------------

PushButton* ButtonDialog::ImplCreatePushButton( sal_uInt16 nBtnFlags )
{
	PushButton* pBtn;
	WinBits 	nStyle = 0;

	if ( nBtnFlags & BUTTONDIALOG_DEFBUTTON )
		nStyle |= WB_DEFBUTTON;
	if ( nBtnFlags & BUTTONDIALOG_CANCELBUTTON )
		pBtn = new CancelButton( this, nStyle );
	else if ( nBtnFlags & BUTTONDIALOG_OKBUTTON )
		pBtn = new OKButton( this, nStyle );
	else if ( nBtnFlags & BUTTONDIALOG_HELPBUTTON )
		pBtn = new HelpButton( this, nStyle );
	else
		pBtn = new PushButton( this, nStyle );

	if ( !(nBtnFlags & BUTTONDIALOG_HELPBUTTON) )
		pBtn->SetClickHdl( LINK( this, ButtonDialog, ImplClickHdl ) );

	return pBtn;
}

// -----------------------------------------------------------------------

ImplBtnDlgItem* ButtonDialog::ImplGetItem( sal_uInt16 nId ) const
{
	ImplBtnDlgItem* pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mnId == nId )
			return pItem;

		pItem = mpItemList->Next();
	}

	return NULL;
}

// -----------------------------------------------------------------------

long ButtonDialog::ImplGetButtonSize()
{
	if ( !mbFormat )
		return mnButtonSize;

	// Calculate ButtonSize
	long	nLastSepSize = 0;
	long	nSepSize = 0;
	long	nButtonCount = 0;
	maCtrlSize = Size( IMPL_MINSIZE_BUTTON_WIDTH, IMPL_MINSIZE_BUTTON_HEIGHT );
	ImplBtnDlgItem* pItem = mpItemList->First();
	while ( pItem )
	{
		nSepSize += nLastSepSize;

		long nTxtWidth = pItem->mpPushButton->GetCtrlTextWidth( pItem->mpPushButton->GetText() );
		nTxtWidth += IMPL_EXTRA_BUTTON_WIDTH;
		if ( nTxtWidth > maCtrlSize.Width() )
			maCtrlSize.Width() = nTxtWidth;
		long nTxtHeight = pItem->mpPushButton->GetTextHeight();
		nTxtHeight += IMPL_EXTRA_BUTTON_HEIGHT;
		if ( nTxtHeight > maCtrlSize.Height() )
			maCtrlSize.Height() = nTxtHeight;

		nSepSize += pItem->mnSepSize;

		if ( GetStyle() & WB_HORZ )
			nLastSepSize = IMPL_SEP_BUTTON_X;
		else
			nLastSepSize = IMPL_SEP_BUTTON_Y;

		nButtonCount++;

		pItem = mpItemList->Next();
	}

	if ( GetStyle() & WB_HORZ )
		mnButtonSize  = nSepSize + (nButtonCount*maCtrlSize.Width());
	else
		mnButtonSize = nSepSize + (nButtonCount*maCtrlSize.Height());

	return mnButtonSize;
}

// -----------------------------------------------------------------------

void ButtonDialog::ImplPosControls()
{
	if ( !mbFormat )
		return;

	// Create PushButtons and determine Sizes
	ImplGetButtonSize();

	// determine dialog size
	ImplBtnDlgItem* pItem;
	Size			aDlgSize = maPageSize;
	long			nX;
	long			nY;
	if ( GetStyle() & WB_HORZ )
	{
		if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Width() )
			aDlgSize.Width() = mnButtonSize+(IMPL_DIALOG_OFFSET*2);
		if ( GetStyle() & WB_LEFT )
			nX = IMPL_DIALOG_OFFSET;
		else if ( GetStyle() & WB_RIGHT )
			nX = aDlgSize.Width()-mnButtonSize-IMPL_DIALOG_OFFSET;
		else
			nX = (aDlgSize.Width()-mnButtonSize)/2;

		aDlgSize.Height() += IMPL_DIALOG_OFFSET+maCtrlSize.Height();
		nY = aDlgSize.Height()-maCtrlSize.Height()-IMPL_DIALOG_OFFSET;
	}
	else
	{
		if ( mnButtonSize+(IMPL_DIALOG_OFFSET*2) > aDlgSize.Height() )
			aDlgSize.Height() = mnButtonSize+(IMPL_DIALOG_OFFSET*2);
		if ( GetStyle() & WB_BOTTOM )
			nY = aDlgSize.Height()-mnButtonSize-IMPL_DIALOG_OFFSET;
		else if ( GetStyle() & WB_VCENTER )
			nY = (aDlgSize.Height()-mnButtonSize)/2;
		else
			nY = IMPL_DIALOG_OFFSET;

		aDlgSize.Width() += IMPL_DIALOG_OFFSET+maCtrlSize.Width();
		nX = aDlgSize.Width()-maCtrlSize.Width()-IMPL_DIALOG_OFFSET;
	}

	// Arrange PushButtons
	pItem = mpItemList->First();
	while ( pItem )
	{
		if ( GetStyle() & WB_HORZ )
			nX += pItem->mnSepSize;
		else
			nY += pItem->mnSepSize;
		pItem->mpPushButton->SetPosSizePixel( Point( nX, nY ), maCtrlSize );
		pItem->mpPushButton->Show();
		if ( GetStyle() & WB_HORZ )
			nX += maCtrlSize.Width()+IMPL_SEP_BUTTON_X;
		else
			nY += maCtrlSize.Height()+IMPL_SEP_BUTTON_Y;

		pItem = mpItemList->Next();
	}

	SetOutputSizePixel( aDlgSize );

	mbFormat = sal_False;
}

// -----------------------------------------------------------------------

IMPL_LINK( ButtonDialog, ImplClickHdl, PushButton*, pBtn )
{
	ImplBtnDlgItem* pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mpPushButton == pBtn )
		{
			mnCurButtonId = pItem->mnId;
			Click();
			break;
		}

		pItem = mpItemList->Next();
	}

	return 0;
}

// -----------------------------------------------------------------------

void ButtonDialog::Resize()
{
}

// -----------------------------------------------------------------------

void ButtonDialog::StateChanged( StateChangedType nType )
{
	if ( nType == STATE_CHANGE_INITSHOW )
	{
		ImplPosControls();
		ImplBtnDlgItem* pItem = mpItemList->First();
		while ( pItem )
		{
			if ( pItem->mpPushButton && pItem->mbOwnButton )
				pItem->mpPushButton->SetZOrder(0, WINDOW_ZORDER_LAST);
			pItem = mpItemList->Next();
		}

		// Focus evt. auf den entsprechenden Button setzen
		if ( mnFocusButtonId != BUTTONDIALOG_BUTTON_NOTFOUND )
		{
			ImplBtnDlgItem* pItem = mpItemList->First();
			while ( pItem )
			{
				if ( pItem->mnId == mnFocusButtonId )
				{
					if ( pItem->mpPushButton->IsVisible() )
						pItem->mpPushButton->GrabFocus();
					break;
				}

				pItem = mpItemList->Next();
			}
		}
	}

	Dialog::StateChanged( nType );
}

// -----------------------------------------------------------------------

void ButtonDialog::Click()
{
	if ( !maClickHdl )
	{
		if ( IsInExecute() )
			EndDialog( GetCurButtonId() );
	}
	else
		maClickHdl.Call( this );
}

// -----------------------------------------------------------------------

void ButtonDialog::AddButton( const XubString& rText, sal_uInt16 nId,
							  sal_uInt16 nBtnFlags, long nSepPixel )
{
	// PageItem anlegen
	ImplBtnDlgItem* pItem	= new ImplBtnDlgItem;
	pItem->mnId 			= nId;
	pItem->mbOwnButton		= sal_True;
	pItem->mnSepSize		= nSepPixel;
	pItem->mpPushButton 	= ImplCreatePushButton( nBtnFlags );
	if ( rText.Len() )
		pItem->mpPushButton->SetText( rText );

	// In die Liste eintragen
	mpItemList->Insert( pItem, LIST_APPEND );

	if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
		mnFocusButtonId = nId;

	mbFormat = sal_True;
}

// -----------------------------------------------------------------------

void ButtonDialog::AddButton( StandardButtonType eType, sal_uInt16 nId,
							  sal_uInt16 nBtnFlags, long nSepPixel )
{
	// PageItem anlegen
	ImplBtnDlgItem* pItem	= new ImplBtnDlgItem;
	pItem->mnId 			= nId;
	pItem->mbOwnButton		= sal_True;
	pItem->mnSepSize		= nSepPixel;

	if ( eType == BUTTON_OK )
		nBtnFlags |= BUTTONDIALOG_OKBUTTON;
	else if ( eType == BUTTON_HELP )
		nBtnFlags |= BUTTONDIALOG_HELPBUTTON;
	else if ( (eType == BUTTON_CANCEL) || (eType == BUTTON_CLOSE) )
		nBtnFlags |= BUTTONDIALOG_CANCELBUTTON;
	pItem->mpPushButton = ImplCreatePushButton( nBtnFlags );

	// Standard-Buttons have the right text already
	if ( !((eType == BUTTON_OK) 	&& (pItem->mpPushButton->GetType() == WINDOW_OKBUTTON)) ||
		 !((eType == BUTTON_CANCEL) && (pItem->mpPushButton->GetType() == WINDOW_CANCELBUTTON)) ||
		 !((eType == BUTTON_HELP)	&& (pItem->mpPushButton->GetType() == WINDOW_HELPBUTTON)) )
	{
		pItem->mpPushButton->SetText( Button::GetStandardText( eType ) );
		pItem->mpPushButton->SetHelpText( Button::GetStandardHelpText( eType ) );
	}

	if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
		mnFocusButtonId = nId;

	// In die Liste eintragen
	mpItemList->Insert( pItem, LIST_APPEND );

	mbFormat = sal_True;
}

// -----------------------------------------------------------------------

void ButtonDialog::AddButton( PushButton* pBtn, sal_uInt16 nId,
							  sal_uInt16 nBtnFlags, long nSepPixel )
{
	// PageItem anlegen
	ImplBtnDlgItem* pItem	= new ImplBtnDlgItem;
	pItem->mnId 			= nId;
	pItem->mbOwnButton		= sal_False;
	pItem->mnSepSize		= nSepPixel;
	pItem->mpPushButton 	= pBtn;

	if ( nBtnFlags & BUTTONDIALOG_FOCUSBUTTON )
		mnFocusButtonId = nId;

	// In die View-Liste eintragen
	mpItemList->Insert( pItem, LIST_APPEND );

	mbFormat = sal_True;
}

// -----------------------------------------------------------------------

void ButtonDialog::RemoveButton( sal_uInt16 nId )
{
	ImplBtnDlgItem* pItem = mpItemList->First();
	while ( pItem )
	{
		if ( pItem->mnId == nId )
		{
			pItem->mpPushButton->Hide();
			if ( pItem->mbOwnButton )
				delete pItem->mpPushButton;
			delete pItem;
			mpItemList->Remove();
			mbFormat = sal_True;
			break;
		}

		pItem = mpItemList->Next();
	}

	DBG_ERRORFILE( "ButtonDialog::RemoveButton(): ButtonId invalid" );
}

// -----------------------------------------------------------------------

void ButtonDialog::Clear()
{
	ImplBtnDlgItem* pItem = mpItemList->First();
	while ( pItem )
	{
		pItem->mpPushButton->Hide();
		if ( pItem->mbOwnButton )
			delete pItem->mpPushButton;
		delete pItem;
		pItem = mpItemList->Next();
	}

	mpItemList->Clear();
	mbFormat = sal_True;
}

// -----------------------------------------------------------------------

sal_uInt16 ButtonDialog::GetButtonCount() const
{
	return (sal_uInt16)mpItemList->Count();
}

// -----------------------------------------------------------------------

sal_uInt16 ButtonDialog::GetButtonId( sal_uInt16 nButton ) const
{
	if ( nButton < mpItemList->Count() )
		return (sal_uInt16)mpItemList->GetObject( nButton )->mnId;
	else
		return BUTTONDIALOG_BUTTON_NOTFOUND;
}

// -----------------------------------------------------------------------

PushButton* ButtonDialog::GetPushButton( sal_uInt16 nId ) const
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	if ( pItem )
		return pItem->mpPushButton;
	else
		return NULL;
}

// -----------------------------------------------------------------------

void ButtonDialog::SetButtonText( sal_uInt16 nId, const XubString& rText )
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	if ( pItem )
	{
		pItem->mpPushButton->SetText( rText );
		mbFormat = sal_True;
	}
}

// -----------------------------------------------------------------------

XubString ButtonDialog::GetButtonText( sal_uInt16 nId ) const
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	if ( pItem )
		return pItem->mpPushButton->GetText();
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ButtonDialog::SetButtonHelpText( sal_uInt16 nId, const XubString& rText )
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	if ( pItem )
		pItem->mpPushButton->SetHelpText( rText );
}

// -----------------------------------------------------------------------

XubString ButtonDialog::GetButtonHelpText( sal_uInt16 nId ) const
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	if ( pItem )
		return pItem->mpPushButton->GetHelpText();
	else
		return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void ButtonDialog::SetButtonHelpId( sal_uInt16 nId, const rtl::OString& rHelpId )
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	if ( pItem )
		pItem->mpPushButton->SetHelpId( rHelpId );
}

// -----------------------------------------------------------------------

rtl::OString ButtonDialog::GetButtonHelpId( sal_uInt16 nId ) const
{
	ImplBtnDlgItem* pItem = ImplGetItem( nId );

	return pItem ? rtl::OString( pItem->mpPushButton->GetHelpId() ) : rtl::OString();
}
