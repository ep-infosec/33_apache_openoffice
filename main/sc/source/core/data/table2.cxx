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
#include <editeng/boxitem.hxx>
#include <tools/urlobj.hxx>
#include <svl/poolcach.hxx>
#include <unotools/charclass.hxx>
#include <math.h>
#include <svl/PasswordHelper.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "patattr.hxx"
#include "docpool.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "attarray.hxx"		// Iterator
#include "markdata.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "conditio.hxx"
#include "chartlis.hxx"
#include "fillinfo.hxx"
#include "bcaslot.hxx"
#include "postit.hxx"
#include "sheetevents.hxx"
#include "globstr.hrc"
#include "segmenttree.hxx"
#include "dbcolect.hxx"	

#include <math.h>

// STATIC DATA -----------------------------------------------------------


sal_Bool ScTable::SetOutlineTable( const ScOutlineTable* pNewOutline )
{
	sal_uInt16 nOldSizeX = 0;
	sal_uInt16 nOldSizeY = 0;
	sal_uInt16 nNewSizeX = 0;
	sal_uInt16 nNewSizeY = 0;

	if (pOutlineTable)
	{
		nOldSizeX = pOutlineTable->GetColArray()->GetDepth();
		nOldSizeY = pOutlineTable->GetRowArray()->GetDepth();
		delete pOutlineTable;
	}

	if (pNewOutline)
	{
		pOutlineTable = new ScOutlineTable( *pNewOutline );
		nNewSizeX = pOutlineTable->GetColArray()->GetDepth();
		nNewSizeY = pOutlineTable->GetRowArray()->GetDepth();
	}
	else
		pOutlineTable = NULL;

	return ( nNewSizeX != nOldSizeX || nNewSizeY != nOldSizeY );		// Groesse geaendert ?
}


void ScTable::StartOutlineTable()
{
	if (!pOutlineTable)
		pOutlineTable = new ScOutlineTable;
}


void ScTable::SetSheetEvents( const ScSheetEvents* pNew )
{
    delete pSheetEvents;
    if (pNew)
        pSheetEvents = new ScSheetEvents(*pNew);
    else
        pSheetEvents = NULL;

    SetCalcNotification( sal_False );       // discard notifications before the events were set

    if (IsStreamValid())
        SetStreamValid(sal_False);
}


void ScTable::SetCalcNotification( sal_Bool bSet )
{
    bCalcNotification = bSet;
}


sal_Bool ScTable::TestInsertRow( SCCOL nStartCol, SCCOL nEndCol, SCSIZE nSize )
{
	sal_Bool bTest = sal_True;

	if ( nStartCol==0 && nEndCol==MAXCOL && pOutlineTable )
		bTest = pOutlineTable->TestInsertRow(nSize);

	for (SCCOL i=nStartCol; (i<=nEndCol) && bTest; i++)
		bTest = aCol[i].TestInsertRow( nSize );

	return bTest;
}


void ScTable::InsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize )
{
	IncRecalcLevel();
    InitializeNoteCaptions();
	if (nStartCol==0 && nEndCol==MAXCOL)
	{
        if (mpRowHeights && pRowFlags)
		{
            mpRowHeights->insertSegment(nStartRow, nSize, false);
            sal_uInt8 nNewFlags = pRowFlags->Insert( nStartRow, nSize);
            // only copy manual size flag, clear all others
            if (nNewFlags && (nNewFlags != CR_MANUALSIZE))
                pRowFlags->SetValue( nStartRow, nStartRow + nSize - 1,
                        nNewFlags & CR_MANUALSIZE);
		}

		if (pOutlineTable)
			pOutlineTable->InsertRow( nStartRow, nSize );

        mpFilteredRows->insertSegment(nStartRow, nSize, true);
        mpHiddenRows->insertSegment(nStartRow, nSize, true);

        if (!maRowManualBreaks.empty())
        {
            std::vector<SCROW> aUpdatedBreaks;

            while ( ! maRowManualBreaks.empty())
            {
                std::set<SCROW>::iterator aLast (--maRowManualBreaks.end());

                // Check if there are more entries that have to be processed.
                if (*aLast < nStartRow)
                    break;

                // Remember the updated break location and erase the entry.
                aUpdatedBreaks.push_back(static_cast<SCROW>(*aLast + nSize));
                maRowManualBreaks.erase(aLast);
            }

            // Insert the updated break locations.
            if ( ! aUpdatedBreaks.empty())
                maRowManualBreaks.insert(aUpdatedBreaks.begin(), aUpdatedBreaks.end());
        }
	}

	for (SCCOL j=nStartCol; j<=nEndCol; j++)
		aCol[j].InsertRow( nStartRow, nSize );
	DecRecalcLevel( false );

    InvalidatePageBreaks();
}


void ScTable::DeleteRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
							sal_Bool* pUndoOutline )
{
	IncRecalcLevel();
    InitializeNoteCaptions();
	if (nStartCol==0 && nEndCol==MAXCOL)
	{
        if (pRowFlags)
            pRowFlags->Remove( nStartRow, nSize);

        if (mpRowHeights)
            mpRowHeights->removeSegment(nStartRow, nStartRow+nSize);

		if (pOutlineTable)
			if (pOutlineTable->DeleteRow( nStartRow, nSize ))
				if (pUndoOutline)
					*pUndoOutline = sal_True;

        mpFilteredRows->removeSegment(nStartRow, nStartRow+nSize);
        mpHiddenRows->removeSegment(nStartRow, nStartRow+nSize);

        if (!maRowManualBreaks.empty())
        {
            std::set<SCROW>::iterator it = maRowManualBreaks.upper_bound( static_cast<SCROW>( nStartRow + nSize - 1));
            maRowManualBreaks.erase( maRowManualBreaks.lower_bound( nStartRow), it);
            while (it != maRowManualBreaks.end())
            {
                SCROW nRow = *it;
                maRowManualBreaks.erase( it++);
                maRowManualBreaks.insert( static_cast<SCROW>( nRow - nSize));
            }
        }
	}

    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
        for (SCCOL j=nStartCol; j<=nEndCol; j++)
            aCol[j].DeleteRow( nStartRow, nSize );
    }
	DecRecalcLevel();

    InvalidatePageBreaks();
}


sal_Bool ScTable::TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize )
{
	sal_Bool bTest = sal_True;

	if ( nStartRow==0 && nEndRow==MAXROW && pOutlineTable )
		bTest = pOutlineTable->TestInsertCol(nSize);

	if ( nSize > static_cast<SCSIZE>(MAXCOL) )
		bTest = sal_False;

	for (SCCOL i=MAXCOL; (i+static_cast<SCCOL>(nSize)>MAXCOL) && bTest; i--)
		bTest = aCol[i].TestInsertCol(nStartRow, nEndRow);

	return bTest;
}


void ScTable::InsertCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize )
{
	IncRecalcLevel();
    InitializeNoteCaptions();
	if (nStartRow==0 && nEndRow==MAXROW)
	{
		if (pColWidth && pColFlags)
        {
            memmove( &pColWidth[nStartCol+nSize], &pColWidth[nStartCol],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColWidth[0]) );
            memmove( &pColFlags[nStartCol+nSize], &pColFlags[nStartCol],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColFlags[0]) );
        }
		if (pOutlineTable)
			pOutlineTable->InsertCol( nStartCol, nSize );

        mpHiddenCols->insertSegment(nStartCol, static_cast<SCCOL>(nSize), true);
        mpFilteredCols->insertSegment(nStartCol, static_cast<SCCOL>(nSize), true);

        if (!maColManualBreaks.empty())
        {
            std::vector<SCCOL> aUpdatedBreaks;

            while ( ! maColManualBreaks.empty())
            {
                std::set<SCCOL>::iterator aLast (--maColManualBreaks.end());

                // Check if there are more entries that have to be processed.
                if (*aLast < nStartRow)
                    break;

                // Remember the updated break location and erase the entry.
                aUpdatedBreaks.push_back(static_cast<SCCOL>(*aLast + nSize));
                maColManualBreaks.erase(aLast);
            }

            // Insert the updated break locations.
            if ( ! aUpdatedBreaks.empty())
                maColManualBreaks.insert(aUpdatedBreaks.begin(), aUpdatedBreaks.end());
        }
	}


	if ((nStartRow == 0) && (nEndRow == MAXROW))
	{
		for (SCSIZE i=0; i < nSize; i++)
			for (SCCOL nCol = MAXCOL; nCol > nStartCol; nCol--)
				aCol[nCol].SwapCol(aCol[nCol-1]);
	}
	else
	{
		for (SCSIZE i=0; static_cast<SCCOL>(i+nSize)+nStartCol <= MAXCOL; i++)
			aCol[MAXCOL - nSize - i].MoveTo(nStartRow, nEndRow, aCol[MAXCOL - i]);
	}

	if (nStartCol>0)						// copy old attributes
	{
		sal_uInt16 nWhichArray[2];
		nWhichArray[0] = ATTR_MERGE;
		nWhichArray[1] = 0;

		for (SCSIZE i=0; i<nSize; i++)
		{
			aCol[nStartCol-1].CopyToColumn( nStartRow, nEndRow, IDF_ATTRIB,
												sal_False, aCol[nStartCol+i] );
			aCol[nStartCol+i].RemoveFlags( nStartRow, nEndRow,
												SC_MF_HOR | SC_MF_VER | SC_MF_AUTO );
			aCol[nStartCol+i].ClearItems( nStartRow, nEndRow, nWhichArray );
		}
	}
	DecRecalcLevel();

    InvalidatePageBreaks();
}


void ScTable::DeleteCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize,
							sal_Bool* pUndoOutline )
{
	IncRecalcLevel();
    InitializeNoteCaptions();
	if (nStartRow==0 && nEndRow==MAXROW)
	{
		if (pColWidth && pColFlags)
        {
            memmove( &pColWidth[nStartCol], &pColWidth[nStartCol+nSize],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColWidth[0]) );
            memmove( &pColFlags[nStartCol], &pColFlags[nStartCol+nSize],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColFlags[0]) );
        }
		if (pOutlineTable)
			if (pOutlineTable->DeleteCol( nStartCol, nSize ))
				if (pUndoOutline)
					*pUndoOutline = sal_True;

        SCCOL nRmSize = nStartCol + static_cast<SCCOL>(nSize);
        mpHiddenCols->removeSegment(nStartCol, nRmSize);
        mpFilteredCols->removeSegment(nStartCol, nRmSize);

        if (!maColManualBreaks.empty())
        {
            std::set<SCCOL>::iterator it = maColManualBreaks.upper_bound( static_cast<SCCOL>( nStartCol + nSize - 1));
            maColManualBreaks.erase( maColManualBreaks.lower_bound( nStartCol), it);
            while (it != maColManualBreaks.end())
            {
                SCCOL nCol = *it;
                maColManualBreaks.erase( it++);
                maColManualBreaks.insert( static_cast<SCCOL>( nCol - nSize));
            }
        }
	}


    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
        for (SCSIZE i = 0; i < nSize; i++)
            aCol[nStartCol + i].DeleteArea(nStartRow, nEndRow, IDF_ALL);
    }

	if ((nStartRow == 0) && (nEndRow == MAXROW))
	{
		for (SCSIZE i=0; i < nSize; i++)
			for (SCCOL nCol = nStartCol; nCol < MAXCOL; nCol++)
				aCol[nCol].SwapCol(aCol[nCol+1]);
	}
	else
	{
		for (SCSIZE i=0; static_cast<SCCOL>(i+nSize)+nStartCol <= MAXCOL; i++)
			aCol[nStartCol + nSize + i].MoveTo(nStartRow, nEndRow, aCol[nStartCol + i]);
	}
	DecRecalcLevel();

    InvalidatePageBreaks();
}


void ScTable::DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nDelFlag)
{
	if (nCol2 > MAXCOL) nCol2 = MAXCOL;
	if (nRow2 > MAXROW) nRow2 = MAXROW;
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
	{
//		IncRecalcLevel();

        {   // scope for bulk broadcast
            ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
            for (SCCOL i = nCol1; i <= nCol2; i++)
                aCol[i].DeleteArea(nRow1, nRow2, nDelFlag);
        }

			//
			// Zellschutz auf geschuetzter Tabelle nicht setzen
			//

		if ( IsProtected() && (nDelFlag & IDF_ATTRIB) )
		{
			ScPatternAttr aPattern(pDocument->GetPool());
			aPattern.GetItemSet().Put( ScProtectionAttr( sal_False ) );
			ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
		}

//		DecRecalcLevel();
	}
}


void ScTable::DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
        for (SCCOL i=0; i<=MAXCOL; i++)
            aCol[i].DeleteSelection( nDelFlag, rMark );
    }

		//
		// Zellschutz auf geschuetzter Tabelle nicht setzen
		//

	if ( IsProtected() && (nDelFlag & IDF_ATTRIB) )
	{
		ScDocumentPool* pPool = pDocument->GetPool();
		SfxItemSet aSet( *pPool, ATTR_PATTERN_START, ATTR_PATTERN_END );
		aSet.Put( ScProtectionAttr( sal_False ) );
		SfxItemPoolCache aCache( pPool, &aSet );
		ApplySelectionCache( &aCache, rMark );
	}
}


