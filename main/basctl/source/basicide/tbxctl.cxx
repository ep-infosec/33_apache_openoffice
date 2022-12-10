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
#include "precompiled_basctl.hxx"


#include <ide_pch.hxx>


#define _BASIDE_POPUPWINDOWTBX
#include <tbxctl.hxx>
#include <svx/svxids.hrc>
#include <iderid.hxx>
#include <tbxctl.hrc>
#include <idetemp.hxx>
#include <sfx2/imagemgr.hxx>
#include <svl/aeitem.hxx>
#include <vcl/toolbox.hxx>

using namespace ::com::sun::star::uno;


static ::rtl::OUString aSubToolBarResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertcontrolsbar" ) );

SFX_IMPL_TOOLBOX_CONTROL( TbxControls, SfxAllEnumItem )

/*************************************************************************
|*
|* WorkWindow Alignment
|*
\************************************************************************/
/*
IMPL_LINK( PopupWindowTbx, SelectHdl, void*, EMPTYARG )
{
    if ( IsInPopupMode() )
		EndPopupMode();

	aSelectLink.Call( &aTbx.GetToolBox() );

	return 0;
}

PopupWindowTbx::PopupWindowTbx( sal_uInt16 nId, WindowAlign eAlign,
								ResId aRIdWin, ResId aRIdTbx,
								SfxBindings& rBind ) :
				SfxPopupWindow	( nId, aRIdWin, rBind ),
				aTbx			( this, GetBindings(), aRIdTbx )
{
	FreeResource();
	aTbx.Initialize();

	ToolBox& rBox = aTbx.GetToolBox();
	rBox.SetAlign( eAlign );
	if( eAlign == WINDOWALIGN_LEFT )
		SetText( String() );

	Size aSize = aTbx.CalcWindowSizePixel();
	rBox.SetSizePixel( aSize );
	SetOutputSizePixel( aSize );
	aSelectLink = rBox.GetSelectHdl();
	rBox.SetSelectHdl( LINK( this, PopupWindowTbx, SelectHdl ) );
}

SfxPopupWindow* PopupWindowTbx::Clone() const
{
	return new PopupWindowTbx( GetId(), aTbx.GetAlign(),
						IDEResId( RID_TBXCONTROLS ),
						IDEResId( RID_TOOLBOX ),
						(SfxBindings&) GetBindings() );
}

void PopupWindowTbx::PopupModeEnd()
{
	aTbx.GetToolBox().EndSelection();
	SfxPopupWindow::PopupModeEnd();
}

void PopupWindowTbx::Update()
{
	ToolBox *pBox = &aTbx.GetToolBox();
	aTbx.Activate( pBox );
	aTbx.Deactivate( pBox );
}

PopupWindowTbx::~PopupWindowTbx()
{
}
*/
/*************************************************************************
|*
|* Klasse fuer Toolbox
|*
\************************************************************************/

