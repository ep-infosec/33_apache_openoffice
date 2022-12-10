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
#include <editeng/eeitem.hxx>


#include <vcl/timer.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>

#include "attrib.hxx"
#include "pagedata.hxx"
#include "tabview.hxx"
#include "tabvwsh.hxx"
#include "printfun.hxx"
#include "stlpool.hxx"
#include "docsh.hxx"
#include "gridwin.hxx"
#include "olinewin.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "viewutil.hxx"
#include "colrowba.hxx"
#include "waitoff.hxx"
#include "globstr.hrc"
#include "scmod.hxx"

#define SC_BLOCKMODE_NONE		0
#define SC_BLOCKMODE_NORMAL		1
#define SC_BLOCKMODE_OWN		2



//
//          Markier - Funktionen
//

void ScTabView::PaintMarks(SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
	if (!ValidCol(nStartCol)) nStartCol = MAXCOL;
	if (!ValidRow(nStartRow)) nStartRow = MAXROW;
	if (!ValidCol(nEndCol)) nEndCol = MAXCOL;
	if (!ValidRow(nEndRow)) nEndRow = MAXROW;

	sal_Bool bLeft = (nStartCol==0 && nEndCol==MAXCOL);
	sal_Bool bTop = (nStartRow==0 && nEndRow==MAXROW);

	if (bLeft)
		PaintLeftArea( nStartRow, nEndRow );
	if (bTop)
		PaintTopArea( nStartCol, nEndCol );

	aViewData.GetDocument()->ExtendMerge( nStartCol, nStartRow, nEndCol, nEndRow,
											aViewData.GetTabNo() );
	PaintArea( nStartCol, nStartRow, nEndCol, nEndRow, SC_UPDATE_MARKS );
}

sal_Bool ScTabView::IsMarking( SCCOL nCol, SCROW nRow, SCTAB nTab ) const
{
	return bIsBlockMode
		&& nBlockStartX == nCol
		&& nBlockStartY == nRow
		&& nBlockStartZ == nTab;
}

void ScTabView::InitOwnBlockMode()
{
	if (!bIsBlockMode)
	{
		//	Wenn keine (alte) Markierung mehr da ist, Anker in SelectionEngine loeschen:

		ScMarkData& rMark = aViewData.GetMarkData();
		if (!rMark.IsMarked() && !rMark.IsMultiMarked())
			GetSelEngine()->CursorPosChanging( sal_False, sal_False );

//		bIsBlockMode = sal_True;
		bIsBlockMode = SC_BLOCKMODE_OWN;			//! Variable umbenennen!
		nBlockStartX = 0;
		nBlockStartY = 0;
		nBlockStartZ = 0;
		nBlockEndX = 0;
		nBlockEndY = 0;
		nBlockEndZ = 0;

		SelectionChanged();		// Status wird mit gesetzer Markierung abgefragt
	}
}

void ScTabView::InitBlockMode( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
								sal_Bool bTestNeg, sal_Bool bCols, sal_Bool bRows, sal_Bool bForceNeg )
{
	if (!bIsBlockMode)
	{
		if (!ValidCol(nCurX)) nCurX = MAXCOL;
		if (!ValidRow(nCurY)) nCurY = MAXROW;

		ScMarkData& rMark = aViewData.GetMarkData();
		SCTAB nTab = aViewData.GetTabNo();

		//	Teil von Markierung aufheben?
		if (bForceNeg)
			bBlockNeg = sal_True;
		else if (bTestNeg)
		{
			if ( bCols )
				bBlockNeg = rMark.IsColumnMarked( nCurX );
			else if ( bRows )
				bBlockNeg = rMark.IsRowMarked( nCurY );
			else
				bBlockNeg = rMark.IsCellMarked( nCurX, nCurY );
		}
		else
			bBlockNeg = sal_False;
		rMark.SetMarkNegative(bBlockNeg);

//		bIsBlockMode = sal_True;
		bIsBlockMode = SC_BLOCKMODE_NORMAL;			//! Variable umbenennen!
		bBlockCols = bCols;
		bBlockRows = bRows;
		nBlockStartX = nBlockStartXOrig = nCurX;
		nBlockStartY = nBlockStartYOrig = nCurY;
		nBlockStartZ = nCurZ;
		nBlockEndX = nOldCurX = nBlockStartX;
		nBlockEndY = nOldCurY = nBlockStartY;
		nBlockEndZ = nBlockStartZ;

		if (bBlockCols)
		{
			nBlockStartY = nBlockStartYOrig = 0;
			nBlockEndY = MAXROW;
		}

		if (bBlockRows)
		{
			nBlockStartX = nBlockStartXOrig = 0;
			nBlockEndX = MAXCOL;
		}

		rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab, nBlockEndX,nBlockEndY, nTab ) );