//	pTable = Clipboard
void ScTable::CopyToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                        ScTable* pTable, sal_Bool bKeepScenarioFlags, sal_Bool bCloneNoteCaptions)
{
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
	{
		//	Inhalte kopieren
		SCCOL i;

		for ( i = nCol1; i <= nCol2; i++)
            aCol[i].CopyToClip(nRow1, nRow2, pTable->aCol[i], bKeepScenarioFlags, bCloneNoteCaptions);

		//	copy widths/heights, and only "hidden", "filtered" and "manual" flags
		//	also for all preceding columns/rows, to have valid positions for drawing objects

		if (pColWidth && pTable->pColWidth)
			for (i=0; i<=nCol2; i++)
				pTable->pColWidth[i] = pColWidth[i];

        pTable->CopyColHidden(*this, 0, nCol2);
        pTable->CopyColFiltered(*this, 0, nCol2);

        if (pRowFlags && pTable->pRowFlags && mpRowHeights && pTable->mpRowHeights)
        {
            pTable->pRowFlags->CopyFromAnded( *pRowFlags, 0, nRow2, CR_MANUALSIZE);
            pTable->CopyRowHeight(*this, 0, nRow2, 0);
        }

        pTable->CopyRowHidden(*this, 0, nRow2);
        pTable->CopyRowFiltered(*this, 0, nRow2);

		//	ggf. Formeln durch Werte ersetzen

		if ( IsProtected() )
			for (i = nCol1; i <= nCol2; i++)
				pTable->aCol[i].RemoveProtected(nRow1, nRow2);
	}
}

void ScTable::CopyToClip(const ScRangeList& rRanges, ScTable* pTable, 
                         bool bKeepScenarioFlags, bool bCloneNoteCaptions)
{
    ScRangeList aRanges(rRanges);
    for (ScRangePtr p = aRanges.First(); p; p = aRanges.Next())
    {
        CopyToClip(p->aStart.Col(), p->aStart.Row(), p->aEnd.Col(), p->aEnd.Row(), 
                   pTable, bKeepScenarioFlags, bCloneNoteCaptions);
    }
}

void ScTable::CopyFromClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
							SCsCOL nDx, SCsROW nDy, sal_uInt16 nInsFlag,
							sal_Bool bAsLink, sal_Bool bSkipAttrForEmpty, ScTable* pTable)
{
	SCCOL i;

	if (nCol2 > MAXCOL) nCol2 = MAXCOL;
	if (nRow2 > MAXROW) nRow2 = MAXROW;
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
	{
		IncRecalcLevel();
		for ( i = nCol1; i <= nCol2; i++)
			aCol[i].CopyFromClip(nRow1, nRow2, nDy, nInsFlag, bAsLink, bSkipAttrForEmpty, pTable->aCol[i - nDx]);

		if ((nInsFlag & IDF_ATTRIB) != 0)
		{
			if (nRow1==0 && nRow2==MAXROW && pColWidth && pTable->pColWidth)
				for (i=nCol1; i<=nCol2; i++)
					pColWidth[i] = pTable->pColWidth[i-nDx];

            if (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pTable->mpRowHeights &&
											 pRowFlags && pTable->pRowFlags)
            {
                CopyRowHeight(*pTable, nRow1, nRow2, -nDy);
                // Must copy CR_MANUALSIZE bit too, otherwise pRowHeight doesn't make sense
				for (SCROW j=nRow1; j<=nRow2; j++)
				{
					if ( pTable->pRowFlags->GetValue(j-nDy) & CR_MANUALSIZE )
						pRowFlags->OrValue( j, CR_MANUALSIZE);
					else
                        pRowFlags->AndValue( j, sal::static_int_cast<sal_uInt8>(~CR_MANUALSIZE));
				}
            }

				//
				// Zellschutz auf geschuetzter Tabelle nicht setzen
				//

			if ( IsProtected() && (nInsFlag & IDF_ATTRIB) )
			{
				ScPatternAttr aPattern(pDocument->GetPool());
				aPattern.GetItemSet().Put( ScProtectionAttr( sal_False ) );
				ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
			}
		}
		DecRecalcLevel();
	}
}


void ScTable::MixData( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
							sal_uInt16 nFunction, sal_Bool bSkipEmpty, ScTable* pSrcTab )
{
	for (SCCOL i=nCol1; i<=nCol2; i++)
		aCol[i].MixData( nRow1, nRow2, nFunction, bSkipEmpty, pSrcTab->aCol[i] );
}


//	Markierung von diesem Dokument
void ScTable::MixMarked( const ScMarkData& rMark, sal_uInt16 nFunction,
						sal_Bool bSkipEmpty, ScTable* pSrcTab )
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].MixMarked( rMark, nFunction, bSkipEmpty, pSrcTab->aCol[i] );
}


void ScTable::TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
								ScTable* pTransClip, sal_uInt16 nFlags, sal_Bool bAsLink )
{
	sal_Bool bWasCut = pDocument->IsCutMode();

	ScDocument* pDestDoc = pTransClip->pDocument;

	for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
	{
		SCROW nRow;
		ScBaseCell* pCell;

		if ( bAsLink && nFlags == IDF_ALL )
		{
			//	#68989# with IDF_ALL, also create links (formulas) for empty cells

			for ( nRow=nRow1; nRow<=nRow2; nRow++ )
			{
				//	create simple formula, as in ScColumn::CreateRefCell

				ScAddress aDestPos( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
				ScSingleRefData aRef;
				aRef.nCol = nCol;
				aRef.nRow = nRow;
				aRef.nTab = nTab;
				aRef.InitFlags();							// -> all absolute
				aRef.SetFlag3D(sal_True);
				aRef.CalcRelFromAbs( aDestPos );
				ScTokenArray aArr;
				aArr.AddSingleReference( aRef );

				ScBaseCell* pNew = new ScFormulaCell( pDestDoc, aDestPos, &aArr );
				pTransClip->PutCell( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), pNew );
			}
		}
		else
		{
			ScColumnIterator aIter( &aCol[nCol], nRow1, nRow2 );
			while (aIter.Next( nRow, pCell ))
			{
                ScAddress aDestPos( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
				ScBaseCell* pNew;
				if ( bAsLink )					// Referenz erzeugen ?
				{
					pNew = aCol[nCol].CreateRefCell( pDestDoc, aDestPos, aIter.GetIndex(), nFlags );
				}
				else							// kopieren
				{
                    ScAddress aOwnPos( nCol, nRow, nTab );
					if (pCell->GetCellType() == CELLTYPE_FORMULA)
					{
                        pNew = pCell->CloneWithNote( aOwnPos, *pDestDoc, aDestPos, SC_CLONECELL_STARTLISTENING );

						//	Referenzen drehen
						//	bei Cut werden Referenzen spaeter per UpdateTranspose angepasst

						if (!bWasCut)
							((ScFormulaCell*)pNew)->TransposeReference();
					}
					else
                    {
                        pNew = pCell->CloneWithNote( aOwnPos, *pDestDoc, aDestPos );
                    }
				}
				pTransClip->PutCell( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), pNew );
			}
		}

		//	Attribute

		SCROW nAttrRow1;
		SCROW nAttrRow2;
		const ScPatternAttr* pPattern;
		ScAttrIterator* pAttrIter = aCol[nCol].CreateAttrIterator( nRow1, nRow2 );
		while ( (pPattern = pAttrIter->Next( nAttrRow1, nAttrRow2 )) != 0 )
		{
			if ( !IsDefaultItem( pPattern ) )
			{
				const SfxItemSet& rSet = pPattern->GetItemSet();
				if ( rSet.GetItemState( ATTR_MERGE, sal_False ) == SFX_ITEM_DEFAULT &&
					 rSet.GetItemState( ATTR_MERGE_FLAG, sal_False ) == SFX_ITEM_DEFAULT &&
					 rSet.GetItemState( ATTR_BORDER, sal_False ) == SFX_ITEM_DEFAULT )
				{
					// no borders or merge items involved - use pattern as-is
					for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
						pTransClip->SetPattern( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), *pPattern, sal_True );
				}
				else
				{
					// transpose borders and merge values, remove merge flags (refreshed after pasting)
					ScPatternAttr aNewPattern( *pPattern );
					SfxItemSet& rNewSet = aNewPattern.GetItemSet();

					const SvxBoxItem& rOldBox = (const SvxBoxItem&)rSet.Get(ATTR_BORDER);
					if ( rOldBox.GetTop() || rOldBox.GetBottom() || rOldBox.GetLeft() || rOldBox.GetRight() )
					{
						SvxBoxItem aNew( ATTR_BORDER );
						aNew.SetLine( rOldBox.GetLine( BOX_LINE_TOP ), BOX_LINE_LEFT );
						aNew.SetLine( rOldBox.GetLine( BOX_LINE_LEFT ), BOX_LINE_TOP );
						aNew.SetLine( rOldBox.GetLine( BOX_LINE_BOTTOM ), BOX_LINE_RIGHT );
						aNew.SetLine( rOldBox.GetLine( BOX_LINE_RIGHT ), BOX_LINE_BOTTOM );
						aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_TOP ), BOX_LINE_LEFT );
						aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_LEFT ), BOX_LINE_TOP );
						aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_BOTTOM ), BOX_LINE_RIGHT );
						aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_RIGHT ), BOX_LINE_BOTTOM );
						rNewSet.Put( aNew );
					}

					const ScMergeAttr& rOldMerge = (const ScMergeAttr&)rSet.Get(ATTR_MERGE);
					if (rOldMerge.IsMerged())
                        rNewSet.Put( ScMergeAttr( Min(
                                        static_cast<SCsCOL>(rOldMerge.GetRowMerge()),
                                        static_cast<SCsCOL>(MAXCOL+1 - (nAttrRow2-nRow1))),
                                    Min(
                                        static_cast<SCsROW>(rOldMerge.GetColMerge()),
                                        static_cast<SCsROW>(MAXROW+1 - (nCol-nCol1)))));
					const ScMergeFlagAttr& rOldFlag = (const ScMergeFlagAttr&)rSet.Get(ATTR_MERGE_FLAG);
					if (rOldFlag.IsOverlapped())
					{
						sal_Int16 nNewFlags = rOldFlag.GetValue() & ~( SC_MF_HOR | SC_MF_VER );
						if ( nNewFlags )
							rNewSet.Put( ScMergeFlagAttr( nNewFlags ) );
						else
							rNewSet.ClearItem( ATTR_MERGE_FLAG );
					}

					for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( static_cast<SCCOL>(nRow-nRow1),
                                static_cast<SCROW>(nCol-nCol1), aNewPattern, sal_True);
				}
			}
		}

		delete pAttrIter;
	}
}


void ScTable::StartAllListeners()
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].StartAllListeners();
}


void ScTable::StartNeededListeners()
{
	for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].StartNeededListeners();
}


void ScTable::BroadcastInArea( SCCOL nCol1, SCROW nRow1,
		SCCOL nCol2, SCROW nRow2 )
{
	if (nCol2 > MAXCOL) nCol2 = MAXCOL;
	if (nRow2 > MAXROW) nRow2 = MAXROW;
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
		for (SCCOL i = nCol1; i <= nCol2; i++)
			aCol[i].BroadcastInArea( nRow1, nRow2 );
}


void ScTable::StartListeningInArea( SCCOL nCol1, SCROW nRow1,
		SCCOL nCol2, SCROW nRow2 )
{
	if (nCol2 > MAXCOL) nCol2 = MAXCOL;
	if (nRow2 > MAXROW) nRow2 = MAXROW;
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
		for (SCCOL i = nCol1; i <= nCol2; i++)
			aCol[i].StartListeningInArea( nRow1, nRow2 );
}


