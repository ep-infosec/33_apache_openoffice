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
#include <svx/algitem.hxx>
#include <unotools/textsearch.hxx>
#include <sfx2/objsh.hxx>

#include "attrib.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "stlsheet.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "refupdat.hxx"
#include "markdata.hxx"
#include "progress.hxx"
#include "hints.hxx"		// fuer Paint-Broadcast
#include "prnsave.hxx"
#include "tabprotection.hxx"
#include "sheetevents.hxx"
#include "segmenttree.hxx"

// -----------------------------------------------------------------------

ScTable::ScTable( ScDocument* pDoc, SCTAB nNewTab, const String& rNewName,
					sal_Bool bColInfo, sal_Bool bRowInfo ) :
	aName( rNewName ),
	aCodeName( rNewName ),
	bScenario( sal_False ),
	bLayoutRTL( sal_False ),
    bLoadingRTL( sal_False ),
	nLinkMode( 0 ),
	aPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) ),
	bPageSizeValid( sal_False ),
	nRepeatStartX( SCCOL_REPEAT_NONE ),
	nRepeatStartY( SCROW_REPEAT_NONE ),
    pTabProtection( NULL ),
	pColWidth( NULL ),
    mpRowHeights( static_cast<ScFlatUInt16RowSegments*>(NULL) ),
	pColFlags( NULL ),
	pRowFlags( NULL ),
    mpHiddenCols(new ScFlatBoolColSegments),
    mpHiddenRows(new ScFlatBoolRowSegments),
    mpFilteredCols(new ScFlatBoolColSegments),
    mpFilteredRows(new ScFlatBoolRowSegments),
	pOutlineTable( NULL ),
    pSheetEvents( NULL ),
	bTableAreaValid( sal_False ),
	bVisible( sal_True ),
    bStreamValid( sal_False ),
    bPendingRowHeights( sal_False ),
    bCalcNotification( sal_False ),
	nTab( nNewTab ),
	nRecalcLvl( 0 ),
	pDocument( pDoc ),
	pSearchParam( NULL ),
	pSearchText ( NULL ),
	pSortCollator( NULL ),
    bPrintEntireSheet( sal_False ),
	pRepeatColRange( NULL ),
	pRepeatRowRange( NULL ),
	nLockCount( 0 ),
	pScenarioRanges( NULL ),
	aScenarioColor( COL_LIGHTGRAY ),
    aTabBgColor( COL_AUTO ),
	nScenarioFlags( 0 ),
	bActiveScenario( sal_False ),
    mbPageBreaksValid(false)
{

	if (bColInfo)
	{
		pColWidth  = new sal_uInt16[ MAXCOL+1 ];
		pColFlags  = new sal_uInt8[ MAXCOL+1 ];

		for (SCCOL i=0; i<=MAXCOL; i++)
		{
			pColWidth[i] = STD_COL_WIDTH;
			pColFlags[i] = 0;
		}
	}

	if (bRowInfo)
	{
        mpRowHeights.reset(new ScFlatUInt16RowSegments(ScGlobal::nStdRowHeight));
        pRowFlags  = new ScBitMaskCompressedArray< SCROW, sal_uInt8>( MAXROW, 0);
	}

	if ( pDocument->IsDocVisible() )
	{
		//	when a sheet is added to a visible document,
		//	initialize its RTL flag from the system locale
		bLayoutRTL = ScGlobal::IsSystemRTL();
	}

	ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
	if (pDrawLayer)
	{
        if ( pDrawLayer->ScAddPage( nTab ) )    // sal_False (not inserted) during Undo
        {
            pDrawLayer->ScRenamePage( nTab, aName );
            sal_uLong nx = (sal_uLong) ((double) (MAXCOL+1) * STD_COL_WIDTH			  * HMM_PER_TWIPS );
            sal_uLong ny = (sal_uLong) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
            pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( nx, ny ), false );
        }
	}

	for (SCCOL k=0; k<=MAXCOL; k++)
		aCol[k].Init( k, nTab, pDocument );
}

ScTable::~ScTable()
{
	if (!pDocument->IsInDtorClear())
	{
		//	nicht im dtor die Pages in der falschen Reihenfolge loeschen
		//	(nTab stimmt dann als Page-Number nicht!)
		//	In ScDocument::Clear wird hinterher per Clear am Draw Layer alles geloescht.

		ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
		if (pDrawLayer)
			pDrawLayer->ScRemovePage( nTab );
	}

	delete[] pColWidth;
	delete[] pColFlags;
	delete pRowFlags;
    delete pSheetEvents;
	delete pOutlineTable;
	delete pSearchParam;
	delete pSearchText;
	delete pRepeatColRange;
	delete pRepeatRowRange;
	delete pScenarioRanges;
	DestroySortCollator();
}

void ScTable::GetName( String& rName ) const
{
	rName = aName;
}

void ScTable::SetName( const String& rNewName )
{
	aName = rNewName;
    aUpperName.Erase();         // invalidated if the name is changed

    // SetStreamValid is handled in ScDocument::RenameTab
}

const String& ScTable::GetUpperName() const
{
    if ( !aUpperName.Len() && aName.Len() )
        aUpperName = ScGlobal::pCharClass->upper( aName );
    return aUpperName;
}

void ScTable::SetVisible( sal_Bool bVis )
{
    if (bVisible != bVis && IsStreamValid())
        SetStreamValid(sal_False);

	bVisible = bVis;
}

void ScTable::SetStreamValid( sal_Bool bSet, sal_Bool bIgnoreLock )
{
    if ( bIgnoreLock || !pDocument->IsStreamValidLocked() )
        bStreamValid = bSet;
}

void ScTable::SetPendingRowHeights( sal_Bool bSet )
{
    bPendingRowHeights = bSet;
}

void ScTable::SetLayoutRTL( sal_Bool bSet )
{
    bLayoutRTL = bSet;
}

void ScTable::SetLoadingRTL( sal_Bool bSet )
{
    bLoadingRTL = bSet;
}

const Color& ScTable::GetTabBgColor() const
{
    return aTabBgColor;
}

void ScTable::SetTabBgColor(const Color& rColor)
{
    if (aTabBgColor != rColor)
    {
        // The tab color has changed.  Set this table 'modified'.
        aTabBgColor = rColor;
        if (IsStreamValid())
            SetStreamValid(false);
    }
}

void ScTable::SetScenario( sal_Bool bFlag )
{
	bScenario = bFlag;
}

