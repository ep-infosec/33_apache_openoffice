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

// System - Includes -----------------------------------------------------




#include "scitems.hxx"
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/waitobj.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <svl/smplhint.hxx>

// INCLUDE ---------------------------------------------------------------

#include "docsh.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "undodat.hxx"
#include "undotab.hxx"
#include "undoblk.hxx"
//#include "pivot.hxx"
#include "dpobject.hxx"
#include "dpshttab.hxx"
#include "dbdocfun.hxx"
#include "consoli.hxx"
#include "dbcolect.hxx"
#include "olinetab.hxx"
#include "patattr.hxx"
#include "attrib.hxx"
#include "docpool.hxx"
#include "uiitems.hxx"
#include "sc.hrc"
#include "waitoff.hxx"
#include "sizedev.hxx"
#include <basic/sbstar.hxx>
#include <basic/basmgr.hxx>

// defined in docfunc.cxx
void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, String& sModuleName, String& sModuleSource );

// ---------------------------------------------------------------------------

//
//	ehemalige viewfunc/dbfunc Methoden
//

void ScDocShell::ErrorMessage( sal_uInt16 nGlobStrId )
{
	//!	StopMarking an der (aktiven) View?

	Window* pParent = GetActiveDialogParent();
	ScWaitCursorOff aWaitOff( pParent );
	sal_Bool bFocus = pParent && pParent->HasFocus();

	if(nGlobStrId==STR_PROTECTIONERR)
	{
		if(IsReadOnly())
		{
			nGlobStrId=STR_READONLYERR;
		}
	}

	InfoBox aBox( pParent, ScGlobal::GetRscString( nGlobStrId ) );
	aBox.Execute();
	if (bFocus)
		pParent->GrabFocus();
}

sal_Bool ScDocShell::IsEditable() const
{
	// import into read-only document is possible - must be extended if other filters use api
	// #i108547# MSOOXML filter uses "IsChangeReadOnlyEnabled" property

	return !IsReadOnly() || aDocument.IsImportingXML() || aDocument.IsChangeReadOnlyEnabled();
}

void ScDocShell::DBAreaDeleted( SCTAB nTab, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW /* nY2 */ )
{
	ScDocShellModificator aModificator( *this );
	aDocument.RemoveFlagsTab( nX1, nY1, nX2, nY1, nTab, SC_MF_AUTO );
	PostPaint( nX1, nY1, nTab, nX2, nY1, nTab, PAINT_GRID );
    // No SetDocumentModified, as the unnamed database range might have to be restored later.
    // The UNO hint is broadcast directly instead, to keep UNO objects in valid state.
    aDocument.BroadcastUno( SfxSimpleHint( SFX_HINT_DATACHANGED ) );
}


ScDBData* lcl_GetDBNearCursor(
    const ScDBCollection* pColl,
    const SCCOL nCol,
    const SCROW nRow,
    const SCTAB nTab )
{
    //!	nach document/dbcolect verschieben

    if (!pColl)
        return NULL;

    ScDBData* pInternalDBData = NULL;
    ScDBData* pNearData = NULL;
    sal_uInt16 nCount = pColl->GetCount();
    SCTAB nAreaTab;
    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        ScDBData* pDB = (*pColl)[i];
        pDB->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if ( nTab == nAreaTab
             && nCol+1 >= nStartCol
             && nCol <= nEndCol+1
             && nRow+1 >= nStartRow
             && nRow <= nEndRow+1 )
        {
            if ( pDB->IsInternalUnnamed()
                 || pDB->IsInternalForAutoFilter() )
            {
                pInternalDBData = pDB;
            }
            else if ( nCol < nStartCol || nCol > nEndCol || nRow < nStartRow || nRow > nEndRow )
            {
                if ( !pNearData )
                    pNearData = pDB;	// ersten angrenzenden Bereich merken
            }
            else
                return pDB;				// nicht "unbenannt" und Cursor steht wirklich drin
        }
    }
    if (pNearData)
        return pNearData;				// angrenzender, wenn nichts direkt getroffen
    return pInternalDBData;
}