#ifdef OLD_SELECTION_PAINT
		InvertBlockMark( nBlockStartX,nBlockStartY,nBlockEndX,nBlockEndY );
#endif
		UpdateSelectionOverlay();

		bNewStartIfMarking = sal_False;		// use only once
	}
}

void ScTabView::SetNewStartIfMarking()
{
	bNewStartIfMarking = sal_True;
}

void ScTabView::DoneBlockMode( sal_Bool bContinue )            // Default FALSE
{
	//	Wenn zwischen Tabellen- und Header SelectionEngine gewechselt wird,
	//	wird evtl. DeselectAll gerufen, weil die andere Engine keinen Anker hat.
	//	Mit bMoveIsShift wird verhindert, dass dann die Selektion aufgehoben wird.

	if (bIsBlockMode && !bMoveIsShift)
	{
		ScMarkData& rMark = aViewData.GetMarkData();
		sal_Bool bFlag = rMark.GetMarkingFlag();
		rMark.SetMarking(sal_False);

		if (bBlockNeg && !bContinue)
			rMark.MarkToMulti();

		if (bContinue)
			rMark.MarkToMulti();
		else
		{
			//	Die Tabelle kann an dieser Stelle ungueltig sein, weil DoneBlockMode
			//	aus SetTabNo aufgerufen wird
			//	(z.B. wenn die aktuelle Tabelle von einer anderen View aus geloescht wird)

			SCTAB nTab = aViewData.GetTabNo();
			ScDocument* pDoc = aViewData.GetDocument();
			if ( pDoc->HasTable(nTab) )
				PaintBlock( sal_True );								// sal_True -> Block loeschen
			else
				rMark.ResetMark();
		}
//		bIsBlockMode = sal_False;
		bIsBlockMode = SC_BLOCKMODE_NONE;			//! Variable umbenennen!

		rMark.SetMarking(bFlag);
		rMark.SetMarkNegative(sal_False);
	}
}