void ScTable::CopyToTable(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
							sal_uInt16 nFlags, sal_Bool bMarked, ScTable* pDestTab,
							const ScMarkData* pMarkData,
							sal_Bool bAsLink, sal_Bool bColRowFlags)
{
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
	{
		if (nFlags)
			for (SCCOL i = nCol1; i <= nCol2; i++)
				aCol[i].CopyToColumn(nRow1, nRow2, nFlags, bMarked,
								pDestTab->aCol[i], pMarkData, bAsLink);

		if (bColRowFlags)		// Spaltenbreiten/Zeilenhoehen/Flags
		{
			//	Charts muessen beim Ein-/Ausblenden angepasst werden
			ScChartListenerCollection* pCharts = pDestTab->pDocument->GetChartListenerCollection();

            bool bFlagChange = false;

			sal_Bool bWidth  = (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth);
            sal_Bool bHeight = (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pDestTab->mpRowHeights);

			if (bWidth||bHeight)
			{
				pDestTab->IncRecalcLevel();

				if (bWidth)
                {
					for (SCCOL i=nCol1; i<=nCol2; i++)
					{
                        bool bThisHidden = ColHidden(i);
                        bool bHiddenChange = (pDestTab->ColHidden(i) != bThisHidden);
                        bool bChange = bHiddenChange || (pDestTab->pColWidth[i] != pColWidth[i]);
						pDestTab->pColWidth[i] = pColWidth[i];
						pDestTab->pColFlags[i] = pColFlags[i];
                        pDestTab->SetColHidden(i, i, bThisHidden);
						//!	Aenderungen zusammenfassen?
                        if (bHiddenChange && pCharts) 
                            pCharts->SetRangeDirty(ScRange( i, 0, nTab, i, MAXROW, nTab ));

                        if (bChange)
                            bFlagChange = true;
					}
                    pDestTab->SetColManualBreaks( maColManualBreaks);
                }

				if (bHeight)
				{
                    bool bChange = pDestTab->GetRowHeight(nRow1, nRow2) != GetRowHeight(nRow1, nRow2);

                    if (bChange)
                        bFlagChange = true;

                    pDestTab->CopyRowHeight(*this, nRow1, nRow2, 0);
                    pDestTab->pRowFlags->CopyFrom(*pRowFlags, nRow1, nRow2);

                    // Hidden flags.
                    // #i116164# Collect information first, then apply the changes,
                    // so RowHidden doesn't rebuild the tree for each row range.
                    std::vector<ScShowRowsEntry> aEntries;
                    for (SCROW i = nRow1; i <= nRow2; ++i)
                    {
                        SCROW nThisLastRow, nDestLastRow;
                        bool bThisHidden = RowHidden(i, NULL, &nThisLastRow);
                        bool bDestHidden = pDestTab->RowHidden(i, NULL, &nDestLastRow);
    
                        // If the segment sizes differ, we take the shorter segment of the two.
                        SCROW nLastRow = ::std::min(nThisLastRow, nDestLastRow);
                        if (nLastRow >= nRow2)
                            // the last row shouldn't exceed the upper bound the caller specified.
                            nLastRow = nRow2;
    
                        //pDestTab->SetRowHidden(i, nLastRow, bThisHidden);
                        aEntries.push_back(ScShowRowsEntry(i, nLastRow, !bThisHidden));

                        bool bThisHiddenChange = (bThisHidden != bDestHidden);
                        if (bThisHiddenChange && pCharts)
                        {
                            // Hidden flags differ.
                            pCharts->SetRangeDirty(ScRange(0, i, nTab, MAXCOL, nLastRow, nTab));
                        }
    
                        if (bThisHiddenChange)
                            bFlagChange = true;
    
                        // Jump to the last row of the identical flag segment.
                        i = nLastRow;
					}

                    std::vector<ScShowRowsEntry>::const_iterator aEnd = aEntries.end();
                    std::vector<ScShowRowsEntry>::const_iterator aIter = aEntries.begin();
                    if ( aIter != aEnd )
                    {
                        pDestTab->mpHiddenRows->setInsertFromBack(true);    // important for undo document
                        while (aIter != aEnd)
                        {
                            pDestTab->SetRowHidden(aIter->mnRow1, aIter->mnRow2, !aIter->mbShow);
                            ++aIter;
                        }
                        pDestTab->mpHiddenRows->setInsertFromBack(false);
                    }

                    // Filtered flags.
                    for (SCROW i = nRow1; i <= nRow2; ++i)
                    {
                        SCROW nLastRow;
                        bool bFiltered = RowFiltered(i, NULL, &nLastRow);
                        if (nLastRow >= nRow2)
                            // the last row shouldn't exceed the upper bound the caller specified.
                            nLastRow = nRow2;
                        pDestTab->SetRowFiltered(i, nLastRow, bFiltered);
                        i = nLastRow;
                    }
                    pDestTab->SetRowManualBreaks( maRowManualBreaks);
				}
				pDestTab->DecRecalcLevel();
			}

            if (bFlagChange)
                pDestTab->InvalidatePageBreaks();

			pDestTab->SetOutlineTable( pOutlineTable );		// auch nur wenn bColRowFlags
		}
	}
}


void ScTable::UndoToTable(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
							sal_uInt16 nFlags, sal_Bool bMarked, ScTable* pDestTab,
							const ScMarkData* pMarkData)
{
	if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
	{
		sal_Bool bWidth  = (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth);
        sal_Bool bHeight = (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pDestTab->mpRowHeights);

		if (bWidth||bHeight)
			IncRecalcLevel();

		for ( SCCOL i = 0; i <= MAXCOL; i++)
		{
			if ( i >= nCol1 && i <= nCol2 )
				aCol[i].UndoToColumn(nRow1, nRow2, nFlags, bMarked, pDestTab->aCol[i],
										pMarkData);
			else
				aCol[i].CopyToColumn(0, MAXROW, IDF_FORMULA, sal_False, pDestTab->aCol[i]);
		}

		if (bWidth||bHeight)
		{
            if (bWidth)
            {
                for (SCCOL i=nCol1; i<=nCol2; i++)
                    pDestTab->pColWidth[i] = pColWidth[i];
                pDestTab->SetColManualBreaks( maColManualBreaks);
            }
            if (bHeight)
            {
                pDestTab->CopyRowHeight(*this, nRow1, nRow2, 0);
                pDestTab->SetRowManualBreaks( maRowManualBreaks);
            }
            DecRecalcLevel();
		}
	}
}


void ScTable::CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].CopyUpdated( pPosTab->aCol[i], pDestTab->aCol[i] );
}

void ScTable::InvalidateTableArea()
{
    bTableAreaValid = sal_False;
}

void ScTable::InvalidatePageBreaks()
{
    mbPageBreaksValid = false;
}

void ScTable::CopyScenarioTo( ScTable* pDestTab ) const
{
	DBG_ASSERT( bScenario, "bScenario == FALSE" );

	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].CopyScenarioTo( pDestTab->aCol[i] );
}

void ScTable::CopyScenarioFrom( const ScTable* pSrcTab )
{
	DBG_ASSERT( bScenario, "bScenario == FALSE" );

	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].CopyScenarioFrom( pSrcTab->aCol[i] );
}

void ScTable::MarkScenarioIn( ScMarkData& rDestMark, sal_uInt16 nNeededBits ) const
{
	DBG_ASSERT( bScenario, "bScenario == FALSE" );

	if ( ( nScenarioFlags & nNeededBits ) != nNeededBits )	// alle Bits gesetzt?
		return;

	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].MarkScenarioIn( rDestMark );
}

sal_Bool ScTable::HasScenarioRange( const ScRange& rRange ) const
{
	DBG_ASSERT( bScenario, "bScenario == FALSE" );

//	ScMarkData aMark;
//	MarkScenarioIn( aMark, 0 );				//! Bits als Parameter von HasScenarioRange?
//	return aMark.IsAllMarked( rRange );

	ScRange aTabRange = rRange;
	aTabRange.aStart.SetTab( nTab );
	aTabRange.aEnd.SetTab( nTab );

	const ScRangeList* pList = GetScenarioRanges();
//	return ( pList && pList->Find( aTabRange ) );

	if (pList)
	{
		sal_uLong nCount = pList->Count();
		for ( sal_uLong j = 0; j < nCount; j++ )
		{
			ScRange* pR = pList->GetObject( j );
			if ( pR->Intersects( aTabRange ) )
				return sal_True;
		}
	}

	return sal_False;
}

void ScTable::InvalidateScenarioRanges()
{
	delete pScenarioRanges;
	pScenarioRanges = NULL;
}

const ScRangeList* ScTable::GetScenarioRanges() const
{
	DBG_ASSERT( bScenario, "bScenario == FALSE" );

	if (!pScenarioRanges)
	{
		((ScTable*)this)->pScenarioRanges = new ScRangeList;
		ScMarkData aMark;
		MarkScenarioIn( aMark, 0 );		// immer
		aMark.FillRangeListWithMarks( pScenarioRanges, sal_False );
	}
	return pScenarioRanges;
}

sal_Bool ScTable::TestCopyScenarioTo( const ScTable* pDestTab ) const
{
	DBG_ASSERT( bScenario, "bScenario == FALSE" );

	if (!pDestTab->IsProtected())
		return sal_True;

	sal_Bool bOk = sal_True;
	for (SCCOL i=0; i<=MAXCOL && bOk; i++)
		bOk = aCol[i].TestCopyScenarioTo( pDestTab->aCol[i] );
	return bOk;
}

void ScTable::PutCell( SCCOL nCol, SCROW nRow, ScBaseCell* pCell )
{
	if (ValidColRow(nCol,nRow))
	{
		if (pCell)
			aCol[nCol].Insert( nRow, pCell );
		else
			aCol[nCol].Delete( nRow );
	}
}


void ScTable::PutCell( SCCOL nCol, SCROW nRow, sal_uLong nFormatIndex, ScBaseCell* pCell )
{
	if (ValidColRow(nCol,nRow))
	{
		if (pCell)
			aCol[nCol].Insert( nRow, nFormatIndex, pCell );
		else
			aCol[nCol].Delete( nRow );
	}
}


void ScTable::PutCell( const ScAddress& rPos, ScBaseCell* pCell )
{
	if (pCell)
		aCol[rPos.Col()].Insert( rPos.Row(), pCell );
	else
		aCol[rPos.Col()].Delete( rPos.Row() );
}


//UNUSED2009-05 void ScTable::PutCell( const ScAddress& rPos, sal_uLong nFormatIndex, ScBaseCell* pCell )
//UNUSED2009-05 {
//UNUSED2009-05     if (pCell)
//UNUSED2009-05         aCol[rPos.Col()].Insert( rPos.Row(), nFormatIndex, pCell );
//UNUSED2009-05     else
//UNUSED2009-05         aCol[rPos.Col()].Delete( rPos.Row() );
//UNUSED2009-05 }


sal_Bool ScTable::SetString( SCCOL nCol, SCROW nRow, SCTAB nTabP, const String& rString, 
                         SvNumberFormatter* pFormatter, bool bDetectNumberFormat )
{
	if (ValidColRow(nCol,nRow))
        return aCol[nCol].SetString( 
            nRow, nTabP, rString, pDocument->GetAddressConvention(), pFormatter, bDetectNumberFormat );
	else
		return sal_False;
}


void ScTable::SetValue( SCCOL nCol, SCROW nRow, const double& rVal )
{
	if (ValidColRow(nCol, nRow))
		aCol[nCol].SetValue( nRow, rVal );
}


void ScTable::GetString( SCCOL nCol, SCROW nRow, String& rString )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].GetString( nRow, rString );
	else
		rString.Erase();
}

void  ScTable::FillDPCache( ScDPTableDataCache * pCache, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow )
{
    for ( sal_uInt16 nCol = nStartCol; nCol <= nEndCol; nCol++ )
        if( ValidCol( nCol ) )
            aCol[nCol].FillDPCache( pCache, nCol - nStartCol, nStartRow, nEndRow );
}


void ScTable::GetInputString( SCCOL nCol, SCROW nRow, String& rString )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].GetInputString( nRow, rString );
	else
		rString.Erase();
}


double ScTable::GetValue( SCCOL nCol, SCROW nRow )
{
	if (ValidColRow( nCol, nRow ))
		return aCol[nCol].GetValue( nRow );
	return 0.0;
}


void ScTable::GetFormula( SCCOL nCol, SCROW nRow, String& rFormula,
						  sal_Bool bAsciiExport )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].GetFormula( nRow, rFormula, bAsciiExport );
	else
		rFormula.Erase();
}


ScPostIt* ScTable::GetNote( SCCOL nCol, SCROW nRow )
{
	return ValidColRow( nCol, nRow ) ? aCol[ nCol ].GetNote( nRow ) : 0;
}


void ScTable::TakeNote( SCCOL nCol, SCROW nRow, ScPostIt*& rpNote )
{
	if( ValidColRow( nCol, nRow ) )
    {
        aCol[ nCol ].TakeNote( nRow, rpNote );
        if( rpNote && rpNote->GetNoteData().mxInitData.get() )
        {
            if( !mxUninitNotes.get() )
                mxUninitNotes.reset( new ScAddress2DVec );
            mxUninitNotes->push_back( ScAddress2D( nCol, nRow ) );
        }
    }
    else
        DELETEZ( rpNote );
}


ScPostIt* ScTable::ReleaseNote( SCCOL nCol, SCROW nRow )
{
	return ValidColRow( nCol, nRow ) ? aCol[ nCol ].ReleaseNote( nRow ) : 0;
}


void ScTable::DeleteNote( SCCOL nCol, SCROW nRow )
{
	if( ValidColRow( nCol, nRow ) )
        aCol[ nCol ].DeleteNote( nRow );
}


void ScTable::InitializeNoteCaptions( bool bForced )
{
    if( mxUninitNotes.get() && (bForced || pDocument->IsUndoEnabled()) )
    {
        for( ScAddress2DVec::iterator aIt = mxUninitNotes->begin(), aEnd = mxUninitNotes->end(); aIt != aEnd; ++aIt )
            if( ScPostIt* pNote = GetNote( aIt->first, aIt->second ) )
                pNote->GetOrCreateCaption( ScAddress( aIt->first, aIt->second, nTab ) );
        mxUninitNotes.reset();
    }
}

CellType ScTable::GetCellType( SCCOL nCol, SCROW nRow ) const
{
	if (ValidColRow( nCol, nRow ))
		return aCol[nCol].GetCellType( nRow );
	return CELLTYPE_NONE;
}


ScBaseCell* ScTable::GetCell( SCCOL nCol, SCROW nRow ) const
{
	if (ValidColRow( nCol, nRow ))
		return aCol[nCol].GetCell( nRow );

	DBG_ERROR("GetCell ausserhalb");
	return NULL;
}

void ScTable::GetFirstDataPos(SCCOL& rCol, SCROW& rRow) const
{
    rCol = 0;
    rRow = MAXROW+1;
    while (aCol[rCol].IsEmptyData() && rCol < MAXCOL)
        ++rCol;
    SCCOL nCol = rCol;
    while (nCol <= MAXCOL && rRow > 0)
    {
        if (!aCol[nCol].IsEmptyData())
            rRow = ::std::min( rRow, aCol[nCol].GetFirstDataPos());
        ++nCol;
    }
}

void ScTable::GetLastDataPos(SCCOL& rCol, SCROW& rRow) const
{
    rCol = MAXCOL;
    rRow = 0;
    while (aCol[rCol].IsEmptyData() && (rCol > 0))
        rCol--;
    SCCOL nCol = rCol;
    while (nCol >= 0 && rRow < MAXROW)
        rRow = ::std::max( rRow, aCol[nCol--].GetLastDataPos());
}