ScDBData* ScDocShell::GetDBData( const ScRange& rMarked, ScGetDBMode eMode, ScGetDBSelection eSel )
{
    const SCCOL nCol = rMarked.aStart.Col();
    const SCROW nRow = rMarked.aStart.Row();
    const SCTAB nTab = rMarked.aStart.Tab();

    SCCOL nStartCol = nCol;
    SCROW nStartRow = nRow;
    SCCOL nEndCol = rMarked.aEnd.Col();
    SCROW nEndRow = rMarked.aEnd.Row();

    ScDBData* pFoundDBData = aDocument.GetDBAtArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow );
    if ( pFoundDBData == NULL )
    {
        pFoundDBData = lcl_GetDBNearCursor( aDocument.GetDBCollection(), nCol, nRow, nTab );
    }

    const bool bSelected =
        ( eSel == SC_DBSEL_FORCE_MARK
          || ( rMarked.aStart != rMarked.aEnd
               && eSel != SC_DBSEL_ROW_DOWN ) );
    const bool bOnlyDown = ( !bSelected
                             && eSel == SC_DBSEL_ROW_DOWN
                             && rMarked.aStart.Row() == rMarked.aEnd.Row());

    bool bUseFoundDBData = false;
    if ( pFoundDBData )
    {
        // check, if found database range can be used
        SCTAB nDummy;
        SCCOL nOldCol1;
        SCROW nOldRow1;
        SCCOL nOldCol2;
        SCROW nOldRow2;
        pFoundDBData->GetArea( nDummy, nOldCol1, nOldRow1, nOldCol2, nOldRow2 );

        const bool bIsUnnamedOne = pFoundDBData->IsInternalUnnamed();
        const bool bIsInternalForAutoFilter = pFoundDBData->IsInternalForAutoFilter();
        if ( !bSelected )
        {
            bUseFoundDBData = true;
            if ( ( bIsUnnamedOne || bIsInternalForAutoFilter )
                 && ( eMode == SC_DB_MAKE || eMode == SC_DB_MAKE_AUTOFILTER ) )
            {
                // If nothing marked or only one row marked, adapt found database range to contiguous area.
                nStartCol = nCol;
                nStartRow = nRow;
                if ( bOnlyDown )
                {
                    nEndCol = rMarked.aEnd.Col();
                    nEndRow = rMarked.aEnd.Row();
                }
                else
                {
                    nEndCol = nStartCol;
                    nEndRow = nStartRow;
                }
                aDocument.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, sal_False, bOnlyDown );
                if ( nOldCol1 != nStartCol || nOldCol2 != nEndCol || nOldRow1 != nStartRow )
                {
                    bUseFoundDBData = false;
                }
                else if ( nOldRow2 != nEndRow )
                {
                    // adapt found internal database range to new end row
                    pFoundDBData->SetArea( nTab, nOldCol1,nOldRow1, nOldCol2,nEndRow );
                }
            }
        }
        else
        {
            if ( nOldCol1 == nStartCol
                 && nOldRow1 == nStartRow
                 && nOldCol2 == nEndCol
                 && nOldRow2 == nEndRow )
            {
                bUseFoundDBData = true;
            }
            else
            {
                bUseFoundDBData = false;
            }
        }

        // adapt internal unnamed database range to an auto filter one
        // otherwise the auto filter is lost when the internal unnamed one is changed/reused/deleted
        if ( bUseFoundDBData
             && eMode == SC_DB_MAKE_AUTOFILTER
             && bIsUnnamedOne )
        {
            pFoundDBData->SetName( aDocument.GetDBCollection()->GetNewDefaultDBName() );
        }

        // no internal database range for Import
        if ( bUseFoundDBData
             && eMode == SC_DB_IMPORT
             && ( bIsUnnamedOne || bIsInternalForAutoFilter ) )
        {
            bUseFoundDBData = false;
        }
    }

    if ( bUseFoundDBData )
    {
        return pFoundDBData;
    }

    if ( eMode == SC_DB_OLD )
    {
        // no existing database range found
        return NULL;
    }
    else // eMode == SC_DB_MAKE||SC_DB_IMPORT||SC_DB_MAKE_AUTOFILTER
    {
        if ( !bSelected )
        {
            nStartCol = nCol;
            nStartRow = nRow;
            if (bOnlyDown)
            {
                nEndCol = rMarked.aEnd.Col();
                nEndRow = rMarked.aEnd.Row();
            }
            else
            {
                nEndCol = nStartCol;
                nEndRow = nStartRow;
            }
            aDocument.GetDataArea( nTab, nStartCol, nStartRow, nEndCol, nEndRow, sal_False, bOnlyDown );
        }

        const sal_Bool bHasHeader = aDocument.HasColHeader( nStartCol, nStartRow, nEndCol, nEndRow, nTab );

        ScDBData* pDBData = NULL;
        sal_uInt16 nUnnamedDBIndex;
        ScDBCollection* pColl = aDocument.GetDBCollection();
        if ( eMode == SC_DB_MAKE &&
             pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nUnnamedDBIndex ) )
        {
            // adapt existing unnamed database range
            pDBData = (*pColl)[nUnnamedDBIndex];

            if ( !pOldAutoDBRange )
            {
                // store the old unnamed database range with its settings for undo
                // (store at the first change, get the state before all changes)
                pOldAutoDBRange = new ScDBData( *pDBData );
            }

            SCCOL nOldX1;
            SCROW nOldY1;
            SCCOL nOldX2;
            SCROW nOldY2;
            SCTAB nOldTab;
            pDBData->GetArea( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );
            DBAreaDeleted( nOldTab, nOldX1, nOldY1, nOldX2, nOldY2 );

            pDBData->SetSortParam( ScSortParam() );
            pDBData->SetQueryParam( ScQueryParam() );
            pDBData->SetSubTotalParam( ScSubTotalParam() );

            pDBData->SetArea( nTab, nStartCol,nStartRow, nEndCol,nEndRow );
            pDBData->SetByRow( sal_True );
            pDBData->SetHeader( bHasHeader );
            pDBData->SetAutoFilter( sal_False );
        }
        else
        {
            ScDBCollection* pUndoColl = NULL;

            String aNewName;
            switch ( eMode )
            {
            case SC_DB_IMPORT:
            {
                aDocument.CompileDBFormula( sal_True );
                pUndoColl = new ScDBCollection( *pColl );

                String aImport = ScGlobal::GetRscString( STR_DBNAME_IMPORT );
                long nCount = 0;
                sal_uInt16 nDummy;
                do
                {
                    ++nCount;
                    aNewName = aImport;
                    aNewName += String::CreateFromInt32( nCount );
                }
                while (pColl->SearchName( aNewName, nDummy ));
            }
            break;

            case SC_DB_MAKE_AUTOFILTER:
            {
                aDocument.CompileDBFormula( sal_True );
                pUndoColl = new ScDBCollection( *pColl );

                aNewName = pColl->GetNewDefaultDBName();
            }
            break;

            case SC_DB_MAKE:
            {
                aNewName = ScGlobal::GetRscString( STR_DB_NONAME );
            }
            break;

            default:
                DBG_ERROR( "<ScDocShell::GetDBData(..)> - unexcepted <eMode>" );
                break;
            }

            pDBData = new ScDBData(
                aNewName, nTab, nStartCol, nStartRow, nEndCol, nEndRow, sal_True, bHasHeader );
            pColl->Insert( pDBData );

            if ( pUndoColl )
            {
                aDocument.CompileDBFormula( sal_False );

                ScDBCollection* pRedoColl = new ScDBCollection( *pColl );
                GetUndoManager()->AddUndoAction( new ScUndoDBData( this, pUndoColl, pRedoColl ) );
            }

            // notify Navigator about database range "Import[X]"
            if ( eMode==SC_DB_IMPORT )
            {
                SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_DBAREAS_CHANGED ) );
            }
        }

        return pDBData;
    }
}