void ScTabView::MarkCursor( SCCOL nCurX, SCROW nCurY, SCTAB nCurZ,
                            sal_Bool bCols, sal_Bool bRows, sal_Bool bCellSelection )
{
	if (!ValidCol(nCurX)) nCurX = MAXCOL;
	if (!ValidRow(nCurY)) nCurY = MAXROW;

	if (!bIsBlockMode)
	{
		DBG_ERROR( "MarkCursor nicht im BlockMode" );
		InitBlockMode( nCurX, nCurY, nCurZ, sal_False, bCols, bRows );
	}

	if (bCols)
		nCurY = MAXROW;
	if (bRows)
		nCurX = MAXCOL;

	ScMarkData& rMark = aViewData.GetMarkData();
	DBG_ASSERT(rMark.IsMarked() || rMark.IsMultiMarked(), "MarkCursor, !IsMarked()");
	ScRange aMarkRange;
	rMark.GetMarkArea(aMarkRange);
	if (( aMarkRange.aStart.Col() != nBlockStartX && aMarkRange.aEnd.Col() != nBlockStartX ) ||
		( aMarkRange.aStart.Row() != nBlockStartY && aMarkRange.aEnd.Row() != nBlockStartY ) ||
		( bIsBlockMode == SC_BLOCKMODE_OWN ))
	{
		//	Markierung ist veraendert worden
		//	(z.B. MarkToSimple, wenn per negativ alles bis auf ein Rechteck geloescht wurde)
		//	oder nach InitOwnBlockMode wird mit Shift-Klick weitermarkiert...

		sal_Bool bOldShift = bMoveIsShift;
		bMoveIsShift = sal_False;				//	wirklich umsetzen
		DoneBlockMode(sal_False);				//!	direkt Variablen setzen? (-> kein Geflacker)
		bMoveIsShift = bOldShift;

		InitBlockMode( aMarkRange.aStart.Col(), aMarkRange.aStart.Row(),
						nBlockStartZ, rMark.IsMarkNegative(), bCols, bRows );
	}

	SCCOL nOldBlockEndX = nBlockEndX;
	SCROW nOldBlockEndY = nBlockEndY;

	if ( nCurX != nOldCurX || nCurY != nOldCurY )
	{
        // Current cursor has moved

		SCTAB		nTab = nCurZ;

#ifdef OLD_SELECTION_PAINT
		SCCOL		nDrawStartCol;
		SCROW		nDrawStartRow;
		SCCOL		nDrawEndCol;
		SCROW		nDrawEndRow;
#endif

        // Set old selection area
		ScUpdateRect aRect( nBlockStartX, nBlockStartY, nOldBlockEndX, nOldBlockEndY );

        if ( bCellSelection )
        {
            // Expand selection area accordingly when the current selection ends
            // with a merged cell.
            SCsCOL nCurXOffset = 0;
            SCsCOL nBlockStartXOffset = 0;
            SCsROW nCurYOffset = 0;
            SCsROW nBlockStartYOffset = 0;
            sal_Bool bBlockStartMerged = sal_False;
            const ScMergeAttr* pMergeAttr = NULL;
            ScDocument* pDocument = aViewData.GetDocument();

            // The following block checks whether or not the "BlockStart" (anchor)
            // cell is merged.  If it's merged, it'll then move the position of the
            // anchor cell to the corner that's diagonally opposite of the
            // direction of a current selection area.  For instance, if a current
            // selection is moving in the upperleft direction, the anchor cell will
            // move to the lower-right corner of the merged anchor cell, and so on.

            pMergeAttr = static_cast<const ScMergeAttr*>(
                pDocument->GetAttr( nBlockStartXOrig, nBlockStartYOrig, nTab, ATTR_MERGE ) );
            if ( pMergeAttr->IsMerged() )
            {
                SCsCOL nColSpan = pMergeAttr->GetColMerge();
                SCsROW nRowSpan = pMergeAttr->GetRowMerge();

                if ( !( nCurX >= nBlockStartXOrig + nColSpan - 1 && nCurY >= nBlockStartYOrig + nRowSpan - 1 ) )
                {
                    nBlockStartX = nCurX >= nBlockStartXOrig ? nBlockStartXOrig : nBlockStartXOrig + nColSpan - 1;
                    nBlockStartY = nCurY >= nBlockStartYOrig ? nBlockStartYOrig : nBlockStartYOrig + nRowSpan - 1;
                    nCurXOffset  = nCurX >= nBlockStartXOrig && nCurX < nBlockStartXOrig + nColSpan - 1 ?
                        nBlockStartXOrig - nCurX + nColSpan - 1 : 0;
                    nCurYOffset  = nCurY >= nBlockStartYOrig && nCurY < nBlockStartYOrig + nRowSpan - 1 ?
                        nBlockStartYOrig - nCurY + nRowSpan - 1 : 0;
                    bBlockStartMerged = sal_True;
                }
            }

            // The following block checks whether or not the current cell is
            // merged.  If it is, it'll then set the appropriate X & Y offset
            // values (nCurXOffset & nCurYOffset) such that the selection area will
            // grow by those specified offset amounts.  Note that the values of
            // nCurXOffset/nCurYOffset may also be specified in the previous code
            // block, in which case whichever value is greater will take on.

            pMergeAttr = static_cast<const ScMergeAttr*>(
                pDocument->GetAttr( nCurX, nCurY, nTab, ATTR_MERGE ) );
            if ( pMergeAttr->IsMerged() )
            {
                SCsCOL nColSpan = pMergeAttr->GetColMerge();
                SCsROW nRowSpan = pMergeAttr->GetRowMerge();

                if ( !( nBlockStartX >= nCurX + nColSpan - 1 && nBlockStartY >= nCurY + nRowSpan - 1 ) )
                {
                    if ( nBlockStartX <= nCurX + nColSpan - 1 )
                    {
                        SCsCOL nCurXOffsetTemp = nCurX < nCurX + nColSpan - 1 ? nColSpan - 1 : 0;
                        nCurXOffset = nCurXOffset > nCurXOffsetTemp ? nCurXOffset : nCurXOffsetTemp;
                    }
                    if ( nBlockStartY <= nCurY + nRowSpan - 1 )
                    {
                        SCsROW nCurYOffsetTemp = nCurY < nCurY + nRowSpan - 1 ? nRowSpan - 1 : 0;
                        nCurYOffset = nCurYOffset > nCurYOffsetTemp ? nCurYOffset : nCurYOffsetTemp;
                    }
                    if ( !( nBlockStartX <= nCurX && nBlockStartY <= nCurY ) &&
                         !( nBlockStartX > nCurX + nColSpan - 1 && nBlockStartY > nCurY + nRowSpan - 1 ) )
                    {
                        nBlockStartXOffset = nBlockStartX > nCurX && nBlockStartX <= nCurX + nColSpan - 1 ? nCurX - nBlockStartX : 0;
                        nBlockStartYOffset = nBlockStartY > nCurY && nBlockStartY <= nCurY + nRowSpan - 1 ? nCurY - nBlockStartY : 0;
                    }
                }
            }
            else
            {
                // The current cell is not merged.  Move the anchor cell to its
                // original position.
                if ( !bBlockStartMerged )
                {
                    nBlockStartX = nBlockStartXOrig;
                    nBlockStartY = nBlockStartYOrig;
                }
            }

            nBlockStartX = nBlockStartX + nBlockStartXOffset >= 0 ? nBlockStartX + nBlockStartXOffset : 0;
            nBlockStartY = nBlockStartY + nBlockStartYOffset >= 0 ? nBlockStartY + nBlockStartYOffset : 0;
            nBlockEndX = nCurX + nCurXOffset > MAXCOL ? MAXCOL : nCurX + nCurXOffset;
            nBlockEndY = nCurY + nCurYOffset > MAXROW ? MAXROW : nCurY + nCurYOffset;
        }
        else
        {
            nBlockEndX = nCurX;
            nBlockEndY = nCurY;
        }
        // end of "if ( bCellSelection )"

        // Set new selection area
		aRect.SetNew( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY );
		rMark.SetMarkArea( ScRange( nBlockStartX, nBlockStartY, nTab, nBlockEndX, nBlockEndY, nTab ) );

#ifdef OLD_SELECTION_PAINT
		sal_Bool bCont;
		sal_Bool bDraw = aRect.GetXorDiff( nDrawStartCol, nDrawStartRow,
										nDrawEndCol, nDrawEndRow, bCont );
		if ( bDraw )
		{
//?			PutInOrder( nDrawStartCol, nDrawEndCol );
//?			PutInOrder( nDrawStartRow, nDrawEndRow );

			HideAllCursors();
			InvertBlockMark( nDrawStartCol, nDrawStartRow, nDrawEndCol, nDrawEndRow );
			if (bCont)
			{
				aRect.GetContDiff( nDrawStartCol, nDrawStartRow, nDrawEndCol, nDrawEndRow );
				InvertBlockMark( nDrawStartCol, nDrawStartRow, nDrawEndCol, nDrawEndRow );
			}
			ShowAllCursors();
		}
#endif
        UpdateSelectionOverlay();

        nOldCurX = nCurX;
        nOldCurY = nCurY;

		aViewData.GetViewShell()->UpdateInputHandler();
//		InvalidateAttribs();
	}

	if ( !bCols && !bRows )
		aHdrFunc.SetAnchorFlag( sal_False );
}