void ScTable::SetLink( sal_uInt8 nMode,
						const String& rDoc, const String& rFlt, const String& rOpt,
						const String& rTab, sal_uLong nRefreshDelay )
{
	nLinkMode = nMode;
	aLinkDoc = rDoc;		// Datei
	aLinkFlt = rFlt;		// Filter
	aLinkOpt = rOpt;		// Filter-Optionen
	aLinkTab = rTab;		// Tabellenname in Quelldatei
	nLinkRefreshDelay = nRefreshDelay;	// refresh delay in seconds, 0==off

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

sal_uInt16 ScTable::GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
									double nPPTX, double nPPTY,
									const Fraction& rZoomX, const Fraction& rZoomY,
									sal_Bool bFormula, const ScMarkData* pMarkData,
									sal_Bool bSimpleTextImport )
{
	return aCol[nCol].GetOptimalColWidth( pDev, nPPTX, nPPTY, rZoomX, rZoomY,
		bFormula, STD_COL_WIDTH - STD_EXTRA_WIDTH, pMarkData, bSimpleTextImport );
}

long ScTable::GetNeededSize( SCCOL nCol, SCROW nRow,
								OutputDevice* pDev,
								double nPPTX, double nPPTY,
								const Fraction& rZoomX, const Fraction& rZoomY,
								sal_Bool bWidth, sal_Bool bTotalSize )
{
	ScNeededSizeOptions aOptions;
	aOptions.bSkipMerged = sal_False;		// zusammengefasste mitzaehlen
	aOptions.bTotalSize  = bTotalSize;

	return aCol[nCol].GetNeededSize
		( nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, aOptions );
}

sal_Bool ScTable::SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
								OutputDevice* pDev,
								double nPPTX, double nPPTY,
								const Fraction& rZoomX, const Fraction& rZoomY,
                                sal_Bool bForce, ScProgress* pOuterProgress, sal_uLong nProgressStart )
{
	DBG_ASSERT( nExtra==0 || bForce, "autom. OptimalHeight mit Extra" );

    if ( !pDocument->IsAdjustHeightEnabled() )
    {
        return sal_False;
    }

	sal_Bool    bChanged = sal_False;
	SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

	ScProgress* pProgress = NULL;
    if ( pOuterProgress )
        pProgress = pOuterProgress;
    else if ( nCount > 1 )
		pProgress = new ScProgress( pDocument->GetDocumentShell(),
							ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), GetWeightedCount() );

	sal_uInt16* pHeight = new sal_uInt16[nCount];                   // Twips !
    memset( pHeight, 0, sizeof(sal_uInt16) * nCount );

	//	zuerst einmal ueber den ganzen Bereich
	//	(mit der letzten Spalte in der Hoffnung, dass die am ehesten noch auf
	//	 Standard formatiert ist)

	aCol[MAXCOL].GetOptimalHeight(
			nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce, 0, 0 );

	//	daraus Standardhoehe suchen, die im unteren Bereich gilt

	sal_uInt16 nMinHeight = pHeight[nCount-1];
	SCSIZE nPos = nCount-1;
	while ( nPos && pHeight[nPos-1] >= nMinHeight )
		--nPos;
	SCROW nMinStart = nStartRow + nPos;

    sal_uLong nWeightedCount = 0;
	for (SCCOL nCol=0; nCol<MAXCOL; nCol++)		// MAXCOL schon oben
	{
		aCol[nCol].GetOptimalHeight(
			nStartRow, nEndRow, pHeight, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
			nMinHeight, nMinStart );

		if (pProgress)
		{
            sal_uLong nWeight = aCol[nCol].GetWeightedCount();
			if (nWeight)		// nochmal denselben Status muss auch nicht sein
			{
				nWeightedCount += nWeight;
                pProgress->SetState( nWeightedCount + nProgressStart );
			}
		}
	}

    IncRecalcLevel();       // #i116460# avoid problems with Excel files

    SCROW nRngStart = 0;
    SCROW nRngEnd = 0;
	sal_uInt16 nLast = 0;
	for (SCSIZE i=0; i<nCount; i++)
	{
        size_t nIndex;
        SCROW nRegionEndRow;
        sal_uInt8 nRowFlag = pRowFlags->GetValue( nStartRow+i, nIndex, nRegionEndRow );
        if ( nRegionEndRow > nEndRow )
            nRegionEndRow = nEndRow;
        SCSIZE nMoreRows = nRegionEndRow - ( nStartRow+i );     // additional equal rows after first

        bool bAutoSize = ((nRowFlag & CR_MANUALSIZE) == 0);
		if ( bAutoSize || bForce )
		{
			if (nExtra)
            {
                if (bAutoSize)
                    pRowFlags->SetValue( nStartRow+i, nRegionEndRow, nRowFlag | CR_MANUALSIZE);
            }
			else if (!bAutoSize)
                pRowFlags->SetValue( nStartRow+i, nRegionEndRow, nRowFlag & ~CR_MANUALSIZE);

            for (SCSIZE nInner = i; nInner <= i + nMoreRows; ++nInner)
            {
                if (nLast)
                {
                    if (pHeight[nInner]+nExtra == nLast)
                        nRngEnd = nStartRow+nInner;
                    else
                    {
                        bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
                        nLast = 0;
                    }
                }
                if (!nLast)
                {
                    nLast = pHeight[nInner]+nExtra;
                    nRngStart = nStartRow+nInner;
                    nRngEnd = nStartRow+nInner;
                }
            }
		}
		else
		{
			if (nLast)
				bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );
			nLast = 0;
		}
        i += nMoreRows;     // already handled - skip
	}
	if (nLast)
		bChanged |= SetRowHeightRange( nRngStart, nRngEnd, nLast, nPPTX, nPPTY );

    DecRecalcLevel();       // #i116460# avoid problems with Excel files

	delete[] pHeight;
    if ( pProgress != pOuterProgress )
        delete pProgress;

	return bChanged;
}

sal_Bool ScTable::GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
	sal_Bool bFound = sal_False;
	SCCOL nMaxX = 0;
	SCROW nMaxY = 0;
	for (SCCOL i=0; i<=MAXCOL; i++)
		if (!aCol[i].IsEmptyVisData(sal_True))		// sal_True = Notizen zaehlen auch
		{
			bFound = sal_True;
			nMaxX = i;
			SCROW nColY = aCol[i].GetLastVisDataPos(sal_True);
			if (nColY > nMaxY)
				nMaxY = nColY;
		}

	rEndCol = nMaxX;
	rEndRow = nMaxY;
	return bFound;
}

