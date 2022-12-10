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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/sound.hxx>

#include "gridwin.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "viewdata.hxx"
#include "pivot.hxx"
//CHINA001 #include "pfiltdlg.hxx"
#include "uiitems.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "pagedata.hxx"
#include "dpobject.hxx"
#include "dpsave.hxx"
#include "dpoutput.hxx"		// ScDPPositionData
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "dpcontrol.hxx"
#include "dpcontrol.hrc"
#include "strload.hxx"
#include "userlist.hxx"

#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include "scabstdlg.hxx" //CHINA001

#include <vector>
#include <hash_map>

using namespace com::sun::star;
using ::com::sun::star::sheet::DataPilotFieldOrientation;
using ::std::vector;
using ::std::auto_ptr;
using ::std::hash_map;
using ::rtl::OUString;
using ::rtl::OUStringHash;

// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

DataPilotFieldOrientation ScGridWindow::GetDPFieldOrientation( SCCOL nCol, SCROW nRow ) const
{
    using namespace ::com::sun::star::sheet;

    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    ScDPObject* pDPObj = pDoc->GetDPAtCursor(nCol, nRow, nTab);
    if (!pDPObj)
        return DataPilotFieldOrientation_HIDDEN;

    sal_uInt16 nOrient = DataPilotFieldOrientation_HIDDEN;

    // Check for page field first.
    if (nCol > 0)
    {
        // look for the dimension header left of the drop-down arrow
        long nField = pDPObj->GetHeaderDim( ScAddress( nCol-1, nRow, nTab ), nOrient );
        if ( nField >= 0 && nOrient == DataPilotFieldOrientation_PAGE )
        {
            sal_Bool bIsDataLayout = sal_False;
            String aFieldName = pDPObj->GetDimName( nField, bIsDataLayout );
            if ( aFieldName.Len() && !bIsDataLayout )
                return DataPilotFieldOrientation_PAGE;
        }
    }

    nOrient = sheet::DataPilotFieldOrientation_HIDDEN;

    // Now, check for row/column field.
    long nField = pDPObj->GetHeaderDim(ScAddress(nCol, nRow, nTab), nOrient);
    if (nField >= 0 && (nOrient == DataPilotFieldOrientation_COLUMN || nOrient == DataPilotFieldOrientation_ROW) )
    {
        sal_Bool bIsDataLayout = sal_False;
        String aFieldName = pDPObj->GetDimName(nField, bIsDataLayout);
        if (aFieldName.Len() && !bIsDataLayout)
            return static_cast<DataPilotFieldOrientation>(nOrient);
    }

    return DataPilotFieldOrientation_HIDDEN;
}

// private method for mouse button handling
sal_Bool ScGridWindow::DoPageFieldSelection( SCCOL nCol, SCROW nRow )
{
    if (GetDPFieldOrientation( nCol, nRow ) == sheet::DataPilotFieldOrientation_PAGE)
    {
        LaunchPageFieldMenu( nCol, nRow );
        return sal_True;
    }
	return sal_False;
}

bool ScGridWindow::DoAutoFilterButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt )
{
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    Point aScrPos  = pViewData->GetScrPos(nCol, nRow, eWhich);
    Point aDiffPix = rMEvt.GetPosPixel();

    aDiffPix -= aScrPos;
    sal_Bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );
    if ( bLayoutRTL )
        aDiffPix.X() = -aDiffPix.X();

    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel( nCol, nRow, nSizeX, nSizeY );
    // The button height should not use the merged cell height, should still use single row height
    nSizeY = pViewData->ToPixel(pDoc->GetRowHeight(nRow, nTab), pViewData->GetPPTY());
    Size aScrSize(nSizeX-1, nSizeY-1);

    // Check if the mouse cursor is clicking on the popup arrow box.
    mpFilterButton.reset(new ScDPFieldButton(this, &GetSettings().GetStyleSettings(), &pViewData->GetZoomX(), &pViewData->GetZoomY(), pDoc));
    mpFilterButton->setBoundingBox(aScrPos, aScrSize, bLayoutRTL);
    mpFilterButton->setPopupLeft(bLayoutRTL);   // #i114944# AutoFilter button is left-aligned in RTL
    Point aPopupPos;
    Size aPopupSize;
    mpFilterButton->getPopupBoundingBox(aPopupPos, aPopupSize);
    Rectangle aRec(aPopupPos, aPopupSize);
    if (aRec.IsInside(rMEvt.GetPosPixel()))
    {
        if ( DoPageFieldSelection( nCol, nRow ) )
            return true;

        bool bFilterActive = IsAutoFilterActive(nCol, nRow, nTab);
        mpFilterButton->setHasHiddenMember(bFilterActive);
        mpFilterButton->setDrawBaseButton(false);
        mpFilterButton->setDrawPopupButton(true);
        mpFilterButton->setPopupPressed(true);
        HideCursor();
        mpFilterButton->draw();
        ShowCursor();
        DoAutoFilterMenue(nCol, nRow, false);
        return true;
    }

    return false;
}

