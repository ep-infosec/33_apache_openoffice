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


#include <basic/sbx.hxx>
#ifndef _SV_CMDEVT_HXX
#include <vcl/cmdevt.hxx>
#endif
#include <vcl/taskpanelist.hxx>
#include <vcl/sound.hxx>
#include <objdlg.hrc>
#include <objdlg.hxx>
#include <bastypes.hxx>
#include <basidesh.hrc>
#include <basidesh.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <sbxitem.hxx>

//#ifndef _SFX_HELP_HXX //autogen
//#include <sfx2/sfxhelp.hxx>
//#endif


ObjectTreeListBox::ObjectTreeListBox( Window* pParent, const ResId& rRes )
	: BasicTreeListBox( pParent, rRes )
{
}

ObjectTreeListBox::~ObjectTreeListBox()
{
}

void ObjectTreeListBox::Command( const CommandEvent& )
{
}

void ObjectTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
	BasicTreeListBox::MouseButtonDown( rMEvt );

    if ( rMEvt.IsLeft() && ( rMEvt.GetClicks() == 2 ) )
    {
        OpenCurrent();
    }
}

void ObjectTreeListBox::KeyInput( const KeyEvent& rEvt )
{
    if ( rEvt.GetKeyCode() == KEY_RETURN && OpenCurrent() )
    {
        return;
    }
    BasicTreeListBox::KeyInput( rEvt );
}

bool ObjectTreeListBox::OpenCurrent()
{
    BasicEntryDescriptor aDesc( GetEntryDescriptor( GetCurEntry() ) );

    if ( aDesc.GetType() == OBJ_TYPE_METHOD )
    {
        BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
        SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
        SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
        if( pDispatcher )
        {
            SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(), aDesc.GetName(), 
                              aDesc.GetMethodName(), ConvertType( aDesc.GetType() ) );
            pDispatcher->Execute( SID_BASICIDE_SHOWSBX,
                                    SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
            return true;
        }
    }
    return false;
}


ObjectCatalog::ObjectCatalog( Window * pParent )
    :FloatingWindow( pParent, IDEResId( RID_BASICIDE_OBJCAT ) )
    ,aMacroTreeList( this, IDEResId( RID_TLB_MACROS ) )
    ,aToolBox(this, IDEResId(RID_TB_TOOLBOX), IDEResId(RID_IMGLST_TB_HC))
    ,aMacroDescr( this, IDEResId( RID_FT_MACRODESCR ) )
{
	FreeResource();

    aToolBox.SetOutStyle( TOOLBOX_STYLE_FLAT );
	aToolBox.SetSizePixel( aToolBox.CalcWindowSizePixel() );
	aToolBox.SetSelectHdl( LINK( this, ObjectCatalog, ToolBoxHdl ) );

    aMacroTreeList.SetStyle( WB_BORDER | WB_TABSTOP |
                             WB_HASLINES | WB_HASLINESATROOT | 
                             WB_HASBUTTONS | WB_HASBUTTONSATROOT |
                             WB_HSCROLL );

	aMacroTreeList.SetSelectHdl( LINK( this, ObjectCatalog, TreeListHighlightHdl ) );
	aMacroTreeList.SetAccessibleName(String(IDEResId(RID_STR_TLB_MACROS)));
	aMacroTreeList.ScanAllEntries();
	aMacroTreeList.GrabFocus();

	CheckButtons();

	Point aPos = IDE_DLL()->GetExtraData()->GetObjectCatalogPos();
	Size aSize = IDE_DLL()->GetExtraData()->GetObjectCatalogSize();
	if ( aPos.X() == INVPOSITION )
	{
		// Zentriert nach AppWin:
		Window* pWin = GetParent();
		aPos = pWin->OutputToScreenPixel( Point( 0, 0 ) );
		Size aAppWinSz = pWin->GetSizePixel();
		Size aDlgWinSz = GetSizePixel();
		aPos.X() += aAppWinSz.Width() / 2;
		aPos.X() -= aDlgWinSz.Width() / 2;
		aPos.Y() += aAppWinSz.Height() / 2;
		aPos.Y() -= aDlgWinSz.Height() / 2;
	}
	SetPosPixel( aPos );
	if ( aSize.Width() )
		SetOutputSizePixel( aSize );

	Resize();	// damit der Resize-Handler die Controls anordnet

	// make object catalog keyboard accessible
	pParent->GetSystemWindow()->GetTaskPaneList()->AddWindow( this );
}

ObjectCatalog::~ObjectCatalog()
{
	GetParent()->GetSystemWindow()->GetTaskPaneList()->RemoveWindow( this );
}

void __EXPORT ObjectCatalog::Move()
{
	IDE_DLL()->GetExtraData()->SetObjectCatalogPos( GetPosPixel() );
}

sal_Bool __EXPORT ObjectCatalog::Close()
{
	aCancelHdl.Call( this );
	return sal_True;
}

