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
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/sdbc/XResultSet.hpp>

#include "dbfunc.hxx"
#include "docsh.hxx"
#include "attrib.hxx"
#include "sc.hrc"
#include "undodat.hxx"
#include "dbcolect.hxx"
#include "globstr.hrc"
#include "global.hxx"
#include "dbdocfun.hxx"
#include "editable.hxx"

//==================================================================

ScDBFunc::ScDBFunc( Window* pParent, ScDocShell& rDocSh, ScTabViewShell* pViewShell ) :
	ScViewFunc( pParent, rDocSh, pViewShell )
{
}

//UNUSED2008-05  ScDBFunc::ScDBFunc( Window* pParent, const ScDBFunc& rDBFunc, ScTabViewShell* pViewShell ) :
//UNUSED2008-05      ScViewFunc( pParent, rDBFunc, pViewShell )
//UNUSED2008-05  {
//UNUSED2008-05  }

ScDBFunc::~ScDBFunc()
{
}

//
//		Hilfsfunktionen
//

void ScDBFunc::GotoDBArea( const String& rDBName )
{
	ScDocument* pDoc = GetViewData()->GetDocument();
	ScDBCollection* pDBCol = pDoc->GetDBCollection();

	sal_uInt16 nFoundAt = 0;
	if ( pDBCol->SearchName( rDBName, nFoundAt ) )
	{
		ScDBData* pData = (*pDBCol)[nFoundAt];
		DBG_ASSERT( pData, "GotoDBArea: Datenbankbereich nicht gefunden!" );

		if ( pData )
		{
			SCTAB nTab = 0;
			SCCOL nStartCol = 0;
			SCROW nStartRow = 0;
			SCCOL nEndCol = 0;
			SCROW nEndRow = 0;

			pData->GetArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
			SetTabNo( nTab );

			MoveCursorAbs( nStartCol, nStartRow, ScFollowMode( SC_FOLLOW_JUMP ),
							   sal_False, sal_False );	// bShift,bControl
			DoneBlockMode();
			InitBlockMode( nStartCol, nStartRow, nTab );
			MarkCursor( nEndCol, nEndRow, nTab );
			SelectionChanged();
		}
	}
}

//	aktuellen Datenbereich fuer Sortieren / Filtern suchen

ScDBData* ScDBFunc::GetDBData( sal_Bool bMark, ScGetDBMode eMode, ScGetDBSelection eSel )
{
	ScDocShell* pDocSh = GetViewData()->GetDocShell();
	ScDBData* pData = NULL;
	ScRange aRange;
	ScMarkType eMarkType = GetViewData()->GetSimpleArea(aRange);
	if ( eMarkType == SC_MARK_SIMPLE || eMarkType == SC_MARK_SIMPLE_FILTERED )
    {
        bool bShrinkColumnsOnly = false;
        if (eSel == SC_DBSEL_ROW_DOWN)
        {
            // Don't alter row range, additional rows may have been selected on 
            // purpose to append data, or to have a fake header row.
            bShrinkColumnsOnly = true;
            // Select further rows only if only one row or a portion thereof is 
            // selected.
            if (aRange.aStart.Row() != aRange.aEnd.Row())
            {
                // If an area is selected shrink that to the actual used 
                // columns, don't draw filter buttons for empty columns.
                eSel = SC_DBSEL_SHRINK_TO_USED_DATA;
            }
            else if (aRange.aStart.Col() == aRange.aEnd.Col())
            {
                // One cell only, if it is not marked obtain entire used data 
                // area.
                const ScMarkData& rMarkData = GetViewData()->GetMarkData();
                if (!(rMarkData.IsMarked() || rMarkData.IsMultiMarked()))
                    eSel = SC_DBSEL_KEEP;
            }
        }
        switch (eSel)
        {
            case SC_DBSEL_SHRINK_TO_SHEET_DATA:
                {
                    // Shrink the selection to sheet data area.
                    ScDocument* pDoc = pDocSh->GetDocument();
                    SCCOL nCol1 = aRange.aStart.Col(), nCol2 = aRange.aEnd.Col();
                    SCROW nRow1 = aRange.aStart.Row(), nRow2 = aRange.aEnd.Row();
                    if (pDoc->ShrinkToDataArea( aRange.aStart.Tab(), nCol1, nRow1, nCol2, nRow2))
                    {
                        aRange.aStart.SetCol(nCol1);
                        aRange.aEnd.SetCol(nCol2);
                        aRange.aStart.SetRow(nRow1);
                        aRange.aEnd.SetRow(nRow2);
                    }
                }
                break;
            case SC_DBSEL_SHRINK_TO_USED_DATA:
            case SC_DBSEL_ROW_DOWN:
                {
                    // Shrink the selection to actual used area.
                    ScDocument* pDoc = pDocSh->GetDocument();
                    SCCOL nCol1 = aRange.aStart.Col(), nCol2 = aRange.aEnd.Col();
                    SCROW nRow1 = aRange.aStart.Row(), nRow2 = aRange.aEnd.Row();
                    bool bShrunk;
                    pDoc->ShrinkToUsedDataArea( bShrunk, aRange.aStart.Tab(), 
                            nCol1, nRow1, nCol2, nRow2, bShrinkColumnsOnly);
                    if (bShrunk)
                    {
                        aRange.aStart.SetCol(nCol1);
                        aRange.aEnd.SetCol(nCol2);
                        aRange.aStart.SetRow(nRow1);
                        aRange.aEnd.SetRow(nRow2);
                    }
                }
                break;
            default:
                ;   // nothing
        }
        pData = pDocSh->GetDBData( aRange, eMode, eSel );
    }
	else if ( eMode != SC_DB_OLD )
		pData = pDocSh->GetDBData(
					ScRange( GetViewData()->GetCurX(), GetViewData()->GetCurY(),
							 GetViewData()->GetTabNo() ),
					eMode, SC_DBSEL_KEEP );

	if ( pData && bMark )
	{
		ScRange aFound;
		pData->GetArea(aFound);
		MarkRange( aFound, sal_False );
	}
	return pData;
}

