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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/module.hxx>
#include <tools/urlobj.hxx>

#define _SVX_ITEMWIN_CXX

#include <vcl/svapp.hxx>

#include <svx/dialogs.hrc>

#define DELAY_TIMEOUT			100

#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xtable.hxx>
#include "svx/drawitem.hxx"
#include <svx/dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <svx/itemwin.hxx>
#include "svx/linectrl.hxx"
#include <svtools/colorcfg.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

#define LOGICAL_EDIT_HEIGHT         12
//========================================================================
// SvxLineBox
//========================================================================

SvxLineBox::SvxLineBox( Window* pParent, const Reference< XFrame >& rFrame, WinBits nBits ) :
	LineLB( pParent, nBits ),
	meBmpMode	( GetSettings().GetStyleSettings().GetHighContrastMode() ? BMP_COLOR_HIGHCONTRAST : BMP_COLOR_NORMAL ),
	nCurPos		( 0 ),
    aLogicalSize(40,140),
    bRelease    ( sal_True ),
	mpSh		( NULL ),
    mxFrame     ( rFrame )
{
    SetSizePixel( LogicToPixel( aLogicalSize, MAP_APPFONT ));
    Show();

	aDelayTimer.SetTimeout( DELAY_TIMEOUT );
	aDelayTimer.SetTimeoutHdl( LINK( this, SvxLineBox, DelayHdl_Impl ) );
	aDelayTimer.Start();
}

// -----------------------------------------------------------------------

SvxLineBox::~SvxLineBox()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLineBox, DelayHdl_Impl, Timer *, EMPTYARG )
{
	if ( GetEntryCount() == 0 )
	{
		mpSh = SfxObjectShell::Current();
		FillControl();
	}
	return 0;
}

// -----------------------------------------------------------------------

void SvxLineBox::Select()
{
    // Call the parent's Select() member to trigger accessibility events.
    LineLB::Select();

	if ( !IsTravelSelect() )
	{
		XLineStyle eXLS;
		sal_uInt16 nPos = GetSelectEntryPos();
//		SfxDispatcher* pDisp = rBindings.GetDispatcher();
		//DBG_ASSERT( pDisp, "invalid Dispatcher" );

		switch ( nPos )
		{
			case 0:
				eXLS = XLINE_NONE;
				break;

			case 1:
				eXLS = XLINE_SOLID;
				break;

			default:
			{
				eXLS = XLINE_DASH;

				if ( nPos != LISTBOX_ENTRY_NOTFOUND &&
					 SfxObjectShell::Current()	&&
					 SfxObjectShell::Current()->GetItem( SID_DASH_LIST ) )
				{
					// LineDashItem wird nur geschickt, wenn es auch einen Dash besitzt.
					// Notify k"ummert sich darum!
					SvxDashListItem aItem( *(const SvxDashListItem*)(
						SfxObjectShell::Current()->GetItem( SID_DASH_LIST ) ) );
					XLineDashItem aLineDashItem( GetSelectEntry(),
						aItem.GetDashList()->GetDash( nPos - 2 )->GetDash() );

                    Any a;
                    Sequence< PropertyValue > aArgs( 1 );
                    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LineDash" ));
                    aLineDashItem.QueryValue ( a );
                    aArgs[0].Value = a;
                    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LineDash" )),
                                                 aArgs );
//                    pDisp->Execute( SID_ATTR_LINE_DASH, SFX_CALLMODE_RECORD, &aLineDashItem, 0L );
				}
			}
			break;
		}

		XLineStyleItem aLineStyleItem( eXLS );
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XLineStyle" ));
        aLineStyleItem.QueryValue ( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                     ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:XLineStyle" )),
                                     aArgs );
//		pDisp->Execute( SID_ATTR_LINE_STYLE, SFX_CALLMODE_RECORD, &aLineStyleItem, 0L );

        nCurPos = GetSelectEntryPos();
		ReleaseFocus_Impl();
	}
}

// -----------------------------------------------------------------------

