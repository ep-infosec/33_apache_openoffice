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


#include "AccessibleSpreadsheet.hxx"
#include "AccessibilityHints.hxx"
#include "AccessibleCell.hxx"
#include "AccessibleDocument.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "unoguard.hxx"
#include "hints.hxx"
#include "scmod.hxx"

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <rtl/uuid.h>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <svtools/colorcfg.hxx>
#include "scresid.hxx"
#include "sc.hrc"
#include <algorithm>

using namespace	::com::sun::star;
using namespace	::com::sun::star::accessibility;

bool CompMinCol(const std::pair<sal_uInt16,sal_uInt16> & pc1,const std::pair<sal_uInt16,sal_uInt16>  &pc2)
{
    return pc1.first < pc2.first;
}
ScMyAddress ScAccessibleSpreadsheet::CalcScAddressFromRangeList(ScRangeList *pMarkedRanges,sal_Int32 nSelectedChildIndex)
{
    if (pMarkedRanges->Count() <= 1)
    {
        ScRange* pRange = pMarkedRanges->First();
        if (pRange)
        {
			// MT IA2: Not used.
            // const int nRowNum = pRange->aEnd.Row() - pRange->aStart.Row() + 1;
            const int nColNum = pRange->aEnd.Col() - pRange->aStart.Col() + 1;
            const int nCurCol = nSelectedChildIndex % nColNum;
            const int nCurRow = (nSelectedChildIndex - nCurCol)/nColNum;
            return ScMyAddress(static_cast<SCCOL>(pRange->aStart.Col() + nCurCol), pRange->aStart.Row() + nCurRow, maActiveCell.Tab());
        }
    }
    else
    {
        sal_Int32 nMinRow = MAXROW;
        sal_Int32 nMaxRow = 0;
        m_vecTempRange.clear();
        ScRange* pRange = pMarkedRanges->First();
        while (pRange)
        {
            if (pRange->aStart.Tab() != pRange->aEnd.Tab())
            {
                if ((maActiveCell.Tab() >= pRange->aStart.Tab()) ||
                    maActiveCell.Tab() <= pRange->aEnd.Tab())
                {
                    m_vecTempRange.push_back(pRange);
                    nMinRow = std::min(pRange->aStart.Row(),nMinRow);
                    nMaxRow = std::max(pRange->aEnd.Row(),nMaxRow);
                }
                else
                    DBG_ERROR("Range of wrong table");
            }
            else if(pRange->aStart.Tab() == maActiveCell.Tab())
            {
                m_vecTempRange.push_back(pRange);
                nMinRow = std::min(pRange->aStart.Row(),nMinRow);
                nMaxRow = std::max(pRange->aEnd.Row(),nMaxRow);
            }
            else
                DBG_ERROR("Range of wrong table");
            pRange = pMarkedRanges->Next();
        }
        int nCurrentIndex = 0 ;
        for(sal_Int32 row = nMinRow ; row <= nMaxRow ; ++row)
        {
            m_vecTempCol.clear();
            {
                VEC_RANGE::const_iterator vi = m_vecTempRange.begin();
                for (; vi < m_vecTempRange.end(); ++vi)
                {
                    ScRange *p = *vi;
                    if ( row >= p->aStart.Row() && row <= p->aEnd.Row())
                    {
                        m_vecTempCol.push_back(std::make_pair(p->aStart.Col(),p->aEnd.Col()));
                    }
                }
            }
            std::sort(m_vecTempCol.begin(),m_vecTempCol.end(),CompMinCol);
            {
                VEC_COL::const_iterator vic = m_vecTempCol.begin();
                for(; vic != m_vecTempCol.end(); ++vic)
                {
                    const PAIR_COL &pariCol = *vic;
                    sal_uInt16 nCol = pariCol.second - pariCol.first + 1;
                    if (nCol + nCurrentIndex > nSelectedChildIndex)
                    {
                        return ScMyAddress(static_cast<SCCOL>(pariCol.first + nSelectedChildIndex - nCurrentIndex), row, maActiveCell.Tab());
                    }
                    nCurrentIndex += nCol;
                }
            }
        }
    }
    return ScMyAddress(0,0,maActiveCell.Tab());
}
sal_Bool ScAccessibleSpreadsheet::CalcScRangeDifferenceMax(ScRange *pSrc,ScRange *pDest,int nMax,VEC_MYADDR &vecRet,int &nSize)
{
    //Src Must be :Src > Dest
    if (pDest->In(*pSrc))
    {//Here is Src In Dest,Src <= Dest
        return sal_False;
    }
	if (!pDest->Intersects(*pSrc))
	{
		int nCellCount = sal_uInt32(pDest->aEnd.Col() - pDest->aStart.Col() + 1)
			* sal_uInt32(pDest->aEnd.Row() - pDest->aStart.Row() + 1)
			* sal_uInt32(pDest->aEnd.Tab() - pDest->aStart.Tab() + 1);
		if (nCellCount + nSize > nMax)
		{
			return sal_True;
		}
		else if(nCellCount > 0)
		{
			nCellCount +=nSize;
			for (sal_Int32 row = pDest->aStart.Row(); row <=  pDest->aEnd.Row();++row)
			{
				for (sal_uInt16 col = pDest->aStart.Col(); col <=  pDest->aEnd.Col();++col)
				{
					vecRet.push_back(ScMyAddress(col,row,pDest->aStart.Tab()));
				}
			}
		}
		return sal_False;
	}
	sal_Int32 nMinRow = pSrc->aStart.Row(); 
	sal_Int32 nMaxRow = pSrc->aEnd.Row();
	for (; nMinRow <= nMaxRow ; ++nMinRow,--nMaxRow)
	{
		for (sal_uInt16 col = pSrc->aStart.Col(); col <=  pSrc->aEnd.Col();++col)
		{
			if (nSize > nMax)
			{
				return sal_True;
			}
			ScMyAddress cell(col,nMinRow,pSrc->aStart.Tab());
			if(!pDest->In(cell))
			{//In Src ,Not In Dest
				vecRet.push_back(cell);
				++nSize;
			}
		}
		if (nMinRow != nMaxRow)
		{
			for (sal_uInt16 col = pSrc->aStart.Col(); col <=  pSrc->aEnd.Col();++col)
			{
				if (nSize > nMax)
				{
					return sal_True;
				}
				ScMyAddress cell(col,nMaxRow,pSrc->aStart.Tab());
				if(!pDest->In(cell))
				{//In Src ,Not In Dest
					vecRet.push_back(cell);
					++nSize;
				}
			}
		}
	}
    return sal_False;
}
//In Src , Not in Dest
sal_Bool ScAccessibleSpreadsheet::CalcScRangeListDifferenceMax(ScRangeList *pSrc,ScRangeList *pDest,int nMax,VEC_MYADDR &vecRet)
{
    if (pSrc == NULL || pDest == NULL)
    {
        return sal_False;
    }
    int nSize =0;
    if (pDest->GetCellCount() == 0)//if the Dest Rang List is empty
    {
        if (pSrc->GetCellCount() > sal_uInt32(nMax))//if the Src Cell count is greater then  nMax
        {
            return sal_True;
        }
        //now the cell count is less then nMax
        vecRet.reserve(10);
        ScRange* pRange = pSrc->First();
        while (pRange)
        {
            for (sal_Int32 row = pRange->aStart.Row(); row <=  pRange->aEnd.Row();++row)
            {
                for (sal_uInt16 col = pRange->aStart.Col(); col <=  pRange->aEnd.Col();++col)
                {
                    vecRet.push_back(ScMyAddress(col,row,pRange->aStart.Tab()));
                }
            }
            pRange = pSrc->Next();
        }
        return sal_False;
    }
    //the Dest Rang List is not empty
    vecRet.reserve(10);
    ScRange* pRange = pSrc->First();
    while (pRange)
    {
        ScRange* pRangeDest = pDest->First();
        while (pRangeDest)
        {   
            if (CalcScRangeDifferenceMax(pRange,pRangeDest,nMax,vecRet,nSize))
            {
                return sal_True;
            }
            pRangeDest = pDest->Next();
        }
        pRange = pSrc->Next();
    }
    return sal_False;
}
//=====  internal  ============================================================