//	Datenbankbereiche aendern (Dialog)

void ScDBFunc::NotifyCloseDbNameDlg( const ScDBCollection& rNewColl, const List& rDelAreaList )
{

	ScDocShell* pDocShell = GetViewData()->GetDocShell();
	ScDocShellModificator aModificator( *pDocShell );
	ScDocument* pDoc = pDocShell->GetDocument();
	ScDBCollection* pOldColl = pDoc->GetDBCollection();
	ScDBCollection* pUndoColl = NULL;
	ScDBCollection* pRedoColl = NULL;
	const sal_Bool bRecord (pDoc->IsUndoEnabled());

	long nDelCount = rDelAreaList.Count();
	for (long nDelPos=0; nDelPos<nDelCount; nDelPos++)
	{
		ScRange* pEntry = (ScRange*) rDelAreaList.GetObject(nDelPos);

		if ( pEntry )
		{
			ScAddress& rStart = pEntry->aStart;
			ScAddress& rEnd   = pEntry->aEnd;
			pDocShell->DBAreaDeleted( rStart.Tab(),
									   rStart.Col(), rStart.Row(),
									   rEnd.Col(),   rEnd.Row() );

			//	Targets am SBA abmelden nicht mehr noetig
		}
	}

	if (bRecord)
		pUndoColl = new ScDBCollection( *pOldColl );

	//	neue Targets am SBA anmelden nicht mehr noetig

	pDoc->CompileDBFormula( sal_True );		// CreateFormulaString
	pDoc->SetDBCollection( new ScDBCollection( rNewColl ) );
	pDoc->CompileDBFormula( sal_False );	// CompileFormulaString
	pOldColl = NULL;
	pDocShell->PostPaint( 0,0,0, MAXCOL,MAXROW,MAXTAB, PAINT_GRID );
	aModificator.SetDocumentModified();
	SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );

	if (bRecord)
	{
		pRedoColl = new ScDBCollection( rNewColl );
		pDocShell->GetUndoManager()->AddUndoAction(
			new ScUndoDBData( pDocShell, pUndoColl, pRedoColl ) );
	}
}

//
//		wirkliche Funktionen
//

// Sortieren

void ScDBFunc::UISort( const ScSortParam& rSortParam, sal_Bool bRecord )
{
	ScDocShell* pDocSh = GetViewData()->GetDocShell();
	ScDocument* pDoc = pDocSh->GetDocument();
	SCTAB nTab = GetViewData()->GetTabNo();
	ScDBData* pDBData = pDoc->GetDBAtArea( nTab, rSortParam.nCol1, rSortParam.nRow1,
													rSortParam.nCol2, rSortParam.nRow2 );
	if (!pDBData)
	{
		DBG_ERROR( "Sort: keine DBData" );
		return;
	}

	ScSubTotalParam aSubTotalParam;
	pDBData->GetSubTotalParam( aSubTotalParam );
	if (aSubTotalParam.bGroupActive[0] && !aSubTotalParam.bRemoveOnly)
	{
		//	Subtotals wiederholen, mit neuer Sortierung

		DoSubTotals( aSubTotalParam, bRecord, &rSortParam );
	}
	else
	{
		Sort( rSortParam, bRecord );		// nur sortieren
	}
}

