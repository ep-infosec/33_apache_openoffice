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

// INCLUDE -------------------------------------------------------------------

#include <svx/simptabl.hxx>
#include <vcl/svapp.hxx>

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <unotools/intlwrapper.hxx>

// SvxSimpTblContainer  ------------------------------------------------------

SvxSimpTblContainer::SvxSimpTblContainer( Window* pParent, WinBits nWinStyle):
		Control(pParent,nWinStyle)
{
	//Do Nothing;
}

SvxSimpTblContainer::SvxSimpTblContainer( Window* pParent, const ResId& rResId):
		Control(pParent,rResId)
{
	//Do Nothing;
}

long SvxSimpTblContainer::PreNotify( NotifyEvent& rNEvt )
{
    long nResult = sal_True;
    if ( rNEvt.GetType() == EVENT_KEYINPUT )
	{
        const KeyCode& aKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nKey = aKeyCode.GetCode();
        if ( nKey == KEY_TAB )
            GetParent()->Notify( rNEvt );
        else if ( m_pTable->IsFocusOnCellEnabled() && ( nKey == KEY_LEFT || nKey == KEY_RIGHT ) )
            return 0;
		else
            nResult = Control::PreNotify( rNEvt );
	}
	else
        nResult = Control::PreNotify( rNEvt );

	return nResult;
}


// SvxSimpleTable ------------------------------------------------------------