void ScGridWindow::DoPushButton( SCCOL nCol, SCROW nRow, const MouseEvent& rMEvt )
{
	ScDocument* pDoc = pViewData->GetDocument();
	SCTAB nTab = pViewData->GetTabNo();

	ScDPObject*	pDPObj	= pDoc->GetDPAtCursor(nCol, nRow, nTab);

	if (pDPObj)
	{
		sal_uInt16 nOrient = sheet::DataPilotFieldOrientation_HIDDEN;
		ScAddress aPos( nCol, nRow, nTab );
		long nField = pDPObj->GetHeaderDim( aPos, nOrient );
		if ( nField >= 0 )
		{
			bDPMouse   = sal_True;
			nDPField   = nField;
			pDragDPObj = pDPObj;

            if (DPTestFieldPopupArrow(rMEvt, aPos, pDPObj))
            {    
                // field name pop up menu has been launched.  Don't activate 
                // field move.
                bDPMouse = false;
                return;
            }

			DPTestMouse( rMEvt, sal_True );
			StartTracking();
		}
		else if ( pDPObj->IsFilterButton(aPos) )
		{
			ReleaseMouse();			// may have been captured in ButtonDown

			ScQueryParam aQueryParam;
			SCTAB nSrcTab = 0;
			const ScSheetSourceDesc* pDesc = pDPObj->GetSheetDesc();
			DBG_ASSERT(pDesc, "no sheet source for filter button");
			if (pDesc)
			{
				aQueryParam = pDesc->aQueryParam;
				nSrcTab = pDesc->aSourceRange.aStart.Tab();
			}

			SfxItemSet aArgSet( pViewData->GetViewShell()->GetPool(),
										SCITEM_QUERYDATA, SCITEM_QUERYDATA );
			aArgSet.Put( ScQueryItem( SCITEM_QUERYDATA, pViewData, &aQueryParam ) );

//CHINA001			ScPivotFilterDlg* pDlg = new ScPivotFilterDlg(
//CHINA001			pViewData->GetViewShell()->GetDialogParent(),
//CHINA001			aArgSet, nSrcTab );
			ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
			DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

			AbstractScPivotFilterDlg* pDlg = pFact->CreateScPivotFilterDlg( pViewData->GetViewShell()->GetDialogParent(), 
																			aArgSet, nSrcTab,
																			RID_SCDLG_PIVOTFILTER);
			DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
			if ( pDlg->Execute() == RET_OK )
			{
				ScSheetSourceDesc aNewDesc;
				if (pDesc)
					aNewDesc = *pDesc;

				const ScQueryItem& rQueryItem = pDlg->GetOutputItem();
				aNewDesc.aQueryParam = rQueryItem.GetQueryData();

				ScDPObject aNewObj( *pDPObj );
				aNewObj.SetSheetDesc( aNewDesc );
				ScDBDocFunc aFunc( *pViewData->GetDocShell() );
				aFunc.DataPilotUpdate( pDPObj, &aNewObj, sal_True, sal_False );
				pViewData->GetView()->CursorPosChanged();		// shells may be switched
			}
			delete pDlg;
		}
		else
			Sound::Beep();
	}
	else
	{
		DBG_ERROR("Da is ja garnix");
	}
}

// -----------------------------------------------------------------------
//
//	Data Pilot interaction
//