long SvxLineBox::PreNotify( NotifyEvent& rNEvt )
{
	sal_uInt16 nType = rNEvt.GetType();

    switch(nType)
    {
        case EVENT_MOUSEBUTTONDOWN:
        case EVENT_GETFOCUS:
            nCurPos = GetSelectEntryPos();
        break;
        case EVENT_LOSEFOCUS:
            SelectEntryPos(nCurPos);
        break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
            if( pKEvt->GetKeyCode().GetCode() == KEY_TAB)
            {
                bRelease = sal_False;
                Select();
            }
        }
        break;
    }
	return LineLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxLineBox::Notify( NotifyEvent& rNEvt )
{
	long nHandled = LineLB::Notify( rNEvt );

	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

		switch ( pKEvt->GetKeyCode().GetCode() )
		{
			case KEY_RETURN:
				Select();
				nHandled = 1;
				break;

			case KEY_ESCAPE:
				SelectEntryPos( nCurPos );
				ReleaseFocus_Impl();
				nHandled = 1;
				break;
        }
	}
	return nHandled;
}

// -----------------------------------------------------------------------

void SvxLineBox::ReleaseFocus_Impl()
{
    if(!bRelease)
    {
        bRelease = sal_True;
        return;
    }

	if( SfxViewShell::Current() )
	{
		Window* pShellWnd = SfxViewShell::Current()->GetWindow();

		if ( pShellWnd )
			pShellWnd->GrabFocus();
	}
}
/* -----------------------------08.03.2002 15:39------------------------------

 ---------------------------------------------------------------------------*/
void SvxLineBox::DataChanged( const DataChangedEvent& rDCEvt )
{
	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
   }

    LineLB::DataChanged( rDCEvt );

	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
		BmpColorMode eMode = GetSettings().GetStyleSettings().GetHighContrastMode() ? BMP_COLOR_HIGHCONTRAST : BMP_COLOR_NORMAL;
		if( eMode != meBmpMode )
		{
			meBmpMode = eMode;
			FillControl();
		}
 	}
}

void SvxLineBox::FillControl()
{
	// FillStyles();
    if ( !mpSh )
        mpSh = SfxObjectShell::Current();

    if( mpSh )
	{
		const SvxDashListItem* pItem = (const SvxDashListItem*)( mpSh->GetItem( SID_DASH_LIST ) );
		if ( pItem )
			Fill( pItem->GetDashList() );
	}
	

//	rBindings.Invalidate( SID_ATTR_LINE_DASH );
}
//========================================================================
// SvxColorBox
//========================================================================

SvxColorBox::SvxColorBox(
    Window* pParent,
    const ::rtl::OUString& rCommand,
    const Reference< XFrame >& rFrame,
    WinBits nBits ) :
	ColorLB( pParent, nBits ),
	nCurPos		( 0 ),
    aLogicalSize(45,80),
    bRelease    ( sal_True ),
    maCommand   ( rCommand ),
    mxFrame     ( rFrame )
{
    SetSizePixel( LogicToPixel( aLogicalSize , MAP_APPFONT));
    Show();

    SfxObjectShell* pSh = SfxObjectShell::Current();

	if ( pSh )
	{
		const SvxColorTableItem* pItem =
			(const SvxColorTableItem*)(	pSh->GetItem( SID_COLOR_TABLE ) );
		if(pItem)
			Fill( pItem->GetColorTable() );
	}
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxColorBox, DelayHdl_Impl, Timer *, EMPTYARG )
{
	SfxObjectShell* pSh = SfxObjectShell::Current();

	if ( pSh )
	{
		const SvxColorTableItem* pItem = (const SvxColorTableItem*)( pSh->GetItem( SID_COLOR_TABLE ) );
		if ( pItem )
			Fill( pItem->GetColorTable() );
//		rBindings.Invalidate( nId );
	}
	return 0;
}

// -----------------------------------------------------------------------

SvxColorBox::~SvxColorBox()
{
}

// -----------------------------------------------------------------------

void SvxColorBox::Update( const XLineColorItem* pItem )
{
	if ( pItem )
		SelectEntry( pItem->GetColorValue() );
	else
		SetNoSelection();
}