void ScDBFunc::Sort( const ScSortParam& rSortParam, sal_Bool bRecord, sal_Bool bPaint )
{
	ScDocShell* pDocSh = GetViewData()->GetDocShell();
	SCTAB nTab = GetViewData()->GetTabNo();
	ScDBDocFunc aDBDocFunc( *pDocSh );
	sal_Bool bSuccess = aDBDocFunc.Sort( nTab, rSortParam, bRecord, bPaint, sal_False );
	if ( bSuccess && !rSortParam.bInplace )
	{
		//	Ziel markieren
		ScRange aDestRange( rSortParam.nDestCol, rSortParam.nDestRow, rSortParam.nDestTab,
							rSortParam.nDestCol + rSortParam.nCol2 - rSortParam.nCol1,
							rSortParam.nDestRow + rSortParam.nRow2 - rSortParam.nRow1,
							rSortParam.nDestTab );
		MarkRange( aDestRange );
	}
}

//	Filtern

void ScDBFunc::Query( const ScQueryParam& rQueryParam, const ScRange* pAdvSource, sal_Bool bRecord )
{
	ScDocShell* pDocSh = GetViewData()->GetDocShell();
	SCTAB nTab = GetViewData()->GetTabNo();
	ScDBDocFunc aDBDocFunc( *pDocSh );
	sal_Bool bSuccess = aDBDocFunc.Query( nTab, rQueryParam, pAdvSource, bRecord, sal_False );

	if (bSuccess)
	{
		sal_Bool bCopy = !rQueryParam.bInplace;
		if (bCopy)
		{
			//	Zielbereich markieren (DB-Bereich wurde ggf. angelegt)
			ScDocument* pDoc = pDocSh->GetDocument();
			ScDBData* pDestData = pDoc->GetDBAtCursor(
											rQueryParam.nDestCol, rQueryParam.nDestRow,
											rQueryParam.nDestTab, sal_True );
			if (pDestData)
			{
				ScRange aDestRange;
				pDestData->GetArea(aDestRange);
				MarkRange( aDestRange );
			}
		}

		if (!bCopy)
        {
			UpdateScrollBars();
            SelectionChanged();     // for attribute states (filtered rows are ignored)
        }

		GetViewData()->GetBindings().Invalidate( SID_UNFILTER );
	}
}

//	Autofilter-Knoepfe ein-/ausblenden

void ScDBFunc::ToggleAutoFilter()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScDBData* pDBData = GetDBData( sal_False, SC_DB_MAKE_AUTOFILTER, SC_DBSEL_ROW_DOWN );
    if ( pDBData == NULL )
    {
        return;
    }

    // use a list action for the AutoFilter buttons (ScUndoAutoFilter) and the filter operation
    const String aUndo = ScGlobal::GetRscString( STR_UNDO_QUERY );
    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

    pDBData->SetByRow( sal_True );
    ScQueryParam aParam;
    pDBData->GetQueryParam( aParam );

    ScDocument* pDoc = GetViewData()->GetDocument();

    bool bHasAutoFilter = true;
    const SCROW  nRow = aParam.nRow1;
    const SCTAB  nTab = GetViewData()->GetTabNo();
    for ( SCCOL nCol=aParam.nCol1; nCol<=aParam.nCol2 && bHasAutoFilter; ++nCol )
    {
        const sal_Int16 nFlag =
            ((ScMergeFlagAttr*) pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();

        if ( (nFlag & SC_MF_AUTO) == 0 )
            bHasAutoFilter = false;
    }

    bool bPaint = false;

    if ( bHasAutoFilter )
    {
        // switch filter buttons
        for ( SCCOL nCol=aParam.nCol1; nCol<=aParam.nCol2; ++nCol )
        {
            const sal_Int16 nFlag =
                ((ScMergeFlagAttr*) pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
            pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
        }

        ScRange aRange;
        pDBData->GetArea( aRange );
        pDocSh->GetUndoManager()->AddUndoAction( new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), sal_False ) );

        pDBData->SetAutoFilter(sal_False);

        //	switch off filter
        const SCSIZE nEC = aParam.GetEntryCount();
        for ( SCSIZE i=0; i<nEC; ++i )
        {
            aParam.GetEntry(i).bDoQuery = sal_False;
        }
        aParam.bDuplicate = sal_True;
        Query( aParam, NULL, sal_True );

        // delete internal database range for auto filter
        if ( pDBData->IsInternalForAutoFilter() )
        {
            ScDBDocFunc aFunc(*pDocSh);
            aFunc.DeleteDBRange( pDBData->GetName(), sal_False );
        }
        pDBData = NULL;

        bPaint = true;
    }
    else
    {
        if ( !pDoc->IsBlockEmpty(
                nTab,
                aParam.nCol1,
                aParam.nRow1,
                aParam.nCol2,
                aParam.nRow2 ) )
        {
            if ( !pDBData->HasHeader() )
            {
                if ( MessBox(
                        GetViewData()->GetDialogParent(),
                        WinBits(WB_YES_NO | WB_DEF_YES),
                        ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),
                        ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 ) ).Execute() == RET_YES )
                {
                    pDBData->SetHeader( sal_True );
                }
            }

            ScRange aRange;
            pDBData->GetArea( aRange );
            pDocSh->GetUndoManager()->AddUndoAction( new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), sal_True ) );

            pDBData->SetAutoFilter(sal_True);

            for ( SCCOL nCol=aParam.nCol1; nCol<=aParam.nCol2; ++nCol )
            {
                const sal_Int16 nFlag =
                    ((ScMergeFlagAttr*) pDoc->GetAttr( nCol, nRow, nTab, ATTR_MERGE_FLAG ))->GetValue();
                pDoc->ApplyAttr( nCol, nRow, nTab, ScMergeFlagAttr( nFlag | SC_MF_AUTO ) );
            }
            pDocSh->PostPaint( aParam.nCol1, nRow, nTab, aParam.nCol2, nRow, nTab, PAINT_GRID );
            bPaint = true;
        }
        else
        {
            ErrorBox aErrorBox(
                GetViewData()->GetDialogParent(),
                WinBits( WB_OK | WB_DEF_OK ),
                ScGlobal::GetRscString( STR_ERR_AUTOFILTER ) );
            aErrorBox.Execute();
        }
    }

    pDocSh->GetUndoManager()->LeaveListAction();

    if ( bPaint )
    {
        aModificator.SetDocumentModified();

        SfxBindings& rBindings = GetViewData()->GetBindings();
        rBindings.Invalidate( SID_AUTO_FILTER );
        rBindings.Invalidate( SID_AUTOFILTER_HIDE );
    }
}