sal_Bool ScTable::HasData( SCCOL nCol, SCROW nRow )
{
	if (ValidColRow(nCol,nRow))
		return aCol[nCol].HasDataAt( nRow );
	else
		return sal_False;
}


sal_Bool ScTable::HasStringData( SCCOL nCol, SCROW nRow )
{
	if (ValidColRow(nCol,nRow))
		return aCol[nCol].HasStringData( nRow );
	else
		return sal_False;
}


sal_Bool ScTable::HasValueData( SCCOL nCol, SCROW nRow )
{
	if (ValidColRow(nCol,nRow))
		return aCol[nCol].HasValueData( nRow );
	else
		return sal_False;
}


sal_Bool ScTable::HasStringCells( SCCOL nStartCol, SCROW nStartRow,
								SCCOL nEndCol, SCROW nEndRow ) const
{
	if ( ValidCol(nEndCol) )
		for ( SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++ )
			if (aCol[nCol].HasStringCells(nStartRow, nEndRow))
				return sal_True;

	return sal_False;
}


//UNUSED2008-05  sal_uInt16 ScTable::GetErrCode( SCCOL nCol, SCROW nRow ) const
//UNUSED2008-05  {
//UNUSED2008-05      if (ValidColRow( nCol, nRow ))
//UNUSED2008-05          return aCol[nCol].GetErrCode( nRow );
//UNUSED2008-05      return 0;
//UNUSED2008-05  }


void ScTable::SetDirtyVar()
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].SetDirtyVar();
}


void ScTable::SetDirty()
{
	sal_Bool bOldAutoCalc = pDocument->GetAutoCalc();
	pDocument->SetAutoCalc( sal_False );	// Mehrfachberechnungen vermeiden
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].SetDirty();
	pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetDirty( const ScRange& rRange )
{
	sal_Bool bOldAutoCalc = pDocument->GetAutoCalc();
	pDocument->SetAutoCalc( sal_False );	// Mehrfachberechnungen vermeiden
	SCCOL nCol2 = rRange.aEnd.Col();
	for (SCCOL i=rRange.aStart.Col(); i<=nCol2; i++)
		aCol[i].SetDirty( rRange );
	pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetTableOpDirty( const ScRange& rRange )
{
	sal_Bool bOldAutoCalc = pDocument->GetAutoCalc();
	pDocument->SetAutoCalc( sal_False );	// no multiple recalculation
	SCCOL nCol2 = rRange.aEnd.Col();
	for (SCCOL i=rRange.aStart.Col(); i<=nCol2; i++)
		aCol[i].SetTableOpDirty( rRange );
	pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetDirtyAfterLoad()
{
	sal_Bool bOldAutoCalc = pDocument->GetAutoCalc();
	pDocument->SetAutoCalc( sal_False );	// Mehrfachberechnungen vermeiden
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].SetDirtyAfterLoad();
	pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetRelNameDirty()
{
	sal_Bool bOldAutoCalc = pDocument->GetAutoCalc();
	pDocument->SetAutoCalc( sal_False );	// Mehrfachberechnungen vermeiden
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].SetRelNameDirty();
	pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetLoadingMedium(bool bLoading)
{
    mpRowHeights->enableTreeSearch(!bLoading);

    // When loading a medium, prefer inserting row heights from the back
    // position since the row heights are stored and read in ascending order
    // during import.
    mpRowHeights->setInsertFromBack(bLoading);
}


void ScTable::CalcAll()
{
	for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CalcAll();
}


void ScTable::CompileAll()
{
	for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].CompileAll();
}


void ScTable::CompileXML( ScProgress& rProgress )
{
	for (SCCOL i=0; i <= MAXCOL; i++)
	{
		aCol[i].CompileXML( rProgress );
	}
}

void ScTable::CalcAfterLoad()
{
	for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].CalcAfterLoad();
}


void ScTable::ResetChanged( const ScRange& rRange )
{
	SCCOL nStartCol = rRange.aStart.Col();
	SCROW nStartRow = rRange.aStart.Row();
	SCCOL nEndCol = rRange.aEnd.Col();
	SCROW nEndRow = rRange.aEnd.Row();

	for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
		aCol[nCol].ResetChanged(nStartRow, nEndRow);
}

//	Attribute

const SfxPoolItem* ScTable::GetAttr( SCCOL nCol, SCROW nRow, sal_uInt16 nWhich ) const
{
	if (ValidColRow(nCol,nRow))
		return aCol[nCol].GetAttr( nRow, nWhich );
	else
		return NULL;
}


sal_uLong ScTable::GetNumberFormat( SCCOL nCol, SCROW nRow ) const
{
	if (ValidColRow(nCol,nRow))
		return aCol[nCol].GetNumberFormat( nRow );
	else
		return 0;
}


const ScPatternAttr* ScTable::GetPattern( SCCOL nCol, SCROW nRow ) const
{
	if (ValidColRow(nCol,nRow))
		return aCol[nCol].GetPattern( nRow );
	else
    {
        DBG_ERROR("wrong column or row");
        return pDocument->GetDefPattern();      // for safety
    }
}


const ScPatternAttr* ScTable::GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if ( ValidColRow( nCol, nStartRow ) && ValidRow( nEndRow ) && (nStartRow <= nEndRow) )
        return aCol[nCol].GetMostUsedPattern( nStartRow, nEndRow );
    else
        return NULL;
}


bool ScTable::HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nMask ) const
{
	bool bFound = false;
	for (SCCOL i=nCol1; i<=nCol2 && !bFound; i++)
		bFound |= aCol[i].HasAttrib( nRow1, nRow2, nMask );
	return bFound;
}


//UNUSED2009-05 sal_Bool ScTable::HasLines( const ScRange& rRange, Rectangle& rSizes ) const
//UNUSED2009-05 {
//UNUSED2009-05     SCCOL nCol1 = rRange.aStart.Col();
//UNUSED2009-05     SCROW nRow1 = rRange.aStart.Row();
//UNUSED2009-05     SCCOL nCol2 = rRange.aEnd.Col();
//UNUSED2009-05     SCROW nRow2 = rRange.aEnd.Row();
//UNUSED2009-05     PutInOrder( nCol1, nCol2 );
//UNUSED2009-05     PutInOrder( nRow1, nRow2 );
//UNUSED2009-05 
//UNUSED2009-05     sal_Bool bFound = sal_False;
//UNUSED2009-05     for (SCCOL i=nCol1; i<=nCol2; i++)
//UNUSED2009-05         if (aCol[i].HasLines( nRow1, nRow2, rSizes, (i==nCol1), (i==nCol2) ))
//UNUSED2009-05             bFound = sal_True;
//UNUSED2009-05 
//UNUSED2009-05     return bFound;
//UNUSED2009-05 }


sal_Bool ScTable::HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const
{
	sal_Bool bFound=sal_False;
	for (SCCOL i=0; i<=MAXCOL && !bFound; i++)
		bFound |= aCol[i].HasAttribSelection( rMark, nMask );
	return bFound;
}


sal_Bool ScTable::ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
						   SCCOL& rEndCol, SCROW& rEndRow,
						   sal_Bool bRefresh, sal_Bool bAttrs )
{
    if (!(ValidCol(nStartCol) && ValidCol(rEndCol)))
    {
        DBG_ERRORFILE("ScTable::ExtendMerge: invalid column number");
        return sal_False;
    }
	sal_Bool bFound=sal_False;
	SCCOL nOldEndX = rEndCol;
	SCROW nOldEndY = rEndRow;
	for (SCCOL i=nStartCol; i<=nOldEndX; i++)
		bFound |= aCol[i].ExtendMerge( i, nStartRow, nOldEndY, rEndCol, rEndRow, bRefresh, bAttrs );
	return bFound;
}


sal_Bool ScTable::IsBlockEmpty( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bIgnoreNotes ) const
{
    if (!(ValidCol(nCol1) && ValidCol(nCol2)))
    {
        DBG_ERRORFILE("ScTable::IsBlockEmpty: invalid column number");
        return sal_False;
    }
	sal_Bool bEmpty = sal_True;
	for (SCCOL i=nCol1; i<=nCol2 && bEmpty; i++)
		bEmpty = aCol[i].IsEmptyBlock( nRow1, nRow2, bIgnoreNotes );
	return bEmpty;
}

SCSIZE ScTable::FillMaxRot( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2,
							SCCOL nCol, SCROW nAttrRow1, SCROW nAttrRow2, SCSIZE nArrY,
							const ScPatternAttr* pPattern, const SfxItemSet* pCondSet )
{
	//	Rueckgabe = neues nArrY

	sal_uInt8 nRotDir = pPattern->GetRotateDir( pCondSet );
	if ( nRotDir != SC_ROTDIR_NONE )
	{
		sal_Bool bHit = sal_True;
		if ( nCol+1 < nX1 )								// column to the left
			bHit = ( nRotDir != SC_ROTDIR_LEFT );
		else if ( nCol > nX2+1 )						// column to the right
			bHit = ( nRotDir != SC_ROTDIR_RIGHT );		// SC_ROTDIR_STANDARD may now also be extended to the left

		if ( bHit )
		{
			double nFactor = 0.0;
			if ( nCol > nX2+1 )
			{
				long nRotVal = ((const SfxInt32Item&) pPattern->
						GetItem( ATTR_ROTATE_VALUE, pCondSet )).GetValue();
				double nRealOrient = nRotVal * F_PI18000;	// 1/100 Grad
				double nCos = cos( nRealOrient );
				double nSin = sin( nRealOrient );
				//!	begrenzen !!!
				//!	zusaetzlich Faktor fuer unterschiedliche PPT X/Y !!!

				//	bei SC_ROTDIR_LEFT kommt immer ein negativer Wert heraus,
				//	wenn der Modus beruecksichtigt wird
				nFactor = -fabs( nCos / nSin );
			}

			for ( SCROW nRow = nAttrRow1; nRow <= nAttrRow2; nRow++ )
			{
                if (!RowHidden(nRow))
				{
					sal_Bool bHitOne = sal_True;
					if ( nCol > nX2+1 )
					{
						// reicht die gedrehte Zelle bis in den sichtbaren Bereich?

						SCCOL nTouchedCol = nCol;
                        long nWidth = static_cast<long>(mpRowHeights->getValue(nRow) * nFactor);
						DBG_ASSERT(nWidth <= 0, "Richtung falsch");
						while ( nWidth < 0 && nTouchedCol > 0 )
						{
							--nTouchedCol;
							nWidth += GetColWidth( nTouchedCol );
						}
						if ( nTouchedCol > nX2 )
							bHitOne = sal_False;
					}

					if (bHitOne)
					{
						while ( nArrY<nArrCount && pRowInfo[nArrY].nRowNo < nRow )
							++nArrY;
						if ( nArrY<nArrCount && pRowInfo[nArrY].nRowNo == nRow )
							pRowInfo[nArrY].nRotMaxCol = nCol;
					}
				}
			}
		}
	}

	return nArrY;
}

void ScTable::FindMaxRotCol( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2 )
{
    if ( !pColWidth || !mpRowHeights || !pColFlags || !pRowFlags )
	{
		DBG_ERROR( "Spalten-/Zeileninfo fehlt" );
		return;
	}

	//	nRotMaxCol ist auf SC_ROTMAX_NONE initialisiert, nRowNo ist schon gesetzt

	SCROW nY1 = pRowInfo[0].nRowNo;
	SCROW nY2 = pRowInfo[nArrCount-1].nRowNo;

	for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
	{
        if (!ColHidden(nCol))
		{
			SCSIZE nArrY = 0;
			ScDocAttrIterator aIter( pDocument, nTab, nCol, nY1, nCol, nY2 );
			SCCOL nAttrCol;
            SCROW nAttrRow1, nAttrRow2;
			const ScPatternAttr* pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
			while ( pPattern )
			{
				const SfxPoolItem* pCondItem;
				if ( pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, sal_True, &pCondItem )
						== SFX_ITEM_SET )
				{
					//	alle Formate durchgehen, damit die Zellen nicht einzeln
					//	angeschaut werden muessen

					sal_uLong nIndex = ((const SfxUInt32Item*)pCondItem)->GetValue();
					ScConditionalFormatList* pList = pDocument->GetCondFormList();
					ScStyleSheetPool* pStylePool = pDocument->GetStyleSheetPool();
					if (pList && pStylePool && nIndex)
					{
						const ScConditionalFormat* pFormat = pList->GetFormat(nIndex);
						if ( pFormat )
						{
							sal_uInt16 nEntryCount = pFormat->Count();
							for (sal_uInt16 nEntry=0; nEntry<nEntryCount; nEntry++)
							{
                                String aStyleName = pFormat->GetEntry(nEntry)->GetStyle();
                                if (aStyleName.Len())
								{
									SfxStyleSheetBase* pStyleSheet =
                                            pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
									if ( pStyleSheet )
									{
										FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
													nCol, nAttrRow1, nAttrRow2,
													nArrY, pPattern, &pStyleSheet->GetItemSet() );
										//	nArrY nicht veraendern
									}
								}
							}
						}
					}
				}

				nArrY = FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
									nCol, nAttrRow1, nAttrRow2,
									nArrY, pPattern, NULL );

				pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
			}
		}
	}
}