sal_Bool ScTable::GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
	sal_Bool bRet = sal_True;				//! merken?
	if (!bTableAreaValid)
	{
		bRet = GetPrintArea( ((ScTable*)this)->nTableAreaX,
								((ScTable*)this)->nTableAreaY, sal_True );
		((ScTable*)this)->bTableAreaValid = sal_True;
	}
	rEndCol = nTableAreaX;
	rEndRow = nTableAreaY;
	return bRet;
}

void ScTable::GetLastAttrCell( SCCOL& rEndCol, SCROW& rEndRow ) const
{
	SCCOL nMaxX = 0;
	SCROW nMaxY = 0;
	SCCOL i;
	for ( i = 0; i <= MAXCOL; i++ )
	{
		SCROW nLastRow;
		aCol[i].GetLastAttr( nLastRow );
		if ( nLastRow > nMaxY && nLastRow > 0 && nLastRow <= MAXROW )
		{
			nMaxY = nLastRow;
			nMaxX = i;
		}
	}
	rEndCol = nMaxX;
	rEndRow = nMaxY;
}
/*		vorher:

	sal_Bool bFound = sal_False;
	SCCOL nMaxX = 0;
	SCROW nMaxY = 0;
	for (SCCOL i=0; i<=MAXCOL; i++)
		if (!aCol[i].IsEmpty())
		{
			bFound = sal_True;
			nMaxX = i;
			SCCOL nColY = aCol[i].GetLastEntryPos();
			if (nColY > nMaxY)
				nMaxY = nColY;
		}

	rEndCol = nMaxX;
	rEndRow = nMaxY;
	return bFound;
*/

const SCCOL SC_COLUMNS_STOP = 30;

sal_Bool ScTable::GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, sal_Bool bNotes ) const
{
	sal_Bool bFound = sal_False;
	SCCOL nMaxX = 0;
	SCROW nMaxY = 0;
	SCCOL i;

	for (i=0; i<=MAXCOL; i++)				// Daten testen
		if (!aCol[i].IsEmptyVisData(bNotes))
		{
			bFound = sal_True;
			if (i>nMaxX)
				nMaxX = i;
			SCROW nColY = aCol[i].GetLastVisDataPos(bNotes);
			if (nColY > nMaxY)
				nMaxY = nColY;
		}

    SCCOL nMaxDataX = nMaxX;

	for (i=0; i<=MAXCOL; i++)				// Attribute testen
	{
		SCROW nLastRow;
		if (aCol[i].GetLastVisibleAttr( nLastRow ))
		{
			bFound = sal_True;
			nMaxX = i;
			if (nLastRow > nMaxY)
				nMaxY = nLastRow;
		}
	}

	if (nMaxX == MAXCOL)					// Attribute rechts weglassen
	{
		--nMaxX;
		while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1]) )
			--nMaxX;
	}

    if ( nMaxX < nMaxDataX )
    {
        nMaxX = nMaxDataX;
    }
    else if ( nMaxX > nMaxDataX )
    {
        SCCOL nAttrStartX = nMaxDataX + 1;
        while ( nAttrStartX < MAXCOL )
        {
            SCCOL nAttrEndX = nAttrStartX;
            while ( nAttrEndX < MAXCOL && aCol[nAttrStartX].IsVisibleAttrEqual(aCol[nAttrEndX+1]) )
                ++nAttrEndX;
            if ( nAttrEndX + 1 - nAttrStartX >= SC_COLUMNS_STOP )
            {
                // found equally-formatted columns behind data -> stop before these columns
                nMaxX = nAttrStartX - 1;

                // also don't include default-formatted columns before that
                SCROW nDummyRow;
                while ( nMaxX > nMaxDataX && !aCol[nMaxX].GetLastVisibleAttr( nDummyRow ) )
                    --nMaxX;
                break;
            }
            nAttrStartX = nAttrEndX + 1;
        }
    }

	rEndCol = nMaxX;
	rEndRow = nMaxY;
	return bFound;
}

sal_Bool ScTable::GetPrintAreaHor( SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rEndCol, sal_Bool /* bNotes */ ) const
{
	sal_Bool bFound = sal_False;
	SCCOL nMaxX = 0;
	SCCOL i;

	for (i=0; i<=MAXCOL; i++)				// Attribute testen
	{
		if (aCol[i].HasVisibleAttrIn( nStartRow, nEndRow ))
		{
			bFound = sal_True;
			nMaxX = i;
		}
	}

	if (nMaxX == MAXCOL)					// Attribute rechts weglassen
	{
		--nMaxX;
		while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1], nStartRow, nEndRow) )
			--nMaxX;
	}

	for (i=0; i<=MAXCOL; i++)				// Daten testen
	{
		if (!aCol[i].IsEmptyBlock( nStartRow, nEndRow ))		//! bNotes ??????
		{
			bFound = sal_True;
			if (i>nMaxX)
				nMaxX = i;
		}
	}

	rEndCol = nMaxX;
	return bFound;
}

sal_Bool ScTable::GetPrintAreaVer( SCCOL nStartCol, SCCOL nEndCol,
								SCROW& rEndRow, sal_Bool bNotes ) const
{
	sal_Bool bFound = sal_False;
	SCROW nMaxY = 0;
	SCCOL i;

	for (i=nStartCol; i<=nEndCol; i++)				// Attribute testen
	{
		SCROW nLastRow;
		if (aCol[i].GetLastVisibleAttr( nLastRow ))
		{
			bFound = sal_True;
			if (nLastRow > nMaxY)
				nMaxY = nLastRow;
		}
	}

	for (i=nStartCol; i<=nEndCol; i++)				// Daten testen
		if (!aCol[i].IsEmptyVisData(bNotes))
		{
			bFound = sal_True;
			SCROW nColY = aCol[i].GetLastVisDataPos(bNotes);
			if (nColY > nMaxY)
				nMaxY = nColY;
		}

	rEndRow = nMaxY;
	return bFound;
}

sal_Bool ScTable::GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const
{
	sal_Bool bFound = sal_False;
	SCCOL nMinX = MAXCOL;
	SCROW nMinY = MAXROW;
	SCCOL i;

	for (i=0; i<=MAXCOL; i++)					// Attribute testen
	{
		SCROW nFirstRow;
		if (aCol[i].GetFirstVisibleAttr( nFirstRow ))
		{
			if (!bFound)
				nMinX = i;
			bFound = sal_True;
			if (nFirstRow < nMinY)
				nMinY = nFirstRow;
		}
	}

	if (nMinX == 0)										// Attribute links weglassen
	{
		if ( aCol[0].IsVisibleAttrEqual(aCol[1]) )		// keine einzelnen
		{
			++nMinX;
			while ( nMinX<MAXCOL && aCol[nMinX].IsVisibleAttrEqual(aCol[nMinX-1]) )
				++nMinX;
		}
	}

	sal_Bool bDatFound = sal_False;
	for (i=0; i<=MAXCOL; i++)					// Daten testen
		if (!aCol[i].IsEmptyVisData(sal_True))
		{
			if (!bDatFound && i<nMinX)
				nMinX = i;
			bFound = bDatFound = sal_True;
			SCROW nColY = aCol[i].GetFirstVisDataPos(sal_True);
			if (nColY < nMinY)
				nMinY = nColY;
		}

	rStartCol = nMinX;
	rStartRow = nMinY;
	return bFound;
}