// -----------------------------------------------------------------------

void SvxColorBox::Select()
{
	// OJ: base class call needed here because otherwise no event is send for accessibility
	ColorLB::Select();
	if ( !IsTravelSelect() )
	{
		XLineColorItem aLineColorItem( GetSelectEntry(), GetSelectEntryColor() );

        INetURLObject aObj( maCommand );

        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aLineColorItem.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                     maCommand,
                                     aArgs );
//        rBindings.GetDispatcher()->Execute( nId, SFX_CALLMODE_RECORD, &aLineColorItem, 0L );

        nCurPos = GetSelectEntryPos();
		ReleaseFocus_Impl();
	}
}

// -----------------------------------------------------------------------

long SvxColorBox::PreNotify( NotifyEvent& rNEvt )
{
	sal_uInt16 nType = rNEvt.GetType();

    switch(nType)
    {
        case  EVENT_MOUSEBUTTONDOWN:
        case EVENT_GETFOCUS:
            nCurPos = GetSelectEntryPos();
        break;
        case EVENT_LOSEFOCUS:
            SelectEntryPos(nCurPos);
        break;
        case EVENT_KEYINPUT:
        {
            const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

            if( pKEvt->GetKeyCode().GetCode() == KEY_TAB)
            {
                bRelease = sal_False;
                Select();
            }
        }
    }

	return ColorLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxColorBox::Notify( NotifyEvent& rNEvt )
{
	long nHandled = ColorLB::Notify( rNEvt );

	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

		switch ( pKEvt->GetKeyCode().GetCode() )
		{
			case KEY_RETURN:
				Select();
				nHandled = 1;
				break;

			case KEY_ESCAPE:
				SelectEntryPos( nCurPos );
				ReleaseFocus_Impl();
				nHandled = 1;
				break;
        }
	}
	return nHandled;
}
/* -----------------------------08.03.2002 15:35------------------------------

 ---------------------------------------------------------------------------*/
void SvxColorBox::DataChanged( const DataChangedEvent& rDCEvt )
{
	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
        Size aDropSize( aLogicalSize.Width(), LOGICAL_EDIT_HEIGHT);
        SetDropDownSizePixel(LogicToPixel(aDropSize, MAP_APPFONT));
    }

    ColorLB::DataChanged( rDCEvt );
}
// -----------------------------------------------------------------------

void SvxColorBox::ReleaseFocus_Impl()
{
    if(!bRelease)
    {
        bRelease = sal_True;
        return;
    }

	if( SfxViewShell::Current() )
	{
		Window* pShellWnd = SfxViewShell::Current()->GetWindow();

		if ( pShellWnd )
			pShellWnd->GrabFocus();
	}
}

//========================================================================
// SvxMetricField
//========================================================================

SvxMetricField::SvxMetricField(
    Window* pParent, const Reference< XFrame >& rFrame, WinBits nBits ) :
	MetricField( pParent, nBits ),
	aCurTxt( String() ),
    mxFrame( rFrame )
{
	Size aSize = Size(GetTextWidth( String::CreateFromAscii("99,99mm") ),GetTextHeight());
	aSize.Width() += 20;
	aSize.Height() += 6;
	SetSizePixel( aSize );
    aLogicalSize = PixelToLogic(aSize, MAP_APPFONT);
	SetUnit( FUNIT_MM );
	SetDecimalDigits( 2 );
	SetMax( 5000 );
	SetMin( 0 );
	SetLast( 5000 );
	SetFirst( 0 );

	eDlgUnit = SfxModule::GetModuleFieldUnit( mxFrame );
	SetFieldUnit( *this, eDlgUnit, sal_False );
	Show();
}

// -----------------------------------------------------------------------

SvxMetricField::~SvxMetricField()
{
}

// -----------------------------------------------------------------------

void SvxMetricField::Update( const XLineWidthItem* pItem )
{
	if ( pItem )
	{
		if ( pItem->GetValue() != GetCoreValue( *this, ePoolUnit ) )
			SetMetricValue( *this, pItem->GetValue(), ePoolUnit );
	}
	else
		SetText( String() );
}