ScAccessibleSpreadsheet::ScAccessibleSpreadsheet(
        ScAccessibleDocument* pAccDoc,
		ScTabViewShell* pViewShell,
		SCTAB nTab,
		ScSplitPos eSplitPos)
	:
	ScAccessibleTableBase (pAccDoc, GetDocument(pViewShell),
        ScRange(ScAddress(0, 0, nTab),ScAddress(MAXCOL, MAXROW, nTab))),
    mbIsSpreadsheet( sal_True ),
	m_bFormulaMode(sal_False),
	m_bFormulaLastMode(sal_False),
	m_pAccFormulaCell(NULL),
	m_nMinX(0),m_nMaxX(0),m_nMinY(0),m_nMaxY(0)
{
    ConstructScAccessibleSpreadsheet( pAccDoc, pViewShell, nTab, eSplitPos );
}

ScAccessibleSpreadsheet::ScAccessibleSpreadsheet(
        ScAccessibleSpreadsheet& rParent, const ScRange& rRange ) :
    ScAccessibleTableBase( rParent.mpAccDoc, rParent.mpDoc, rRange),
    mbIsSpreadsheet( sal_False )
{
    ConstructScAccessibleSpreadsheet( rParent.mpAccDoc, rParent.mpViewShell, rParent.mnTab, rParent.meSplitPos );
}

ScAccessibleSpreadsheet::~ScAccessibleSpreadsheet()
{
	if (mpMarkedRanges)
		delete mpMarkedRanges;
	if (mpViewShell)
		mpViewShell->RemoveAccessibilityObject(*this);
}

void ScAccessibleSpreadsheet::ConstructScAccessibleSpreadsheet(
    ScAccessibleDocument* pAccDoc,
    ScTabViewShell* pViewShell,
    SCTAB nTab,
    ScSplitPos eSplitPos)
{
    mpViewShell = pViewShell;
    mpMarkedRanges = 0;
    mpSortedMarkedCells = 0;
    mpAccDoc = pAccDoc;
    mpAccCell = 0;
    meSplitPos = eSplitPos;
    mnTab = nTab;
    mbHasSelection = sal_False;
    mbDelIns = sal_False;
    mbIsFocusSend = sal_False;
    maVisCells = GetVisCells(GetVisArea(mpViewShell, meSplitPos));
    if (mpViewShell)
    {
        mpViewShell->AddAccessibilityObject(*this);

        const ScViewData& rViewData = *mpViewShell->GetViewData();
        const ScMarkData& rMarkData = rViewData.GetMarkData();
        maActiveCell = rViewData.GetCurPos();
        mbHasSelection = rMarkData.GetTableSelect(maActiveCell.Tab()) &&
                    (rMarkData.IsMarked() || rMarkData.IsMultiMarked());
        mpAccCell = GetAccessibleCellAt(maActiveCell.Row(), maActiveCell.Col());
        mpAccCell->acquire();
        mpAccCell->Init();
		ScDocument* pScDoc= GetDocument(mpViewShell);
		if (pScDoc)
		{
			pScDoc->GetName( maActiveCell.Tab(), m_strOldTabName );
		}
    }	
}

void SAL_CALL ScAccessibleSpreadsheet::disposing()
{
    ScUnoGuard aGuard;
	if (mpViewShell)
	{
		mpViewShell->RemoveAccessibilityObject(*this);
		mpViewShell = NULL;
	}
    if (mpAccCell)
    {
        mpAccCell->release();
        mpAccCell = NULL;
    }

	ScAccessibleTableBase::disposing();
}

void ScAccessibleSpreadsheet::CompleteSelectionChanged(sal_Bool bNewState)
{
	if (IsFormulaMode())
	{
		return ;
	}
	if (mpMarkedRanges)
		DELETEZ(mpMarkedRanges);
	mbHasSelection = bNewState;

    AccessibleEventObject aEvent;
	aEvent.EventId = AccessibleEventId::STATE_CHANGED;
	if (bNewState)
		aEvent.NewValue = uno::makeAny(AccessibleStateType::SELECTED);
	else
		aEvent.OldValue = uno::makeAny(AccessibleStateType::SELECTED);
	aEvent.Source = uno::Reference< XAccessibleContext >(this);

	CommitChange(aEvent);
}

void ScAccessibleSpreadsheet::LostFocus()
{
	AccessibleEventObject aEvent;
	aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
	aEvent.Source = uno::Reference< XAccessibleContext >(this);
    uno::Reference< XAccessible > xOld = mpAccCell;
	aEvent.OldValue <<= xOld;

	CommitChange(aEvent);

    CommitFocusLost();
}

void ScAccessibleSpreadsheet::GotFocus()
{
	AccessibleEventObject aEvent;
	aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
	aEvent.Source = uno::Reference< XAccessibleContext >(this);
    uno::Reference< XAccessible > xNew;
	if (IsFormulaMode())
	{
		if (!m_pAccFormulaCell || !m_bFormulaLastMode)
		{
			ScAddress aFormulaAddr;
			if(!GetFormulaCurrentFocusCell(aFormulaAddr))
			{
				return;
			}
			m_pAccFormulaCell = GetAccessibleCellAt(aFormulaAddr.Row(),aFormulaAddr.Col());

			m_pAccFormulaCell->acquire();
			m_pAccFormulaCell->Init();


		}
	    xNew = m_pAccFormulaCell;
	}
	else
	{
		if(mpAccCell->GetCellAddress() == maActiveCell)
		{
			xNew = mpAccCell;
		}
		else
		{
			CommitFocusCell(maActiveCell);
			return ;
		}		
	}
	aEvent.NewValue <<= xNew;

	CommitChange(aEvent);
}

void ScAccessibleSpreadsheet::BoundingBoxChanged()
{
    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::BOUNDRECT_CHANGED;
    aEvent.Source = uno::Reference< XAccessibleContext >(this);

    CommitChange(aEvent);
}

void ScAccessibleSpreadsheet::VisAreaChanged()
{
	AccessibleEventObject aEvent;
	aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
	aEvent.Source = uno::Reference< XAccessibleContext >(this);

	CommitChange(aEvent);
}

	//=====  SfxListener  =====================================================