void ScGridWindow::DPTestMouse( const MouseEvent& rMEvt, sal_Bool bMove )
{
	DBG_ASSERT(pDragDPObj, "pDragDPObj missing");

	//	scroll window if at edges
	//!	move this to separate method

	sal_Bool bTimer = sal_False;
	Point aPixel = rMEvt.GetPosPixel();

	SCsCOL nDx = 0;
	SCsROW nDy = 0;
	if ( aPixel.X() < 0 )
		nDx = -1;
	if ( aPixel.Y() < 0 )
		nDy = -1;
	Size aSize = GetOutputSizePixel();
	if ( aPixel.X() >= aSize.Width() )
		nDx = 1;
	if ( aPixel.Y() >= aSize.Height() )
		nDy = 1;
	if ( nDx != 0 || nDy != 0 )
	{
		UpdateDragRect( sal_False, Rectangle() );

		if ( nDx  != 0)
			pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
		if ( nDy != 0 )
			pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );

		bTimer = sal_True;
	}

	//	---

	SCsCOL	nPosX;
	SCsROW	nPosY;
	pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), eWhich, nPosX, nPosY );
	sal_Bool	bMouseLeft;
	sal_Bool	bMouseTop;
	pViewData->GetMouseQuadrant( aPixel, eWhich, nPosX, nPosY, bMouseLeft, bMouseTop );

	ScAddress aPos( nPosX, nPosY, pViewData->GetTabNo() );

	Rectangle aPosRect;
	sal_uInt16 nOrient;
	long nDimPos;
	sal_Bool bHasRange = pDragDPObj->GetHeaderDrag( aPos, bMouseLeft, bMouseTop, nDPField,
												aPosRect, nOrient, nDimPos );
	UpdateDragRect( bHasRange && bMove, aPosRect );

    sal_Bool bIsDataLayout;
    sal_Int32 nDimFlags = 0;
    String aDimName = pDragDPObj->GetDimName( nDPField, bIsDataLayout, &nDimFlags );
    bool bAllowed = !bHasRange || ScDPObject::IsOrientationAllowed( nOrient, nDimFlags );

	if (bMove)			// set mouse pointer
	{
        PointerStyle ePointer = POINTER_PIVOT_DELETE;
        if ( !bAllowed )
            ePointer = POINTER_NOTALLOWED;
        else if ( bHasRange )
			switch (nOrient)
			{
				case sheet::DataPilotFieldOrientation_COLUMN: ePointer = POINTER_PIVOT_COL;	break;
				case sheet::DataPilotFieldOrientation_ROW:	  ePointer = POINTER_PIVOT_ROW;	break;
				case sheet::DataPilotFieldOrientation_PAGE:
				case sheet::DataPilotFieldOrientation_DATA:	  ePointer = POINTER_PIVOT_FIELD;	break;
			}
		SetPointer( ePointer );
	}
	else				// execute change
	{
		if (!bHasRange)
			nOrient = sheet::DataPilotFieldOrientation_HIDDEN;

        if ( bIsDataLayout && ( nOrient != sheet::DataPilotFieldOrientation_COLUMN &&
                                nOrient != sheet::DataPilotFieldOrientation_ROW ) )
		{
			//	removing data layout is not allowed
			pViewData->GetView()->ErrorMessage(STR_PIVOT_MOVENOTALLOWED);
		}
        else if ( bAllowed )
		{
			ScDPSaveData aSaveData( *pDragDPObj->GetSaveData() );

			ScDPSaveDimension* pDim;
			if ( bIsDataLayout )
				pDim = aSaveData.GetDataLayoutDimension();
			else
				pDim = aSaveData.GetDimensionByName(aDimName);
			pDim->SetOrientation( nOrient );
			aSaveData.SetPosition( pDim, nDimPos );

			//!	docfunc method with ScDPSaveData as argument?

			ScDPObject aNewObj( *pDragDPObj );
			aNewObj.SetSaveData( aSaveData );
			ScDBDocFunc aFunc( *pViewData->GetDocShell() );
			// when dragging fields, allow re-positioning (bAllowMove)
			aFunc.DataPilotUpdate( pDragDPObj, &aNewObj, sal_True, sal_False, sal_True );
			pViewData->GetView()->CursorPosChanged();		// shells may be switched
		}
	}

	if (bTimer && bMove)
		pViewData->GetView()->SetTimer( this, rMEvt );			// repeat event
	else
		pViewData->GetView()->ResetTimer();
}

bool ScGridWindow::DPTestFieldPopupArrow(const MouseEvent& rMEvt, const ScAddress& rPos, ScDPObject* pDPObj)
{
    sal_Bool bLayoutRTL = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );

    // Get the geometry of the cell.
    Point aScrPos = pViewData->GetScrPos(rPos.Col(), rPos.Row(), eWhich);
    long nSizeX, nSizeY;
    pViewData->GetMergeSizePixel(rPos.Col(), rPos.Row(), nSizeX, nSizeY);
    Size aScrSize(nSizeX-1, nSizeY-1);

    // Check if the mouse cursor is clicking on the popup arrow box.
    ScDPFieldButton aBtn(this, &GetSettings().GetStyleSettings());
    aBtn.setBoundingBox(aScrPos, aScrSize, bLayoutRTL);
    aBtn.setPopupLeft(false);   // DataPilot popup is always right-aligned for now
    Point aPopupPos;
    Size aPopupSize;
    aBtn.getPopupBoundingBox(aPopupPos, aPopupSize);
    Rectangle aRec(aPopupPos, aPopupSize);
    if (aRec.IsInside(rMEvt.GetPosPixel()))
    {
        // Mouse cursor inside the popup arrow box.  Launch the field menu.
        DPLaunchFieldPopupMenu(OutputToScreenPixel(aScrPos), aScrSize, rPos, pDPObj);
        return true;
    }

    return false;
}

namespace {

struct DPFieldPopupData : public ScDPFieldPopupWindow::ExtendedData
{
    ScPivotParam    maDPParam;
    ScDPObject*     mpDPObj;
    long            mnDim;
};

class DPFieldPopupOKAction : public ScMenuFloatingWindow::Action
{
public:
    explicit DPFieldPopupOKAction(ScGridWindow* p) : 
        mpGridWindow(p) {}

    virtual void execute()
    {
        mpGridWindow->UpdateDPFromFieldPopupMenu();
    }
private:
    ScGridWindow* mpGridWindow;
};

class PopupSortAction : public ScMenuFloatingWindow::Action
{
public:
    enum SortType { ASCENDING, DESCENDING, CUSTOM };

    explicit PopupSortAction(const ScAddress& rPos, SortType eType, sal_uInt16 nUserListIndex, ScTabViewShell* pViewShell) :
        maPos(rPos), meType(eType), mnUserListIndex(nUserListIndex), mpViewShell(pViewShell) {}

    virtual void execute()
    {
        switch (meType)
        {
            case ASCENDING:
                mpViewShell->DataPilotSort(maPos, true);
            break;
            case DESCENDING:
                mpViewShell->DataPilotSort(maPos, false);
            break;
            case CUSTOM:
                mpViewShell->DataPilotSort(maPos, true, &mnUserListIndex);
            break;
            default:
                ;
        }
    }

private:
    ScAddress       maPos;
    SortType        meType;
    sal_uInt16      mnUserListIndex;
    ScTabViewShell* mpViewShell;
};

}