// -----------------------------------------------------------------------

void SvxMetricField::Modify()
{
	MetricField::Modify();
	long nTmp = GetCoreValue( *this, ePoolUnit );
	XLineWidthItem aLineWidthItem( nTmp );

    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LineWidth" ));
    aLineWidthItem.QueryValue( a );
    aArgs[0].Value = a;
    SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
                                 ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:LineWidth" )),
                                 aArgs );
//	rBindings.GetDispatcher()->Execute( SID_ATTR_LINE_WIDTH, SFX_CALLMODE_RECORD, &aLineWidthItem, 0L );
}

// -----------------------------------------------------------------------

void SvxMetricField::ReleaseFocus_Impl()
{
	if( SfxViewShell::Current() )
	{
		Window* pShellWnd = SfxViewShell::Current()->GetWindow();
		if ( pShellWnd )
			pShellWnd->GrabFocus();
	}
}

// -----------------------------------------------------------------------

void SvxMetricField::Down()
{
    // #124425# make the OS2 case OS2-only (if still needed); under
    // non-OS2 it prevents the value to go to zero on down presses,
    // s do not use there. Also added a fix for OS2, but could not test
#ifdef OS2
    const sal_Int64 nValue(GetValue() - GetSpinSize());

    // prevent a jump to spin max on OS2
    if ( nValue >= GetMin() )
    {
        MetricField::Down();
    }
    else if ( nValue < GetMin() )
    {
        // still set to GetMin() when spin min is reached
        SetMetricValue( *this, GetMin(), ePoolUnit );
    }

#else
    MetricField::Down();
#endif
}

// -----------------------------------------------------------------------

void SvxMetricField::Up()
{
	MetricField::Up();
}

// -----------------------------------------------------------------------

void SvxMetricField::SetCoreUnit( SfxMapUnit eUnit )
{
	ePoolUnit = eUnit;
}

// -----------------------------------------------------------------------

void SvxMetricField::RefreshDlgUnit()
{
	FieldUnit eTmpUnit = SfxModule::GetModuleFieldUnit( mxFrame );
	if ( eDlgUnit != eTmpUnit )
	{
		eDlgUnit = eTmpUnit;
		SetFieldUnit( *this, eDlgUnit, sal_False );
	}
}

// -----------------------------------------------------------------------

long SvxMetricField::PreNotify( NotifyEvent& rNEvt )
{
	sal_uInt16 nType = rNEvt.GetType();

	if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
		aCurTxt = GetText();

	return MetricField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxMetricField::Notify( NotifyEvent& rNEvt )
{
	long nHandled = MetricField::Notify( rNEvt );

	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
		const KeyCode& rKey = pKEvt->GetKeyCode();
		SfxViewShell* pSh = SfxViewShell::Current();

		if ( rKey.GetModifier() && rKey.GetGroup() != KEYGROUP_CURSOR && pSh )
			pSh->KeyInput( *pKEvt );
		else
		{
			bool bHandled = sal_False;

			switch ( rKey.GetCode() )
			{
				case KEY_RETURN:
					Reformat();
					bHandled = sal_True;
					break;

				case KEY_ESCAPE:
					SetText( aCurTxt );
					bHandled = sal_True;
					break;
			}

			if ( bHandled )
			{
				nHandled = 1;
				Modify();
				ReleaseFocus_Impl();
			}
		}
	}
	return nHandled;
}
/* -----------------------------08.03.2002 15:32------------------------------

 ---------------------------------------------------------------------------*/
void SvxMetricField::DataChanged( const DataChangedEvent& rDCEvt )
{
	if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        SetSizePixel(LogicToPixel(aLogicalSize, MAP_APPFONT));
    }

    MetricField::DataChanged( rDCEvt );
}

//========================================================================
// SvxFillTypeBox
//========================================================================