void ScTable::GetDataArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
                           sal_Bool bIncludeOld, bool bOnlyDown ) const
{
	sal_Bool bLeft       = sal_False;
	sal_Bool bRight  = sal_False;
	sal_Bool bTop        = sal_False;
	sal_Bool bBottom = sal_False;
	sal_Bool bChanged;
	sal_Bool bFound;
	SCCOL i;
	SCROW nTest;

	do
	{
		bChanged = sal_False;

        if (!bOnlyDown)
        {
            SCROW nStart = rStartRow;
            SCROW nEnd = rEndRow;
            if (nStart>0) --nStart;
            if (nEnd<MAXROW) ++nEnd;

            if (rEndCol < MAXCOL)
                if (!aCol[rEndCol+1].IsEmptyBlock(nStart,nEnd))
                {
                    ++rEndCol;
                    bChanged = sal_True;
                    bRight = sal_True;
                }

            if (rStartCol > 0)
                if (!aCol[rStartCol-1].IsEmptyBlock(nStart,nEnd))
                {
                    --rStartCol;
                    bChanged = sal_True;
                    bLeft = sal_True;
                }

            if (rStartRow > 0)
            {
                nTest = rStartRow-1;
                bFound = sal_False;
                for (i=rStartCol; i<=rEndCol && !bFound; i++)
                    if (aCol[i].HasDataAt(nTest))
                        bFound = sal_True;
                if (bFound)
                {
                    --rStartRow;
                    bChanged = sal_True;
                    bTop = sal_True;
                }
            }
        }

		if (rEndRow < MAXROW)
		{
			nTest = rEndRow+1;
			bFound = sal_False;
			for (i=rStartCol; i<=rEndCol && !bFound; i++)
				if (aCol[i].HasDataAt(nTest))
					bFound = sal_True;
			if (bFound)
			{
				++rEndRow;
				bChanged = sal_True;
				bBottom = sal_True;
			}
		}
	}
	while( bChanged );

	if ( !bIncludeOld )
	{
		if ( !bLeft && rStartCol < MAXCOL && rStartCol < rEndCol )
			if ( aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
				++rStartCol;
		if ( !bRight && rEndCol > 0 && rStartCol < rEndCol )
			if ( aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
				--rEndCol;
		if ( !bTop && rStartRow < MAXROW && rStartRow < rEndRow )
		{
			bFound = sal_False;
			for (i=rStartCol; i<=rEndCol && !bFound; i++)
				if (aCol[i].HasDataAt(rStartRow))
					bFound = sal_True;
			if (!bFound)
				++rStartRow;
		}
		if ( !bBottom && rEndRow > 0 && rStartRow < rEndRow )
		{
			bFound = sal_False;
			for (i=rStartCol; i<=rEndCol && !bFound; i++)
				if (aCol[i].HasDataAt(rEndRow))
					bFound = sal_True;
			if (!bFound)
				--rEndRow;
		}
	}
}


bool ScTable::ShrinkToUsedDataArea( bool& o_bShrunk, SCCOL& rStartCol, SCROW& rStartRow,
        SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const
{
    o_bShrunk = false;

    PutInOrder( rStartCol, rEndCol);
    PutInOrder( rStartRow, rEndRow);
    if (rStartCol < 0)
        rStartCol = 0, o_bShrunk = true;
    if (rStartRow < 0)
        rStartRow = 0, o_bShrunk = true;
    if (rEndCol > MAXCOL)
        rEndCol = MAXCOL, o_bShrunk = true;
    if (rEndRow > MAXROW)
        rEndRow = MAXROW, o_bShrunk = true;

    bool bChanged;
    do
    {
        bChanged = false;

        while (rStartCol < rEndCol)
        {
            if (aCol[rEndCol].IsEmptyBlock( rStartRow, rEndRow))
            {
                --rEndCol;
                bChanged = true;
            }
            else
                break;  // while
        }

        while (rStartCol < rEndCol)
        {
            if (aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow))
            {
                ++rStartCol;
                bChanged = true;
            }
            else
                break;  // while
        }

        if (!bColumnsOnly)
        {
            if (rStartRow < rEndRow)
            {
                bool bFound = false;
                for (SCCOL i=rStartCol; i<=rEndCol && !bFound; i++)
                    if (aCol[i].HasDataAt( rStartRow))
                        bFound = true;
                if (!bFound)
                {
                    ++rStartRow;
                    bChanged = true;
                }
            }

            if (rStartRow < rEndRow)
            {
                bool bFound = false;
                for (SCCOL i=rStartCol; i<=rEndCol && !bFound; i++)
                    if (aCol[i].HasDataAt( rEndRow))
                        bFound = true;
                if (!bFound)
                {
                    --rEndRow;
                    bChanged = true;
                }
            }
        }

        if (bChanged)
            o_bShrunk = true;
    } while( bChanged );

    return rStartCol != rEndCol || (bColumnsOnly ? 
            !aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow) :
            (rStartRow != rEndRow || aCol[rStartCol].HasDataAt( rStartRow)));
}


SCSIZE ScTable::GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
										SCCOL nEndCol, SCROW nEndRow, ScDirection eDir )
{
	SCSIZE nCount = 0;
	SCCOL nCol;
	if ((eDir == DIR_BOTTOM) || (eDir == DIR_TOP))
	{
		nCount = static_cast<SCSIZE>(nEndRow - nStartRow);
		for (nCol = nStartCol; nCol <= nEndCol; nCol++)
			nCount = Min(nCount, aCol[nCol].GetEmptyLinesInBlock(nStartRow, nEndRow, eDir));
	}
	else if (eDir == DIR_RIGHT)
	{
		nCol = nEndCol;
		while (((SCsCOL)nCol >= (SCsCOL)nStartCol) &&
				 aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
		{
			nCount++;
			nCol--;
		}
	}
	else
	{
		nCol = nStartCol;
		while ((nCol <= nEndCol) && aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
		{
			nCount++;
			nCol++;
		}
	}
	return nCount;
}

sal_Bool ScTable::IsEmptyLine( SCROW nRow, SCCOL nStartCol, SCCOL nEndCol )
{
	sal_Bool bFound = sal_False;
	for (SCCOL i=nStartCol; i<=nEndCol && !bFound; i++)
		if (aCol[i].HasDataAt(nRow))
			bFound = sal_True;
	return !bFound;
}

void ScTable::LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow )
{
	while ( rStartCol<rEndCol && aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
		++rStartCol;

	while ( rStartCol<rEndCol && aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
		--rEndCol;

	while ( rStartRow<rEndRow && IsEmptyLine(rStartRow, rStartCol, rEndCol) )
		++rStartRow;

	while ( rStartRow<rEndRow && IsEmptyLine(rEndRow, rStartCol, rEndCol) )
		--rEndRow;
}

void ScTable::FindAreaPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY )
{
	if (nMovX)
	{
		SCsCOL nNewCol = (SCsCOL) rCol;
		sal_Bool bThere = aCol[nNewCol].HasVisibleDataAt(rRow);
		sal_Bool bFnd;
		if (bThere)
		{
			do
			{
                nNewCol = sal::static_int_cast<SCsCOL>( nNewCol + nMovX );
				bFnd = (nNewCol>=0 && nNewCol<=MAXCOL) ? aCol[nNewCol].HasVisibleDataAt(rRow) : sal_False;
			}
			while (bFnd);
            nNewCol = sal::static_int_cast<SCsCOL>( nNewCol - nMovX );

			if (nNewCol == (SCsCOL)rCol)
				bThere = sal_False;
		}

		if (!bThere)
		{
			do
			{
                nNewCol = sal::static_int_cast<SCsCOL>( nNewCol + nMovX );
				bFnd = (nNewCol>=0 && nNewCol<=MAXCOL) ? aCol[nNewCol].HasVisibleDataAt(rRow) : sal_True;
			}
			while (!bFnd);
		}

		if (nNewCol<0) nNewCol=0;
		if (nNewCol>MAXCOL) nNewCol=MAXCOL;
		rCol = (SCCOL) nNewCol;
	}

	if (nMovY)
		aCol[rCol].FindDataAreaPos(rRow,nMovY);
}

sal_Bool ScTable::ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
								sal_Bool bMarked, sal_Bool bUnprotected )
{
	if (!ValidCol(nCol) || !ValidRow(nRow))
		return sal_False;

    if (pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED))
        // Skip an overlapped cell.
        return false;

	if (bMarked && !rMark.IsCellMarked(nCol,nRow))
		return sal_False;

	if (bUnprotected && ((const ScProtectionAttr*)
						GetAttr(nCol,nRow,ATTR_PROTECTION))->GetProtection())
		return sal_False;

	if (bMarked || bUnprotected)		//! auch sonst ???
	{
		//	#53697# ausgeblendete muessen uebersprungen werden, weil der Cursor sonst
		//	auf der naechsten Zelle landet, auch wenn die geschuetzt/nicht markiert ist.
		//!	per Extra-Parameter steuern, nur fuer Cursor-Bewegung ???

        if (RowHidden(nRow))
			return sal_False;

        if (ColHidden(nCol))
			return sal_False;
	}

	return sal_True;
}