void ScAccessibleSpreadsheet::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
	if (rHint.ISA( SfxSimpleHint ) )
	{
		const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
		if ((rRef.GetId() == SC_HINT_ACC_CURSORCHANGED))
		{
			if (mpViewShell)
			{
				ScViewData *pViewData = mpViewShell->GetViewData();

				m_bFormulaMode = pViewData->IsRefMode() || SC_MOD()->IsFormulaMode();
				if ( m_bFormulaMode )
				{
					NotifyRefMode();
					m_bFormulaLastMode = true;
					return ;
				}
				if (m_bFormulaLastMode)
				{//Last Notify Mode  Is Formula Mode.
					m_vecFormulaLastMyAddr.clear();
					RemoveFormulaSelection(sal_True);
					if(m_pAccFormulaCell)
					{
						m_pAccFormulaCell->release();
						m_pAccFormulaCell =NULL;
					}
					//Remove All Selection
				}
				m_bFormulaLastMode = m_bFormulaMode;

				AccessibleEventObject aEvent;
				aEvent.Source = uno::Reference< XAccessible >(this);
				ScAddress aNewCell = pViewData->GetCurPos();
				if(aNewCell.Tab() != maActiveCell.Tab())
				{
					aEvent.EventId = AccessibleEventId::PAGE_CHANGED;
					ScAccessibleDocument *pAccDoc = 
						static_cast<ScAccessibleDocument*>(getAccessibleParent().get());
					if(pAccDoc)
					{
						pAccDoc->CommitChange(aEvent);
					}
				}
				sal_Bool bNewPosCell = (aNewCell != maActiveCell) || mpViewShell->GetForceFocusOnCurCell(); // i123629
				sal_Bool bNewPosCellFocus=sal_False;
				if ( bNewPosCell && IsFocused() && aNewCell.Tab() == maActiveCell.Tab() )
				{//single Focus
					bNewPosCellFocus=sal_True;
				}
				ScMarkData &refScMarkData = pViewData->GetMarkData();
				// MT IA2: Not used
				// int nSelCount = refScMarkData.GetSelectCount();
				sal_Bool bIsMark =refScMarkData.IsMarked();
				sal_Bool bIsMultMark = refScMarkData.IsMultiMarked();
				sal_Bool bNewMarked = refScMarkData.GetTableSelect(aNewCell.Tab()) && ( bIsMark || bIsMultMark );				
//				sal_Bool bNewCellSelected = isAccessibleSelected(aNewCell.Row(), aNewCell.Col());
				sal_uInt16 nTab = pViewData->GetTabNo();
				ScRange aMarkRange;
				refScMarkData.GetMarkArea(aMarkRange);
				aEvent.OldValue <<= ::com::sun::star::uno::Any();
				//Mark All
				if ( !bNewPosCellFocus && 
					(bNewMarked || bIsMark || bIsMultMark ) &&
					aMarkRange == ScRange( 0,0,nTab, MAXCOL,MAXROW,nTab ) )
				{
					aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_WITHIN;
					aEvent.NewValue <<= ::com::sun::star::uno::Any();
					CommitChange(aEvent);
					return ;
				}					
				if (!mpMarkedRanges)
				{
					mpMarkedRanges = new ScRangeList();
				}				
				refScMarkData.FillRangeListWithMarks(mpMarkedRanges, sal_True);
	
				//For Whole Col Row  
				sal_Bool bWholeRow = ::labs(aMarkRange.aStart.Row() - aMarkRange.aEnd.Row()) == MAXROW ;
				sal_Bool bWholeCol = ::abs(aMarkRange.aStart.Col() - aMarkRange.aEnd.Col()) == MAXCOL ;
				if ((bNewMarked || bIsMark || bIsMultMark ) && (bWholeCol || bWholeRow))
				{
					if ( aMarkRange != m_aLastWithInMarkRange )
					{
						RemoveSelection(refScMarkData);						
						if(bNewPosCell)
						{
							CommitFocusCell(aNewCell);
						}
						sal_Bool bLastIsWholeColRow = 
						::labs(m_aLastWithInMarkRange.aStart.Row() - m_aLastWithInMarkRange.aEnd.Row()) == MAXROW && bWholeRow || 
						::abs(m_aLastWithInMarkRange.aStart.Col() - m_aLastWithInMarkRange.aEnd.Col()) == MAXCOL && bWholeCol ;
						sal_Bool bSelSmaller= 
							bLastIsWholeColRow && 
							!aMarkRange.In(m_aLastWithInMarkRange) && 
							aMarkRange.Intersects(m_aLastWithInMarkRange);
						if( !bSelSmaller )
						{
							aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_WITHIN;
							aEvent.NewValue <<= ::com::sun::star::uno::Any();
							CommitChange(aEvent);
						}
						m_aLastWithInMarkRange = aMarkRange;
					}
					return ;
				}
				m_aLastWithInMarkRange = aMarkRange;
				int nNewMarkCount = mpMarkedRanges->GetCellCount();
				sal_Bool bSendSingle= (0 == nNewMarkCount) && bNewPosCell;
				if (bSendSingle)
				{
					RemoveSelection(refScMarkData);
					if(bNewPosCellFocus)
					{
						CommitFocusCell(aNewCell);
					}
					uno::Reference< XAccessible > xChild ;
					if (bNewPosCellFocus)
					{
						xChild = mpAccCell;
					}
					else
					{
						xChild = getAccessibleCellAt(aNewCell.Row(),aNewCell.Col());
						
						maActiveCell = aNewCell;
						aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS;							
						aEvent.NewValue <<= xChild;
						aEvent.OldValue <<= uno::Reference< XAccessible >();
						CommitChange(aEvent);
					}
					aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
					aEvent.NewValue <<= xChild;
					CommitChange(aEvent);
					OSL_ASSERT(m_mapSelectionSend.count(aNewCell) == 0 );
					m_mapSelectionSend.insert(MAP_ADDR_XACC::value_type(aNewCell,xChild));
					
				}
				else
				{
					ScRange aDelRange;
					sal_Bool bIsDel = pViewData->GetDelMark( aDelRange );
					if ( (!bIsDel || (bIsDel && aMarkRange != aDelRange)) && 
						bNewMarked && 
						nNewMarkCount > 0 && 
						!IsSameMarkCell() )
					{
						RemoveSelection(refScMarkData);						
						if(bNewPosCellFocus)
						{
							CommitFocusCell(aNewCell);
						}
						VEC_MYADDR vecNew;
                        if(CalcScRangeListDifferenceMax(mpMarkedRanges,&m_LastMarkedRanges,10,vecNew))
						{
							aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_WITHIN;
							aEvent.NewValue <<= ::com::sun::star::uno::Any();
							CommitChange(aEvent);
						}
						else
						{
							VEC_MYADDR::iterator viAddr = vecNew.begin(); 
							for(; viAddr < vecNew.end() ; ++viAddr )
							{
								uno::Reference< XAccessible > xChild = getAccessibleCellAt(viAddr->Row(),viAddr->Col());
								if (!(bNewPosCellFocus && *viAddr == aNewCell) )
								{
									aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS;							
									aEvent.NewValue <<= xChild;							
									CommitChange(aEvent);
								}
								aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_ADD;
								aEvent.NewValue <<= xChild;
								CommitChange(aEvent);
								m_mapSelectionSend.insert(MAP_ADDR_XACC::value_type(*viAddr,xChild));
							}
						}	
					}
				}
				if (bNewPosCellFocus && maActiveCell != aNewCell)
				{
					CommitFocusCell(aNewCell);
				}
                m_LastMarkedRanges = *mpMarkedRanges;		
			}
		}
		else if ((rRef.GetId() == SC_HINT_DATACHANGED))
		{
			if (!mbDelIns)
				CommitTableModelChange(maRange.aStart.Row(), maRange.aStart.Col(), maRange.aEnd.Row(), maRange.aEnd.Col(), AccessibleTableModelChangeType::UPDATE);
			else
				mbDelIns = sal_False;
            ScViewData *pViewData = mpViewShell->GetViewData();
            ScAddress aNewCell = pViewData->GetCurPos();
            if( maActiveCell == aNewCell)
            {
                ScDocument* pScDoc= GetDocument(mpViewShell);
                if (pScDoc)
                {
                    String valStr;
                    pScDoc->GetString(aNewCell.Col(),aNewCell.Row(),aNewCell.Tab(), valStr);
                    if(m_strCurCellValue != valStr)
                    {
                        AccessibleEventObject aEvent;
                        aEvent.EventId = AccessibleEventId::VALUE_CHANGED;
                        mpAccCell->CommitChange(aEvent);
                        m_strCurCellValue=valStr;
                    }
					String tabName;
					pScDoc->GetName( maActiveCell.Tab(), tabName );
					if( m_strOldTabName != tabName )
					{
						AccessibleEventObject aEvent;
                        aEvent.EventId = AccessibleEventId::NAME_CHANGED;
						String sOldName(ScResId(STR_ACC_TABLE_NAME));
						sOldName.SearchAndReplaceAscii("%1", m_strOldTabName);
						aEvent.OldValue <<= ::rtl::OUString( sOldName );
						String sNewName(ScResId(STR_ACC_TABLE_NAME));
						sNewName.SearchAndReplaceAscii("%1", tabName);
						aEvent.NewValue <<= ::rtl::OUString( sNewName );
						CommitChange( aEvent );
						m_strOldTabName = tabName;
					}
                }
            }
        }
        // no longer needed, because the document calls the VisAreaChanged method
/*		else if (rRef.GetId() == SC_HINT_ACC_VISAREACHANGED)
		{
			AccessibleEventObject aEvent;
			aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
			aEvent.Source = uno::Reference< XAccessibleContext >(this);

			CommitChange(aEvent);*/
        // commented out, because to use a ModelChangeEvent is not the right way
        // at the moment there is no way, but the Java/Gnome Api should be extended sometime
/*			if (mpViewShell)
			{
				Rectangle aNewVisCells(GetVisCells(GetVisArea(mpViewShell, meSplitPos)));

				Rectangle aNewPos(aNewVisCells);

				if (aNewVisCells.IsOver(maVisCells))
					aNewPos.Union(maVisCells);
				else
					CommitTableModelChange(maVisCells.Top(), maVisCells.Left(), maVisCells.Bottom(), maVisCells.Right(), AccessibleTableModelChangeType::UPDATE);

				maVisCells = aNewVisCells;

				CommitTableModelChange(aNewPos.Top(), aNewPos.Left(), aNewPos.Bottom(), aNewPos.Right(), AccessibleTableModelChangeType::UPDATE);
			}
		}*/
        // no longer needed, because the document calls the BoundingBoxChanged method
/*        else if (rRef.GetId() == SC_HINT_ACC_WINDOWRESIZED)
        {
			AccessibleEventObject aEvent;
			aEvent.EventId = AccessibleEventId::BOUNDRECT_CHANGED;
			aEvent.Source = uno::Reference< XAccessibleContext >(this);

			CommitChange(aEvent);
        }*/
	}
	else if (rHint.ISA( ScUpdateRefHint ))
	{
		const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;
		if (rRef.GetMode() == URM_INSDEL && rRef.GetDz() == 0) //#107250# test whether table is inserted or deleted
		{
			if (((rRef.GetRange().aStart.Col() == maRange.aStart.Col()) &&
				(rRef.GetRange().aEnd.Col() == maRange.aEnd.Col())) ||
				((rRef.GetRange().aStart.Row() == maRange.aStart.Row()) &&
				(rRef.GetRange().aEnd.Row() == maRange.aEnd.Row())))
			{
				// ignore next SC_HINT_DATACHANGED notification
				mbDelIns = sal_True;

				sal_Int16 nId(0);
                SCsCOL nX(rRef.GetDx());
                SCsROW nY(rRef.GetDy());
                ScRange aRange(rRef.GetRange());
				if ((nX < 0) || (nY < 0))
                {
                    DBG_ASSERT(!((nX < 0) && (nY < 0)), "should not be possible to remove row and column at the same time");
					nId = AccessibleTableModelChangeType::DELETE;
                    if (nX < 0)
                    {
                        nX = -nX;
                        nY = aRange.aEnd.Row() - aRange.aStart.Row();
                    }
                    else
                    {
                        nY = -nY;
                        nX = aRange.aEnd.Col() - aRange.aStart.Col();
                    }
                }
				else if ((nX > 0) || (nY > 0))
                {
                    DBG_ASSERT(!((nX > 0) && (nY > 0)), "should not be possible to add row and column at the same time");
					nId = AccessibleTableModelChangeType::INSERT;
                    if (nX < 0)
                        nY = aRange.aEnd.Row() - aRange.aStart.Row();
                    else
                        nX = aRange.aEnd.Col() - aRange.aStart.Col();
                }
				else
				{
					DBG_ERROR("is it a deletion or a insertion?");
				}

				CommitTableModelChange(rRef.GetRange().aStart.Row(),
                    rRef.GetRange().aStart.Col(),
                    rRef.GetRange().aStart.Row() + nY,
                    rRef.GetRange().aStart.Col() + nX, nId);

				AccessibleEventObject aEvent;
				aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
				aEvent.Source = uno::Reference< XAccessibleContext >(this);
                uno::Reference< XAccessible > xNew = mpAccCell;
				aEvent.NewValue <<= xNew;

				CommitChange(aEvent);
			}
		}
	}

	ScAccessibleTableBase::Notify(rBC, rHint);
}
void ScAccessibleSpreadsheet::RemoveSelection(ScMarkData &refScMarkData)
{
	AccessibleEventObject aEvent;
	aEvent.Source = uno::Reference< XAccessible >(this);
	aEvent.OldValue <<= ::com::sun::star::uno::Any();
	MAP_ADDR_XACC::iterator miRemove = m_mapSelectionSend.begin();
	for(;  miRemove != m_mapSelectionSend.end() ;)
	{
		if (refScMarkData.IsCellMarked(miRemove->first.Col(),miRemove->first.Row(),sal_True) ||
			refScMarkData.IsCellMarked(miRemove->first.Col(),miRemove->first.Row(),sal_False) )
		{
			++miRemove;
			continue;
		}
		aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_REMOVE;
		aEvent.NewValue <<= miRemove->second;							
		CommitChange(aEvent);
		MAP_ADDR_XACC::iterator miNext = miRemove;
		++miNext;
		m_mapSelectionSend.erase(miRemove);
		miRemove = miNext;
	}
}
void ScAccessibleSpreadsheet::CommitFocusCell(const ScAddress &aNewCell)
{
	OSL_ASSERT(!IsFormulaMode());
	if(IsFormulaMode())
	{
		return ;
	}
	AccessibleEventObject aEvent;
	aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
	aEvent.Source = uno::Reference< XAccessible >(this);
	uno::Reference< XAccessible > xOld = mpAccCell;
	mpAccCell->release();
	mpAccCell=NULL;
	aEvent.OldValue <<= xOld;
	mpAccCell = GetAccessibleCellAt(aNewCell.Row(), aNewCell.Col());
	mpAccCell->acquire();
	mpAccCell->Init();
	uno::Reference< XAccessible > xNew = mpAccCell;
	aEvent.NewValue <<= xNew;
	maActiveCell = aNewCell;
    ScDocument* pScDoc= GetDocument(mpViewShell);
    if (pScDoc)
    {
        pScDoc->GetString(maActiveCell.Col(),maActiveCell.Row(),maActiveCell.Tab(), m_strCurCellValue);
    }
	CommitChange(aEvent);
}
sal_Bool ScAccessibleSpreadsheet::IsSameMarkCell()
{
    return m_LastMarkedRanges == *mpMarkedRanges;
}
	//=====  XAccessibleTable  ================================================

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleSpreadsheet::getAccessibleRowHeaders(  )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    uno::Reference< XAccessibleTable > xAccessibleTable;
    if( mpDoc && mbIsSpreadsheet )
    {
        if( const ScRange* pRowRange = mpDoc->GetRepeatRowRange( mnTab ) )
        {
            SCROW nStart = pRowRange->aStart.Row();
            SCROW nEnd = pRowRange->aEnd.Row();
            if( (0 <= nStart) && (nStart <= nEnd) && (nEnd <= MAXROW) )
                xAccessibleTable.set( new ScAccessibleSpreadsheet( *this, ScRange( 0, nStart, mnTab, MAXCOL, nEnd, mnTab ) ) );
        }
    }
    return xAccessibleTable;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessibleSpreadsheet::getAccessibleColumnHeaders(  )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    uno::Reference< XAccessibleTable > xAccessibleTable;
    if( mpDoc && mbIsSpreadsheet )
    {
        if( const ScRange* pColRange = mpDoc->GetRepeatColRange( mnTab ) )
        {
            SCCOL nStart = pColRange->aStart.Col();
            SCCOL nEnd = pColRange->aEnd.Col();
            if( (0 <= nStart) && (nStart <= nEnd) && (nEnd <= MAXCOL) )
                xAccessibleTable.set( new ScAccessibleSpreadsheet( *this, ScRange( nStart, 0, mnTab, nEnd, MAXROW, mnTab ) ) );
        }
    }
    return xAccessibleTable;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleSpreadsheet::getSelectedAccessibleRows(  )
    				throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	uno::Sequence<sal_Int32> aSequence;
	if (IsFormulaMode())
	{
		return aSequence;
	}
	if (mpViewShell && mpViewShell->GetViewData())
	{
		aSequence.realloc(maRange.aEnd.Row() - maRange.aStart.Row() + 1);
		const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
		sal_Int32* pSequence = aSequence.getArray();
		sal_Int32 nCount(0);
		for (SCROW i = maRange.aStart.Row(); i <= maRange.aEnd.Row(); ++i)
		{
			if (rMarkdata.IsRowMarked(i))
			{
				pSequence[nCount] = i;
				++nCount;
			}
		}
		aSequence.realloc(nCount);
	}
	else
		aSequence.realloc(0);
	return aSequence;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleSpreadsheet::getSelectedAccessibleColumns(  )
    				throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	uno::Sequence<sal_Int32> aSequence;
	if (IsFormulaMode())
	{
		return aSequence;
	}
	if (mpViewShell && mpViewShell->GetViewData())
	{
		aSequence.realloc(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
		const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
		sal_Int32* pSequence = aSequence.getArray();
		sal_Int32 nCount(0);
		for (SCCOL i = maRange.aStart.Col(); i <= maRange.aEnd.Col(); ++i)
		{
			if (rMarkdata.IsColumnMarked(i))
			{
				pSequence[nCount] = i;
				++nCount;
			}
		}
		aSequence.realloc(nCount);
	}
	else
		aSequence.realloc(0);
	return aSequence;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::isAccessibleRowSelected( sal_Int32 nRow )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	if (IsFormulaMode())
	{
		return sal_False;
	}

    if ((nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

	sal_Bool bResult(sal_False);
	if (mpViewShell && mpViewShell->GetViewData())
	{
		const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
		bResult = rMarkdata.IsRowMarked((SCROW)nRow);
	}
	return bResult;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
	ScUnoGuard aGuard;
    IsObjectValid();

	if (IsFormulaMode())
	{
		return sal_False;
	}
    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0))
        throw lang::IndexOutOfBoundsException();

    sal_Bool bResult(sal_False);
	if (mpViewShell && mpViewShell->GetViewData())
	{
		const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
		bResult = rMarkdata.IsColumnMarked((SCCOL)nColumn);
	}
	return bResult;
}