void ScTabView::UpdateSelectionOverlay()
{
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateSelectionOverlay();
}

void ScTabView::UpdateShrinkOverlay()
{
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateShrinkOverlay();
}

void ScTabView::UpdateAllOverlays()
{
    for (sal_uInt16 i=0; i<4; i++)
        if ( pGridWin[i] && pGridWin[i]->IsVisible() )
            pGridWin[i]->UpdateAllOverlays();
}

//!
//!	PaintBlock in zwei Methoden aufteilen: RepaintBlock und RemoveBlock o.ae.
//!

void ScTabView::PaintBlock( sal_Bool bReset )
{
	ScDocument* pDoc = aViewData.GetDocument();
	ScMarkData& rMark = aViewData.GetMarkData();
	SCTAB nTab = aViewData.GetTabNo();
	sal_Bool bMark = rMark.IsMarked();
	sal_Bool bMulti = rMark.IsMultiMarked();
	if (bMark || bMulti)
	{
		ScRange aMarkRange;
		HideAllCursors();
		if (bMulti)
		{
			sal_Bool bFlag = rMark.GetMarkingFlag();
			rMark.SetMarking(sal_False);
			rMark.MarkToMulti();
			rMark.GetMultiMarkArea(aMarkRange);
			rMark.MarkToSimple();
			rMark.SetMarking(bFlag);

			bMark = rMark.IsMarked();
			bMulti = rMark.IsMultiMarked();
		}
		else
			rMark.GetMarkArea(aMarkRange);

		nBlockStartX = aMarkRange.aStart.Col();
		nBlockStartY = aMarkRange.aStart.Row();
		nBlockStartZ = aMarkRange.aStart.Tab();
		nBlockEndX = aMarkRange.aEnd.Col();
		nBlockEndY = aMarkRange.aEnd.Row();
		nBlockEndZ = aMarkRange.aEnd.Tab();

		sal_Bool bDidReset = sal_False;

		if ( nTab>=nBlockStartZ && nTab<=nBlockEndZ )
		{
			if ( bReset )
			{
				// Invertieren beim Loeschen nur auf aktiver View
				if ( aViewData.IsActive() )
				{
					sal_uInt16 i;
					if ( bMulti )
					{
#ifdef OLD_SELECTION_PAINT
						for (i=0; i<4; i++)
							if (pGridWin[i] && pGridWin[i]->IsVisible())
								pGridWin[i]->InvertSimple( nBlockStartX, nBlockStartY,
															nBlockEndX, nBlockEndY,
															sal_True, sal_True );
#endif
						rMark.ResetMark();
                        UpdateSelectionOverlay();
						bDidReset = sal_True;
					}
					else
					{
#ifdef OLD_SELECTION_PAINT
						// (mis)use InvertBlockMark to remove all of the selection
						// -> set bBlockNeg (like when removing parts of a selection)
						//	  and convert everything to Multi

						rMark.MarkToMulti();
						sal_Bool bOld = bBlockNeg;
						bBlockNeg = sal_True;
						// #73130# (negative) MarkArea must be set in case of repaint
						rMark.SetMarkArea( ScRange( nBlockStartX,nBlockStartY, nTab,
													nBlockEndX,nBlockEndY, nTab ) );

						InvertBlockMark( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY );

						bBlockNeg = bOld;
#endif
						rMark.ResetMark();
                        UpdateSelectionOverlay();
						bDidReset = sal_True;
					}

					//	repaint if controls are touched (#69680# in both cases)
					// #i74768# Forms are rendered by DrawingLayer's EndDrawLayers()
					static bool bSuppressControlExtraStuff(true);

					if(!bSuppressControlExtraStuff)
					{
						Rectangle aMMRect = pDoc->GetMMRect(nBlockStartX,nBlockStartY,nBlockEndX,nBlockEndY, nTab);
						if (pDoc->HasControl( nTab, aMMRect ))
						{
							for (i=0; i<4; i++)
							{
								if (pGridWin[i] && pGridWin[i]->IsVisible())
								{
									//	MapMode muss logischer (1/100mm) sein !!!
									pDoc->InvalidateControls( pGridWin[i], nTab, aMMRect );
									pGridWin[i]->Update();
								}
							}
						}
					}
				}
			}
			else
				PaintMarks( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY );
		}

		if ( bReset && !bDidReset )
			rMark.ResetMark();

		ShowAllCursors();
	}
}