void ScTable::GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
								sal_Bool bMarked, sal_Bool bUnprotected, const ScMarkData& rMark )
{
	if (bUnprotected && !IsProtected())		// Tabelle ueberhaupt geschuetzt?
		bUnprotected = sal_False;

	sal_uInt16 nWrap = 0;
	SCsCOL nCol = rCol;
	SCsROW nRow = rRow;

    nCol = sal::static_int_cast<SCsCOL>( nCol + nMovX );
    nRow = sal::static_int_cast<SCsROW>( nRow + nMovY );

	DBG_ASSERT( !nMovY || !bUnprotected,
				"GetNextPos mit bUnprotected horizontal nicht implementiert" );

	if ( nMovY && bMarked )
	{
		sal_Bool bUp = ( nMovY < 0 );
		nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        while ( VALIDROW(nRow) && 
                (RowHidden(nRow) || pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED)) )
		{
			//	#53697# ausgeblendete ueberspringen (s.o.)
			nRow += nMovY;
			nRow = rMark.GetNextMarked( nCol, nRow, bUp );
		}

		while ( nRow < 0 || nRow > MAXROW )
		{
            nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );
			while ( VALIDCOL(nCol) && ColHidden(nCol) )
                nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );   //	#53697# skip hidden rows (see above)
			if (nCol < 0)
			{
				nCol = MAXCOL;
				if (++nWrap >= 2)
					return;
			}
			else if (nCol > MAXCOL)
			{
				nCol = 0;
				if (++nWrap >= 2)
					return;
			}
			if (nRow < 0)
				nRow = MAXROW;
			else if (nRow > MAXROW)
				nRow = 0;
			nRow = rMark.GetNextMarked( nCol, nRow, bUp );
            while ( VALIDROW(nRow) && 
                    (RowHidden(nRow) || pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED)) )
			{
				//	#53697# ausgeblendete ueberspringen (s.o.)
				nRow += nMovY;
				nRow = rMark.GetNextMarked( nCol, nRow, bUp );
			}
		}
	}

	if ( nMovX && ( bMarked || bUnprotected ) )
	{
		// initiales Weiterzaehlen wrappen:
		if (nCol<0)
		{
			nCol = MAXCOL;
			--nRow;
			if (nRow<0)
				nRow = MAXROW;
		}
		if (nCol>MAXCOL)
		{
			nCol = 0;
			++nRow;
			if (nRow>MAXROW)
				nRow = 0;
		}

		if ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) )
		{
			SCsROW* pNextRows = new SCsROW[MAXCOL+1];
			SCCOL i;

			if ( nMovX > 0 )							//	vorwaerts
			{
				for (i=0; i<=MAXCOL; i++)
					pNextRows[i] = (i<nCol) ? (nRow+1) : nRow;
				do
				{
					SCsROW nNextRow = pNextRows[nCol] + 1;
					if ( bMarked )
						nNextRow = rMark.GetNextMarked( nCol, nNextRow, sal_False );
					if ( bUnprotected )
						nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, sal_False );
					pNextRows[nCol] = nNextRow;

					SCsROW nMinRow = MAXROW+1;
					for (i=0; i<=MAXCOL; i++)
						if (pNextRows[i] < nMinRow)		// bei gleichen den linken
						{
							nMinRow = pNextRows[i];
							nCol = i;
						}
					nRow = nMinRow;

					if ( nRow > MAXROW )
					{
						if (++nWrap >= 2) break;		// ungueltigen Wert behalten
						nCol = 0;
                        nRow = 0;
						for (i=0; i<=MAXCOL; i++)
							pNextRows[i] = 0;			// alles ganz von vorne
					}
				}
				while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
			}
			else										//	rueckwaerts
			{
				for (i=0; i<=MAXCOL; i++)
					pNextRows[i] = (i>nCol) ? (nRow-1) : nRow;
				do
				{
					SCsROW nNextRow = pNextRows[nCol] - 1;
					if ( bMarked )
						nNextRow = rMark.GetNextMarked( nCol, nNextRow, sal_True );
					if ( bUnprotected )
						nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, sal_True );
					pNextRows[nCol] = nNextRow;

					SCsROW nMaxRow = -1;
					for (i=0; i<=MAXCOL; i++)
						if (pNextRows[i] >= nMaxRow)	// bei gleichen den rechten
						{
							nMaxRow = pNextRows[i];
							nCol = i;
						}
					nRow = nMaxRow;

					if ( nRow < 0 )
					{
						if (++nWrap >= 2) break;		// ungueltigen Wert behalten
						nCol = MAXCOL;
						nRow = MAXROW;
						for (i=0; i<=MAXCOL; i++)
							pNextRows[i] = MAXROW;		// alles ganz von vorne
					}
				}
				while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
			}

			delete[] pNextRows;
		}
	}

	//	ungueltige Werte kommen z.b. bei Tab heraus,
	//	wenn nicht markiert und nicht geschuetzt ist (linker / rechter Rand),
	//	dann Werte unveraendert lassen

	if (VALIDCOLROW(nCol,nRow))
	{
		rCol = nCol;
		rRow = nRow;
	}
}