void __EXPORT ObjectCatalog::Resize()
{
	Size aOutSz = GetOutputSizePixel();
	IDE_DLL()->GetExtraData()->SetObjectCatalogSize( aOutSz );

	Point aTreePos = aMacroTreeList.GetPosPixel();
	Size aDescrSz = aMacroDescr.GetSizePixel();

	Size aTreeSz;
	long nCtrlWidth = aOutSz.Width() - 2*aTreePos.X();
	aTreeSz.Width() = nCtrlWidth;
	aTreeSz.Height() = aOutSz.Height() - aTreePos.Y() -
						2*aTreePos.X() - aDescrSz.Height();

	if ( aTreeSz.Height() > 0 )
	{
		aMacroTreeList.SetSizePixel( aTreeSz );

		Point aDescrPos( aTreePos.X(), aTreePos.Y()+aTreeSz.Height()+aTreePos.X() );

		aMacroDescr.SetPosSizePixel( aDescrPos, Size( nCtrlWidth, aDescrSz.Height() ) );

		String aDesc = aMacroDescr.GetText();
		aMacroDescr.SetText(String());
		aMacroDescr.SetText(aDesc);
	}

	// Die Buttons oben bleiben immer unveraendert stehen...
}

IMPL_LINK( ObjectCatalog, ToolBoxHdl, ToolBox*, pToolBox )
{
	sal_uInt16 nCurItem = pToolBox->GetCurItemId();
	switch ( nCurItem )
	{
		case TBITEM_SHOW:
		{
            SfxAllItemSet aArgs( SFX_APP()->GetPool() );
            SfxRequest aRequest( SID_BASICIDE_APPEAR, SFX_CALLMODE_SYNCHRON, aArgs );
            SFX_APP()->ExecuteSlot( aRequest );

            SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
			DBG_ASSERT( pCurEntry, "Entry?!" );
            BasicEntryDescriptor aDesc( aMacroTreeList.GetEntryDescriptor( pCurEntry ) );
            BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
            SfxViewFrame* pViewFrame = pIDEShell ? pIDEShell->GetViewFrame() : NULL;
            SfxDispatcher* pDispatcher = pViewFrame ? pViewFrame->GetDispatcher() : NULL;
			if ( aDesc.GetType() == OBJ_TYPE_MODULE || 
				 aDesc.GetType() == OBJ_TYPE_DIALOG ||
				 aDesc.GetType() == OBJ_TYPE_METHOD )
			{
				if( pDispatcher )
				{
                    SbxItem aSbxItem( SID_BASICIDE_ARG_SBX, aDesc.GetDocument(), aDesc.GetLibName(), aDesc.GetName(), 
                                      aDesc.GetMethodName(), aMacroTreeList.ConvertType( aDesc.GetType() ) );
					pDispatcher->Execute( SID_BASICIDE_SHOWSBX,
										  SFX_CALLMODE_SYNCHRON, &aSbxItem, 0L );
				}
			}
			else
			{
				ErrorBox( this, WB_OK, String( IDEResId( RID_STR_OBJNOTFOUND ) ) ).Execute();
				aMacroTreeList.GetModel()->Remove( pCurEntry );
				CheckButtons();
			}
		}
		break;
	}

	return 0;
}



void ObjectCatalog::CheckButtons()
{
	SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
	BasicEntryType eType = pCurEntry ? ((BasicEntry*)pCurEntry->GetUserData())->GetType() : OBJ_TYPE_UNKNOWN;
	if ( eType == OBJ_TYPE_DIALOG || eType == OBJ_TYPE_MODULE || eType == OBJ_TYPE_METHOD )
		aToolBox.EnableItem( TBITEM_SHOW, sal_True );
	else
		aToolBox.EnableItem( TBITEM_SHOW, sal_False );
}



IMPL_LINK_INLINE_START( ObjectCatalog, TreeListHighlightHdl, SvTreeListBox *, pBox )
{
	if ( pBox->IsSelected( pBox->GetHdlEntry() ) )
		UpdateFields();
	return 0;
}
IMPL_LINK_INLINE_END( ObjectCatalog, TreeListHighlightHdl, SvTreeListBox *, pBox )


void ObjectCatalog::UpdateFields()
{
	SvLBoxEntry* pCurEntry = aMacroTreeList.GetCurEntry();
	if ( pCurEntry )
	{
		CheckButtons();
		aMacroDescr.SetText( String() );
		SbxVariable* pVar = aMacroTreeList.FindVariable( pCurEntry );
		if ( pVar )
		{
			SbxInfoRef xInfo = pVar->GetInfo();
			if ( xInfo.Is() )
				aMacroDescr.SetText( xInfo->GetComment() );
		}
    }
}


void ObjectCatalog::UpdateEntries()
{
	aMacroTreeList.UpdateEntries();
}

void ObjectCatalog::SetCurrentEntry( BasicEntryDescriptor& rDesc )
{ 
    aMacroTreeList.SetCurrentEntry( rDesc );
}

ObjectCatalogToolBox_Impl::ObjectCatalogToolBox_Impl(
    Window * pParent, ResId const & rResId,
    ResId const & rImagesHighContrastId):
    ToolBox(pParent, rResId),
    m_aImagesNormal(GetImageList()),
    m_aImagesHighContrast(rImagesHighContrastId),
    m_bHighContrast(false)
{
    setImages();
}

// virtual
void ObjectCatalogToolBox_Impl::DataChanged(DataChangedEvent const & rDCEvt)
{
    ToolBox::DataChanged(rDCEvt);
    if ((rDCEvt.GetType() == DATACHANGED_SETTINGS
         || rDCEvt.GetType() == DATACHANGED_DISPLAY)
        && (rDCEvt.GetFlags() & SETTINGS_STYLE) != 0)
        setImages();
}

void ObjectCatalogToolBox_Impl::setImages()
{
    bool bHC = GetSettings().GetStyleSettings().GetHighContrastMode();
    if (bHC != m_bHighContrast)
    {
        SetImageList(bHC ? m_aImagesHighContrast : m_aImagesNormal);
        m_bHighContrast = bHC;
    }
}