ScAccessibleCell* ScAccessibleSpreadsheet::GetAccessibleCellAt(sal_Int32 nRow, sal_Int32 nColumn)
{
    ScAccessibleCell* pAccessibleCell = NULL;
	if (IsFormulaMode())
	{
		ScAddress aCellAddress(static_cast<SCCOL>(nColumn), nRow, mpViewShell->GetViewData()->GetTabNo());
		if ((aCellAddress == m_aFormulaActiveCell) && m_pAccFormulaCell)
		{
			pAccessibleCell = m_pAccFormulaCell;
		}
		else
			pAccessibleCell = new ScAccessibleCell(this, mpViewShell, aCellAddress, GetAccessibleIndexFormula(nRow, nColumn), meSplitPos, mpAccDoc);
	}
	else
	{
	ScAddress aCellAddress(static_cast<SCCOL>(maRange.aStart.Col() + nColumn),
		static_cast<SCROW>(maRange.aStart.Row() + nRow), maRange.aStart.Tab());
    if ((aCellAddress == maActiveCell) && mpAccCell)
    {
        pAccessibleCell = mpAccCell;
    }
    else
	    pAccessibleCell = new ScAccessibleCell(this, mpViewShell, aCellAddress, getAccessibleIndex(nRow, nColumn), meSplitPos, mpAccDoc);
	}

    return pAccessibleCell;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleSpreadsheet::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
    				throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	if (!IsFormulaMode())
	{
    if (nRow > (maRange.aEnd.Row() - maRange.aStart.Row()) ||
        nRow < 0 ||
        nColumn > (maRange.aEnd.Col() - maRange.aStart.Col()) ||
        nColumn < 0)
        throw lang::IndexOutOfBoundsException();
	}
    uno::Reference<XAccessible> xAccessible;
    ScAccessibleCell* pAccessibleCell = GetAccessibleCellAt(nRow, nColumn);
    xAccessible = pAccessibleCell;
	pAccessibleCell->Init();
	return xAccessible;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
    throw (uno::RuntimeException, lang::IndexOutOfBoundsException)
{
	ScUnoGuard aGuard;
    IsObjectValid();

	if (IsFormulaMode())
	{
		ScAddress addr(static_cast<SCCOL>(nColumn), nRow, 0);
		return IsScAddrFormulaSel(addr);
	}
    if ((nColumn > (maRange.aEnd.Col() - maRange.aStart.Col())) || (nColumn < 0) ||
        (nRow > (maRange.aEnd.Row() - maRange.aStart.Row())) || (nRow < 0))
        throw lang::IndexOutOfBoundsException();

	sal_Bool bResult(sal_False);
	if (mpViewShell)
	{
		const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
		bResult = rMarkdata.IsCellMarked(static_cast<SCCOL>(nColumn), static_cast<SCROW>(nRow));
	}
	return bResult;
}

	//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleSpreadsheet::getAccessibleAtPoint(
	const awt::Point& rPoint )
		throw (uno::RuntimeException)
{
	uno::Reference< XAccessible > xAccessible;
    if (containsPoint(rPoint))
    {
    	ScUnoGuard aGuard;
        IsObjectValid();
	    if (mpViewShell)
	    {
		    SCsCOL nX;
            SCsROW nY;
		    mpViewShell->GetViewData()->GetPosFromPixel( rPoint.X, rPoint.Y, meSplitPos, nX, nY);
			try{
		    xAccessible = getAccessibleCellAt(nY, nX);
			}
			catch( ::com::sun::star::lang::IndexOutOfBoundsException e)
			{
				return NULL;
			}
	    }
    }
	return xAccessible;
}