bool lcl_GetLabelIndex( size_t& rLabelIndex, long nDimension, const ScDPLabelDataVector& rLabelArray )
{
    size_t n = rLabelArray.size();
    for (size_t i = 0; i < n; ++i)
        if (static_cast<long>(rLabelArray[i].mnCol) == nDimension)
        {
            rLabelIndex = i;
            return true;
        }
    return false;    
}

void ScGridWindow::DPLaunchFieldPopupMenu(
    const Point& rScrPos, const Size& rScrSize, const ScAddress& rPos, ScDPObject* pDPObj)
{
    // We need to get the list of field members.
    auto_ptr<DPFieldPopupData> pDPData(new DPFieldPopupData);
    pDPObj->FillLabelData(pDPData->maDPParam);
    pDPData->mpDPObj = pDPObj;

    sal_uInt16 nOrient;
    pDPData->mnDim = pDPObj->GetHeaderDim(rPos, nOrient);

    // #i116457# FillLabelData skips empty column names, so mnDim can't be used directly as index into maLabelArray.
    size_t nLabelIndex = 0;
    if (!lcl_GetLabelIndex( nLabelIndex, pDPData->mnDim, pDPData->maDPParam.maLabelArray ))
        return;

    const ScDPLabelData& rLabelData = pDPData->maDPParam.maLabelArray[nLabelIndex];

    mpDPFieldPopup.reset(new ScDPFieldPopupWindow(this, pViewData->GetDocument()));
    mpDPFieldPopup->setName(OUString::createFromAscii("Pivot table field member popup"));
    mpDPFieldPopup->setExtendedData(pDPData.release());
    mpDPFieldPopup->setOKAction(new DPFieldPopupOKAction(this));
    {
        // Populate field members.
        size_t n = rLabelData.maMembers.size();
        mpDPFieldPopup->setMemberSize(n);
        for (size_t i = 0; i < n; ++i)
        {
            const ScDPLabelData::Member& rMem = rLabelData.maMembers[i];
            mpDPFieldPopup->addMember(rMem.getDisplayName(), rMem.mbVisible);
        }
        mpDPFieldPopup->initMembers();
    }

    vector<OUString> aUserSortNames;
    ScUserList* pUserList = ScGlobal::GetUserList();
    if (pUserList)
    {
        sal_uInt16 n = pUserList->GetCount();
        aUserSortNames.reserve(n);
        for (sal_uInt16 i = 0; i < n; ++i)
        {
            ScUserListData* pData = static_cast<ScUserListData*>((*pUserList)[i]);
            aUserSortNames.push_back(pData->GetString());
        }
    }

    // Populate the menus.
    ScTabViewShell* pViewShell = pViewData->GetViewShell();
    mpDPFieldPopup->addMenuItem(
        ScRscStrLoader(RID_POPUP_FILTER, STR_MENU_SORT_ASC).GetString(), true, 
        new PopupSortAction(rPos, PopupSortAction::ASCENDING, 0, pViewShell));
    mpDPFieldPopup->addMenuItem(
        ScRscStrLoader(RID_POPUP_FILTER, STR_MENU_SORT_DESC).GetString(), true,
        new PopupSortAction(rPos, PopupSortAction::DESCENDING, 0, pViewShell));
    ScMenuFloatingWindow* pSubMenu = mpDPFieldPopup->addSubMenuItem(
        ScRscStrLoader(RID_POPUP_FILTER, STR_MENU_SORT_CUSTOM).GetString(), !aUserSortNames.empty());

    if (pSubMenu && !aUserSortNames.empty())
    {
        size_t n = aUserSortNames.size();
        for (size_t i = 0; i < n; ++i)
        {    
            pSubMenu->addMenuItem(
                aUserSortNames[i], true, 
                new PopupSortAction(rPos, PopupSortAction::CUSTOM, static_cast<sal_uInt16>(i), pViewShell));
        }
    }

    sal_Bool bLayoutRTL = pViewData->GetDocument()->IsLayoutRTL( pViewData->GetTabNo() );

    Rectangle aCellRect(rScrPos, rScrSize);
    const Size& rPopupSize = mpDPFieldPopup->getWindowSize();
    if (bLayoutRTL)
    {
        // RTL: rScrPos is logical-left (visual right) position, always right-align with that
        aCellRect.SetPos(Point(rScrPos.X() - rPopupSize.Width() + 1, rScrPos.Y()));
    }
    else if (rScrSize.getWidth() > rPopupSize.getWidth())
    {
        // If the cell width is larger than the popup window width, launch it 
        // right-aligned with the cell.
        long nXOffset = rScrSize.getWidth() - rPopupSize.getWidth();
        aCellRect.SetPos(Point(rScrPos.X() + nXOffset, rScrPos.Y()));
    }
    mpDPFieldPopup->SetPopupModeEndHdl( LINK(this, ScGridWindow, PopupModeEndHdl) );
    mpDPFieldPopup->StartPopupMode(aCellRect, (FLOATWIN_POPUPMODE_DOWN | FLOATWIN_POPUPMODE_GRABFOCUS));
}