sal_Bool ScTable::HasBlockMatrixFragment( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
	// nix:0, mitte:1, unten:2, links:4, oben:8, rechts:16, offen:32
	sal_uInt16 nEdges;

	if ( nCol1 == nCol2 )
	{	// linke und rechte Spalte
		const sal_uInt16 n = 4 | 16;
		nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, n );
		// nicht (4 und 16) oder 1 oder 32
		if ( nEdges && (((nEdges & n) != n) || (nEdges & 33)) )
			return sal_True;		// linke oder rechte Kante fehlt oder offen
	}
	else
	{	// linke Spalte
		nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, 4 );
		// nicht 4 oder 1 oder 32
		if ( nEdges && (((nEdges & 4) != 4) || (nEdges & 33)) )
			return sal_True;		// linke Kante fehlt oder offen
		// rechte Spalte
		nEdges = aCol[nCol2].GetBlockMatrixEdges( nRow1, nRow2, 16 );
		// nicht 16 oder 1 oder 32
		if ( nEdges && (((nEdges & 16) != 16) || (nEdges & 33)) )
			return sal_True;		// rechte Kante fehlt oder offen
	}

	if ( nRow1 == nRow2 )
	{	// obere und untere Zeile
		sal_Bool bOpen = sal_False;
		const sal_uInt16 n = 2 | 8;
		for ( SCCOL i=nCol1; i<=nCol2; i++)
		{
			nEdges = aCol[i].GetBlockMatrixEdges( nRow1, nRow1, n );
			if ( nEdges )
			{
				if ( (nEdges & n) != n )
					return sal_True;		// obere oder untere Kante fehlt
				if ( nEdges & 4 )
					bOpen = sal_True;		// linke Kante oeffnet, weitersehen
				else if ( !bOpen )
					return sal_True;		// es gibt was, was nicht geoeffnet wurde
				if ( nEdges & 16 )
					bOpen = sal_False;		// rechte Kante schliesst
			}
		}
		if ( bOpen )
			return sal_True;				// es geht noch weiter
	}
	else
	{
		sal_uInt16 j, n;
        SCROW nR;
		// erst obere Zeile, dann untere Zeile
		for ( j=0, nR=nRow1, n=8; j<2; j++, nR=nRow2, n=2 )
		{
			sal_Bool bOpen = sal_False;
			for ( SCCOL i=nCol1; i<=nCol2; i++)
			{
				nEdges = aCol[i].GetBlockMatrixEdges( nR, nR, n );
				if ( nEdges )
				{
					// in oberere Zeile keine obere Kante bzw.
					// in unterer Zeile keine untere Kante
					if ( (nEdges & n) != n )
						return sal_True;
					if ( nEdges & 4 )
						bOpen = sal_True;		// linke Kante oeffnet, weitersehen
					else if ( !bOpen )
						return sal_True;		// es gibt was, was nicht geoeffnet wurde
					if ( nEdges & 16 )
						bOpen = sal_False;		// rechte Kante schliesst
				}
			}
			if ( bOpen )
				return sal_True;				// es geht noch weiter
		}
	}
	return sal_False;
}


sal_Bool ScTable::HasSelectionMatrixFragment( const ScMarkData& rMark ) const
{
	sal_Bool bFound=sal_False;
	for (SCCOL i=0; i<=MAXCOL && !bFound; i++)
		bFound |= aCol[i].HasSelectionMatrixFragment(rMark);
	return bFound;
}


sal_Bool ScTable::IsBlockEditable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
			SCROW nRow2, sal_Bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    if ( !ValidColRow( nCol2, nRow2 ) )
    {
        DBG_ERRORFILE("IsBlockEditable: invalid column or row");
        if (pOnlyNotBecauseOfMatrix)
            *pOnlyNotBecauseOfMatrix = sal_False;
        return sal_False;
    }

	sal_Bool bIsEditable = sal_True;
	if ( nLockCount )
		bIsEditable = sal_False;
    else if ( IsProtected() && !pDocument->IsScenario(nTab) )
    {
        if((bIsEditable = !HasAttrib( nCol1, nRow1, nCol2, nRow2, HASATTR_PROTECTED )) != sal_False)
        {
            // If Sheet is protected and cells are not protected then
            // check the active scenario protect flag if this range is
            // on the active scenario range. Note the 'copy back' must also
            // be set to apply protection.
            sal_uInt16 nScenTab = nTab+1;
            while(pDocument->IsScenario(nScenTab))
            {
                ScRange aEditRange(nCol1, nRow1, nScenTab, nCol2, nRow2, nScenTab);
                if(pDocument->IsActiveScenario(nScenTab) && pDocument->HasScenarioRange(nScenTab, aEditRange))
                {
                    sal_uInt16 nFlags;
                    pDocument->GetScenarioFlags(nScenTab,nFlags);
                    bIsEditable = !((nFlags & SC_SCENARIO_PROTECT) && (nFlags & SC_SCENARIO_TWOWAY));
                    break;
                }
                nScenTab++;
            }
        }
    }
    else if (pDocument->IsScenario(nTab))
    {
        // Determine if the preceding sheet is protected
        SCTAB nActualTab = nTab;
        do
        {
            nActualTab--;
        }
        while(pDocument->IsScenario(nActualTab));

        if(pDocument->IsTabProtected(nActualTab))
        {
            ScRange aEditRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
            if(pDocument->HasScenarioRange(nTab, aEditRange))
            {
                sal_uInt16 nFlags;
                pDocument->GetScenarioFlags(nTab,nFlags);
                bIsEditable = !(nFlags & SC_SCENARIO_PROTECT);
            }
        }
    }
	if ( bIsEditable )
	{
		if ( HasBlockMatrixFragment( nCol1, nRow1, nCol2, nRow2 ) )
		{
			bIsEditable = sal_False;
			if ( pOnlyNotBecauseOfMatrix )
				*pOnlyNotBecauseOfMatrix = sal_True;
		}
		else if ( pOnlyNotBecauseOfMatrix )
			*pOnlyNotBecauseOfMatrix = sal_False;
	}
	else if ( pOnlyNotBecauseOfMatrix )
		*pOnlyNotBecauseOfMatrix = sal_False;
	return bIsEditable;
}


sal_Bool ScTable::IsSelectionEditable( const ScMarkData& rMark,
			sal_Bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
	sal_Bool bIsEditable = sal_True;
	if ( nLockCount )
		bIsEditable = sal_False;
    else if ( IsProtected() && !pDocument->IsScenario(nTab) )
    {
        if((bIsEditable = !HasAttribSelection( rMark, HASATTR_PROTECTED )) != sal_False)
        {
            // If Sheet is protected and cells are not protected then
            // check the active scenario protect flag if this area is
            // in the active scenario range.
            ScRangeList aRanges;
            rMark.FillRangeListWithMarks( &aRanges, sal_False );
            sal_uLong nRangeCount = aRanges.Count();
            SCTAB nScenTab = nTab+1;
            while(pDocument->IsScenario(nScenTab) && bIsEditable)
            {
                if(pDocument->IsActiveScenario(nScenTab))
                {
                    for (sal_uLong i=0; i<nRangeCount && bIsEditable; i++)
                    {
                        ScRange aRange = *aRanges.GetObject(i);
                        if(pDocument->HasScenarioRange(nScenTab, aRange))
                        {
                            sal_uInt16 nFlags;
                            pDocument->GetScenarioFlags(nScenTab,nFlags);
                            bIsEditable = !((nFlags & SC_SCENARIO_PROTECT) && (nFlags & SC_SCENARIO_TWOWAY));
                        }
                    }
                }
                nScenTab++;
            }
        }
    }
    else if (pDocument->IsScenario(nTab))
    {
        // Determine if the preceding sheet is protected
        SCTAB nActualTab = nTab;
        do
        {
            nActualTab--;
        }
        while(pDocument->IsScenario(nActualTab));

        if(pDocument->IsTabProtected(nActualTab))
        {
            ScRangeList aRanges;
            rMark.FillRangeListWithMarks( &aRanges, sal_False );
            sal_uLong nRangeCount = aRanges.Count();
            for (sal_uLong i=0; i<nRangeCount && bIsEditable; i++)
            {
                ScRange aRange = *aRanges.GetObject(i);
                if(pDocument->HasScenarioRange(nTab, aRange))
                {
                    sal_uInt16 nFlags;
                    pDocument->GetScenarioFlags(nTab,nFlags);
                    bIsEditable = !(nFlags & SC_SCENARIO_PROTECT);
                }
            }
        }
    }
	if ( bIsEditable )
	{
		if ( HasSelectionMatrixFragment( rMark ) )
		{
			bIsEditable = sal_False;
			if ( pOnlyNotBecauseOfMatrix )
				*pOnlyNotBecauseOfMatrix = sal_True;
		}
		else if ( pOnlyNotBecauseOfMatrix )
			*pOnlyNotBecauseOfMatrix = sal_False;
	}
	else if ( pOnlyNotBecauseOfMatrix )
		*pOnlyNotBecauseOfMatrix = sal_False;
	return bIsEditable;
}



void ScTable::LockTable()
{
	++nLockCount;
}


void ScTable::UnlockTable()
{
	if (nLockCount)
		--nLockCount;
	else
	{
		DBG_ERROR("UnlockTable ohne LockTable");
	}
}


void ScTable::MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, sal_Bool bDeep ) const
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].MergeSelectionPattern( rState, rMark, bDeep );
}


void ScTable::MergePatternArea( ScMergePatternState& rState, SCCOL nCol1, SCROW nRow1,
													SCCOL nCol2, SCROW nRow2, sal_Bool bDeep ) const
{
	for (SCCOL i=nCol1; i<=nCol2; i++)
		aCol[i].MergePatternArea( rState, nRow1, nRow2, bDeep );
}


void ScTable::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
					SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const
{
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
	{
		PutInOrder(nStartCol, nEndCol);
		PutInOrder(nStartRow, nEndRow);
		for (SCCOL i=nStartCol; i<=nEndCol; i++)
			aCol[i].MergeBlockFrame( pLineOuter, pLineInner, rFlags,
									nStartRow, nEndRow, (i==nStartCol), nEndCol-i );
	}
}


void ScTable::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
					SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
	{
		PutInOrder(nStartCol, nEndCol);
		PutInOrder(nStartRow, nEndRow);
		for (SCCOL i=nStartCol; i<=nEndCol; i++)
			aCol[i].ApplyBlockFrame( pLineOuter, pLineInner,
									nStartRow, nEndRow, (i==nStartCol), nEndCol-i );
	}
}


void ScTable::ApplyPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].ApplyPattern( nRow, rAttr );
}


void ScTable::ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
									 const ScPatternAttr& rAttr )
{
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
	{
		PutInOrder(nStartCol, nEndCol);
		PutInOrder(nStartRow, nEndRow);
		for (SCCOL i = nStartCol; i <= nEndCol; i++)
			aCol[i].ApplyPatternArea(nStartRow, nEndRow, rAttr);
	}
}

void ScTable::ApplyPooledPatternArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
									 const ScPatternAttr& rPooledAttr, const ScPatternAttr& rAttr )
{
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
	{
		PutInOrder(nStartCol, nEndCol);
		PutInOrder(nStartRow, nEndRow);
		for (SCCOL i = nStartCol; i <= nEndCol; i++)
		{
			sal_Bool bSet = sal_True;
			SCROW nStar, nEnd;
			const ScPatternAttr* pAttr = aCol[i].GetPatternRange(nStar, nEnd, nStartRow);
			if (nStar >nStartRow || nEnd < nEndRow || pAttr!=pDocument->GetDefPattern())
				bSet = sal_False; 

			if (bSet) 
				aCol[i].SetPatternArea(nStartRow, nEndRow, rPooledAttr);				
			else		
				aCol[i].ApplyPatternArea(nStartRow, nEndRow, rAttr);
		}
	}
}

void ScTable::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
		const ScPatternAttr& rPattern, short nNewType )
{
	SCCOL nEndCol = rRange.aEnd.Col();
	for ( SCCOL nCol = rRange.aStart.Col(); nCol <= nEndCol; nCol++ )
	{
		aCol[nCol].ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
	}
}



void ScTable::ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet& rStyle )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].ApplyStyle( nRow, rStyle );
}


void ScTable::ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScStyleSheet& rStyle )
{
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
	{
		PutInOrder(nStartCol, nEndCol);
		PutInOrder(nStartRow, nEndRow);
		for (SCCOL i = nStartCol; i <= nEndCol; i++)
			aCol[i].ApplyStyleArea(nStartRow, nEndRow, rStyle);
	}
}


void ScTable::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].ApplySelectionStyle( rStyle, rMark );
}


void ScTable::ApplySelectionLineStyle( const ScMarkData& rMark,
							const SvxBorderLine* pLine, sal_Bool bColorOnly )
{
	if ( bColorOnly && !pLine )
		return;

	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}


const ScStyleSheet* ScTable::GetStyle( SCCOL nCol, SCROW nRow ) const
{
	if (ValidColRow(nCol, nRow))
		return aCol[nCol].GetStyle(nRow);
	else
		return NULL;
}


const ScStyleSheet* ScTable::GetSelectionStyle( const ScMarkData& rMark, sal_Bool& rFound ) const
{
	rFound = sal_False;

	sal_Bool	bEqual = sal_True;
	sal_Bool	bColFound;

	const ScStyleSheet* pStyle = NULL;
	const ScStyleSheet* pNewStyle;

	for (SCCOL i=0; i<=MAXCOL && bEqual; i++)
		if (rMark.HasMultiMarks(i))
		{
			pNewStyle = aCol[i].GetSelectionStyle( rMark, bColFound );
			if (bColFound)
			{
				rFound = sal_True;
				if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
					bEqual = sal_False;												// unterschiedliche
				pStyle = pNewStyle;
			}
		}

	return bEqual ? pStyle : NULL;
}