ScDBData* ScDocShell::GetOldAutoDBRange()
{
    ScDBData* pRet = pOldAutoDBRange;
    pOldAutoDBRange = NULL;
    return pRet;                    // has to be deleted by caller!
}

void ScDocShell::CancelAutoDBRange()
{
    // called when dialog is cancelled
    if ( pOldAutoDBRange )
    {
        sal_uInt16 nNoNameIndex;
        ScDBCollection* pColl = aDocument.GetDBCollection();
        if ( pColl->SearchName( ScGlobal::GetRscString( STR_DB_NONAME ), nNoNameIndex ) )
        {
            ScDBData* pNoNameData = (*pColl)[nNoNameIndex];

            SCCOL nRangeX1;
            SCROW nRangeY1;
            SCCOL nRangeX2;
            SCROW nRangeY2;
            SCTAB nRangeTab;
            pNoNameData->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
            DBAreaDeleted( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );

            *pNoNameData = *pOldAutoDBRange;    // restore old settings

            if ( pOldAutoDBRange->HasAutoFilter() )
            {
                // restore AutoFilter buttons
                pOldAutoDBRange->GetArea( nRangeTab, nRangeX1, nRangeY1, nRangeX2, nRangeY2 );
                aDocument.ApplyFlagsTab( nRangeX1, nRangeY1, nRangeX2, nRangeY1, nRangeTab, SC_MF_AUTO );
                PostPaint( nRangeX1, nRangeY1, nRangeTab, nRangeX2, nRangeY1, nRangeTab, PAINT_GRID );
            }
        }

        delete pOldAutoDBRange;
        pOldAutoDBRange = NULL;
    }
}


		//	Hoehen anpassen
		//!	mit docfunc zusammenfassen

sal_Bool ScDocShell::AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab )
{
	ScSizeDeviceProvider aProv(this);
	Fraction aZoom(1,1);
	sal_Bool bChange = aDocument.SetOptimalHeight( nStartRow,nEndRow, nTab, 0, aProv.GetDevice(),
												aProv.GetPPTX(),aProv.GetPPTY(), aZoom,aZoom, sal_False );
	if (bChange)
		PostPaint( 0,nStartRow,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID|PAINT_LEFT );

	return bChange;
}