sal_Bool ScTable::GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark )
{
	const ScMarkArray* pMarkArray = rMark.GetArray();
	DBG_ASSERT(pMarkArray,"GetNextMarkedCell ohne MarkArray");
	if ( !pMarkArray )
		return sal_False;

	++rRow;					// naechste Zelle ist gesucht

	while ( rCol <= MAXCOL )
	{
		const ScMarkArray& rArray = pMarkArray[rCol];
		while ( rRow <= MAXROW )
		{
			SCROW nStart = (SCROW) rArray.GetNextMarked( (SCsROW) rRow, sal_False );
			if ( nStart <= MAXROW )
			{
				SCROW nEnd = rArray.GetMarkEnd( nStart, sal_False );
				ScColumnIterator aColIter( &aCol[rCol], nStart, nEnd );
				SCROW nCellRow;
				ScBaseCell* pCell = NULL;
				while ( aColIter.Next( nCellRow, pCell ) )
				{
					if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
					{
						rRow = nCellRow;
						return sal_True;			// Zelle gefunden
					}
				}
				rRow = nEnd + 1;				// naechsten markierten Bereich suchen
			}
			else
				rRow = MAXROW + 1;				// Ende der Spalte
		}
		rRow = 0;
		++rCol;									// naechste Spalte testen
	}

	return sal_False;								// alle Spalten durch
}

void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
									SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
									SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bUpdateNoteCaptionPos )
{
	if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )		// only within the table
	{
        InitializeNoteCaptions();
		ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
		if ( eUpdateRefMode != URM_COPY && pDrawLayer )
		{
			if ( eUpdateRefMode == URM_MOVE )
			{												// source range
                nCol1 = sal::static_int_cast<SCCOL>( nCol1 - nDx );
                nRow1 = sal::static_int_cast<SCROW>( nRow1 - nDy );
                nCol2 = sal::static_int_cast<SCCOL>( nCol2 - nDx );
                nRow2 = sal::static_int_cast<SCROW>( nRow2 - nDy );
			}
			pDrawLayer->MoveArea( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy,
									(eUpdateRefMode == URM_INSDEL), bUpdateNoteCaptionPos );
		}
	}
}

void ScTable::UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
					 SCCOL nCol2, SCROW nRow2, SCTAB nTab2, SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
					 ScDocument* pUndoDoc, sal_Bool bIncludeDraw, bool bUpdateNoteCaptionPos )
{
	SCCOL i;
	SCCOL iMax;
	if ( eUpdateRefMode == URM_COPY )
	{
		i = nCol1;
		iMax = nCol2;
	}
	else
	{
		i = 0;
		iMax = MAXCOL;
	}
	for ( ; i<=iMax; i++)
		aCol[i].UpdateReference( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2,
									nDx, nDy, nDz, pUndoDoc );

	if ( bIncludeDraw )
		UpdateDrawRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz, bUpdateNoteCaptionPos );

	if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )		// print ranges: only within the table
	{
        SCTAB nSTab = nTab;
        SCTAB nETab = nTab;
        SCCOL nSCol = 0;
        SCROW nSRow = 0;
        SCCOL nECol = 0;
        SCROW nERow = 0;
		sal_Bool bRecalcPages = sal_False;

        for ( ScRangeVec::iterator aIt = aPrintRanges.begin(), aEnd = aPrintRanges.end(); aIt != aEnd; ++aIt )
        {
            nSCol = aIt->aStart.Col();
            nSRow = aIt->aStart.Row();
            nECol = aIt->aEnd.Col();
            nERow = aIt->aEnd.Row();

            // do not try to modify sheet index of print range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *aIt = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = sal_True;
            }
        }

		if ( pRepeatColRange )
		{
			nSCol = pRepeatColRange->aStart.Col();
			nSRow = pRepeatColRange->aStart.Row();
			nECol = pRepeatColRange->aEnd.Col();
			nERow = pRepeatColRange->aEnd.Row();

            // do not try to modify sheet index of repeat range
			if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
			{
                *pRepeatColRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
				bRecalcPages = sal_True;
				nRepeatStartX = nSCol;	// fuer UpdatePageBreaks
				nRepeatEndX = nECol;
			}
		}

		if ( pRepeatRowRange )
		{
			nSCol = pRepeatRowRange->aStart.Col();
			nSRow = pRepeatRowRange->aStart.Row();
			nECol = pRepeatRowRange->aEnd.Col();
			nERow = pRepeatRowRange->aEnd.Row();

            // do not try to modify sheet index of repeat range
			if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
			{
                *pRepeatRowRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
				bRecalcPages = sal_True;
				nRepeatStartY = nSRow;	// fuer UpdatePageBreaks
				nRepeatEndY = nERow;
			}
		}

		//	updating print ranges is not necessary with multiple print ranges
		if ( bRecalcPages && GetPrintRangeCount() <= 1 )
		{
			UpdatePageBreaks(NULL);

            pDocument->RepaintRange( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab) );
		}
	}
}