const ScStyleSheet*	ScTable::GetAreaStyle( sal_Bool& rFound, SCCOL nCol1, SCROW nRow1,
													SCCOL nCol2, SCROW nRow2 ) const
{
	rFound = sal_False;

	sal_Bool	bEqual = sal_True;
	sal_Bool	bColFound;

	const ScStyleSheet* pStyle = NULL;
	const ScStyleSheet* pNewStyle;

	for (SCCOL i=nCol1; i<=nCol2 && bEqual; i++)
	{
		pNewStyle = aCol[i].GetAreaStyle(bColFound, nRow1, nRow2);
		if (bColFound)
		{
			rFound = sal_True;
			if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
				bEqual = sal_False;												// unterschiedliche
			pStyle = pNewStyle;
		}
	}

	return bEqual ? pStyle : NULL;
}


sal_Bool ScTable::IsStyleSheetUsed( const ScStyleSheet& rStyle, sal_Bool bGatherAllStyles ) const
{
	sal_Bool bIsUsed = sal_False;

	for ( SCCOL i=0; i<=MAXCOL; i++ )
    {
		if ( aCol[i].IsStyleSheetUsed( rStyle, bGatherAllStyles ) )
        {
            if ( !bGatherAllStyles )
                return sal_True;
            bIsUsed = sal_True;
        }
    }

	return bIsUsed;
}


void ScTable::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, sal_Bool bRemoved,
								OutputDevice* pDev,
								double nPPTX, double nPPTY,
								const Fraction& rZoomX, const Fraction& rZoomY )
{
    ScFlatBoolRowSegments aUsedRows;
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].FindStyleSheet(pStyleSheet, aUsedRows, bRemoved);

    SCROW nRow = 0;
    while (nRow <= MAXROW)
    {
        ScFlatBoolRowSegments::RangeData aData;
        if (!aUsedRows.getRangeData(nRow, aData))
            // search failed!
            return;

        SCROW nEndRow = aData.mnRow2;
        if (aData.mbValue)
            SetOptimalHeight(nRow, nEndRow, 0, pDev, nPPTX, nPPTY, rZoomX, rZoomY, sal_False);

        nRow = nEndRow + 1;
    }
}


sal_Bool ScTable::ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
									sal_Int16 nFlags )
{
	sal_Bool bChanged = sal_False;
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
		for (SCCOL i = nStartCol; i <= nEndCol; i++)
			bChanged |= aCol[i].ApplyFlags(nStartRow, nEndRow, nFlags);
	return bChanged;
}


sal_Bool ScTable::RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
									sal_Int16 nFlags )
{
	sal_Bool bChanged = sal_False;
	if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
		for (SCCOL i = nStartCol; i <= nEndCol; i++)
			bChanged |= aCol[i].RemoveFlags(nStartRow, nEndRow, nFlags);
	return bChanged;
}


void ScTable::SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr, sal_Bool bPutToPool )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].SetPattern( nRow, rAttr, bPutToPool );
}


void ScTable::ApplyAttr( SCCOL nCol, SCROW nRow, const SfxPoolItem& rAttr )
{
	if (ValidColRow(nCol,nRow))
		aCol[nCol].ApplyAttr( nRow, rAttr );
}


void ScTable::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark )
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].ApplySelectionCache( pCache, rMark );
}


void ScTable::ChangeSelectionIndent( sal_Bool bIncrement, const ScMarkData& rMark )
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].ChangeSelectionIndent( bIncrement, rMark );
}


void ScTable::ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark )
{
	for (SCCOL i=0; i<=MAXCOL; i++)
		aCol[i].ClearSelectionItems( pWhich, rMark );
}


//	Spaltenbreiten / Zeilenhoehen

void ScTable::SetColWidth( SCCOL nCol, sal_uInt16 nNewWidth )
{
	if (VALIDCOL(nCol) && pColWidth)
	{
		if (!nNewWidth)
		{
//			DBG_ERROR("Spaltenbreite 0 in SetColWidth");
			nNewWidth = STD_COL_WIDTH;
		}

		if ( nNewWidth != pColWidth[nCol] )
		{
			IncRecalcLevel();
            InitializeNoteCaptions();
			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
			if (pDrawLayer)
				pDrawLayer->WidthChanged( nTab, nCol, ((long) nNewWidth) - (long) pColWidth[nCol] );
			pColWidth[nCol] = nNewWidth;
			DecRecalcLevel();

            InvalidatePageBreaks();
		}
	}
	else
	{
		DBG_ERROR("Falsche Spaltennummer oder keine Breiten");
	}
}


void ScTable::SetRowHeight( SCROW nRow, sal_uInt16 nNewHeight )
{
    if (VALIDROW(nRow) && mpRowHeights)
	{
		if (!nNewHeight)
		{
			DBG_ERROR("Zeilenhoehe 0 in SetRowHeight");
			nNewHeight = ScGlobal::nStdRowHeight;
		}

        sal_uInt16 nOldHeight = mpRowHeights->getValue(nRow);
		if ( nNewHeight != nOldHeight )
		{
			IncRecalcLevel();
            InitializeNoteCaptions();
			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
			if (pDrawLayer)
				pDrawLayer->HeightChanged( nTab, nRow, ((long) nNewHeight) - (long) nOldHeight );
            mpRowHeights->setValue(nRow, nRow, nNewHeight);
			DecRecalcLevel();

            InvalidatePageBreaks();
		}
	}
	else
	{
		DBG_ERROR("Falsche Zeilennummer oder keine Hoehen");
	}
}

namespace {

/** 
 * Check if the new pixel size is different from the old size between 
 * specified ranges. 
 */
bool lcl_pixelSizeChanged(
    ScFlatUInt16RowSegments& rRowHeights, SCROW nStartRow, SCROW nEndRow, 
    sal_uInt16 nNewHeight, double nPPTY)
{
    long nNewPix = static_cast<long>(nNewHeight * nPPTY);

    ScFlatUInt16RowSegments::ForwardIterator aFwdIter(rRowHeights);
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        sal_uInt16 nHeight;
        if (!aFwdIter.getValue(nRow, nHeight))
            break;

        if (nHeight != nNewHeight)
        {    
            bool bChanged = (nNewPix != static_cast<long>(nHeight * nPPTY));
            if (bChanged)
                return true;
        }

        // Skip ahead to the last position of the current range.
        nRow = aFwdIter.getLastPos();
    }
    return false;
}

}

sal_Bool ScTable::SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight,
                                    double /* nPPTX */, double nPPTY )
{
	sal_Bool bChanged = sal_False;
    if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && mpRowHeights)
	{
		IncRecalcLevel();
        InitializeNoteCaptions();
		if (!nNewHeight)
		{
			DBG_ERROR("Zeilenhoehe 0 in SetRowHeight");
			nNewHeight = ScGlobal::nStdRowHeight;
		}

        sal_Bool bSingle = sal_False;   // sal_True = process every row for its own
		ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
		if (pDrawLayer)
			if (pDrawLayer->HasObjectsInRows( nTab, nStartRow, nEndRow ))
				bSingle = sal_True;

		if (bSingle)
		{
            ScFlatUInt16RowSegments::RangeData aData;
            mpRowHeights->getRangeData(nStartRow, aData);
            if (nNewHeight == aData.mnValue && nEndRow <= aData.mnRow2)
                bSingle = sal_False;    // no difference in this range
		}
		if (bSingle)
		{
			if (nEndRow-nStartRow < 20)
            {
                if (!bChanged)
                    bChanged = lcl_pixelSizeChanged(*mpRowHeights, nStartRow, nEndRow, nNewHeight, nPPTY);

                /*  #i94028# #i94991# If drawing objects are involved, each row
                    has to be changed for its own, because each call to
                    ScDrawLayer::HeightChanged expects correct row heights
                    above passed row in the document. Cannot use array iterator
                    because array changes in every cycle. */
                if( pDrawLayer )
                {
                    for( SCROW nRow = nStartRow; nRow <= nEndRow ; ++nRow )
                    {
                        pDrawLayer->HeightChanged( nTab, nRow,
                             static_cast<long>(nNewHeight) - static_cast<long>(mpRowHeights->getValue(nRow)));
                        mpRowHeights->setValue(nRow, nRow, nNewHeight);
                    }
                }
                else
                    mpRowHeights->setValue(nStartRow, nEndRow, nNewHeight);
            }
			else
			{
				SCROW nMid = (nStartRow+nEndRow) / 2;
				if (SetRowHeightRange( nStartRow, nMid, nNewHeight, 1.0, 1.0 ))
					bChanged = sal_True;
				if (SetRowHeightRange( nMid+1, nEndRow, nNewHeight, 1.0, 1.0 ))
					bChanged = sal_True;
			}
		}
		else
		{
			if (pDrawLayer)
			{
                // #i115025# When comparing to nNewHeight for the whole range, the height
                // including hidden rows has to be used (same behavior as 3.2).
                unsigned long nOldHeights = mpRowHeights->getSumValue(nStartRow, nEndRow);
                // FIXME: should we test for overflows?
				long nHeightDif = (long) (unsigned long) nNewHeight *
                    (nEndRow - nStartRow + 1) - nOldHeights;
				pDrawLayer->HeightChanged( nTab, nEndRow, nHeightDif );
			}

            if (!bChanged)
                bChanged = lcl_pixelSizeChanged(*mpRowHeights, nStartRow, nEndRow, nNewHeight, nPPTY);

            mpRowHeights->setValue(nStartRow, nEndRow, nNewHeight);
		}
		DecRecalcLevel();

        if (bChanged)
            InvalidatePageBreaks();
	}
	else
	{
		DBG_ERROR("Falsche Zeilennummer oder keine Hoehen");
	}

	return bChanged;
}

void ScTable::SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight )
{
    if (!ValidRow(nStartRow) || !ValidRow(nEndRow) || !mpRowHeights)
        return;

    if (!nNewHeight)
        nNewHeight = ScGlobal::nStdRowHeight;

    mpRowHeights->setValue(nStartRow, nEndRow, nNewHeight);
}

void ScTable::SetColWidthOnly( SCCOL nCol, sal_uInt16 nNewWidth )
{
	if (!VALIDCOL(nCol) || !pColWidth)
        return;

    if (!nNewWidth)
        nNewWidth = STD_COL_WIDTH;

    pColWidth[nCol] = nNewWidth;
}
void ScTable::SetManualHeight( SCROW nStartRow, SCROW nEndRow, sal_Bool bManual )
{
	if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && pRowFlags)
	{
		if (bManual)
            pRowFlags->OrValue( nStartRow, nEndRow, CR_MANUALSIZE);
		else
            pRowFlags->AndValue( nStartRow, nEndRow, sal::static_int_cast<sal_uInt8>(~CR_MANUALSIZE));
	}
	else
	{
		DBG_ERROR("Falsche Zeilennummer oder keine Zeilenflags");
	}
}


sal_uInt16 ScTable::GetColWidth( SCCOL nCol ) const
{
	DBG_ASSERT(VALIDCOL(nCol),"Falsche Spaltennummer");

	if (VALIDCOL(nCol) && pColFlags && pColWidth)
	{
        if (ColHidden(nCol))
			return 0;
		else
			return pColWidth[nCol];
	}
	else
		return (sal_uInt16) STD_COL_WIDTH;
}


sal_uInt16 ScTable::GetOriginalWidth( SCCOL nCol ) const		// immer die eingestellte
{
	DBG_ASSERT(VALIDCOL(nCol),"Falsche Spaltennummer");

	if (VALIDCOL(nCol) && pColWidth)
		return pColWidth[nCol];
	else
		return (sal_uInt16) STD_COL_WIDTH;
}


sal_uInt16 ScTable::GetCommonWidth( SCCOL nEndCol )
{
	//	get the width that is used in the largest continuous column range (up to nEndCol)

	if ( !ValidCol(nEndCol) )
	{
		DBG_ERROR("wrong column");
		nEndCol = MAXCOL;
	}

	sal_uInt16 nMaxWidth = 0;
	sal_uInt16 nMaxCount = 0;
	SCCOL nRangeStart = 0;
	while ( nRangeStart <= nEndCol )
	{
		//	skip hidden columns
		while ( nRangeStart <= nEndCol && ColHidden(nRangeStart) )
			++nRangeStart;
		if ( nRangeStart <= nEndCol )
		{
			sal_uInt16 nThisCount = 0;
			sal_uInt16 nThisWidth = pColWidth[nRangeStart];
			SCCOL nRangeEnd = nRangeStart;
			while ( nRangeEnd <= nEndCol && pColWidth[nRangeEnd] == nThisWidth )
			{
				++nThisCount;
				++nRangeEnd;

				//	skip hidden columns
				while ( nRangeEnd <= nEndCol && ColHidden(nRangeEnd) )
					++nRangeEnd;
			}

			if ( nThisCount > nMaxCount )
			{
				nMaxCount = nThisCount;
				nMaxWidth = nThisWidth;
			}

			nRangeStart = nRangeEnd;		// next range
		}
	}

	return nMaxWidth;
}