TbxControls::TbxControls( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
		SfxToolBoxControl( nSlotId, nId, rTbx )
{
	nLastSlot = USHRT_MAX;

	rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
	rTbx.Invalidate();
}

/*************************************************************************
|*
|* Wenn man ein PopupWindow erzeugen will
|*
\************************************************************************/
SfxPopupWindowType TbxControls::GetPopupWindowType() const
{
	if( nLastSlot == USHRT_MAX )
		return(SFX_POPUPWINDOW_ONCLICK);
	return(SFX_POPUPWINDOW_ONTIMEOUT);
}

void TbxControls::StateChanged( sal_uInt16 nSID, SfxItemState eState,
  const SfxPoolItem* pState )
{
    if( pState )
	{
		SfxAllEnumItem* pItem = PTR_CAST(SfxAllEnumItem, pState);
		if( pItem )
		{
			sal_uInt16 nLastEnum = pItem->GetValue();
			sal_uInt16 nTemp = 0;
			switch( nLastEnum )
			{
				case SVX_SNAP_PUSHBUTTON:       nTemp = SID_INSERT_PUSHBUTTON; break;
				case SVX_SNAP_CHECKBOX:         nTemp = SID_INSERT_CHECKBOX; break;
				case SVX_SNAP_RADIOBUTTON:      nTemp = SID_INSERT_RADIOBUTTON; break;
				case SVX_SNAP_SPINBUTTON:       nTemp = SID_INSERT_SPINBUTTON; break;
				case SVX_SNAP_FIXEDTEXT:        nTemp = SID_INSERT_FIXEDTEXT; break;
				case SVX_SNAP_GROUPBOX:         nTemp = SID_INSERT_GROUPBOX; break;
				case SVX_SNAP_LISTBOX:          nTemp = SID_INSERT_LISTBOX; break;
				case SVX_SNAP_COMBOBOX:         nTemp = SID_INSERT_COMBOBOX; break;
				case SVX_SNAP_EDIT:             nTemp = SID_INSERT_EDIT; break;
				case SVX_SNAP_HSCROLLBAR:       nTemp = SID_INSERT_HSCROLLBAR; break;
				case SVX_SNAP_VSCROLLBAR:       nTemp = SID_INSERT_VSCROLLBAR; break;
				case SVX_SNAP_PREVIEW:          nTemp = SID_INSERT_PREVIEW; break;
				case SVX_SNAP_URLBUTTON:        nTemp = SID_INSERT_URLBUTTON; break;
				case SVX_SNAP_IMAGECONTROL:     nTemp = SID_INSERT_IMAGECONTROL; break;
				case SVX_SNAP_PROGRESSBAR:      nTemp = SID_INSERT_PROGRESSBAR; break;
				case SVX_SNAP_HFIXEDLINE:       nTemp = SID_INSERT_HFIXEDLINE; break;
				case SVX_SNAP_VFIXEDLINE:       nTemp = SID_INSERT_VFIXEDLINE; break;
				case SVX_SNAP_DATEFIELD:        nTemp = SID_INSERT_DATEFIELD; break;
				case SVX_SNAP_TIMEFIELD:        nTemp = SID_INSERT_TIMEFIELD; break;
				case SVX_SNAP_NUMERICFIELD:     nTemp = SID_INSERT_NUMERICFIELD; break;
				case SVX_SNAP_CURRENCYFIELD:    nTemp = SID_INSERT_CURRENCYFIELD; break;
				case SVX_SNAP_FORMATTEDFIELD:   nTemp = SID_INSERT_FORMATTEDFIELD; break;
				case SVX_SNAP_PATTERNFIELD:     nTemp = SID_INSERT_PATTERNFIELD; break;
				case SVX_SNAP_FILECONTROL:      nTemp = SID_INSERT_FILECONTROL; break;
				case SVX_SNAP_TREECONTROL:      nTemp = SID_INSERT_TREECONTROL; break;
            }
			if( nTemp )
			{
                rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                aSlotURL += rtl::OUString::valueOf( sal_Int32( nTemp ));
                Image aImage = GetImage( m_xFrame, 
                                         aSlotURL,
                                         hasBigImages(),
                                         GetToolBox().GetSettings().GetStyleSettings().GetHighContrastMode() );
                ToolBox& rBox = GetToolBox();
                rBox.SetItemImage(GetId(), aImage);
				nLastSlot = nLastEnum;
			}
		}
	}
	SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

void TbxControls::Select( sal_uInt16 nModifier )
{
	(void)nModifier;
	SfxAllEnumItem aItem( SID_CHOOSE_CONTROLS, nLastSlot );
	SfxViewFrame* pCurFrame = SfxViewFrame::Current();
	DBG_ASSERT( pCurFrame != NULL, "No current view frame!" );
	SfxDispatcher* pDispatcher = pCurFrame ? pCurFrame->GetDispatcher() : NULL;
	if( pDispatcher )
	{
		pDispatcher->Execute( SID_CHOOSE_CONTROLS, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
	}
}

/*************************************************************************
|*
|* Hier wird das Fenster erzeugt
|* Lage der Toolbox mit GetToolBox() abfragbar
|* rItemRect sind die Screen-Koordinaten
|*
\************************************************************************/
SfxPopupWindow*	TbxControls::CreatePopupWindow()
{
    if ( GetSlotId() == SID_CHOOSE_CONTROLS )
        createAndPositionSubToolBar( aSubToolBarResName );

/*
    if (GetId() == SID_CHOOSE_CONTROLS)
	{
		PopupWindowTbx *pWin =
			new PopupWindowTbx( GetId(),
								GetToolBox().IsHorizontal() ?
									WINDOWALIGN_LEFT : WINDOWALIGN_TOP,
								IDEResId( RID_TBXCONTROLS ),
								IDEResId( RID_TOOLBOX ),
								GetBindings() );
		pWin->StartPopupMode(&GetToolBox(), sal_True);
		pWin->Update();
		pWin->StartSelection();
		pWin->Show();
		return(pWin);
	}
*/
	return(0);
}