void ScTable::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
									ScDocument* pUndoDoc )
{
	for ( SCCOL i=0; i<=MAXCOL; i++ )
		aCol[i].UpdateTranspose( rSource, rDest, pUndoDoc );
}

void ScTable::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
	for ( SCCOL i=0; i<=MAXCOL; i++ )
		aCol[i].UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScTable::UpdateInsertTab(SCTAB nTable)
{
	if (nTab >= nTable) nTab++;
	for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTab(nTable);

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

//UNUSED2008-05  void ScTable::UpdateInsertTabOnlyCells(SCTAB nTable)
//UNUSED2008-05  {
//UNUSED2008-05      for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTabOnlyCells(nTable);
//UNUSED2008-05  }

void ScTable::UpdateDeleteTab( SCTAB nTable, sal_Bool bIsMove, ScTable* pRefUndo )
{
	if (nTab > nTable) nTab--;

	SCCOL i;
	if (pRefUndo)
		for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, &pRefUndo->aCol[i]);
	else
		for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, NULL);

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

void ScTable::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo,
		ScProgress& rProgress )
{
	nTab = nTabNo;
	for ( SCCOL i=0; i <= MAXCOL; i++ )
	{
		aCol[i].UpdateMoveTab( nOldPos, nNewPos, nTabNo );
		rProgress.SetState( rProgress.GetState() + aCol[i].GetCodeCount() );
	}

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

void ScTable::UpdateCompile( sal_Bool bForceIfNameInUse )
{
	for (SCCOL i=0; i <= MAXCOL; i++)
	{
		aCol[i].UpdateCompile( bForceIfNameInUse );
	}
}

void ScTable::SetTabNo(SCTAB nNewTab)
{
	nTab = nNewTab;
	for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].SetTabNo(nNewTab);
}

sal_Bool ScTable::IsRangeNameInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
							   sal_uInt16 nIndex) const
{
	sal_Bool bInUse = sal_False;
	for (SCCOL i = nCol1; !bInUse && (i <= nCol2) && (ValidCol(i)); i++)
		bInUse = aCol[i].IsRangeNameInUse(nRow1, nRow2, nIndex);
	return bInUse;
}

void ScTable::FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
							   std::set<sal_uInt16>& rIndexes) const
{
    for (SCCOL i = nCol1; i <= nCol2 && ValidCol(i); i++)
        aCol[i].FindRangeNamesInUse(nRow1, nRow2, rIndexes);
}

void ScTable::ReplaceRangeNamesInUse(SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScRangeData::IndexMap& rMap )
{
    for (SCCOL i = nCol1; i <= nCol2 && (ValidCol(i)); i++)
    {
        aCol[i].ReplaceRangeNamesInUse( nRow1, nRow2, rMap );
    }
}

void ScTable::ExtendPrintArea( OutputDevice* pDev,
                    SCCOL /* nStartCol */, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow )
{
    if ( !pColFlags || !pRowFlags )
    {
        DBG_ERROR("keine ColInfo oder RowInfo in ExtendPrintArea");
        return;
    }

    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    // First, mark those columns that we need to skip i.e. hidden and empty columns.

    ScFlatBoolColSegments aSkipCols;
    aSkipCols.setInsertFromBack(true); // speed optimazation.
    aSkipCols.setFalse(0, MAXCOL);
    for (SCCOL i = 0; i <= MAXCOL; ++i)
    {
        SCCOL nLastCol = i;
        if (ColHidden(i, NULL, &nLastCol))
        {
            // Columns are hidden in this range.
            aSkipCols.setTrue(i, nLastCol);
        }
        else
        {
            // These columns are visible.  Check for empty columns.
            for (SCCOL j = i; j <= nLastCol; ++j)
            {
                if (aCol[j].GetCellCount() == 0)
                    // empty
                    aSkipCols.setTrue(j,j);
            }
        }
        i = nLastCol;
    }

    ScFlatBoolColSegments::RangeData aColData;
    for (SCCOL nCol = rEndCol; nCol >= 0; --nCol)
    {
        if (!aSkipCols.getRangeData(nCol, aColData))
            // Failed to get the data.  This should never happen!
            return;

        if (aColData.mbValue)
        {
            // Skip these columns.
            nCol = aColData.mnCol1; // move toward 0.
            continue;
        }

        // These are visible and non-empty columns.
        for (SCCOL nDataCol = nCol; 0 <= nDataCol && nDataCol >= aColData.mnCol1; --nDataCol)
        {
            SCCOL nPrintCol = nDataCol;
            VisibleDataCellIterator aIter(*mpHiddenRows, aCol[nDataCol]);
            ScBaseCell* pCell = aIter.reset(nStartRow);
            if (!pCell)
                // No visible cells found in this column.  Skip it.
                continue;

            while (pCell)
            {
                SCCOL nNewCol = nDataCol;
                SCROW nRow = aIter.getRow();
                if (nRow > nEndRow)
                    // Went past the last row position.  Bail out.
                    break;

                MaybeAddExtraColumn(nNewCol, nRow, pDev, nPPTX, nPPTY);
                if (nNewCol > nPrintCol)
                    nPrintCol = nNewCol;
                pCell = aIter.next();
            }

            if (nPrintCol > rEndCol)
                // Make sure we don't shrink the print area.
                rEndCol = nPrintCol;
        }
        nCol = aColData.mnCol1; // move toward 0.
    }
}

void ScTable::MaybeAddExtraColumn(SCCOL& rCol, SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY)
{
    ScBaseCell* pCell = aCol[rCol].GetCell(nRow);
    if (!pCell || !pCell->HasStringData())
        return;

    bool bFormula = false;  //! ueberge
    long nPixel = pCell->GetTextWidth();

    // Breite bereits im Idle-Handler berechnet?
    if ( TEXTWIDTH_DIRTY == nPixel )
    {
        ScNeededSizeOptions aOptions;
        aOptions.bTotalSize  = sal_True;
        aOptions.bFormula    = bFormula;
        aOptions.bSkipMerged = sal_False;

        Fraction aZoom(1,1);
        nPixel = aCol[rCol].GetNeededSize(
            nRow, pDev, nPPTX, nPPTY, aZoom, aZoom, true, aOptions );
        pCell->SetTextWidth( (sal_uInt16)nPixel );
    }

    long nTwips = (long) (nPixel / nPPTX);
    long nDocW = GetColWidth( rCol );

    long nMissing = nTwips - nDocW;
    if ( nMissing > 0 )
    {
        //  look at alignment

        const ScPatternAttr* pPattern = GetPattern( rCol, nRow );
        const SfxItemSet* pCondSet = NULL;
        if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
            pCondSet = pDocument->GetCondResult( rCol, nRow, nTab );

        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                        pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet )).GetValue();
        if ( eHorJust == SVX_HOR_JUSTIFY_CENTER )
            nMissing /= 2;                          // distributed into both directions
        else
        {
            // STANDARD is LEFT (only text is handled here)
            bool bRight = ( eHorJust == SVX_HOR_JUSTIFY_RIGHT );
            if ( IsLayoutRTL() )
                bRight = !bRight;
            if ( bRight )
                nMissing = 0;       // extended only to the left (logical)
        }
    }

    SCCOL nNewCol = rCol;
    while (nMissing > 0 && nNewCol < MAXCOL)
    {
        ScBaseCell* pNextCell = aCol[nNewCol+1].GetCell(nRow);
        if (pNextCell && pNextCell->GetCellType() != CELLTYPE_NOTE)
            // Cell content in a next column ends display of this string.
            nMissing = 0;
        else
            nMissing -= GetColWidth(++nNewCol);
    }
    rCol = nNewCol;
}