sal_uInt16 ScTable::GetRowHeight( SCROW nRow, SCROW* pStartRow, SCROW* pEndRow, bool bHiddenAsZero ) const
{
    DBG_ASSERT(VALIDROW(nRow),"Invalid row number");

    if (VALIDROW(nRow) && mpRowHeights)
    {
        if (bHiddenAsZero && RowHidden( nRow, pStartRow, pEndRow))
            return 0;
        else
        {
            ScFlatUInt16RowSegments::RangeData aData;
            if (!mpRowHeights->getRangeData(nRow, aData))
            {
                if (pStartRow)
                    *pStartRow = nRow;
                if (pEndRow)
                    *pEndRow = nRow;
                // TODO: What should we return in case the search fails?
                return 0;
            }

            // If bHiddenAsZero, pStartRow and pEndRow were initialized to 
            // boundaries of a non-hidden segment. Assume that the previous and 
            // next segment are hidden then and limit the current height 
            // segment.
            if (pStartRow)
                *pStartRow = (bHiddenAsZero ? std::max( *pStartRow, aData.mnRow1) : aData.mnRow1);
            if (pEndRow)
                *pEndRow = (bHiddenAsZero ? std::min( *pEndRow, aData.mnRow2) : aData.mnRow2);
            return aData.mnValue;
        }
    }
    else
    {
        if (pStartRow)
            *pStartRow = nRow;
        if (pEndRow)
            *pEndRow = nRow;
        return (sal_uInt16) ScGlobal::nStdRowHeight;
    }
}


sal_uLong ScTable::GetRowHeight( SCROW nStartRow, SCROW nEndRow ) const
{
	DBG_ASSERT(VALIDROW(nStartRow) && VALIDROW(nEndRow),"Falsche Zeilennummer");

	if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && mpRowHeights)
	{
        sal_uLong nHeight = 0;
        SCROW nRow = nStartRow;
        while (nRow <= nEndRow)
        {
            SCROW nLastRow = -1;
            if (!RowHidden(nRow, nLastRow))
            {
                if (nLastRow > nEndRow)
                    nLastRow = nEndRow;
                nHeight += mpRowHeights->getSumValue(nRow, nLastRow);
            }
            nRow = nLastRow + 1;
        }
        return nHeight;
	}
	else
		return (sal_uLong) ((nEndRow - nStartRow + 1) * ScGlobal::nStdRowHeight);
}


sal_uLong ScTable::GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, double fScale ) const
{
	DBG_ASSERT(VALIDROW(nStartRow) && VALIDROW(nEndRow),"Falsche Zeilennummer");

	if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && mpRowHeights)
	{
        sal_uLong nHeight = 0;
        SCROW nRow = nStartRow;
        while (nRow <= nEndRow)
        {
            SCROW nLastRow = -1;
            if (!RowHidden(nRow, nLastRow))
            {
                if (nLastRow > nEndRow)
                    nLastRow = nEndRow;

                // #i117315# can't use getSumValue, because individual values must be rounded
                while (nRow <= nLastRow)
                {
                    ScFlatUInt16RowSegments::RangeData aData;
                    if (!mpRowHeights->getRangeData(nRow, aData))
                        return nHeight;   // shouldn't happen

                    SCROW nSegmentEnd = std::min( nLastRow, aData.mnRow2 );

                    // round-down a single height value, multiply resulting (pixel) values
                    sal_uLong nOneHeight = static_cast<sal_uLong>( aData.mnValue * fScale );
                    nHeight += nOneHeight * ( nSegmentEnd + 1 - nRow );

                    nRow = nSegmentEnd + 1;
                }
            }
            nRow = nLastRow + 1;
        }
        return nHeight;
	}
	else
		return (sal_uLong) ((nEndRow - nStartRow + 1) * ScGlobal::nStdRowHeight * fScale);
}


sal_uInt16 ScTable::GetOriginalHeight( SCROW nRow ) const		// non-0 even if hidden
{
	DBG_ASSERT(VALIDROW(nRow),"wrong row number");

    if (VALIDROW(nRow) && mpRowHeights)
        return mpRowHeights->getValue(nRow);
	else
		return (sal_uInt16) ScGlobal::nStdRowHeight;
}


//	Spalten-/Zeilen-Flags


SCROW ScTable::GetHiddenRowCount( SCROW nRow )
{
    if (!ValidRow(nRow))
        return 0;

    SCROW nLastRow = -1;
    if (!RowHidden(nRow, nLastRow) || !ValidRow(nLastRow))
        return 0;

    return nLastRow - nRow + 1;
}


//!		ShowRows / DBShowRows zusammenfassen

void ScTable::ShowCol(SCCOL nCol, bool bShow)
{
	if (VALIDCOL(nCol))
	{
        bool bWasVis = !ColHidden(nCol);
		if (bWasVis != bShow)
		{
			IncRecalcLevel();
            InitializeNoteCaptions();
			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
			if (pDrawLayer)
			{
				if (bShow)
					pDrawLayer->WidthChanged( nTab, nCol, (long) pColWidth[nCol] );
				else
					pDrawLayer->WidthChanged( nTab, nCol, -(long) pColWidth[nCol] );
			}

            SetColHidden(nCol, nCol, !bShow);
	    DecRecalcLevel();

			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
			if ( pCharts )
				pCharts->SetRangeDirty(ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ));
		}
	}
	else
	{
		DBG_ERROR("Falsche Spaltennummer oder keine Flags");
	}
}


void ScTable::ShowRow(SCROW nRow, bool bShow)
{
	if (VALIDROW(nRow) && pRowFlags)
	{
        bool bWasVis = !RowHidden(nRow);
		if (bWasVis != bShow)
		{
			IncRecalcLevel();
            InitializeNoteCaptions();
			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
			if (pDrawLayer)
			{
				if (bShow)
                    pDrawLayer->HeightChanged(
                        nTab, nRow, static_cast<long>(mpRowHeights->getValue(nRow)));
				else
                    pDrawLayer->HeightChanged(
                        nTab, nRow, -static_cast<long>(mpRowHeights->getValue(nRow)));
			}

            SetRowHidden(nRow, nRow, !bShow);
			if (bShow)
                SetRowFiltered(nRow, nRow, false);
		DecRecalcLevel();

			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
			if ( pCharts )
				pCharts->SetRangeDirty(ScRange( 0, nRow, nTab, MAXCOL, nRow, nTab ));

            InvalidatePageBreaks();
		}
	}
	else
	{
		DBG_ERROR("Falsche Zeilennummer oder keine Flags");
	}
}


void ScTable::DBShowRow(SCROW nRow, bool bShow)
{
	if (VALIDROW(nRow) && pRowFlags)
	{
        bool bWasVis = !RowHidden(nRow);
		IncRecalcLevel();
        InitializeNoteCaptions();
		if (bWasVis != bShow)
		{
			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
			if (pDrawLayer)
			{
				if (bShow)
                    pDrawLayer->HeightChanged(
                        nTab, nRow, static_cast<long>(mpRowHeights->getValue(nRow)));
				else
                    pDrawLayer->HeightChanged(
                        nTab, nRow, -static_cast<long>(mpRowHeights->getValue(nRow)));
			}
		}

		//	Filter-Flag immer setzen, auch wenn Hidden unveraendert
        SetRowHidden(nRow, nRow, !bShow);
        SetRowFiltered(nRow, nRow, !bShow);
	DecRecalcLevel();

		if (bWasVis != bShow)
		{
			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
			if ( pCharts )
				pCharts->SetRangeDirty(ScRange( 0, nRow, nTab, MAXCOL, nRow, nTab ));

			if (pOutlineTable)
				UpdateOutlineRow( nRow, nRow, bShow );

            InvalidatePageBreaks();
		}
	}
	else
	{
		DBG_ERROR("Falsche Zeilennummer oder keine Flags");
	}
}


void ScTable::DBShowRows(SCROW nRow1, SCROW nRow2, bool bShow, bool bSetFlags)
{
    // #i116164# IncRecalcLevel/DecRecalcLevel is in ScTable::Query
	SCROW nStartRow = nRow1;
    InitializeNoteCaptions();
	while (nStartRow <= nRow2)
	{
        SCROW nEndRow = -1;
        bool bWasVis = !RowHidden(nStartRow, nEndRow);
        if (nEndRow > nRow2)
            nEndRow = nRow2;

		sal_Bool bChanged = ( bWasVis != bShow );
		if ( bChanged && bSetFlags )
		{
			ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
			if (pDrawLayer)
			{
                long nHeight = static_cast<long>(mpRowHeights->getSumValue(nStartRow, nEndRow));
				if (bShow)
					pDrawLayer->HeightChanged( nTab, nStartRow, nHeight );
				else
					pDrawLayer->HeightChanged( nTab, nStartRow, -nHeight );
			}
		}

        // #i116164# Directly modify the flags only if there are drawing objects within the area.
        // Otherwise, all modifications are made together in ScTable::Query, so the tree isn't constantly rebuilt.
        if ( bSetFlags )
        {
            SetRowHidden(nStartRow, nEndRow, !bShow);
            SetRowFiltered(nStartRow, nEndRow, !bShow);
        }

		if ( bChanged )
		{
			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
			if ( pCharts )
				pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));
		}

		nStartRow = nEndRow + 1;
	}

	//	#i12341# For Show/Hide rows, the outlines are updated separately from the outside.
	//	For filtering, the changes aren't visible to the caller, so UpdateOutlineRow has
	//	to be done here.
	if (pOutlineTable)
		UpdateOutlineRow( nRow1, nRow2, bShow );
}


void ScTable::ShowRows(SCROW nRow1, SCROW nRow2, bool bShow)
{
	SCROW nStartRow = nRow1;
	IncRecalcLevel();
    InitializeNoteCaptions();

    // #i116164# if there are no drawing objects within the row range, a single HeightChanged call is enough
    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    bool bHasObjects = pDrawLayer && pDrawLayer->HasObjectsInRows( nTab, nRow1, nRow2, false );
    long nOldHeight = 0;
    if ( pDrawLayer && !bHasObjects )
        nOldHeight = static_cast<long>(GetRowHeight(nRow1, nRow2));

	while (nStartRow <= nRow2)
	{
        SCROW nEndRow = -1;
        bool bWasVis = !RowHidden(nStartRow, nEndRow);
        if (nEndRow > nRow2)
            nEndRow = nRow2;

		sal_Bool bChanged = ( bWasVis != bShow );
        if ( bChanged && bHasObjects )
		{
			if (pDrawLayer)
			{
                long nHeight = static_cast<long>(mpRowHeights->getSumValue(nStartRow, nEndRow));
				if (bShow)
					pDrawLayer->HeightChanged( nTab, nStartRow, nHeight );
				else
					pDrawLayer->HeightChanged( nTab, nStartRow, -nHeight );
			}
		}

        // #i116164# Directly modify the flags only if there are drawing objects within the area.
        // Otherwise, all rows are modified together after the loop, so the tree isn't constantly rebuilt.
        if ( bHasObjects )
        {
            SetRowHidden(nStartRow, nEndRow, !bShow);
            if (bShow)
                SetRowFiltered(nStartRow, nEndRow, false);
        }

		if ( bChanged )
		{
			ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
			if ( pCharts )
				pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));

            InvalidatePageBreaks();
		}

		nStartRow = nEndRow + 1;
	}

    if ( !bHasObjects )
    {
        // #i116164# set the flags for the whole range at once
        SetRowHidden(nRow1, nRow2, !bShow);
        if (bShow)
            SetRowFiltered(nRow1, nRow2, false);

        if ( pDrawLayer )
        {
            // if there are no objects in the range, a single HeightChanged call is enough
            long nNewHeight = 0;
            if ( bShow )
                nNewHeight = static_cast<long>(GetRowHeight(nRow1, nRow2));
            if ( nNewHeight != nOldHeight )
                pDrawLayer->HeightChanged( nTab, nRow1, nNewHeight - nOldHeight );
        }
    }

	DecRecalcLevel();
}

sal_Bool ScTable::IsDataFiltered() const
{
	sal_Bool bAnyQuery = sal_False;
	ScDBData* pDBData = pDocument->GetFilterDBAtTable(nTab);
	if ( pDBData )
	{
		ScQueryParam aParam;
		pDBData->GetQueryParam( aParam );
		if ( aParam.GetEntry(0).bDoQuery )
			bAnyQuery = sal_True;
	}
	return bAnyQuery;
}

void ScTable::SetColFlags( SCCOL nCol, sal_uInt8 nNewFlags )
{
	if (VALIDCOL(nCol) && pColFlags)
		pColFlags[nCol] = nNewFlags;
	else
	{
		DBG_ERROR("Falsche Spaltennummer oder keine Flags");
	}
}


void ScTable::SetRowFlags( SCROW nRow, sal_uInt8 nNewFlags )
{
	if (VALIDROW(nRow) && pRowFlags)
		pRowFlags->SetValue( nRow, nNewFlags);
	else
	{
		DBG_ERROR("Falsche Zeilennummer oder keine Flags");
	}
}


void ScTable::SetRowFlags( SCROW nStartRow, SCROW nEndRow, sal_uInt8 nNewFlags )
{
	if (VALIDROW(nStartRow) && VALIDROW(nEndRow) && pRowFlags)
		pRowFlags->SetValue( nStartRow, nEndRow, nNewFlags);
	else
	{
		DBG_ERROR("Falsche Zeilennummer(n) oder keine Flags");
	}
}


sal_uInt8 ScTable::GetColFlags( SCCOL nCol ) const
{
	if (VALIDCOL(nCol) && pColFlags)
		return pColFlags[nCol];
	else
		return 0;
}


sal_uInt8 ScTable::GetRowFlags( SCROW nRow ) const
{
	if (VALIDROW(nRow) && pRowFlags)
		return pRowFlags->GetValue(nRow);
	else
		return 0;
}