void ScTabView::SelectAll( sal_Bool bContinue )
{
	ScMarkData& rMark = aViewData.GetMarkData();
	SCTAB nTab = aViewData.GetTabNo();

	if (rMark.IsMarked())
	{
		ScRange aMarkRange;
		rMark.GetMarkArea( aMarkRange );
		if ( aMarkRange == ScRange( 0,0,nTab, MAXCOL,MAXROW,nTab ) )
			return;
	}

	DoneBlockMode( bContinue );
	InitBlockMode( 0,0,nTab );
	MarkCursor( MAXCOL,MAXROW,nTab );

	SelectionChanged();
}

void ScTabView::SelectAllTables()
{
	ScDocument* pDoc = aViewData.GetDocument();
	ScMarkData& rMark = aViewData.GetMarkData();
//    SCTAB nTab = aViewData.GetTabNo();
	SCTAB nCount = pDoc->GetTableCount();

	if (nCount>1)
	{
		for (SCTAB i=0; i<nCount; i++)
			rMark.SelectTable( i, sal_True );

		//		Markierungen werden per Default nicht pro Tabelle gehalten
//		pDoc->ExtendMarksFromTable( nTab );

		aViewData.GetDocShell()->PostPaintExtras();
        SfxBindings& rBind = aViewData.GetBindings();
        rBind.Invalidate( FID_FILL_TAB );
        rBind.Invalidate( FID_TAB_DESELECTALL );
	}
}

void ScTabView::DeselectAllTables()
{
    ScDocument* pDoc = aViewData.GetDocument();
    ScMarkData& rMark = aViewData.GetMarkData();
    SCTAB nTab = aViewData.GetTabNo();
    SCTAB nCount = pDoc->GetTableCount();

    for (SCTAB i=0; i<nCount; i++)
        rMark.SelectTable( i, ( i == nTab ) );

    aViewData.GetDocShell()->PostPaintExtras();
    SfxBindings& rBind = aViewData.GetBindings();
    rBind.Invalidate( FID_FILL_TAB );
    rBind.Invalidate( FID_TAB_DESELECTALL );
}