void ScTable::DoColResize( SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd )
{
	for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
		aCol[nCol].Resize(aCol[nCol].GetCellCount() + nAdd);
}

#define SET_PRINTRANGE( p1, p2 ) \
	if ( (p2) )								\
	{										\
		if ( (p1) )							\
			*(p1) = *(p2);					\
		else								\
			(p1) = new ScRange( *(p2) );	\
	}										\
	else									\
		DELETEZ( (p1) )

void ScTable::SetRepeatColRange( const ScRange* pNew )
{
	SET_PRINTRANGE( pRepeatColRange, pNew );

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

void ScTable::SetRepeatRowRange( const ScRange* pNew )
{
	SET_PRINTRANGE( pRepeatRowRange, pNew );

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

void ScTable::ClearPrintRanges()
{
    aPrintRanges.clear();
    bPrintEntireSheet = sal_False;
    InvalidatePageBreaks();     // #i117952# forget page breaks for an old print range

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

void ScTable::AddPrintRange( const ScRange& rNew )
{
    bPrintEntireSheet = sal_False;
    if( aPrintRanges.size() < 0xFFFF )
        aPrintRanges.push_back( rNew );

    if (IsStreamValid())
        SetStreamValid(sal_False);
}

//UNUSED2009-05 void ScTable::SetPrintRange( const ScRange& rNew )
//UNUSED2009-05 {
//UNUSED2009-05     ClearPrintRanges();
//UNUSED2009-05     AddPrintRange( rNew );
//UNUSED2009-05 }

void ScTable::SetPrintEntireSheet()
{
    if( !IsPrintEntireSheet() )
    {
        ClearPrintRanges();
        bPrintEntireSheet = sal_True;
    }
}

const ScRange* ScTable::GetPrintRange(sal_uInt16 nPos) const
{
    return (nPos < GetPrintRangeCount()) ? &aPrintRanges[ nPos ] : NULL;
}

void ScTable::FillPrintSaver( ScPrintSaverTab& rSaveTab ) const
{
    rSaveTab.SetAreas( aPrintRanges, bPrintEntireSheet );
	rSaveTab.SetRepeat( pRepeatColRange, pRepeatRowRange );
}

void ScTable::RestorePrintRanges( const ScPrintSaverTab& rSaveTab )
{
    aPrintRanges = rSaveTab.GetPrintRanges();
    bPrintEntireSheet = rSaveTab.IsEntireSheet();
	SetRepeatColRange( rSaveTab.GetRepeatCol() );
	SetRepeatRowRange( rSaveTab.GetRepeatRow() );

    InvalidatePageBreaks();     // #i117952# forget page breaks for an old print range
    UpdatePageBreaks(NULL);
}

SCROW ScTable::VisibleDataCellIterator::ROW_NOT_FOUND = -1;

ScTable::VisibleDataCellIterator::VisibleDataCellIterator(ScFlatBoolRowSegments& rRowSegs, ScColumn& rColumn) :
    mrRowSegs(rRowSegs),
    mrColumn(rColumn),
    mpCell(NULL),
    mnCurRow(ROW_NOT_FOUND), 
    mnUBound(ROW_NOT_FOUND)
{
}

ScTable::VisibleDataCellIterator::~VisibleDataCellIterator()
{
}

ScBaseCell* ScTable::VisibleDataCellIterator::reset(SCROW nRow)
{
    if (nRow > MAXROW)
    {    
        mnCurRow = ROW_NOT_FOUND;
        return NULL;
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mrRowSegs.getRangeData(nRow, aData))
    {    
        mnCurRow = ROW_NOT_FOUND;
        return NULL;
    }

    if (!aData.mbValue)
    {    
        // specified row is visible.  Take it.
        mnCurRow = nRow;
        mnUBound = aData.mnRow2;
    }
    else
    {
        // specified row is not-visible.  The first visible row is the start of
        // the next segment.
        mnCurRow = aData.mnRow2 + 1;
        mnUBound = mnCurRow; // get range data on the next iteration.
        if (mnCurRow > MAXROW)
        {    
            // Make sure the row doesn't exceed our current limit.
            mnCurRow = ROW_NOT_FOUND;
            return NULL;
        }
    }

    mpCell = mrColumn.GetCell(mnCurRow);
    if (mpCell)
        // First visible cell found.
        return mpCell;

    // Find a first visible cell below this row (if any).
    return next();
}

ScBaseCell* ScTable::VisibleDataCellIterator::next()
{
    if (mnCurRow == ROW_NOT_FOUND)
        return NULL;

    while (mrColumn.GetNextDataPos(mnCurRow))
    {
        if (mnCurRow > mnUBound)
        {
            // We don't know the visibility of this row range.  Query it.
            ScFlatBoolRowSegments::RangeData aData;
            if (!mrRowSegs.getRangeData(mnCurRow, aData))
            {
                mnCurRow = ROW_NOT_FOUND;
                return NULL;
            }

            if (aData.mbValue)
            {
                // This row is invisible.  Skip to the last invisible row and
                // try again.
                mnCurRow = mnUBound = aData.mnRow2;
                continue;
            }

            // This row is visible.
            mnUBound = aData.mnRow2;
        }

        mpCell = mrColumn.GetCell(mnCurRow);
        if (mpCell)
            return mpCell;
    }
    mnCurRow = ROW_NOT_FOUND;
    return NULL;    
}

SCROW ScTable::VisibleDataCellIterator::getRow() const
{
    return mnCurRow;
}