SCROW ScTable::GetLastFlaggedRow() const
{
    SCROW nLastFound = 0;
    if (pRowFlags)
    {    
        SCROW nRow = pRowFlags->GetLastAnyBitAccess( 0, sal::static_int_cast<sal_uInt8>(CR_ALL) );
        if (ValidRow(nRow))
            nLastFound = nRow;
    }

    if (!maRowManualBreaks.empty())
        nLastFound = ::std::max(nLastFound, *maRowManualBreaks.rbegin());

    if (mpHiddenRows)
    {
        SCROW nRow = mpHiddenRows->findLastNotOf(false);
        if (ValidRow(nRow))
            nLastFound = ::std::max(nLastFound, nRow);
    }

    if (mpFilteredRows)
    {
        SCROW nRow = mpFilteredRows->findLastNotOf(false);
        if (ValidRow(nRow))
            nLastFound = ::std::max(nLastFound, nRow);
    }

    return nLastFound;
}


SCCOL ScTable::GetLastChangedCol() const
{
	if ( !pColFlags )
		return 0;

	SCCOL nLastFound = 0;
	for (SCCOL nCol = 1; nCol <= MAXCOL; nCol++)
        if ((pColFlags[nCol] & CR_ALL) || (pColWidth[nCol] != STD_COL_WIDTH))
			nLastFound = nCol;

	return nLastFound;
}


SCROW ScTable::GetLastChangedRow() const
{
	if ( !pRowFlags )
		return 0;

    SCROW nLastFlags = GetLastFlaggedRow();

    // Find the last row position where the height is NOT the standard row
    // height.
    // KOHEI: Test this to make sure it does what it's supposed to.
    SCROW nLastHeight = mpRowHeights->findLastNotOf(ScGlobal::nStdRowHeight);
    if (!ValidRow(nLastHeight))
        nLastHeight = 0;

	return std::max( nLastFlags, nLastHeight);
}


sal_Bool ScTable::UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, sal_Bool bShow )
{
	if (pOutlineTable && pColFlags)
    {
        ScBitMaskCompressedArray< SCCOLROW, sal_uInt8> aArray( MAXCOL, pColFlags, MAXCOLCOUNT);
		return pOutlineTable->GetColArray()->ManualAction( nStartCol, nEndCol, bShow, *this, true );
    }
	else
		return sal_False;
}


sal_Bool ScTable::UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, sal_Bool bShow )
{
	if (pOutlineTable && pRowFlags)
		return pOutlineTable->GetRowArray()->ManualAction( nStartRow, nEndRow, bShow, *this, false );
	else
		return sal_False;
}


void ScTable::ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 )
{
    // Column-wise expansion

    while (rX1 > 0 && ColHidden(rX1-1))
        --rX1;

    while (rX2 < MAXCOL && ColHidden(rX2+1))
        ++rX2;

    // Row-wise expansion

    if (rY1 > 0)
	{
        ScFlatBoolRowSegments::RangeData aData;
        if (mpHiddenRows->getRangeData(rY1-1, aData) && aData.mbValue)
        {
            SCROW nStartRow = aData.mnRow1;
            if (ValidRow(nStartRow))
                rY1 = nStartRow;
        }
    }
    if (rY2 < MAXROW)
    {
        SCROW nEndRow = -1;
        if (RowHidden(rY2+1, nEndRow) && ValidRow(nEndRow))
            rY2 = nEndRow;
	}
}


void ScTable::StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 )
{
    while ( rX2>rX1 && ColHidden(rX2) )
        --rX2;
    while ( rX2>rX1 && ColHidden(rX1) )
        ++rX1;

    if (rY1 < rY2)
	{
        ScFlatBoolRowSegments::RangeData aData;
        if (mpHiddenRows->getRangeData(rY2, aData) && aData.mbValue)
        {
            SCROW nStartRow = aData.mnRow1;
            if (ValidRow(nStartRow) && nStartRow >= rY1)
                rY2 = nStartRow;
        }
    }

    if (rY1 < rY2)
    {
        SCROW nEndRow = -1;
        if (RowHidden(rY1, nEndRow) && ValidRow(nEndRow) && nEndRow <= rY2)
            rY1 = nEndRow;
	}
}


//	Auto-Outline

template< typename T >
short DiffSign( T a, T b )
{
	return (a<b) ? -1 :
			(a>b) ? 1 : 0;
}


void ScTable::DoAutoOutline( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
	sal_Bool bSizeChanged = sal_False;
	sal_Bool bMissed	  = sal_False;

	SCCOL nCol;
	SCROW nRow;
	SCROW i;
	sal_Bool bFound;
	ScOutlineArray* pArray;
	ScBaseCell* pCell;
	ScRange aRef;
/*	ScPatternAttr aBoldPattern( pDocument->GetPool() );				//!	spezielle Format-Vorlage
	aBoldPattern.GetItemSet().Put( SvxWeightItem( WEIGHT_BOLD ) );
*/

	StartOutlineTable();

							// Zeilen

	SCROW	nCount = nEndRow-nStartRow+1;
	sal_Bool*	pUsed = new sal_Bool[nCount];
	for (i=0; i<nCount; i++)
		pUsed[i] = sal_False;
	for (nCol=nStartCol; nCol<=nEndCol; nCol++)
		if (!aCol[nCol].IsEmptyData())
			aCol[nCol].FindUsed( nStartRow, nEndRow, pUsed );

	pArray = pOutlineTable->GetRowArray();
	for (nRow=nStartRow; nRow<=nEndRow; nRow++)
		if (pUsed[nRow-nStartRow])
		{
			bFound = sal_False;
			for (nCol=nStartCol; nCol<=nEndCol && !bFound; nCol++)
				if (!aCol[nCol].IsEmptyData())
				{
					pCell = aCol[nCol].GetCell( nRow );
					if (pCell)
						if ( pCell->GetCellType() == CELLTYPE_FORMULA )
							if (((ScFormulaCell*)pCell)->HasRefListExpressibleAsOneReference( aRef ))
								if ( aRef.aStart.Col() == nCol && aRef.aEnd.Col() == nCol &&
									 aRef.aStart.Tab() == nTab && aRef.aEnd.Tab() == nTab &&
									 DiffSign( aRef.aStart.Row(), nRow ) ==
										DiffSign( aRef.aEnd.Row(), nRow ) )
								{
									if (pArray->Insert( aRef.aStart.Row(), aRef.aEnd.Row(), bSizeChanged ))
									{
//										ApplyPatternArea( nStartCol, nRow, nEndCol, nRow, aBoldPattern );
										bFound = sal_True;
									}
									else
										bMissed = sal_True;
								}
				}
		}

	delete[] pUsed;

							// Spalten

	pArray = pOutlineTable->GetColArray();
	for (nCol=nStartCol; nCol<=nEndCol; nCol++)
	{
		if (!aCol[nCol].IsEmptyData())
		{
			bFound = sal_False;
			ScColumnIterator aIter( &aCol[nCol], nStartRow, nEndRow );
			while ( aIter.Next( nRow, pCell ) && !bFound )
			{
				if ( pCell->GetCellType() == CELLTYPE_FORMULA )
					if (((ScFormulaCell*)pCell)->HasRefListExpressibleAsOneReference( aRef ))
						if ( aRef.aStart.Row() == nRow && aRef.aEnd.Row() == nRow &&
							 aRef.aStart.Tab() == nTab && aRef.aEnd.Tab() == nTab &&
							 DiffSign( aRef.aStart.Col(), nCol ) ==
								DiffSign( aRef.aEnd.Col(), nCol ) )
						{
							if (pArray->Insert( aRef.aStart.Col(), aRef.aEnd.Col(), bSizeChanged ))
							{
//								ApplyPatternArea( nCol, nStartRow, nCol, nEndRow, aBoldPattern );
								bFound = sal_True;
							}
							else
								bMissed = sal_True;
						}
			}
		}
	}
}

									//	CopyData - fuer Query in anderen Bereich

void ScTable::CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
							SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab )
{
	//!		wenn fuer mehrere Zeilen benutzt, nach Spalten optimieren!

	ScAddress aSrc( nStartCol, nStartRow, nTab );
	ScAddress aDest( nDestCol, nDestRow, nDestTab );
	ScRange aRange( aSrc, aDest );
	sal_Bool bThisTab = ( nDestTab == nTab );
	SCROW nDestY = nDestRow;
	for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
	{
		aSrc.SetRow( nRow );
		aDest.SetRow( nDestY );
		SCCOL nDestX = nDestCol;
		for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
		{
			aSrc.SetCol( nCol );
			aDest.SetCol( nDestX );
			ScBaseCell* pCell = GetCell( nCol, nRow );
			if (pCell)
			{
                pCell = pCell->CloneWithoutNote( *pDocument );
				if (pCell->GetCellType() == CELLTYPE_FORMULA)
				{
					((ScFormulaCell*)pCell)->UpdateReference( URM_COPY, aRange,
									((SCsCOL) nDestCol) - ((SCsCOL) nStartCol),
									((SCsROW) nDestRow) - ((SCsROW) nStartRow),
									((SCsTAB) nDestTab) - ((SCsTAB) nTab) );
					((ScFormulaCell*)pCell)->aPos = aDest;
				}
			}
			if (bThisTab)
			{
				PutCell( nDestX, nDestY, pCell );
				SetPattern( nDestX, nDestY, *GetPattern( nCol, nRow ), sal_True );
			}
			else
			{
				pDocument->PutCell( aDest, pCell );
				pDocument->SetPattern( aDest, *GetPattern( nCol, nRow ), sal_True );
			}

			++nDestX;
		}
		++nDestY;
	}
}


sal_Bool ScTable::RefVisible(ScFormulaCell* pCell)
{
	ScRange aRef;

	if (pCell->HasOneReference(aRef))
	{
		if (aRef.aStart.Col()==aRef.aEnd.Col() && aRef.aStart.Tab()==aRef.aEnd.Tab())
		{
            SCROW nEndRow;
            if (!RowFiltered(aRef.aStart.Row(), NULL, &nEndRow))
                // row not filtered.
                nEndRow = ::std::numeric_limits<SCROW>::max();

            if (!ValidRow(nEndRow) || nEndRow < aRef.aEnd.Row())
                return sal_True;    // at least partly visible
            return sal_False;       // completely invisible
		}
	}

	return sal_True;						// irgendwie anders
}


void ScTable::GetUpperCellString(SCCOL nCol, SCROW nRow, String& rStr)
{
	GetInputString(nCol, nRow, rStr);
	rStr.EraseTrailingChars();
	rStr.EraseLeadingChars();
	ScGlobal::pCharClass->toUpper(rStr);
}


// Berechnen der Groesse der Tabelle und setzen der Groesse an der DrawPage

void ScTable::SetDrawPageSize(bool bResetStreamValid, bool bUpdateNoteCaptionPos)
{
	ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
	if( pDrawLayer )
	{
        double fValX = GetColOffset( MAXCOL + 1 ) * HMM_PER_TWIPS;
        double fValY = GetRowOffset( MAXROW + 1 ) * HMM_PER_TWIPS;
        const long nMax = ::std::numeric_limits<long>::max();
        // #i113884# Avoid int32 overflow with possible negative results than can cause bad effects.
        // If the draw page size is smaller than all rows, only the bottom of the sheet is affected.
        long x = ( fValX > (double)nMax ) ? nMax : (long) fValX;
        long y = ( fValY > (double)nMax ) ? nMax : (long) fValY;

		if ( IsLayoutRTL() )		// IsNegativePage
			x = -x;

		pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( x, y ), bUpdateNoteCaptionPos );
	}

    // #i102616# actions that modify the draw page size count as sheet modification
    // (exception: InitDrawLayer)
    if (bResetStreamValid && IsStreamValid())
        SetStreamValid(sal_False);
}


sal_uLong ScTable::GetRowOffset( SCROW nRow ) const
{
	sal_uLong n = 0;
    if ( mpHiddenRows && mpRowHeights )
	{
        if (nRow == 0)
            return 0;
        else if (nRow == 1)
            return GetRowHeight(0);

        n = GetTotalRowHeight(0, nRow-1);
#ifdef DBG_UTIL
        if (n == ::std::numeric_limits<unsigned long>::max())
            DBG_ERRORFILE("ScTable::GetRowOffset: row heights overflow");
#endif
	}
	else
	{
		DBG_ERROR("GetRowOffset: Daten fehlen");
	}
	return n;
}

SCROW ScTable::GetRowForHeight(sal_uLong nHeight) const
{
    sal_uInt32 nSum = 0;

    ScFlatBoolRowSegments::RangeData aData;
    for (SCROW nRow = 0; nRow <= MAXROW; ++nRow)
    {
        if (!mpHiddenRows->getRangeData(nRow, aData))
            break;

        if (aData.mbValue)
        {
            nRow = aData.mnRow2;
            continue;
        }

        sal_uInt32 nNew = mpRowHeights->getValue(nRow);
        nSum += nNew;
        if (nSum > nHeight)
        {
            return nRow < MAXROW ? nRow + 1 : MAXROW;
        }
    }
    return -1;
}


sal_uLong ScTable::GetColOffset( SCCOL nCol ) const
{
	sal_uLong n = 0;
	if ( pColWidth )
	{
		SCCOL i;
		for( i = 0; i < nCol; i++ )
            if (!ColHidden(i))
				n += pColWidth[i];
	}
	else
	{
		DBG_ERROR("GetColumnOffset: Daten fehlen");
	}
	return n;
}

ScColumn* ScTable::GetColumnByIndex(sal_Int32 index)
{
	if( index <= MAXCOL && index >= 0 )
	{
		return &(aCol[index]);
	}
	return NULL;
}