void ScDocShell::UpdateAllRowHeights( const ScMarkData* pTabMark )
{
	// update automatic row heights

	ScSizeDeviceProvider aProv(this);
	Fraction aZoom(1,1);
    aDocument.UpdateAllRowHeights( aProv.GetDevice(), aProv.GetPPTX(), aProv.GetPPTY(), aZoom, aZoom, pTabMark );
}

void ScDocShell::UpdatePendingRowHeights( SCTAB nUpdateTab, bool bBefore )
{
    sal_Bool bIsUndoEnabled = aDocument.IsUndoEnabled();
    aDocument.EnableUndo( sal_False );
    aDocument.LockStreamValid( true );      // ignore draw page size (but not formula results)
    if ( bBefore )          // check all sheets up to nUpdateTab
    {
        SCTAB nTabCount = aDocument.GetTableCount();
        if ( nUpdateTab >= nTabCount )
            nUpdateTab = nTabCount-1;     // nUpdateTab is inclusive

        ScMarkData aUpdateSheets;
        SCTAB nTab;
        for (nTab=0; nTab<=nUpdateTab; ++nTab)
            if ( aDocument.IsPendingRowHeights( nTab ) )
                aUpdateSheets.SelectTable( nTab, sal_True );

        if (aUpdateSheets.GetSelectCount())
            UpdateAllRowHeights(&aUpdateSheets);        // update with a single progress bar

        for (nTab=0; nTab<=nUpdateTab; ++nTab)
            if ( aUpdateSheets.GetTableSelect( nTab ) )
            {
                aDocument.UpdatePageBreaks( nTab );
                aDocument.SetPendingRowHeights( nTab, sal_False );
            }
    }
    else                    // only nUpdateTab
    {
        if ( aDocument.IsPendingRowHeights( nUpdateTab ) )
        {
            AdjustRowHeight( 0, MAXROW, nUpdateTab );
            aDocument.UpdatePageBreaks( nUpdateTab );
            aDocument.SetPendingRowHeights( nUpdateTab, sal_False );
        }
    }
    aDocument.LockStreamValid( false );
    aDocument.EnableUndo( bIsUndoEnabled );
}

void ScDocShell::RefreshPivotTables( const ScRange& rSource )
{
	//!	rename to RefreshDataPilotTables?

	ScDPCollection* pColl = aDocument.GetDPCollection();
	if ( pColl )
	{
		//	DataPilotUpdate doesn't modify the collection order like PivotUpdate did,
		//	so a simple loop can be used.

		sal_uInt16 nCount = pColl->GetCount();
		for ( sal_uInt16 i=0; i<nCount; i++ )
		{
			ScDPObject* pOld = (*pColl)[i];
			if ( pOld )
			{
				const ScSheetSourceDesc* pSheetDesc = pOld->GetSheetDesc();
				if ( pSheetDesc && pSheetDesc->aSourceRange.Intersects( rSource ) )
				{
					ScDPObject* pNew = new ScDPObject( *pOld );
					ScDBDocFunc aFunc( *this );
					aFunc.DataPilotUpdate( pOld, pNew, sal_True, sal_False );
					delete pNew;	// DataPilotUpdate copies settings from "new" object
				}
			}
		}
	}
}

String lcl_GetAreaName( ScDocument* pDoc, ScArea* pArea )
{
    String aName;
    bool bOk = false;
    ScDBData* pData = pDoc->GetDBAtArea( pArea->nTab, pArea->nColStart, pArea->nRowStart, pArea->nColEnd, pArea->nRowEnd );
    if (pData)
    {
        pData->GetName( aName );
        if ( !pData->IsInternalUnnamed() )
            bOk = true;
    }

    if (!bOk)
    {
        pDoc->GetName( pArea->nTab, aName );
    }

    return aName;
}