SvxFillTypeBox::SvxFillTypeBox( Window* pParent, WinBits nBits ) :
	FillTypeLB( pParent, nBits | WB_TABSTOP ),
	nCurPos	( 0 ),
    bSelect ( sal_False ),
    bRelease(sal_True)
{
    SetSizePixel( LogicToPixel( Size(40, 40 ),MAP_APPFONT ));
    Fill();
	SelectEntryPos( XFILL_SOLID );
	Show();
}

// -----------------------------------------------------------------------

SvxFillTypeBox::~SvxFillTypeBox()
{
}

// -----------------------------------------------------------------------

long SvxFillTypeBox::PreNotify( NotifyEvent& rNEvt )
{
	sal_uInt16 nType = rNEvt.GetType();

	if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
		nCurPos = GetSelectEntryPos();
	else if ( EVENT_LOSEFOCUS == nType
		&& Application::GetFocusWindow()
		&& !IsWindowOrChild( Application::GetFocusWindow(), sal_True ) )
	{
		if ( !bSelect )
			SelectEntryPos( nCurPos );
		else
			bSelect = sal_False;
	}

	return FillTypeLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxFillTypeBox::Notify( NotifyEvent& rNEvt )
{
	long nHandled = FillTypeLB::Notify( rNEvt );

	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
		switch ( pKEvt->GetKeyCode().GetCode() )
		{
            case KEY_RETURN:
                nHandled = 1;
                ( (Link&)GetSelectHdl() ).Call( this );
            break;
            case KEY_TAB:
                bRelease = sal_False;
                ( (Link&)GetSelectHdl() ).Call( this );
                bRelease = sal_True;
                break;

			case KEY_ESCAPE:
				SelectEntryPos( nCurPos );
				ReleaseFocus_Impl();
				nHandled = 1;
				break;
		}
	}
	return nHandled;
}

// -----------------------------------------------------------------------

void SvxFillTypeBox::ReleaseFocus_Impl()
{
	if( SfxViewShell::Current() )
	{
		Window* pShellWnd = SfxViewShell::Current()->GetWindow();

		if ( pShellWnd )
			pShellWnd->GrabFocus();
	}
}

//========================================================================
// SvxFillAttrBox
//========================================================================

SvxFillAttrBox::SvxFillAttrBox( Window* pParent, WinBits nBits ) :

	FillAttrLB( pParent, nBits | WB_TABSTOP ),

    nCurPos( 0 ),
    bRelease( sal_True )

{
	SetPosPixel( Point( 90, 0 ) );
    SetSizePixel( LogicToPixel( Size(50, 80 ), MAP_APPFONT ));
	Show();
}

// -----------------------------------------------------------------------

SvxFillAttrBox::~SvxFillAttrBox()
{
}

// -----------------------------------------------------------------------

long SvxFillAttrBox::PreNotify( NotifyEvent& rNEvt )
{
	sal_uInt16 nType = rNEvt.GetType();

	if ( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
		nCurPos = GetSelectEntryPos();

	return FillAttrLB::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long SvxFillAttrBox::Notify( NotifyEvent& rNEvt )
{
	long nHandled = FillAttrLB::Notify( rNEvt );

	if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
		const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

		switch ( pKEvt->GetKeyCode().GetCode() )
		{
			case KEY_RETURN:
				( (Link&)GetSelectHdl() ).Call( this );
				nHandled = 1;
            break;
            case KEY_TAB:
                bRelease = sal_False;
                GetSelectHdl().Call( this );
                bRelease = sal_True;
            break;
			case KEY_ESCAPE:
				SelectEntryPos( nCurPos );
				ReleaseFocus_Impl();
				nHandled = 1;
				break;
		}
	}
	return nHandled;
}

// -----------------------------------------------------------------------

void SvxFillAttrBox::Select()
{
	FillAttrLB::Select();
}

// -----------------------------------------------------------------------

void SvxFillAttrBox::ReleaseFocus_Impl()
{
	if( SfxViewShell::Current() )
	{
		Window* pShellWnd = SfxViewShell::Current()->GetWindow();

		if ( pShellWnd )
			pShellWnd->GrabFocus();
	}
}