void SAL_CALL ScAccessibleSpreadsheet::grabFocus(  )
		throw (uno::RuntimeException)
{
	if (getAccessibleParent().is())
	{
		uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
		if (xAccessibleComponent.is())
			xAccessibleComponent->grabFocus();
	}
}

sal_Int32 SAL_CALL ScAccessibleSpreadsheet::getForeground(  )
        throw (uno::RuntimeException)
{
    return COL_BLACK;
}

sal_Int32 SAL_CALL ScAccessibleSpreadsheet::getBackground(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    return SC_MOD()->GetColorConfig().GetColorValue( ::svtools::DOCCOLOR ).nColor;
}

    //=====  XAccessibleContext  ==============================================

uno::Reference<XAccessibleRelationSet> SAL_CALL ScAccessibleSpreadsheet::getAccessibleRelationSet(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    utl::AccessibleRelationSetHelper* pRelationSet = NULL;
    if(mpAccDoc)
        pRelationSet = mpAccDoc->GetRelationSet(NULL);
    if (!pRelationSet)
        pRelationSet = new utl::AccessibleRelationSetHelper();
    return pRelationSet;
}

uno::Reference<XAccessibleStateSet> SAL_CALL
	ScAccessibleSpreadsheet::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
	uno::Reference<XAccessibleStateSet> xParentStates;
	if (getAccessibleParent().is())
	{
		uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
		xParentStates = xParentContext->getAccessibleStateSet();
	}
	utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
	if (IsDefunc(xParentStates))
		pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::MANAGES_DESCENDANTS);
	    if (IsEditable(xParentStates))
		    pStateSet->AddState(AccessibleStateType::EDITABLE);
	    pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::FOCUSABLE);
        if (IsFocused())
            pStateSet->AddState(AccessibleStateType::FOCUSED);
	    pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
	    pStateSet->AddState(AccessibleStateType::OPAQUE);
	    pStateSet->AddState(AccessibleStateType::SELECTABLE);
	    if (IsCompleteSheetSelected())
		    pStateSet->AddState(AccessibleStateType::SELECTED);
	    if (isShowing())
		    pStateSet->AddState(AccessibleStateType::SHOWING);
	    if (isVisible())
		    pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
	return pStateSet;
}

	///=====  XAccessibleSelection  ===========================================