void ScDocShell::DoConsolidate( const ScConsolidateParam& rParam, sal_Bool bRecord )
{
	ScConsData aData;

	sal_uInt16 nPos;
	SCCOL nColSize = 0;
	SCROW nRowSize = 0;
	sal_Bool bErr = sal_False;
	for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
	{
		ScArea* pArea = rParam.ppDataAreas[nPos];
		nColSize = Max( nColSize, SCCOL( pArea->nColEnd - pArea->nColStart + 1 ) );
		nRowSize = Max( nRowSize, SCROW( pArea->nRowEnd - pArea->nRowStart + 1 ) );

										// Test, ob Quelldaten verschoben wuerden
		if (rParam.bReferenceData)
			if (pArea->nTab == rParam.nTab && pArea->nRowEnd >= rParam.nRow)
				bErr = sal_True;
	}

	if (bErr)
	{
		InfoBox aBox( GetActiveDialogParent(),
				ScGlobal::GetRscString( STR_CONSOLIDATE_ERR1 ) );
		aBox.Execute();
		return;
	}

	//		ausfuehren

	WaitObject aWait( GetActiveDialogParent() );
	ScDocShellModificator aModificator( *this );

	ScRange aOldDest;
	ScDBData* pDestData = aDocument.GetDBAtCursor( rParam.nCol, rParam.nRow, rParam.nTab, sal_True );
	if (pDestData)
		pDestData->GetArea(aOldDest);

	aData.SetSize( nColSize, nRowSize );
	aData.SetFlags( rParam.eFunction, rParam.bByCol, rParam.bByRow, rParam.bReferenceData );
	if ( rParam.bByCol || rParam.bByRow )
		for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
		{
			ScArea* pArea = rParam.ppDataAreas[nPos];
			aData.AddFields( &aDocument, pArea->nTab, pArea->nColStart, pArea->nRowStart,
														pArea->nColEnd, pArea->nRowEnd );
		}
	aData.DoneFields();
	for (nPos=0; nPos<rParam.nDataAreaCount; nPos++)
	{
		ScArea* pArea = rParam.ppDataAreas[nPos];
		aData.AddData( &aDocument, pArea->nTab, pArea->nColStart, pArea->nRowStart,
													pArea->nColEnd, pArea->nRowEnd );
		aData.AddName( lcl_GetAreaName(&aDocument,pArea) );
	}

	aData.GetSize( nColSize, nRowSize );
	if (bRecord && nColSize > 0 && nRowSize > 0)
	{
		ScDBData* pUndoData = pDestData ? new ScDBData(*pDestData) : NULL;

		SCTAB nDestTab = rParam.nTab;
		ScArea aDestArea( rParam.nTab, rParam.nCol, rParam.nRow,
							rParam.nCol+nColSize-1, rParam.nRow+nRowSize-1 );
		if (rParam.bByCol) ++aDestArea.nColEnd;
		if (rParam.bByRow) ++aDestArea.nRowEnd;

		if (rParam.bReferenceData)
		{
			SCTAB nTabCount = aDocument.GetTableCount();
			SCROW nInsertCount = aData.GetInsertCount();

			// alte Outlines
			ScOutlineTable* pTable = aDocument.GetOutlineTable( nDestTab );
			ScOutlineTable* pUndoTab = pTable ? new ScOutlineTable( *pTable ) : NULL;

			ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
			pUndoDoc->InitUndo( &aDocument, 0, nTabCount-1, sal_False, sal_True );

			// Zeilenstatus
			aDocument.CopyToDocument( 0,0,nDestTab, MAXCOL,MAXROW,nDestTab,
									IDF_NONE, sal_False, pUndoDoc );

			// alle Formeln
			aDocument.CopyToDocument( 0,0,0, MAXCOL,MAXROW,nTabCount-1,
										IDF_FORMULA, sal_False, pUndoDoc );

			// komplette Ausgangszeilen
			aDocument.CopyToDocument( 0,aDestArea.nRowStart,nDestTab,
									MAXCOL,aDestArea.nRowEnd,nDestTab,
									IDF_ALL, sal_False, pUndoDoc );

			// alten Ausgabebereich
			if (pDestData)
				aDocument.CopyToDocument( aOldDest, IDF_ALL, sal_False, pUndoDoc );

			GetUndoManager()->AddUndoAction(
					new ScUndoConsolidate( this, aDestArea, rParam, pUndoDoc,
											sal_True, nInsertCount, pUndoTab, pUndoData ) );
		}
		else
		{
			ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
			pUndoDoc->InitUndo( &aDocument, aDestArea.nTab, aDestArea.nTab );

			aDocument.CopyToDocument( aDestArea.nColStart, aDestArea.nRowStart, aDestArea.nTab,
									aDestArea.nColEnd, aDestArea.nRowEnd, aDestArea.nTab,
									IDF_ALL, sal_False, pUndoDoc );

			// alten Ausgabebereich
			if (pDestData)
				aDocument.CopyToDocument( aOldDest, IDF_ALL, sal_False, pUndoDoc );

			GetUndoManager()->AddUndoAction(
					new ScUndoConsolidate( this, aDestArea, rParam, pUndoDoc,
											sal_False, 0, NULL, pUndoData ) );
		}
	}

	if (pDestData)										// Zielbereich loeschen / anpassen
	{
		aDocument.DeleteAreaTab(aOldDest, IDF_CONTENTS);
		pDestData->SetArea( rParam.nTab, rParam.nCol, rParam.nRow,
							rParam.nCol + nColSize - 1, rParam.nRow + nRowSize - 1 );
		pDestData->SetHeader( rParam.bByRow );
	}

	aData.OutputToDocument( &aDocument, rParam.nCol, rParam.nRow, rParam.nTab );

	SCCOL nPaintStartCol = rParam.nCol;
	SCROW nPaintStartRow = rParam.nRow;
	SCCOL nPaintEndCol = nPaintStartCol + nColSize - 1;
	SCROW nPaintEndRow = nPaintStartRow + nRowSize - 1;
	sal_uInt16 nPaintFlags = PAINT_GRID;
	if (rParam.bByCol)
		++nPaintEndRow;
	if (rParam.bByRow)
		++nPaintEndCol;
	if (rParam.bReferenceData)
	{
		nPaintStartCol = 0;
		nPaintEndCol = MAXCOL;
		nPaintEndRow = MAXROW;
		nPaintFlags |= PAINT_LEFT | PAINT_SIZE;
	}
	if (pDestData)
	{
		if ( aOldDest.aEnd.Col() > nPaintEndCol )
			nPaintEndCol = aOldDest.aEnd.Col();
		if ( aOldDest.aEnd.Row() > nPaintEndRow )
			nPaintEndRow = aOldDest.aEnd.Row();
	}
	PostPaint( nPaintStartCol, nPaintStartRow, rParam.nTab,
				nPaintEndCol, nPaintEndRow, rParam.nTab, nPaintFlags );
	aModificator.SetDocumentModified();
}