void ScGridWindow::UpdateDPFromFieldPopupMenu()
{
    typedef hash_map<OUString, OUString, OUStringHash> MemNameMapType;
    typedef hash_map<OUString, bool, OUStringHash> MemVisibilityType;

    if (!mpDPFieldPopup.get())
        return;

    DPFieldPopupData* pDPData = static_cast<DPFieldPopupData*>(mpDPFieldPopup->getExtendedData());
    if (!pDPData)
        return;

    ScDPObject* pDPObj = pDPData->mpDPObj;
    ScDPObject aNewDPObj(*pDPObj);
    aNewDPObj.BuildAllDimensionMembers();
    ScDPSaveData* pSaveData = aNewDPObj.GetSaveData();

    sal_Bool bIsDataLayout;
    String aDimName = pDPObj->GetDimName(pDPData->mnDim, bIsDataLayout);
    ScDPSaveDimension* pDim = pSaveData->GetDimensionByName(aDimName);
    if (!pDim)
        return;

    size_t nLabelIndex = 0;
    lcl_GetLabelIndex( nLabelIndex, pDPData->mnDim, pDPData->maDPParam.maLabelArray );

    // Build a map of layout names to original names.
    const ScDPLabelData& rLabelData = pDPData->maDPParam.maLabelArray[nLabelIndex];
    MemNameMapType aMemNameMap;
    for (vector<ScDPLabelData::Member>::const_iterator itr = rLabelData.maMembers.begin(), itrEnd = rLabelData.maMembers.end();
           itr != itrEnd; ++itr)
        aMemNameMap.insert(MemNameMapType::value_type(itr->maLayoutName, itr->maName));

    // The raw result may contain a mixture of layout names and original names.
    MemVisibilityType aRawResult;
    mpDPFieldPopup->getResult(aRawResult);

    MemVisibilityType aResult;
    for (MemVisibilityType::const_iterator itr = aRawResult.begin(), itrEnd = aRawResult.end(); itr != itrEnd; ++itr)
    {
        MemNameMapType::const_iterator itrNameMap = aMemNameMap.find(itr->first);
        if (itrNameMap == aMemNameMap.end())
            // This is an original member name.  Use it as-is.
            aResult.insert(MemVisibilityType::value_type(itr->first, itr->second));
        else
        {
            // This is a layout name.  Get the original member name and use it.
            aResult.insert(MemVisibilityType::value_type(itrNameMap->second, itr->second));
        }
    }
    pDim->UpdateMemberVisibility(aResult);

    ScDBDocFunc aFunc(*pViewData->GetDocShell());
    aFunc.DataPilotUpdate(pDPObj, &aNewDPObj, true, false);
}

void ScGridWindow::DPMouseMove( const MouseEvent& rMEvt )
{
	DPTestMouse( rMEvt, sal_True );
}

void ScGridWindow::DPMouseButtonUp( const MouseEvent& rMEvt )
{
	bDPMouse = sal_False;
	ReleaseMouse();

	DPTestMouse( rMEvt, sal_False );
	SetPointer( Pointer( POINTER_ARROW ) );
}

// -----------------------------------------------------------------------

void ScGridWindow::UpdateDragRect( sal_Bool bShowRange, const Rectangle& rPosRect )
{
	SCCOL nStartX = ( rPosRect.Left()   >= 0 ) ? static_cast<SCCOL>(rPosRect.Left())   : SCCOL_MAX;
	SCROW nStartY = ( rPosRect.Top()    >= 0 ) ? static_cast<SCROW>(rPosRect.Top())    : SCROW_MAX;
	SCCOL nEndX   = ( rPosRect.Right()  >= 0 ) ? static_cast<SCCOL>(rPosRect.Right())  : SCCOL_MAX;
	SCROW nEndY   = ( rPosRect.Bottom() >= 0 ) ? static_cast<SCROW>(rPosRect.Bottom()) : SCROW_MAX;

	if ( bShowRange == bDragRect && nDragStartX == nStartX && nDragEndX == nEndX &&
									nDragStartY == nStartY && nDragEndY == nEndY )
	{
		return;			// everything unchanged
	}

	// if ( bDragRect )
	//	DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, sal_False );
	if ( bShowRange )
	{
		nDragStartX = nStartX;
		nDragStartY = nStartY;
		nDragEndX = nEndX;
		nDragEndY = nEndY;
		bDragRect = sal_True;
		// DrawDragRect( nDragStartX, nDragStartY, nDragEndX, nDragEndY, sal_False );
	}
	else
		bDragRect = sal_False;

    UpdateDragRectOverlay();
}

// -----------------------------------------------------------------------

//	Page-Break-Modus