sal_Bool lcl_FitsInWindow( double fScaleX, double fScaleY, sal_uInt16 nZoom,
						long nWindowX, long nWindowY, ScDocument* pDoc, SCTAB nTab,
						SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
						SCCOL nFixPosX, SCROW nFixPosY )
{
	double fZoomFactor = (double)Fraction(nZoom,100);
	fScaleX *= fZoomFactor;
	fScaleY *= fZoomFactor;

	long nBlockX = 0;
	SCCOL nCol;
	for (nCol=0; nCol<nFixPosX; nCol++)
	{
		//	for frozen panes, add both parts
		sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
		if (nColTwips)
		{
			nBlockX += (long)(nColTwips * fScaleX);
			if (nBlockX > nWindowX)
				return sal_False;
		}
	}
	for (nCol=nStartCol; nCol<=nEndCol; nCol++)
	{
		sal_uInt16 nColTwips = pDoc->GetColWidth( nCol, nTab );
		if (nColTwips)
		{
			nBlockX += (long)(nColTwips * fScaleX);
			if (nBlockX > nWindowX)
				return sal_False;
		}
	}

	long nBlockY = 0;
    for (SCROW nRow = 0; nRow <= nFixPosY-1; ++nRow)
	{
        if (pDoc->RowHidden(nRow, nTab))
            continue;

		//	for frozen panes, add both parts
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
		if (nRowTwips)
		{
			nBlockY += (long)(nRowTwips * fScaleY);
			if (nBlockY > nWindowY)
				return sal_False;
		}
	}
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
	{
        sal_uInt16 nRowTwips = pDoc->GetRowHeight(nRow, nTab);
		if (nRowTwips)
		{
			nBlockY += (long)(nRowTwips * fScaleY);
			if (nBlockY > nWindowY)
				return sal_False;
		}
	}

	return sal_True;
}