void ScDocShell::UseScenario( SCTAB nTab, const String& rName, sal_Bool bRecord )
{
	if (!aDocument.IsScenario(nTab))
	{
		SCTAB	nTabCount = aDocument.GetTableCount();
		SCTAB	nSrcTab = SCTAB_MAX;
		SCTAB	nEndTab = nTab;
		String aCompare;
		while ( nEndTab+1 < nTabCount && aDocument.IsScenario(nEndTab+1) )
		{
			++nEndTab;
			if (nSrcTab > MAXTAB)			// noch auf der Suche nach dem Szenario?
			{
				aDocument.GetName( nEndTab, aCompare );
				if (aCompare == rName)
					nSrcTab = nEndTab;		// gefunden
			}
		}
		if (ValidTab(nSrcTab))
		{
			if ( aDocument.TestCopyScenario( nSrcTab, nTab ) )			// Zellschutz testen
			{
				ScDocShellModificator aModificator( *this );
				ScMarkData aScenMark;
				aDocument.MarkScenario( nSrcTab, nTab, aScenMark );
				ScRange aMultiRange;
				aScenMark.GetMultiMarkArea( aMultiRange );
				SCCOL nStartCol = aMultiRange.aStart.Col();
				SCROW nStartRow = aMultiRange.aStart.Row();
				SCCOL nEndCol = aMultiRange.aEnd.Col();
				SCROW nEndRow = aMultiRange.aEnd.Row();

				if (bRecord)
				{
					ScDocument* pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
					pUndoDoc->InitUndo( &aDocument, nTab,nEndTab );				// auch alle Szenarien
					//	angezeigte Tabelle:
					aDocument.CopyToDocument( nStartCol,nStartRow,nTab,
									nEndCol,nEndRow,nTab, IDF_ALL,sal_True, pUndoDoc, &aScenMark );
					//	Szenarien
					for (SCTAB i=nTab+1; i<=nEndTab; i++)
					{
						pUndoDoc->SetScenario( i, sal_True );
						String aComment;
						Color  aColor;
						sal_uInt16 nScenFlags;
						aDocument.GetScenarioData( i, aComment, aColor, nScenFlags );
						pUndoDoc->SetScenarioData( i, aComment, aColor, nScenFlags );
						sal_Bool bActive = aDocument.IsActiveScenario( i );
						pUndoDoc->SetActiveScenario( i, bActive );
						//	Bei Zurueckkopier-Szenarios auch Inhalte
						if ( nScenFlags & SC_SCENARIO_TWOWAY )
							aDocument.CopyToDocument( 0,0,i, MAXCOL,MAXROW,i,
														IDF_ALL,sal_False, pUndoDoc );
					}

					GetUndoManager()->AddUndoAction(
						new ScUndoUseScenario( this, aScenMark,
										ScArea( nTab,nStartCol,nStartRow,nEndCol,nEndRow ),
										pUndoDoc, rName ) );
				}

				aDocument.CopyScenario( nSrcTab, nTab );
				aDocument.SetDirty();

				//	alles painten, weil in anderen Bereichen das aktive Szenario
				//	geaendert sein kann
				//!	nur, wenn sichtbare Rahmen vorhanden?
				PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );
				aModificator.SetDocumentModified();
			}
			else
			{
				InfoBox aBox(GetActiveDialogParent(),
					ScGlobal::GetRscString( STR_PROTECTIONERR ) );
				aBox.Execute();
			}
		}
		else
		{
			InfoBox aBox(GetActiveDialogParent(),
				ScGlobal::GetRscString( STR_SCENARIO_NOTFOUND ) );
			aBox.Execute();
		}
	}
	else
	{
		DBG_ERROR( "UseScenario auf Szenario-Blatt" );
	}
}