sal_uInt16 ScGridWindow::HitPageBreak( const Point& rMouse, ScRange* pSource,
									SCCOLROW* pBreak, SCCOLROW* pPrev )
{
	sal_uInt16 nFound = SC_PD_NONE;		// 0
	ScRange aSource;
	SCCOLROW nBreak = 0;
	SCCOLROW nPrev = 0;

	ScPageBreakData* pPageData = pViewData->GetView()->GetPageBreakData();
	if ( pPageData )
	{
		sal_Bool bHori = sal_False;
		sal_Bool bVert = sal_False;
        SCCOL nHitX = 0;
        SCROW nHitY = 0;

		long nMouseX = rMouse.X();
		long nMouseY = rMouse.Y();
        SCsCOL nPosX;
        SCsROW nPosY;
        pViewData->GetPosFromPixel( nMouseX, nMouseY, eWhich, nPosX, nPosY );
		Point aTL = pViewData->GetScrPos( nPosX, nPosY, eWhich );
		Point aBR = pViewData->GetScrPos( nPosX+1, nPosY+1, eWhich );

		//	Horizontal mehr Toleranz als vertikal, weil mehr Platz ist
		if ( nMouseX <= aTL.X() + 4 )
		{
			bHori = sal_True;
			nHitX = nPosX;
		}
		else if ( nMouseX >= aBR.X() - 6 )
		{
			bHori = sal_True;
			nHitX = nPosX+1;					// linker Rand der naechsten Zelle
		}
		if ( nMouseY <= aTL.Y() + 2 )
		{
			bVert = sal_True;
			nHitY = nPosY;
		}
		else if ( nMouseY >= aBR.Y() - 4 )
		{
			bVert = sal_True;
			nHitY = nPosY+1;					// oberer Rand der naechsten Zelle
		}

		if ( bHori || bVert )
		{
            sal_uInt16 nCount = sal::static_int_cast<sal_uInt16>( pPageData->GetCount() );
			for (sal_uInt16 nPos=0; nPos<nCount && !nFound; nPos++)
			{
				ScPrintRangeData& rData = pPageData->GetData(nPos);
				ScRange aRange = rData.GetPrintRange();
				sal_Bool bLHit = ( bHori && nHitX == aRange.aStart.Col() );
				sal_Bool bRHit = ( bHori && nHitX == aRange.aEnd.Col() + 1 );
				sal_Bool bTHit = ( bVert && nHitY == aRange.aStart.Row() );
				sal_Bool bBHit = ( bVert && nHitY == aRange.aEnd.Row() + 1 );
				sal_Bool bInsideH = ( nPosX >= aRange.aStart.Col() && nPosX <= aRange.aEnd.Col() );
				sal_Bool bInsideV = ( nPosY >= aRange.aStart.Row() && nPosY <= aRange.aEnd.Row() );

				if ( bLHit )
				{
					if ( bTHit )
						nFound = SC_PD_RANGE_TL;
					else if ( bBHit )
						nFound = SC_PD_RANGE_BL;
					else if ( bInsideV )
						nFound = SC_PD_RANGE_L;
				}
				else if ( bRHit )
				{
					if ( bTHit )
						nFound = SC_PD_RANGE_TR;
					else if ( bBHit )
						nFound = SC_PD_RANGE_BR;
					else if ( bInsideV )
						nFound = SC_PD_RANGE_R;
				}
				else if ( bTHit && bInsideH )
					nFound = SC_PD_RANGE_T;
				else if ( bBHit && bInsideH )
					nFound = SC_PD_RANGE_B;
				if (nFound)
					aSource = aRange;

				//	Umbrueche

				if ( bVert && bInsideH && !nFound )
				{
					size_t nRowCount = rData.GetPagesY();
					const SCROW* pRowEnd = rData.GetPageEndY();
					for (size_t nRowPos=0; nRowPos+1<nRowCount; nRowPos++)
						if ( pRowEnd[nRowPos]+1 == nHitY )
						{
							nFound = SC_PD_BREAK_V;
							aSource = aRange;
							nBreak = nHitY;
							if ( nRowPos )
								nPrev = pRowEnd[nRowPos-1]+1;
							else
								nPrev = aRange.aStart.Row();
						}
				}
				if ( bHori && bInsideV && !nFound )
				{
					size_t nColCount = rData.GetPagesX();
					const SCCOL* pColEnd = rData.GetPageEndX();
					for (size_t nColPos=0; nColPos+1<nColCount; nColPos++)
						if ( pColEnd[nColPos]+1 == nHitX )
						{
							nFound = SC_PD_BREAK_H;
							aSource = aRange;
							nBreak = nHitX;
							if ( nColPos )
								nPrev = pColEnd[nColPos-1]+1;
							else
								nPrev = aRange.aStart.Col();
						}
				}
			}
		}
	}

	if (pSource)
		*pSource = aSource;		// Druckbereich
	if (pBreak)
		*pBreak = nBreak;		// X/Y Position des verchobenen Seitenumbruchs
	if (pPrev)
		*pPrev = nPrev;			// X/Y Anfang der Seite, die am Umbruch zuende ist
	return nFound;
}