void SAL_CALL
		ScAccessibleSpreadsheet::selectAccessibleChild( sal_Int32 nChildIndex )
		throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    if (mpViewShell)
	{
		sal_Int32 nCol(getAccessibleColumn(nChildIndex));
		sal_Int32 nRow(getAccessibleRow(nChildIndex));

		SelectCell(nRow, nCol, sal_False);
	}
}

void SAL_CALL
		ScAccessibleSpreadsheet::clearAccessibleSelection(  )
		throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	if (mpViewShell)
	{
		if (!IsFormulaMode())
		mpViewShell->Unmark();
	}
}

void SAL_CALL
		ScAccessibleSpreadsheet::selectAllAccessibleChildren(  )
		throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	if (mpViewShell)
	{
		if (IsFormulaMode())
		{
			ScViewData *pViewData = mpViewShell->GetViewData();		
			mpViewShell->InitRefMode( 0, 0, pViewData->GetTabNo(), SC_REFTYPE_REF );
			pViewData->SetRefStart(0,0,pViewData->GetTabNo());
			pViewData->SetRefStart(MAXCOL,MAXROW,pViewData->GetTabNo());
			mpViewShell->UpdateRef(MAXCOL, MAXROW, pViewData->GetTabNo());
		}
		else
		mpViewShell->SelectAll();
	}
}

sal_Int32 SAL_CALL
		ScAccessibleSpreadsheet::getSelectedAccessibleChildCount(  )
		throw (uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	sal_Int32 nResult(0);
	if (mpViewShell)
	{
		if (IsFormulaMode())
		{
			nResult =  GetRowAll() * GetColAll() ;
		}
		else
		{		
		if (!mpMarkedRanges)
		{
			mpMarkedRanges = new ScRangeList();
            ScMarkData aMarkData(mpViewShell->GetViewData()->GetMarkData());
            //aMarkData.MarkToMulti();
			aMarkData.FillRangeListWithMarks(mpMarkedRanges, sal_False);
		}
		// is possible, because there shouldn't be overlapped ranges in it
		if (mpMarkedRanges)
			nResult = mpMarkedRanges->GetCellCount();
		}
	}
	return nResult;
}

uno::Reference<XAccessible > SAL_CALL
		ScAccessibleSpreadsheet::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
		throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();
	uno::Reference < XAccessible > xAccessible;
	if (IsFormulaMode())
	{
		if(CheckChildIndex(nSelectedChildIndex))
		{
			ScAddress addr = GetChildIndexAddress(nSelectedChildIndex);
			xAccessible = getAccessibleCellAt(addr.Row(), addr.Col());
		}
		return xAccessible;
	}
	if (mpViewShell)
	{
		if (!mpMarkedRanges)
		{
			mpMarkedRanges = new ScRangeList();
			mpViewShell->GetViewData()->GetMarkData().FillRangeListWithMarks(mpMarkedRanges, sal_False);
		}
		if (mpMarkedRanges)
		{
			//if (!mpSortedMarkedCells)
			//	CreateSortedMarkedCells();
			//if (mpSortedMarkedCells)
			//{
			//	if ((nSelectedChildIndex < 0) ||
			//		(mpSortedMarkedCells->size() <= static_cast<sal_uInt32>(nSelectedChildIndex)))
			//		throw lang::IndexOutOfBoundsException();
			//	else
			//		xAccessible = getAccessibleCellAt((*mpSortedMarkedCells)[nSelectedChildIndex].Row(), (*mpSortedMarkedCells)[nSelectedChildIndex].Col());
			if ((nSelectedChildIndex < 0) ||
					(mpMarkedRanges->GetCellCount() <= static_cast<sal_uInt32>(nSelectedChildIndex)))
			{
				throw lang::IndexOutOfBoundsException();
			}
			ScMyAddress addr = CalcScAddressFromRangeList(mpMarkedRanges,nSelectedChildIndex);
			if( m_mapSelectionSend.find(addr) != m_mapSelectionSend.end() )
				xAccessible = m_mapSelectionSend[addr];
			else			
				xAccessible = getAccessibleCellAt(addr.Row(), addr.Col());
		}
	}
	return xAccessible;
}

void SAL_CALL
		ScAccessibleSpreadsheet::deselectAccessibleChild( sal_Int32 nChildIndex )
		throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
	ScUnoGuard aGuard;
    IsObjectValid();

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    if (mpViewShell)
	{
		sal_Int32 nCol(getAccessibleColumn(nChildIndex));
		sal_Int32 nRow(getAccessibleRow(nChildIndex));

		if (IsFormulaMode())
		{
			if(IsScAddrFormulaSel(
				ScAddress(static_cast<SCCOL>(nCol), nRow,mpViewShell->GetViewData()->GetTabNo()))
				)
			{
				SelectCell(nRow, nCol, sal_True);
			}
			return ;
		}
		if (mpViewShell->GetViewData()->GetMarkData().IsCellMarked(static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow)))
			SelectCell(nRow, nCol, sal_True);
	}
}

void ScAccessibleSpreadsheet::SelectCell(sal_Int32 nRow, sal_Int32 nCol, sal_Bool bDeselect)
{
	if (IsFormulaMode())
	{
		if (bDeselect)
		{//??
			return ;
		}
		else
		{
			ScViewData *pViewData = mpViewShell->GetViewData();

			mpViewShell->InitRefMode( static_cast<SCCOL>(nCol), nRow, pViewData->GetTabNo(), SC_REFTYPE_REF );
			mpViewShell->UpdateRef(static_cast<SCCOL>(nCol), nRow, pViewData->GetTabNo());
		}
		return ;
	}
	mpViewShell->SetTabNo( maRange.aStart.Tab() );

	mpViewShell->DoneBlockMode( sal_True ); // continue selecting
	mpViewShell->InitBlockMode( static_cast<SCCOL>(nCol), static_cast<SCROW>(nRow), maRange.aStart.Tab(), bDeselect, sal_False, sal_False );

	mpViewShell->SelectionChanged();
}

	//=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleSpreadsheet::getImplementationName(void)
        throw (uno::RuntimeException)
{
	return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleSpreadsheet"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
	ScAccessibleSpreadsheet::getSupportedServiceNames (void)
        throw (uno::RuntimeException)
{
	uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleTableBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

	pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.AccessibleSpreadsheet"));

	return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
	ScAccessibleSpreadsheet::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
	static uno::Sequence<sal_Int8> aId;
	if (aId.getLength() == 0)
	{
		aId.realloc (16);
		rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
	}
	return aId;
}

///=====  XAccessibleEventBroadcaster  =====================================

void SAL_CALL ScAccessibleSpreadsheet::addEventListener(const uno::Reference<XAccessibleEventListener>& xListener)
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    ScAccessibleTableBase::addEventListener(xListener);
}

	//====  internal  =========================================================