//		nur ausblenden, keine Daten veraendern

void ScDBFunc::HideAutoFilter()
{
    ScDocShell* pDocSh = GetViewData()->GetDocShell();
    ScDocShellModificator aModificator( *pDocSh );

    ScDBData* pDBData = GetDBData( sal_False );
    SCTAB nTab;
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    pDBData->GetArea(nTab, nCol1, nRow1, nCol2, nRow2);

    {
        ScDocument* pDoc = pDocSh->GetDocument();
        for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
        {
            const sal_Int16 nFlag =
                ((ScMergeFlagAttr*) pDoc->GetAttr( nCol, nRow1, nTab, ATTR_MERGE_FLAG ))->GetValue();
            pDoc->ApplyAttr( nCol, nRow1, nTab, ScMergeFlagAttr( nFlag & ~SC_MF_AUTO ) );
        }
    }

    const String aUndo = ScGlobal::GetRscString( STR_UNDO_QUERY );
    pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );
    {
        ScRange aRange;
        pDBData->GetArea( aRange );
        pDocSh->GetUndoManager()->AddUndoAction(
            new ScUndoAutoFilter( pDocSh, aRange, pDBData->GetName(), sal_False ) );

        pDBData->SetAutoFilter(sal_False);

        // delete internal database range for auto filter
        if ( pDBData->IsInternalForAutoFilter() )
        {
            ScDBDocFunc aFunc(*pDocSh);
            aFunc.DeleteDBRange( pDBData->GetName(), sal_False );
        }
        pDBData = NULL;
    }
    pDocSh->GetUndoManager()->LeaveListAction();

    pDocSh->PostPaint( nCol1,nRow1,nTab, nCol2,nRow1,nTab, PAINT_GRID );
    aModificator.SetDocumentModified();

    SfxBindings& rBindings = GetViewData()->GetBindings();
    rBindings.Invalidate( SID_AUTO_FILTER );
    rBindings.Invalidate( SID_AUTOFILTER_HIDE );
}

//		Re-Import

sal_Bool ScDBFunc::ImportData( const ScImportParam& rParam, sal_Bool bRecord )
{
	ScDocument* pDoc = GetViewData()->GetDocument();
	ScEditableTester aTester( pDoc, GetViewData()->GetTabNo(), rParam.nCol1,rParam.nRow1,
															rParam.nCol2,rParam.nRow2 );
	if ( !aTester.IsEditable() )
	{
		ErrorMessage(aTester.GetMessageId());
		return sal_False;
	}

	ScDBDocFunc aDBDocFunc( *GetViewData()->GetDocShell() );
    return aDBDocFunc.DoImport( GetViewData()->GetTabNo(), rParam, NULL, bRecord );
}