void ScGridWindow::PagebreakMove( const MouseEvent& rMEvt, sal_Bool bUp )
{
	//!	Scrolling und Umschalten mit RFMouseMove zusammenfassen !
	//!	(Weginvertieren vor dem Scrolling ist anders)

	//	Scrolling

	sal_Bool bTimer = sal_False;
	Point aPos = rMEvt.GetPosPixel();
	SCsCOL nDx = 0;
	SCsROW nDy = 0;
	if ( aPos.X() < 0 ) nDx = -1;
	if ( aPos.Y() < 0 ) nDy = -1;
	Size aSize = GetOutputSizePixel();
	if ( aPos.X() >= aSize.Width() )
		nDx = 1;
	if ( aPos.Y() >= aSize.Height() )
		nDy = 1;
	if ( nDx != 0 || nDy != 0 )
	{
		if ( bPagebreakDrawn )			// weginvertieren
		{
			// DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
			//				aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), sal_False );
			bPagebreakDrawn = sal_False;
            UpdateDragRectOverlay();
		}

		if ( nDx != 0 ) pViewData->GetView()->ScrollX( nDx, WhichH(eWhich) );
		if ( nDy != 0 ) pViewData->GetView()->ScrollY( nDy, WhichV(eWhich) );
		bTimer = sal_True;
	}

	//	Umschalten bei Fixierung (damit Scrolling funktioniert)

	if ( eWhich == pViewData->GetActivePart() )		//??
	{
		if ( pViewData->GetHSplitMode() == SC_SPLIT_FIX )
			if ( nDx > 0 )
			{
				if ( eWhich == SC_SPLIT_TOPLEFT )
					pViewData->GetView()->ActivatePart( SC_SPLIT_TOPRIGHT );
				else if ( eWhich == SC_SPLIT_BOTTOMLEFT )
					pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
			}

		if ( pViewData->GetVSplitMode() == SC_SPLIT_FIX )
			if ( nDy > 0 )
			{
				if ( eWhich == SC_SPLIT_TOPLEFT )
					pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMLEFT );
				else if ( eWhich == SC_SPLIT_TOPRIGHT )
					pViewData->GetView()->ActivatePart( SC_SPLIT_BOTTOMRIGHT );
			}
	}

	//	ab hier neu

	//	gesucht wird eine Position zwischen den Zellen (vor nPosX / nPosY)
	SCsCOL nPosX;
	SCsROW nPosY;
	pViewData->GetPosFromPixel( aPos.X(), aPos.Y(), eWhich, nPosX, nPosY );
	sal_Bool bLeft, bTop;
	pViewData->GetMouseQuadrant( aPos, eWhich, nPosX, nPosY, bLeft, bTop );
	if ( !bLeft ) ++nPosX;
	if ( !bTop )  ++nPosY;

	sal_Bool bBreak = ( nPagebreakMouse == SC_PD_BREAK_H || nPagebreakMouse == SC_PD_BREAK_V );
	sal_Bool bHide = sal_False;
	sal_Bool bToEnd = sal_False;
	ScRange aDrawRange = aPagebreakSource;
	if ( bBreak )
	{
		if ( nPagebreakMouse == SC_PD_BREAK_H )
		{
			if ( nPosX > aPagebreakSource.aStart.Col() &&
				 nPosX <= aPagebreakSource.aEnd.Col() + 1 )		// ans Ende ist auch erlaubt
			{
				bToEnd = ( nPosX == aPagebreakSource.aEnd.Col() + 1 );
				aDrawRange.aStart.SetCol( nPosX );
				aDrawRange.aEnd.SetCol( nPosX - 1 );
			}
			else
				bHide = sal_True;
		}
		else
		{
			if ( nPosY > aPagebreakSource.aStart.Row() &&
				 nPosY <= aPagebreakSource.aEnd.Row() + 1 )		// ans Ende ist auch erlaubt
			{
				bToEnd = ( nPosY == aPagebreakSource.aEnd.Row() + 1 );
				aDrawRange.aStart.SetRow( nPosY );
				aDrawRange.aEnd.SetRow( nPosY - 1 );
			}
			else
				bHide = sal_True;
		}
	}
	else
	{
		if ( nPagebreakMouse & SC_PD_RANGE_L )
			aDrawRange.aStart.SetCol( nPosX );
		if ( nPagebreakMouse & SC_PD_RANGE_T )
			aDrawRange.aStart.SetRow( nPosY );
		if ( nPagebreakMouse & SC_PD_RANGE_R )
		{
			if ( nPosX > 0 )
				aDrawRange.aEnd.SetCol( nPosX-1 );
			else
				bHide = sal_True;
		}
		if ( nPagebreakMouse & SC_PD_RANGE_B )
		{
			if ( nPosY > 0 )
				aDrawRange.aEnd.SetRow( nPosY-1 );
			else
				bHide = sal_True;
		}
		if ( aDrawRange.aStart.Col() > aDrawRange.aEnd.Col() ||
			 aDrawRange.aStart.Row() > aDrawRange.aEnd.Row() )
			bHide = sal_True;
	}

	if ( !bPagebreakDrawn || bUp || aDrawRange != aPagebreakDrag )
	{
		//	zeichnen...

		if ( bPagebreakDrawn )
		{
			// weginvertieren
			// DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
			//				aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), sal_False );
			bPagebreakDrawn = sal_False;
		}
		aPagebreakDrag = aDrawRange;
		if ( !bUp && !bHide )
		{
			// hininvertieren
			// DrawDragRect( aPagebreakDrag.aStart.Col(), aPagebreakDrag.aStart.Row(),
			//				aPagebreakDrag.aEnd.Col(), aPagebreakDrag.aEnd.Row(), sal_False );
			bPagebreakDrawn = sal_True;
		}
        UpdateDragRectOverlay();
	}

	//	bei ButtonUp die Aenderung ausfuehren

	if ( bUp )
	{
		ScViewFunc* pViewFunc = pViewData->GetView();
		ScDocShell* pDocSh = pViewData->GetDocShell();
		ScDocument* pDoc = pDocSh->GetDocument();
		SCTAB nTab = pViewData->GetTabNo();
		sal_Bool bUndo (pDoc->IsUndoEnabled());

		if ( bBreak )
		{
			sal_Bool bColumn = ( nPagebreakMouse == SC_PD_BREAK_H );
			SCCOLROW nNew = bColumn ? static_cast<SCCOLROW>(nPosX) : static_cast<SCCOLROW>(nPosY);
			if ( nNew != nPagebreakBreak )
			{
				if (bUndo)
				{
					String aUndo = ScGlobal::GetRscString( STR_UNDO_DRAG_BREAK );
					pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
				}

				sal_Bool bGrow = !bHide && nNew > nPagebreakBreak;
				if ( bColumn )
				{
                    if (pDoc->HasColBreak(static_cast<SCCOL>(nPagebreakBreak), nTab) & BREAK_MANUAL)
					{
						ScAddress aOldAddr( static_cast<SCCOL>(nPagebreakBreak), nPosY, nTab );
						pViewFunc->DeletePageBreak( sal_True, sal_True, &aOldAddr, sal_False );
					}
					if ( !bHide && !bToEnd )	// am Ende nicht
					{
						ScAddress aNewAddr( static_cast<SCCOL>(nNew), nPosY, nTab );
						pViewFunc->InsertPageBreak( sal_True, sal_True, &aNewAddr, sal_False );
					}
					if ( bGrow )
					{
						//	vorigen Break auf hart, und Skalierung aendern
                        bool bManualBreak = (pDoc->HasColBreak(static_cast<SCCOL>(nPagebreakPrev), nTab) & BREAK_MANUAL);
                        if ( static_cast<SCCOL>(nPagebreakPrev) > aPagebreakSource.aStart.Col() && !bManualBreak )
						{
							ScAddress aPrev( static_cast<SCCOL>(nPagebreakPrev), nPosY, nTab );
							pViewFunc->InsertPageBreak( sal_True, sal_True, &aPrev, sal_False );
						}

						if (!pDocSh->AdjustPrintZoom( ScRange(
									  static_cast<SCCOL>(nPagebreakPrev),0,nTab, static_cast<SCCOL>(nNew-1),0,nTab ) ))
							bGrow = sal_False;
					}
				}
				else
				{
                    if (pDoc->HasRowBreak(nPagebreakBreak, nTab) & BREAK_MANUAL)
					{
						ScAddress aOldAddr( nPosX, nPagebreakBreak, nTab );
						pViewFunc->DeletePageBreak( sal_False, sal_True, &aOldAddr, sal_False );
					}
					if ( !bHide && !bToEnd )	// am Ende nicht
					{
						ScAddress aNewAddr( nPosX, nNew, nTab );
						pViewFunc->InsertPageBreak( sal_False, sal_True, &aNewAddr, sal_False );
					}
					if ( bGrow )
					{
						//	vorigen Break auf hart, und Skalierung aendern
                        bool bManualBreak = (pDoc->HasRowBreak(nPagebreakPrev, nTab) & BREAK_MANUAL);
                        if ( nPagebreakPrev > aPagebreakSource.aStart.Row() && !bManualBreak )
						{
							ScAddress aPrev( nPosX, nPagebreakPrev, nTab );
							pViewFunc->InsertPageBreak( sal_False, sal_True, &aPrev, sal_False );
						}

						if (!pDocSh->AdjustPrintZoom( ScRange(
									  0,nPagebreakPrev,nTab, 0,nNew-1,nTab ) ))
							bGrow = sal_False;
					}
				}

				if (bUndo)
				{
					pDocSh->GetUndoManager()->LeaveListAction();
				}

				if (!bGrow)		// sonst in AdjustPrintZoom schon passiert
				{
					pViewFunc->UpdatePageBreakData( sal_True );
					pDocSh->SetDocumentModified();
				}
			}
		}
		else if ( bHide || aPagebreakDrag != aPagebreakSource )
		{
			//	Druckbereich setzen

			String aNewRanges;
			sal_uInt16 nOldCount = pDoc->GetPrintRangeCount( nTab );
			if ( nOldCount )
			{
				for (sal_uInt16 nPos=0; nPos<nOldCount; nPos++)
				{
					const ScRange* pOld = pDoc->GetPrintRange( nTab, nPos );
					if ( pOld )
					{
						String aTemp;
						if ( *pOld != aPagebreakSource )
							pOld->Format( aTemp, SCA_VALID );
						else if ( !bHide )
							aPagebreakDrag.Format( aTemp, SCA_VALID );
						if (aTemp.Len())
						{
							if ( aNewRanges.Len() )
								aNewRanges += ';';
							aNewRanges += aTemp;
						}
					}
				}
			}
			else if (!bHide)
				aPagebreakDrag.Format( aNewRanges, SCA_VALID );

            pViewFunc->SetPrintRanges( pDoc->IsPrintEntireSheet( nTab ), &aNewRanges, NULL, NULL, sal_False );
		}
	}

	//	Timer fuer Scrolling

	if (bTimer && !bUp)
		pViewData->GetView()->SetTimer( this, rMEvt );			// Event wiederholen
	else
		pViewData->GetView()->ResetTimer();
}