void ScDocShell::ModifyScenario( SCTAB nTab, const String& rName, const String& rComment,
									const Color& rColor, sal_uInt16 nFlags )
{
	//	Undo
	String aOldName;
	aDocument.GetName( nTab, aOldName );
	String aOldComment;
	Color aOldColor;
	sal_uInt16 nOldFlags;
	aDocument.GetScenarioData( nTab, aOldComment, aOldColor, nOldFlags );
	GetUndoManager()->AddUndoAction(
		new ScUndoScenarioFlags( this, nTab,
				aOldName, rName, aOldComment, rComment,
				aOldColor, rColor, nOldFlags, nFlags ) );

	//	ausfuehren
	ScDocShellModificator aModificator( *this );
	aDocument.RenameTab( nTab, rName );
	aDocument.SetScenarioData( nTab, rComment, rColor, nFlags );
	PostPaintGridAll();
	aModificator.SetDocumentModified();

	if ( rName != aOldName )
		SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

	SfxBindings* pBindings = GetViewBindings();
	if (pBindings)
		pBindings->Invalidate( SID_SELECT_SCENARIO );
}

SCTAB ScDocShell::MakeScenario( SCTAB nTab, const String& rName, const String& rComment,
									const Color& rColor, sal_uInt16 nFlags,
									ScMarkData& rMark, sal_Bool bRecord )
{
	rMark.MarkToMulti();
	if (rMark.IsMultiMarked())
	{
		SCTAB nNewTab = nTab + 1;
		while (aDocument.IsScenario(nNewTab))
			++nNewTab;

		sal_Bool bCopyAll = ( (nFlags & SC_SCENARIO_COPYALL) != 0 );
		const ScMarkData* pCopyMark = NULL;
		if (!bCopyAll)
			pCopyMark = &rMark;

		ScDocShellModificator aModificator( *this );

        if (bRecord)
            aDocument.BeginDrawUndo();      // drawing layer must do its own undo actions

		if (aDocument.CopyTab( nTab, nNewTab, pCopyMark ))
		{
			if (bRecord)
			{
				GetUndoManager()->AddUndoAction(
                        new ScUndoMakeScenario( this, nTab, nNewTab,
												rName, rComment, rColor, nFlags, rMark ));
			}

			aDocument.RenameTab( nNewTab, rName, sal_False );			// ohne Formel-Update
			aDocument.SetScenario( nNewTab, sal_True );
			aDocument.SetScenarioData( nNewTab, rComment, rColor, nFlags );

			ScMarkData aDestMark = rMark;
			aDestMark.SelectOneTable( nNewTab );

			//!		auf Filter / Buttons / Merging testen !

			ScPatternAttr aProtPattern( aDocument.GetPool() );
			aProtPattern.GetItemSet().Put( ScProtectionAttr( sal_True ) );
			aDocument.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nNewTab, aProtPattern );

			ScPatternAttr aPattern( aDocument.GetPool() );
			aPattern.GetItemSet().Put( ScMergeFlagAttr( SC_MF_SCENARIO ) );
			aPattern.GetItemSet().Put( ScProtectionAttr( sal_True ) );
			aDocument.ApplySelectionPattern( aPattern, aDestMark );

			if (!bCopyAll)
				aDocument.SetVisible( nNewTab, sal_False );

			//	dies ist dann das aktive Szenario
			aDocument.CopyScenario( nNewTab, nTab, sal_True );	// sal_True - nicht aus Szenario kopieren

			if (nFlags & SC_SCENARIO_SHOWFRAME)
				PostPaint( 0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID );	// Rahmen painten
			PostPaintExtras();											// Tabellenreiter
			aModificator.SetDocumentModified();

			SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

			return nNewTab;
		}
	}
	return nTab;
}