Rectangle ScAccessibleSpreadsheet::GetBoundingBoxOnScreen() const
	throw (uno::RuntimeException)
{
	Rectangle aRect;
	if (mpViewShell)
	{
		Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
		if (pWindow)
			aRect = pWindow->GetWindowExtentsRelative(NULL);
	}
	return aRect;
}

Rectangle ScAccessibleSpreadsheet::GetBoundingBox() const
	throw (uno::RuntimeException)
{
	Rectangle aRect;
	if (mpViewShell)
	{
		Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
		if (pWindow)
            //#101986#; extends to the same window, because the parent is the document and it has the same window
			aRect = pWindow->GetWindowExtentsRelative(pWindow);
	}
	return aRect;
}

sal_Bool ScAccessibleSpreadsheet::IsDefunc(
	const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
	return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
		(rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleSpreadsheet::IsEditable(
    const uno::Reference<XAccessibleStateSet>& /* rxParentStates */)
{
	if (IsFormulaMode())
	{
		return sal_False;
	}
	sal_Bool bProtected(sal_False);
	if (mpDoc && mpDoc->IsTabProtected(maRange.aStart.Tab()))
		bProtected = sal_True;
	return !bProtected;
}

sal_Bool ScAccessibleSpreadsheet::IsFocused()
{
    sal_Bool bFocused(sal_False);
    if (mpViewShell)
    {
        if (mpViewShell->GetViewData()->GetActivePart() == meSplitPos)
            bFocused = mpViewShell->GetActiveWin()->HasFocus();
    }
    return bFocused;
}

sal_Bool ScAccessibleSpreadsheet::IsCompleteSheetSelected()
{
	if (IsFormulaMode())
	{
		return sal_False;
	}
	sal_Bool bResult(sal_False);
	if(mpViewShell)
	{
        //#103800#; use a copy of MarkData
        ScMarkData aMarkData(mpViewShell->GetViewData()->GetMarkData());
		aMarkData.MarkToMulti();
		if (aMarkData.IsAllMarked(maRange))
			bResult = sal_True;
	}
	return bResult;
}

ScDocument* ScAccessibleSpreadsheet::GetDocument(ScTabViewShell* pViewShell)
{
	ScDocument* pDoc = NULL;
	if (pViewShell)
		pDoc = pViewShell->GetViewData()->GetDocument();
	return pDoc;
}

Rectangle ScAccessibleSpreadsheet::GetVisArea(ScTabViewShell* pViewShell, ScSplitPos eSplitPos)
{
	Rectangle aVisArea;
	if (pViewShell)
	{
		Window* pWindow = pViewShell->GetWindowByPos(eSplitPos);
		if (pWindow)
		{
			aVisArea.SetPos(pViewShell->GetViewData()->GetPixPos(eSplitPos));
			aVisArea.SetSize(pWindow->GetSizePixel());
		}
	}
	return aVisArea;
}

Rectangle ScAccessibleSpreadsheet::GetVisCells(const Rectangle& rVisArea)
{
	if (mpViewShell)
	{
        SCsCOL nStartX, nEndX;
        SCsROW nStartY, nEndY;

		mpViewShell->GetViewData()->GetPosFromPixel( 1, 1, meSplitPos, nStartX, nStartY);
		mpViewShell->GetViewData()->GetPosFromPixel( rVisArea.GetWidth(), rVisArea.GetHeight(), meSplitPos, nEndX, nEndY);

		return Rectangle(nStartX, nStartY, nEndX, nEndY);
	}
	else
		return Rectangle();
}
sal_Bool SAL_CALL ScAccessibleSpreadsheet::selectRow( sal_Int32 row ) 
throw (lang::IndexOutOfBoundsException, uno::RuntimeException) 
{
	if (IsFormulaMode())
	{
		return sal_False;
	}

	mpViewShell->SetTabNo( maRange.aStart.Tab() );
	mpViewShell->DoneBlockMode( sal_True ); // continue selecting
	mpViewShell->InitBlockMode( 0, row, maRange.aStart.Tab(), sal_False, sal_False, sal_True );
	mpViewShell->MarkCursor( MAXCOL, row, maRange.aStart.Tab(), sal_False, sal_True );
	mpViewShell->SelectionChanged();
	return sal_True;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::selectColumn( sal_Int32 column ) 
		throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
	if (IsFormulaMode())
	{
		return sal_False;
	}

	mpViewShell->SetTabNo( maRange.aStart.Tab() );
	mpViewShell->DoneBlockMode( sal_True ); // continue selecting
	mpViewShell->InitBlockMode( static_cast<SCCOL>(column), 0, maRange.aStart.Tab(), sal_False, sal_True, sal_False );
	mpViewShell->MarkCursor( static_cast<SCCOL>(column), MAXROW, maRange.aStart.Tab(), sal_True, sal_False );	
    mpViewShell->SelectionChanged();
	return sal_True;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::unselectRow( sal_Int32 row ) 
		throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{	
	if (IsFormulaMode())
	{
		return sal_False;
	}

	mpViewShell->SetTabNo( maRange.aStart.Tab() );
	mpViewShell->DoneBlockMode( sal_True ); // continue selecting
	mpViewShell->InitBlockMode( 0, row, maRange.aStart.Tab(), sal_False, sal_False, sal_True, sal_True );
	mpViewShell->MarkCursor( MAXCOL, row, maRange.aStart.Tab(), sal_False, sal_True );	
	mpViewShell->SelectionChanged();
	mpViewShell->DoneBlockMode( sal_True );
	return sal_True;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::unselectColumn( sal_Int32 column ) 
		throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{	
	if (IsFormulaMode())
	{
		return sal_False;
	}

	mpViewShell->SetTabNo( maRange.aStart.Tab() );
	mpViewShell->DoneBlockMode( sal_True ); // continue selecting
	mpViewShell->InitBlockMode( static_cast<SCCOL>(column), 0, maRange.aStart.Tab(), sal_False, sal_True, sal_False, sal_True );
	mpViewShell->MarkCursor( static_cast<SCCOL>(column), MAXROW, maRange.aStart.Tab(), sal_True, sal_False );	
	mpViewShell->SelectionChanged();		
	mpViewShell->DoneBlockMode( sal_True );
	return sal_True;
}

void ScAccessibleSpreadsheet::FireFirstCellFocus()
{	
	if (IsFormulaMode())
	{
		return ;
	}
	if (mbIsFocusSend)
	{
		return ;
	}
	mbIsFocusSend = sal_True;
	AccessibleEventObject aEvent;
	aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
	aEvent.Source = uno::Reference< XAccessible >(this);
	aEvent.NewValue <<= getAccessibleCellAt(maActiveCell.Row(), maActiveCell.Col());
	CommitChange(aEvent);
}
void ScAccessibleSpreadsheet::NotifyRefMode()
{
	ScViewData *pViewData = mpViewShell->GetViewData();
	sal_uInt16 nRefStartX =pViewData->GetRefStartX();
	sal_Int32 nRefStartY=pViewData->GetRefStartY();
	sal_uInt16 nRefEndX=pViewData->GetRefEndX(); 
	sal_Int32 nRefEndY=pViewData->GetRefEndY(); 
	ScAddress aFormulaAddr;
	if(!GetFormulaCurrentFocusCell(aFormulaAddr))
	{
		return ;
	}
	if (m_aFormulaActiveCell != aFormulaAddr)
	{//New Focus
		m_nMinX =std::min(nRefStartX,nRefEndX);
		m_nMaxX =std::max(nRefStartX,nRefEndX);
		m_nMinY = std::min(nRefStartY,nRefEndY);
		m_nMaxY = std::max(nRefStartY,nRefEndY);
		RemoveFormulaSelection();
		AccessibleEventObject aEvent;
		aEvent.Source = uno::Reference< XAccessible >(this);
		aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED;
		aEvent.Source = uno::Reference< XAccessible >(this);
		uno::Reference< XAccessible > xOld = m_pAccFormulaCell;
		aEvent.OldValue <<= xOld;
		m_pAccFormulaCell = GetAccessibleCellAt(aFormulaAddr.Row(), aFormulaAddr.Col());
        m_pAccFormulaCell->acquire();
        m_pAccFormulaCell->Init();
		uno::Reference< XAccessible > xNew = m_pAccFormulaCell;
		aEvent.NewValue <<= xNew;	
		CommitChange(aEvent);
		if (nRefStartX == nRefEndX && nRefStartY == nRefEndY)
		{//Selection Single
			aEvent.EventId = AccessibleEventId::SELECTION_CHANGED;
			aEvent.NewValue <<= xNew;
			CommitChange(aEvent);
			m_mapFormulaSelectionSend.insert(MAP_ADDR_XACC::value_type(aFormulaAddr,xNew));
			m_vecFormulaLastMyAddr.clear();
			m_vecFormulaLastMyAddr.push_back(aFormulaAddr);
		}
		else
		{
			VEC_MYADDR vecCurSel;
			int nCurSize =  (m_nMaxX - m_nMinX +1)*(m_nMaxY - m_nMinY +1) ;
			vecCurSel.reserve(nCurSize);
			for (sal_uInt16 x = m_nMinX ; x <= m_nMaxX ; ++x)
			{
				for (sal_Int32 y = m_nMinY ; y <= m_nMaxY ; ++y)
				{
					ScMyAddress aAddr(x,y,0);
					vecCurSel.push_back(aAddr);
				}
			}
			std::sort(vecCurSel.begin(), vecCurSel.end());
			VEC_MYADDR vecNew;							
			std::set_difference(vecCurSel.begin(),vecCurSel.end(),
				m_vecFormulaLastMyAddr.begin(),m_vecFormulaLastMyAddr.end(),						
				std::back_insert_iterator<VEC_MYADDR>(vecNew));
			int nNewSize = vecNew.size();
			if ( nNewSize > 10 )
			{
				aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_WITHIN;
				aEvent.NewValue <<= ::com::sun::star::uno::Any();
				CommitChange(aEvent);
			}
			else
			{
				VEC_MYADDR::iterator viAddr = vecNew.begin(); 
				for(; viAddr != vecNew.end() ; ++viAddr )
				{
					uno::Reference< XAccessible > xChild;
					if (*viAddr == aFormulaAddr)
					{
						xChild = m_pAccFormulaCell;
					}
					else
					{
						xChild = getAccessibleCellAt(viAddr->Row(),viAddr->Col());
						aEvent.EventId = AccessibleEventId::ACTIVE_DESCENDANT_CHANGED_NOFOCUS;							
						aEvent.NewValue <<= xChild;							
						CommitChange(aEvent);
					}					
					aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_ADD;
					aEvent.NewValue <<= xChild;
					CommitChange(aEvent);
					m_mapFormulaSelectionSend.insert(MAP_ADDR_XACC::value_type(*viAddr,xChild));
				}
			}						
			m_vecFormulaLastMyAddr.swap(vecCurSel);					
		}
	}
	m_aFormulaActiveCell = aFormulaAddr;
}
void ScAccessibleSpreadsheet::RemoveFormulaSelection(sal_Bool bRemoveAll )
{
	AccessibleEventObject aEvent;
	aEvent.Source = uno::Reference< XAccessible >(this);
	aEvent.OldValue <<= ::com::sun::star::uno::Any();
	MAP_ADDR_XACC::iterator miRemove = m_mapFormulaSelectionSend.begin();
	for(;  miRemove != m_mapFormulaSelectionSend.end() ;)
	{
		if( !bRemoveAll && IsScAddrFormulaSel(miRemove->first) )
		{
			++miRemove;
			continue;
		}
		aEvent.EventId = AccessibleEventId::SELECTION_CHANGED_REMOVE;
		aEvent.NewValue <<= miRemove->second;							
		CommitChange(aEvent);
		MAP_ADDR_XACC::iterator miNext = miRemove;
		++miNext;
		m_mapFormulaSelectionSend.erase(miRemove);
		miRemove = miNext;
	}	
}
sal_Bool ScAccessibleSpreadsheet::IsScAddrFormulaSel(const ScAddress &addr) const
{
	if( addr.Col() >= m_nMinX && addr.Col() <= m_nMaxX &&
		addr.Row() >= m_nMinY && addr.Row() <= m_nMaxY &&
		addr.Tab() == mpViewShell->GetViewData()->GetTabNo() )
	{
		return sal_True;
	}
	return sal_False;
}
sal_Bool ScAccessibleSpreadsheet::CheckChildIndex(sal_Int32 nIndex) const
{
	sal_Int32 nMaxIndex = (m_nMaxX - m_nMinX +1)*(m_nMaxY - m_nMinY +1) -1 ;
	return nIndex <= nMaxIndex && nIndex >= 0 ;
}
ScAddress ScAccessibleSpreadsheet::GetChildIndexAddress(sal_Int32 nIndex) const
{
	sal_Int32 nRowAll = GetRowAll();
	sal_uInt16  nColAll = GetColAll();
	if (nIndex < 0 || nIndex >=  nRowAll * nColAll )
	{
		return ScAddress();
	}
	return ScAddress(		
		static_cast<SCCOL>((nIndex - nIndex % nRowAll) / nRowAll +  + m_nMinX),
		nIndex % nRowAll + m_nMinY, 
		mpViewShell->GetViewData()->GetTabNo()
		);
}
sal_Int32 ScAccessibleSpreadsheet::GetAccessibleIndexFormula( sal_Int32 nRow, sal_Int32 nColumn )
{
	sal_uInt16 nColRelative = sal_uInt16(nColumn) - GetColAll();
	sal_Int32 nRowRelative = nRow - GetRowAll();
	if (nRow < 0 || nColumn < 0  || nRowRelative >= GetRowAll() || nColRelative >= GetColAll() )
	{
		return -1;
	}
	return GetRowAll() * nRowRelative + nColRelative;
}
sal_Bool ScAccessibleSpreadsheet::IsFormulaMode() 
{ 
	ScViewData *pViewData = mpViewShell->GetViewData();
	m_bFormulaMode = pViewData->IsRefMode() || SC_MOD()->IsFormulaMode();
	return m_bFormulaMode ;
}
sal_Bool ScAccessibleSpreadsheet::GetFormulaCurrentFocusCell(ScAddress &addr)
{
	ScViewData *pViewData = mpViewShell->GetViewData();
	sal_uInt16 nRefX=0;
	sal_Int32 nRefY=0;
	if(m_bFormulaLastMode)
	{
		nRefX=pViewData->GetRefEndX(); 
		nRefY=pViewData->GetRefEndY(); 
	}
	else
	{
		nRefX=pViewData->GetRefStartX(); 
		nRefY=pViewData->GetRefStartY(); 
	}
	if( /* Always true: nRefX >= 0 && */ nRefX <= MAXCOL && nRefY >= 0 && nRefY <= MAXROW)
	{
		addr = ScAddress(nRefX,nRefY,pViewData->GetTabNo());
		return sal_True;
	}
	return sal_False;
}
uno::Reference < XAccessible > ScAccessibleSpreadsheet::GetActiveCell()
{
	if( m_mapSelectionSend.find( maActiveCell ) != m_mapSelectionSend.end() )
			return m_mapSelectionSend[maActiveCell];
		else
			return getAccessibleCellAt(maActiveCell.Row(), maActiveCell .Col());
}