SvxSimpleTable::SvxSimpleTable( Window* pParent,WinBits nBits ):
        SvHeaderTabListBox(pParent,WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
		aPrivContainer(pParent,nBits|WB_DIALOGCONTROL),
		aHeaderBar(pParent,WB_BUTTONSTYLE | WB_BORDER | WB_TABSTOP),
        nHeaderItemId(1),
        bResizeFlag(sal_False),
        bPaintFlag(sal_True)
{
	bSortDirection=sal_True;
	nSortCol=0xFFFF;
	nOldPos=0;

	SetParent(&aPrivContainer);
	aHeaderBar.SetParent(&aPrivContainer);
    aPrivContainer.SetTable( this );

	aHeaderBar.SetStartDragHdl(LINK( this, SvxSimpleTable, StartDragHdl));
	aHeaderBar.SetDragHdl(LINK( this, SvxSimpleTable, DragHdl));
	aHeaderBar.SetEndDragHdl(LINK( this, SvxSimpleTable, EndDragHdl));
	aHeaderBar.SetSelectHdl(LINK( this, SvxSimpleTable, HeaderBarClick));
	aHeaderBar.SetDoubleClickHdl(LINK( this, SvxSimpleTable, HeaderBarDblClick));

    EnableCellFocus();
    DisableTransientChildren();
    InitHeaderBar( &aHeaderBar );

    aHeaderBar.Show();
    SvHeaderTabListBox::Show();
}


SvxSimpleTable::SvxSimpleTable( Window* pParent,const ResId& rResId):
        SvHeaderTabListBox(pParent,WB_CLIPCHILDREN | WB_HSCROLL | WB_TABSTOP),
		aPrivContainer(pParent,rResId),
		aHeaderBar(pParent,WB_BUTTONSTYLE | WB_BORDER  | WB_TABSTOP),
        nHeaderItemId(1),
        bResizeFlag(sal_True),
        bPaintFlag(sal_True)
{

	bSortDirection=sal_True;
	nOldPos=0;
	nSortCol=0xFFFF;

	pMyParentWin=pParent;
	SetParent(&aPrivContainer);
	aHeaderBar.SetParent(&aPrivContainer);
    aPrivContainer.SetTable( this );

	WinBits nBits=aPrivContainer.GetStyle()|WB_DIALOGCONTROL;
	aPrivContainer.SetStyle(nBits);

	aHeaderBar.SetStartDragHdl(LINK( this, SvxSimpleTable, StartDragHdl));
	aHeaderBar.SetDragHdl(LINK( this, SvxSimpleTable, DragHdl));
	aHeaderBar.SetEndDragHdl(LINK( this, SvxSimpleTable, EndDragHdl));
	aHeaderBar.SetSelectHdl(LINK( this, SvxSimpleTable, HeaderBarClick));
	aHeaderBar.SetDoubleClickHdl(LINK( this, SvxSimpleTable, HeaderBarDblClick));

	Size theWinSize=aPrivContainer.GetOutputSizePixel();
	Size HbSize=aHeaderBar.GetSizePixel();

	HbSize.Width()=theWinSize.Width();
	theWinSize.Height()-=HbSize.Height();
	Point	thePos(0,0);

	aHeaderBar.SetPosPixel(thePos);
	aHeaderBar.SetSizePixel(HbSize);

	thePos.Y()+=HbSize.Height();
    SvHeaderTabListBox::SetPosPixel(thePos);
    SvHeaderTabListBox::SetSizePixel(theWinSize);

    EnableCellFocus();
    DisableTransientChildren();
    InitHeaderBar( &aHeaderBar );

    aHeaderBar.Show();
    SvHeaderTabListBox::Show();
}

SvxSimpleTable::~SvxSimpleTable()
{
	SetParent(pMyParentWin);
	aPrivContainer.SetParent(this);
	aHeaderBar.SetParent(this);
}


void SvxSimpleTable::UpdateViewSize()
{
	Size theWinSize=aPrivContainer.GetOutputSizePixel();
	Size HbSize=aHeaderBar.GetSizePixel();

	HbSize.Width()=theWinSize.Width();
	theWinSize.Height()-=HbSize.Height();
	Point	thePos(0,0);

	aHeaderBar.SetPosPixel(thePos);
	aHeaderBar.SetSizePixel(HbSize);

	thePos.Y()+=HbSize.Height();
    SvHeaderTabListBox::SetPosPixel(thePos);
    SvHeaderTabListBox::SetSizePixel(theWinSize);
	Invalidate();
}

void SvxSimpleTable::NotifyScrolled()
{
	long nOffset=-GetXOffset();
	if(nOldPos!=nOffset)
	{
		aHeaderBar.SetOffset(nOffset);
		aHeaderBar.Invalidate();
		aHeaderBar.Update();
		nOldPos=nOffset;
	}
    SvHeaderTabListBox::NotifyScrolled();
}

void SvxSimpleTable::SetTabs()
{
    SvHeaderTabListBox::SetTabs();

	sal_uInt16 nPrivTabCount = TabCount();
	if ( nPrivTabCount )
	{
		if ( nPrivTabCount > aHeaderBar.GetItemCount() )
			nPrivTabCount = aHeaderBar.GetItemCount();

        sal_uInt16 i, nNewSize = static_cast< sal_uInt16 >( GetTab(0) ), nPos = 0;
		for ( i = 1; i < nPrivTabCount; ++i )
		{
            nNewSize = static_cast< sal_uInt16 >( GetTab(i) ) - nPos;
			aHeaderBar.SetItemSize( i, nNewSize );
			nPos = (sal_uInt16)GetTab(i);
		}

		aHeaderBar.SetItemSize( i, HEADERBAR_FULLSIZE ); // because no tab for last entry
	}
}

void SvxSimpleTable::SetTabs( long* pTabs, MapUnit eMapUnit)
{
    SvHeaderTabListBox::SetTabs(pTabs,eMapUnit);
}

void SvxSimpleTable::Paint( const Rectangle& rRect )
{
    SvHeaderTabListBox::Paint(rRect );

	sal_uInt16 nPrivTabCount = TabCount();
	sal_uInt16 nPos = 0;
	sal_uInt16 nNewSize = ( nPrivTabCount > 0 ) ? (sal_uInt16)GetTab(0) : 0;

	long nOffset=-GetXOffset();
	nOldPos=nOffset;

	aHeaderBar.SetOffset(nOffset);
	aHeaderBar.Invalidate();

	if(nPrivTabCount && bPaintFlag)
	{
		if(nPrivTabCount>aHeaderBar.GetItemCount())
				nPrivTabCount=aHeaderBar.GetItemCount();

		for(sal_uInt16 i=1;i<nPrivTabCount;i++)
		{
            nNewSize = static_cast< sal_uInt16 >( GetTab(i) ) - nPos;
            aHeaderBar.SetItemSize( i, nNewSize );
            nPos= static_cast< sal_uInt16 >( GetTab(i) );
		}
	}
	bPaintFlag=sal_True;
}
void SvxSimpleTable::InsertHeaderEntry(const XubString& rText,sal_uInt16 nCol,
									   HeaderBarItemBits nBits)
{
	xub_StrLen nEnd = rText.Search( sal_Unicode( '\t' ) );
	if( nEnd == STRING_NOTFOUND )
	{
		aHeaderBar.InsertItem(nHeaderItemId++, rText, 0, nBits, nCol);
	}
	else
	{
		xub_StrLen nCount = rText.GetTokenCount( sal_Unicode( '\t' ) );

		for( xub_StrLen i=0; i<nCount; i++ )
		{
			String	aString=rText.GetToken(i, sal_Unicode( '\t' ) );
			aHeaderBar.InsertItem(nHeaderItemId++, aString, 0, nBits, nCol);
		}
	}
	SetTabs();
}

void SvxSimpleTable::ClearAll()
{
	aHeaderBar.Clear();
	Clear();
}
void SvxSimpleTable::ClearHeader()
{
	aHeaderBar.Clear();
}

void SvxSimpleTable::ShowTable()
{
	aPrivContainer.Show();
}

void SvxSimpleTable::HideTable()
{
	aPrivContainer.Hide();
}

sal_Bool SvxSimpleTable::IsVisible() const
{
	return aPrivContainer.IsVisible();
}

void SvxSimpleTable::EnableTable()
{
	aPrivContainer.Enable();
}

void SvxSimpleTable::DisableTable()
{
	aPrivContainer.Disable();
}

sal_Bool SvxSimpleTable::IsEnabled() const
{
	return aPrivContainer.IsEnabled();
}

void SvxSimpleTable::TableToTop()
{
	aPrivContainer.ToTop();
}

void SvxSimpleTable::SetPosPixel( const Point& rNewPos )
{
	aPrivContainer.SetPosPixel(rNewPos);
}

Point SvxSimpleTable::GetPosPixel() const
{
	return aPrivContainer.GetPosPixel();
}

void SvxSimpleTable::SetPosSizePixel( const Point& rNewPos, Size& rNewSize )
{
	aPrivContainer.SetPosPixel(rNewPos);
	aPrivContainer.SetSizePixel(rNewSize);
}

void SvxSimpleTable::SetPosSize( const Point& rNewPos, const Size& rNewSize )
{
	aPrivContainer.SetPosPixel(rNewPos);
    SvHeaderTabListBox::SetPosSizePixel(rNewPos,rNewSize);
}

Size SvxSimpleTable::GetSizePixel() const
{
	return aPrivContainer.GetSizePixel();
}

Size SvxSimpleTable::GetOutputSizePixel() const
{
	return aPrivContainer.GetOutputSizePixel();
}

void SvxSimpleTable::SetSizePixel(const Size& rNewSize )
{
	aPrivContainer.SetSizePixel(rNewSize);
	UpdateViewSize();
}

void SvxSimpleTable::SetOutputSizePixel(const Size& rNewSize )
{
	aPrivContainer.SetOutputSizePixel(rNewSize);
	UpdateViewSize();
}

sal_uInt16 SvxSimpleTable::GetSelectedCol()
{
	return (aHeaderBar.GetCurItemId()-1);
}

void SvxSimpleTable::SortByCol(sal_uInt16 nCol,sal_Bool bDir)
{
	bSortDirection=bDir;
	if(nSortCol!=0xFFFF)
		aHeaderBar.SetItemBits(nSortCol+1,HIB_STDSTYLE);

	if (nCol != 0xFFFF)
	{
		if(bDir)
		{
			aHeaderBar.SetItemBits( nCol+1, HIB_STDSTYLE | HIB_DOWNARROW);
			GetModel()->SetSortMode(SortAscending);
		}
		else
		{
			aHeaderBar.SetItemBits( nCol+1, HIB_STDSTYLE | HIB_UPARROW);
			GetModel()->SetSortMode(SortDescending);
		}
		nSortCol=nCol;
		GetModel()->SetCompareHdl( LINK( this, SvxSimpleTable, CompareHdl));
		GetModel()->Resort();
	}
	else
		GetModel()->SetSortMode(SortNone);
	nSortCol=nCol;
}

void SvxSimpleTable::HBarClick()
{
	sal_uInt16 nId=aHeaderBar.GetCurItemId();

	if (aHeaderBar.GetItemBits(nId) & HIB_CLICKABLE)
	{
		if(nId==nSortCol+1)
		{
			SortByCol(nId-1,!bSortDirection);
		}
		else
		{
			SortByCol(nId-1,bSortDirection);
		}

		aHeaderBarClickLink.Call(this);
	}
}

void SvxSimpleTable::HBarDblClick()
{
	aHeaderBarDblClickLink.Call(this);
}

void SvxSimpleTable::HBarStartDrag()
{
	if(!aHeaderBar.IsItemMode())
	{
		Rectangle aSizeRect(Point(0,0),
            SvHeaderTabListBox::GetOutputSizePixel());
		aSizeRect.Left()=-GetXOffset()+aHeaderBar.GetDragPos();
		aSizeRect.Right()=-GetXOffset()+aHeaderBar.GetDragPos();
		ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
	}
}
void SvxSimpleTable::HBarDrag()
{
	HideTracking();
	if(!aHeaderBar.IsItemMode())
	{
		Rectangle aSizeRect(Point(0,0),
            SvHeaderTabListBox::GetOutputSizePixel());
		aSizeRect.Left()=-GetXOffset()+aHeaderBar.GetDragPos();
		aSizeRect.Right()=-GetXOffset()+aHeaderBar.GetDragPos();
		ShowTracking( aSizeRect, SHOWTRACK_SPLIT );
	}
}
void SvxSimpleTable::HBarEndDrag()
{
	HideTracking();
	sal_uInt16 nPrivTabCount=TabCount();
	sal_uInt16 nPos=0;
	sal_uInt16 nNewSize=0;

	if(nPrivTabCount)
	{
		if(nPrivTabCount>aHeaderBar.GetItemCount())
				nPrivTabCount=aHeaderBar.GetItemCount();

		//for(sal_uInt16 i=1;i<=nPrivTabCount;i++)
		for(sal_uInt16 i=1;i<nPrivTabCount;i++)
		{
            nNewSize = static_cast< sal_uInt16 >( aHeaderBar.GetItemSize(i) ) + nPos;
            SetTab( i, nNewSize, MAP_PIXEL );
            nPos = nNewSize;
		}
	}
	bPaintFlag=sal_False;
	Invalidate();
	Update();
}

CommandEvent SvxSimpleTable::GetCommandEvent() const
{
	return aCEvt;
}

void SvxSimpleTable::Command( const CommandEvent& rCEvt )
{
	aCEvt=rCEvt;
	aCommandLink.Call(this);
    SvHeaderTabListBox::Command(rCEvt);
}

IMPL_LINK( SvxSimpleTable, StartDragHdl, HeaderBar*, pCtr)
{
	if(pCtr==&aHeaderBar)
	{
		HBarStartDrag();
	}
	return 0;
}

IMPL_LINK( SvxSimpleTable, DragHdl, HeaderBar*, pCtr)
{
	if(pCtr==&aHeaderBar)
	{
		HBarDrag();
	}
	return 0;
}

IMPL_LINK( SvxSimpleTable, EndDragHdl, HeaderBar*, pCtr)
{
	if(pCtr==&aHeaderBar)
	{
		HBarEndDrag();
	}
	return 0;
}

IMPL_LINK( SvxSimpleTable, HeaderBarClick, HeaderBar*, pCtr)
{
	if(pCtr==&aHeaderBar)
	{
		HBarClick();
	}
	return 0;
}

IMPL_LINK( SvxSimpleTable, HeaderBarDblClick, HeaderBar*, pCtr)
{
	if(pCtr==&aHeaderBar)
	{
		HBarDblClick();
	}
	return 0;
}

SvLBoxItem* SvxSimpleTable::GetEntryAtPos( SvLBoxEntry* pEntry, sal_uInt16 nPos ) const
{
	DBG_ASSERT(pEntry,"GetEntryText:Invalid Entry");
	SvLBoxItem* pItem = NULL;

	if( pEntry )
	{
		sal_uInt16 nCount = pEntry->ItemCount();

		nPos++;

		if( nTreeFlags & TREEFLAG_CHKBTN ) nPos++;

		if( nPos < nCount )
		{
			pItem = pEntry->GetItem( nPos);
		}
	}
	return pItem;
}

StringCompare SvxSimpleTable::ColCompare(SvLBoxEntry* pLeft,SvLBoxEntry* pRight)
{
	StringCompare eCompare=COMPARE_EQUAL;

	SvLBoxItem* pLeftItem = GetEntryAtPos( pLeft, nSortCol);
	SvLBoxItem* pRightItem = GetEntryAtPos( pRight, nSortCol);


	if(pLeftItem != NULL && pRightItem != NULL)
	{
		sal_uInt16 nLeftKind=pLeftItem->IsA();
		sal_uInt16 nRightKind=pRightItem->IsA();

		if(nRightKind == SV_ITEM_ID_LBOXSTRING &&
			nLeftKind == SV_ITEM_ID_LBOXSTRING )
		{
			IntlWrapper aIntlWrapper( ::comphelper::getProcessServiceFactory(), Application::GetSettings().GetLocale() );
			const CollatorWrapper* pCollator = aIntlWrapper.getCaseCollator();

			eCompare=(StringCompare)pCollator->compareString( ((SvLBoxString*)pLeftItem)->GetText(),
									((SvLBoxString*)pRightItem)->GetText());

			if(eCompare==COMPARE_EQUAL) eCompare=COMPARE_LESS;
		}
	}
	return eCompare;
}

IMPL_LINK( SvxSimpleTable, CompareHdl, SvSortData*, pData)
{
	SvLBoxEntry* pLeft = (SvLBoxEntry*)(pData->pLeft );
	SvLBoxEntry* pRight = (SvLBoxEntry*)(pData->pRight );
	return (long) ColCompare(pLeft,pRight);
}