sal_uInt16 ScTabView::CalcZoom( SvxZoomType eType, sal_uInt16 nOldZoom )
{
	sal_uInt16 nZoom = 0; // Ergebnis

	switch ( eType )
	{
		case SVX_ZOOM_PERCENT: // rZoom ist kein besonderer prozentualer Wert
			nZoom = nOldZoom;
			break;

		case SVX_ZOOM_OPTIMAL:	// nZoom entspricht der optimalen Gr"o\se
			{
				ScMarkData& rMark = aViewData.GetMarkData();
				ScDocument* pDoc = aViewData.GetDocument();

				if (!rMark.IsMarked() && !rMark.IsMultiMarked())
					nZoom = 100;				// nothing selected
				else
				{
					SCTAB	nTab = aViewData.GetTabNo();
					ScRange aMarkRange;
					if ( aViewData.GetSimpleArea( aMarkRange ) != SC_MARK_SIMPLE )
						rMark.GetMultiMarkArea( aMarkRange );

					SCCOL	nStartCol = aMarkRange.aStart.Col();
					SCROW	nStartRow = aMarkRange.aStart.Row();
					SCTAB	nStartTab = aMarkRange.aStart.Tab();
					SCCOL	nEndCol = aMarkRange.aEnd.Col();
					SCROW	nEndRow = aMarkRange.aEnd.Row();
					SCTAB	nEndTab = aMarkRange.aEnd.Tab();

					if ( nTab < nStartTab && nTab > nEndTab )
						nTab = nStartTab;

					ScSplitPos eUsedPart = aViewData.GetActivePart();

					SCCOL nFixPosX = 0;
					SCROW nFixPosY = 0;
					if ( aViewData.GetHSplitMode() == SC_SPLIT_FIX )
					{
						//	use right part
						eUsedPart = (WhichV(eUsedPart)==SC_SPLIT_TOP) ? SC_SPLIT_TOPRIGHT : SC_SPLIT_BOTTOMRIGHT;
						nFixPosX = aViewData.GetFixPosX();
						if ( nStartCol < nFixPosX )
							nStartCol = nFixPosX;
					}
					if ( aViewData.GetVSplitMode() == SC_SPLIT_FIX )
					{
						//	use bottom part
						eUsedPart = (WhichH(eUsedPart)==SC_SPLIT_LEFT) ? SC_SPLIT_BOTTOMLEFT : SC_SPLIT_BOTTOMRIGHT;
						nFixPosY = aViewData.GetFixPosY();
						if ( nStartRow < nFixPosY )
							nStartRow = nFixPosY;
					}

					if (pGridWin[eUsedPart])
					{
						//	Because scale is rounded to pixels, the only reliable way to find
						//	the right scale is to check if a zoom fits

						Size aWinSize = pGridWin[eUsedPart]->GetOutputSizePixel();

						//	for frozen panes, use sum of both parts for calculation

						if ( nFixPosX != 0 )
							aWinSize.Width() += GetGridWidth( SC_SPLIT_LEFT );
						if ( nFixPosY != 0 )
							aWinSize.Height() += GetGridHeight( SC_SPLIT_TOP );

						ScDocShell* pDocSh = aViewData.GetDocShell();
						double nPPTX = ScGlobal::nScreenPPTX / pDocSh->GetOutputFactor();
						double nPPTY = ScGlobal::nScreenPPTY;

						sal_uInt16 nMin = MINZOOM;
						sal_uInt16 nMax = MAXZOOM;
						while ( nMax > nMin )
						{
							sal_uInt16 nTest = (nMin+nMax+1)/2;
							if ( lcl_FitsInWindow(
										nPPTX, nPPTY, nTest, aWinSize.Width(), aWinSize.Height(),
										pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow,
										nFixPosX, nFixPosY ) )
								nMin = nTest;
							else
								nMax = nTest-1;
						}
						DBG_ASSERT( nMin == nMax, "Schachtelung ist falsch" );
						nZoom = nMin;

						if ( nZoom != nOldZoom )
						{
							// scroll to block only in active split part
							// (the part for which the size was calculated)

							if ( nStartCol <= nEndCol )
								aViewData.SetPosX( WhichH(eUsedPart), nStartCol );
							if ( nStartRow <= nEndRow )
								aViewData.SetPosY( WhichV(eUsedPart), nStartRow );
						}
					}
				}
			}
			break;

			case SVX_ZOOM_WHOLEPAGE:	// nZoom entspricht der ganzen Seite oder
			case SVX_ZOOM_PAGEWIDTH:	// nZoom entspricht der Seitenbreite
				{
					SCTAB				nCurTab		= aViewData.GetTabNo();
					ScDocument*			pDoc		= aViewData.GetDocument();
					ScStyleSheetPool*	pStylePool  = pDoc->GetStyleSheetPool();
					SfxStyleSheetBase*	pStyleSheet =
											pStylePool->Find( pDoc->GetPageStyle( nCurTab ),
															  SFX_STYLE_FAMILY_PAGE );

					DBG_ASSERT( pStyleSheet, "PageStyle not found :-/" );

					if ( pStyleSheet )
					{
						ScPrintFunc aPrintFunc( aViewData.GetDocShell(),
												aViewData.GetViewShell()->GetPrinter(sal_True),
												nCurTab );

						Size aPageSize = aPrintFunc.GetDataSize();

						//	use the size of the largest GridWin for normal split,
						//	or both combined for frozen panes, with the (document) size
						//	of the frozen part added to the page size
						//	(with frozen panes, the size of the individual parts
						//	depends on the scale that is to be calculated)

						if ( !pGridWin[SC_SPLIT_BOTTOMLEFT] ) return 0;
						Size aWinSize = pGridWin[SC_SPLIT_BOTTOMLEFT]->GetOutputSizePixel();
						ScSplitMode eHMode = aViewData.GetHSplitMode();
						if ( eHMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_BOTTOMRIGHT] )
						{
							long nOtherWidth = pGridWin[SC_SPLIT_BOTTOMRIGHT]->
														GetOutputSizePixel().Width();
							if ( eHMode == SC_SPLIT_FIX )
							{
								aWinSize.Width() += nOtherWidth;
								for ( SCCOL nCol = aViewData.GetPosX(SC_SPLIT_LEFT);
										nCol < aViewData.GetFixPosX(); nCol++ )
									aPageSize.Width() += pDoc->GetColWidth( nCol, nCurTab );
							}
							else if ( nOtherWidth > aWinSize.Width() )
								aWinSize.Width() = nOtherWidth;
						}
						ScSplitMode eVMode = aViewData.GetVSplitMode();
						if ( eVMode != SC_SPLIT_NONE && pGridWin[SC_SPLIT_TOPLEFT] )
						{
							long nOtherHeight = pGridWin[SC_SPLIT_TOPLEFT]->
														GetOutputSizePixel().Height();
							if ( eVMode == SC_SPLIT_FIX )
							{
								aWinSize.Height() += nOtherHeight;
                                aPageSize.Height() += pDoc->GetRowHeight(
                                        aViewData.GetPosY(SC_SPLIT_TOP),
                                        aViewData.GetFixPosY()-1, nCurTab);
							}
							else if ( nOtherHeight > aWinSize.Height() )
								aWinSize.Height() = nOtherHeight;
						}

						double nPPTX = ScGlobal::nScreenPPTX / aViewData.GetDocShell()->GetOutputFactor();
						double nPPTY = ScGlobal::nScreenPPTY;

						long nZoomX = (long) ( aWinSize.Width() * 100 /
											   ( aPageSize.Width() * nPPTX ) );
						long nZoomY = (long) ( aWinSize.Height() * 100 /
											   ( aPageSize.Height() * nPPTY ) );
						long nNew = nZoomX;

						if (eType == SVX_ZOOM_WHOLEPAGE && nZoomY < nNew)
							nNew = nZoomY;

						nZoom = (sal_uInt16) nNew;
					}
				}
				break;

		default:
			DBG_ERROR("Unknown Zoom-Revision");
			nZoom = 0;
	}

	return nZoom;
}