sal_Bool ScDocShell::MoveTable( SCTAB nSrcTab, SCTAB nDestTab, sal_Bool bCopy, sal_Bool bRecord )
{
	ScDocShellModificator aModificator( *this );

    // #i92477# be consistent with ScDocFunc::InsertTable: any index past the last sheet means "append"
    // #i101139# nDestTab must be the target position, not APPEND (for CopyTabProtection etc.)
    if ( nDestTab >= aDocument.GetTableCount() )
        nDestTab = aDocument.GetTableCount();

	if (bCopy)
	{
		if (bRecord)
			aDocument.BeginDrawUndo();			// drawing layer must do its own undo actions

        String sSrcCodeName;
        aDocument.GetCodeName( nSrcTab, sSrcCodeName );
		if (!aDocument.CopyTab( nSrcTab, nDestTab ))
		{
			//!	EndDrawUndo?
			return sal_False;
		}
		else
		{
			SCTAB nAdjSource = nSrcTab;
			if ( nDestTab <= nSrcTab )
				++nAdjSource;				// new position of source table after CopyTab

			if ( aDocument.IsTabProtected( nAdjSource ) )
                aDocument.CopyTabProtection(nAdjSource, nDestTab);

			if (bRecord)
			{
				SvShorts aSrcList;
				SvShorts aDestList;
				aSrcList.push_front(nSrcTab);
				aDestList.push_front(nDestTab);
				GetUndoManager()->AddUndoAction(
						new ScUndoCopyTab( this, aSrcList, aDestList ) );
			}
			
			sal_Bool bVbaEnabled = aDocument.IsInVBAMode();
                        if ( bVbaEnabled )
                        {
			    StarBASIC* pStarBASIC = GetBasic();
                            String aLibName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
                            if ( GetBasicManager()->GetName().Len() > 0 )
                            {
                                aLibName = GetBasicManager()->GetName();
                                pStarBASIC = GetBasicManager()->GetLib( aLibName );
                            }
                            SCTAB nTabToUse = nDestTab; 
                            if ( nDestTab == SC_TAB_APPEND )
                                nTabToUse = aDocument.GetMaxTableNumber() - 1;
                            String sCodeName;
                            String sSource;
                            com::sun::star::uno::Reference< com::sun::star::script::XLibraryContainer > xLibContainer = GetBasicContainer();
                            com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > xLib;
                            if( xLibContainer.is() )
                            {
                                com::sun::star::uno::Any aLibAny = xLibContainer->getByName( aLibName );
                                aLibAny >>= xLib;
                            }
                            if( xLib.is() )
                            {
                                rtl::OUString sRTLSource;
                                xLib->getByName( sSrcCodeName ) >>= sRTLSource;
                                sSource = sRTLSource;
                            } 
                            VBA_InsertModule( aDocument, nTabToUse, sCodeName, sSource );
                        }
                }
		Broadcast( ScTablesHint( SC_TAB_COPIED, nSrcTab, nDestTab ) );
	}
	else
	{
		if ( aDocument.GetChangeTrack() )
			return sal_False;

		if ( nSrcTab<nDestTab && nDestTab!=SC_TAB_APPEND )
			nDestTab--;

		if ( nSrcTab == nDestTab )
		{
			//!	allow only for api calls?
			return sal_True;	// nothing to do, but valid
		}

		if (!aDocument.MoveTab( nSrcTab, nDestTab ))
			return sal_False;
		else if (bRecord)
		{
			SvShorts aSrcList;
			SvShorts aDestList;
			aSrcList.push_front(nSrcTab);
			aDestList.push_front(nDestTab);
			GetUndoManager()->AddUndoAction(
					new ScUndoMoveTab( this, aSrcList, aDestList ) );
		}

		Broadcast( ScTablesHint( SC_TAB_MOVED, nSrcTab, nDestTab ) );
	}

	PostPaintGridAll();
	PostPaintExtras();
	aModificator.SetDocumentModified();
	SFX_APP()->Broadcast( SfxSimpleHint( SC_HINT_TABLES_CHANGED ) );

	return sal_True;
}


IMPL_LINK( ScDocShell, RefreshDBDataHdl, ScRefreshTimer*, pRefreshTimer )
{
	ScDBDocFunc aFunc(*this);

	sal_Bool bContinue = sal_True;
    ScDBData* pDBData = static_cast<ScDBData*>(pRefreshTimer);
	ScImportParam aImportParam;
	pDBData->GetImportParam( aImportParam );
	if (aImportParam.bImport && !pDBData->HasImportSelection())
	{
		ScRange aRange;
		pDBData->GetArea( aRange );
        bContinue = aFunc.DoImport( aRange.aStart.Tab(), aImportParam, NULL, sal_True, sal_False ); //! Api-Flag as parameter
		// internal operations (sort, query, subtotal) only if no error
		if (bContinue)
		{
			aFunc.RepeatDB( pDBData->GetName(), sal_True, sal_True );
			RefreshPivotTables(aRange);
		}
	}

	return bContinue != 0;
}