//	wird z.B. gerufen, wenn sich das View-Fenster verschiebt:

void ScTabView::StopMarking()
{
	ScSplitPos eActive = aViewData.GetActivePart();
	if (pGridWin[eActive])
		pGridWin[eActive]->StopMarking();

	ScHSplitPos eH = WhichH(eActive);
	if (pColBar[eH])
		pColBar[eH]->StopMarking();

	ScVSplitPos eV = WhichV(eActive);
	if (pRowBar[eV])
		pRowBar[eV]->StopMarking();
}

void ScTabView::HideNoteMarker()
{
	for (sal_uInt16 i=0; i<4; i++)
		if (pGridWin[i] && pGridWin[i]->IsVisible())
			pGridWin[i]->HideNoteMarker();
}

void ScTabView::MakeDrawLayer()
{
	if (!pDrawView)
	{
		aViewData.GetDocShell()->MakeDrawLayer();

		//	pDrawView wird per Notify gesetzt
		DBG_ASSERT(pDrawView,"ScTabView::MakeDrawLayer funktioniert nicht");

		// #114409#
		for(sal_uInt16 a(0); a < 4; a++)
		{
			if(pGridWin[a])
			{
				pGridWin[a]->DrawLayerCreated();
			}
		}
	}
}

void ScTabView::ErrorMessage( sal_uInt16 nGlobStrId )
{
    if ( SC_MOD()->IsInExecuteDrop() )
    {
        // #i28468# don't show error message when called from Drag&Drop, silently abort instead
        return;
    }

	StopMarking();		// falls per Focus aus MouseButtonDown aufgerufen

	Window* pParent = aViewData.GetDialogParent();
	ScWaitCursorOff aWaitOff( pParent );
	sal_Bool bFocus = pParent && pParent->HasFocus();

	if(nGlobStrId==STR_PROTECTIONERR)
	{
		if(aViewData.GetDocShell()->IsReadOnly())
		{
			nGlobStrId=STR_READONLYERR;
		}
	}

	InfoBox aBox( pParent, ScGlobal::GetRscString( nGlobStrId ) );
	aBox.Execute();
	if (bFocus)
		pParent->GrabFocus();
}

Window* ScTabView::GetParentOrChild( sal_uInt16 nChildId )
{
	SfxViewFrame* pViewFrm = aViewData.GetViewShell()->GetViewFrame();

	if ( pViewFrm->HasChildWindow(nChildId) )
	{
		SfxChildWindow* pChild = pViewFrm->GetChildWindow(nChildId);
		if (pChild)
		{
			Window* pWin = pChild->GetWindow();
			if (pWin && pWin->IsVisible())
				return pWin;
		}
	}

	return aViewData.GetDialogParent();
}

void ScTabView::UpdatePageBreakData( sal_Bool bForcePaint )
{
	ScPageBreakData* pNewData = NULL;

	if (aViewData.IsPagebreakMode())
	{
		ScDocShell* pDocSh = aViewData.GetDocShell();
		ScDocument* pDoc = pDocSh->GetDocument();
		SCTAB nTab = aViewData.GetTabNo();

		sal_uInt16 nCount = pDoc->GetPrintRangeCount(nTab);
		if (!nCount)
			nCount = 1;
		pNewData = new ScPageBreakData(nCount);

		ScPrintFunc aPrintFunc( pDocSh, pDocSh->GetPrinter(), nTab, 0,0,NULL, NULL, pNewData );
		//	ScPrintFunc fuellt im ctor die PageBreakData
		if ( nCount > 1 )
		{
			aPrintFunc.ResetBreaks(nTab);
			pNewData->AddPages();
		}

		//	Druckbereiche veraendert?
		if ( bForcePaint || ( pPageBreakData && !pPageBreakData->IsEqual( *pNewData ) ) )
			PaintGrid();
	}

	delete pPageBreakData;
	pPageBreakData = pNewData;
}



