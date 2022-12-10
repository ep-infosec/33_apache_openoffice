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

#include <tools/debug.hxx>

#include "markdata.hxx"
#include "markarr.hxx"
#include "rangelst.hxx"

// STATIC DATA -----------------------------------------------------------

//------------------------------------------------------------------------

ScMarkData::ScMarkData() :
	pMultiSel( NULL )
{
	for (SCTAB i=0; i<=MAXTAB; i++)
		bTabMarked[i] = sal_False;

	ResetMark();
}

ScMarkData::ScMarkData(const ScMarkData& rData) :
	aMarkRange( rData.aMarkRange ),
	aMultiRange( rData.aMultiRange ),
	pMultiSel( NULL )
{
	bMarked		 = rData.bMarked;
	bMultiMarked = rData.bMultiMarked;
	bMarking	 = rData.bMarking;
	bMarkIsNeg	 = rData.bMarkIsNeg;

	for (SCTAB i=0; i<=MAXTAB; i++)
		bTabMarked[i] = rData.bTabMarked[i];

	if (rData.pMultiSel)
	{
		pMultiSel = new ScMarkArray[MAXCOLCOUNT];
		for (SCCOL j=0; j<MAXCOLCOUNT; j++)
			rData.pMultiSel[j].CopyMarksTo( pMultiSel[j] );
	}
}

ScMarkData&	ScMarkData::operator=(const ScMarkData& rData)
{
	if ( &rData == this )
		return *this;

	delete[] pMultiSel;
	pMultiSel = NULL;

	aMarkRange	 = rData.aMarkRange;
	aMultiRange  = rData.aMultiRange;
	bMarked		 = rData.bMarked;
	bMultiMarked = rData.bMultiMarked;
	bMarking	 = rData.bMarking;
	bMarkIsNeg	 = rData.bMarkIsNeg;

	for (SCTAB i=0; i<=MAXTAB; i++)
		bTabMarked[i] = rData.bTabMarked[i];

	if (rData.pMultiSel)
	{
		pMultiSel = new ScMarkArray[MAXCOLCOUNT];
		for (SCCOL j=0; j<MAXCOLCOUNT; j++)
			rData.pMultiSel[j].CopyMarksTo( pMultiSel[j] );
	}

	return *this;
}

ScMarkData::~ScMarkData()
{
	delete[] pMultiSel;
}

void ScMarkData::ResetMark()
{
	delete[] pMultiSel;
	pMultiSel = NULL;

	bMarked = bMultiMarked = sal_False;
	bMarking = bMarkIsNeg = sal_False;
}

void ScMarkData::SetMarkArea( const ScRange& rRange )
{
	aMarkRange = rRange;
	aMarkRange.Justify();
	if ( !bMarked )
	{
		// #77987# Upon creation of a document ScFormatShell GetTextAttrState
		// may query (default) attributes although no sheet is marked yet.
		// => mark that one.
		if ( !GetSelectCount() )
			bTabMarked[ aMarkRange.aStart.Tab() ] = sal_True;
		bMarked = sal_True;
	}
}

void ScMarkData::GetMarkArea( ScRange& rRange ) const
{
	rRange = aMarkRange;		//! inline ?
}

void ScMarkData::GetMultiMarkArea( ScRange& rRange ) const
{
	rRange = aMultiRange;
}

void ScMarkData::SetMultiMarkArea( const ScRange& rRange, sal_Bool bMark )
{
	if (!pMultiSel)
	{
		pMultiSel = new ScMarkArray[MAXCOL+1];

		// if simple mark range is set, copy to multi marks
		if ( bMarked && !bMarkIsNeg )
		{
			bMarked = sal_False;
			SetMultiMarkArea( aMarkRange, sal_True );
		}
	}

	SCCOL nStartCol = rRange.aStart.Col();
	SCROW nStartRow = rRange.aStart.Row();
	SCCOL nEndCol = rRange.aEnd.Col();
	SCROW nEndRow = rRange.aEnd.Row();
	PutInOrder( nStartRow, nEndRow );
	PutInOrder( nStartCol, nEndCol );

	SCCOL nCol;
	for (nCol=nStartCol; nCol<=nEndCol; nCol++)
		pMultiSel[nCol].SetMarkArea( nStartRow, nEndRow, bMark );

	if ( bMultiMarked )					// aMultiRange updaten
	{
		if ( nStartCol < aMultiRange.aStart.Col() )
			aMultiRange.aStart.SetCol( nStartCol );
		if ( nStartRow < aMultiRange.aStart.Row() )
			aMultiRange.aStart.SetRow( nStartRow );
		if ( nEndCol > aMultiRange.aEnd.Col() )
			aMultiRange.aEnd.SetCol( nEndCol );
		if ( nEndRow > aMultiRange.aEnd.Row() )
			aMultiRange.aEnd.SetRow( nEndRow );
	}
	else
	{
		aMultiRange = rRange;			// neu
		bMultiMarked = sal_True;
	}
}

void ScMarkData::SetAreaTab( SCTAB nTab )
{
	aMarkRange.aStart.SetTab(nTab);
	aMarkRange.aEnd.SetTab(nTab);
	aMultiRange.aStart.SetTab(nTab);
	aMultiRange.aEnd.SetTab(nTab);
}

void ScMarkData::SelectOneTable( SCTAB nTab )
{
	for (SCTAB i=0; i<=MAXTAB; i++)
		bTabMarked[i] = ( nTab == i );
}

SCTAB ScMarkData::GetSelectCount() const
{
	SCTAB nCount = 0;
	for (SCTAB i=0; i<=MAXTAB; i++)
		if (bTabMarked[i])
			++nCount;

	return nCount;
}

SCTAB ScMarkData::GetFirstSelected() const
{
	for (SCTAB i=0; i<=MAXTAB; i++)
		if (bTabMarked[i])
			return i;

	DBG_ERROR("GetFirstSelected: keine markiert");
	return 0;
}

void ScMarkData::MarkToMulti()
{
	if ( bMarked && !bMarking )
	{
		SetMultiMarkArea( aMarkRange, !bMarkIsNeg );
		bMarked = sal_False;

		//	check if all multi mark ranges have been removed
		if ( bMarkIsNeg && !HasAnyMultiMarks() )
			ResetMark();
	}
}

void ScMarkData::MarkToSimple()
{
	if ( bMarking )
		return;

	if ( bMultiMarked && bMarked )
		MarkToMulti();					// may result in bMarked and bMultiMarked reset

	if ( bMultiMarked )
	{
		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

		ScRange aNew = aMultiRange;

		sal_Bool bOk = sal_False;
		SCCOL nStartCol = aNew.aStart.Col();
		SCCOL nEndCol   = aNew.aEnd.Col();

		while ( nStartCol < nEndCol && !pMultiSel[nStartCol].HasMarks() )
			++nStartCol;
		while ( nStartCol < nEndCol && !pMultiSel[nEndCol].HasMarks() )
			--nEndCol;

		//	Zeilen werden nur aus MarkArray genommen
		SCROW nStartRow, nEndRow;
		if ( pMultiSel[nStartCol].HasOneMark( nStartRow, nEndRow ) )
		{
			bOk = sal_True;
			SCROW nCmpStart, nCmpEnd;
			for (SCCOL nCol=nStartCol+1; nCol<=nEndCol && bOk; nCol++)
				if ( !pMultiSel[nCol].HasOneMark( nCmpStart, nCmpEnd )
						|| nCmpStart != nStartRow || nCmpEnd != nEndRow )
					bOk = sal_False;
		}

		if (bOk)
		{
			aNew.aStart.SetCol(nStartCol);
			aNew.aStart.SetRow(nStartRow);
			aNew.aEnd.SetCol(nEndCol);
			aNew.aEnd.SetRow(nEndRow);

			ResetMark();
			aMarkRange = aNew;
			bMarked = sal_True;
			bMarkIsNeg = sal_False;
		}
	}
}

sal_Bool ScMarkData::IsCellMarked( SCCOL nCol, SCROW nRow, sal_Bool bNoSimple ) const
{
	if ( bMarked && !bNoSimple && !bMarkIsNeg )
		if ( aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
			 aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
			return sal_True;

	if (bMultiMarked)
	{
		//!	hier auf negative Markierung testen ?

		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
		return pMultiSel[nCol].GetMark( nRow );
	}

	return sal_False;
}

sal_Bool ScMarkData::IsColumnMarked( SCCOL nCol ) const
{
	//	bMarkIsNeg inzwischen auch fuer Spaltenkoepfe
	//!	GetMarkColumnRanges fuer komplett markierte Spalten

	if ( bMarked && !bMarkIsNeg &&
					aMarkRange.aStart.Col() <= nCol && aMarkRange.aEnd.Col() >= nCol &&
					aMarkRange.aStart.Row() == 0	&& aMarkRange.aEnd.Row() == MAXROW )
		return sal_True;

	if ( bMultiMarked && pMultiSel[nCol].IsAllMarked(0,MAXROW) )
		return sal_True;

	return sal_False;
}

sal_Bool ScMarkData::IsRowMarked( SCROW nRow ) const
{
	//	bMarkIsNeg inzwischen auch fuer Zeilenkoepfe
	//!	GetMarkRowRanges fuer komplett markierte Zeilen

	if ( bMarked && !bMarkIsNeg &&
					aMarkRange.aStart.Col() == 0	&& aMarkRange.aEnd.Col() == MAXCOL &&
					aMarkRange.aStart.Row() <= nRow && aMarkRange.aEnd.Row() >= nRow )
		return sal_True;

	if ( bMultiMarked )
	{
		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");
		for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
			if (!pMultiSel[nCol].GetMark(nRow))
				return sal_False;
		return sal_True;
	}

	return sal_False;
}

void ScMarkData::MarkFromRangeList( const ScRangeList& rList, sal_Bool bReset )
{
	if (bReset)
	{
		for (SCTAB i=0; i<=MAXTAB; i++)
			bTabMarked[i] = sal_False;				// Tabellen sind nicht in ResetMark
		ResetMark();
	}

	sal_uLong nCount = rList.Count();
	if ( nCount == 1 && !bMarked && !bMultiMarked )
	{
		ScRange aRange = *rList.GetObject(0);
		SetMarkArea( aRange );
		SelectTable( aRange.aStart.Tab(), sal_True );
	}
	else
	{
		for (sal_uLong i=0; i<nCount; i++)
		{
			ScRange aRange = *rList.GetObject(i);
			SetMultiMarkArea( aRange, sal_True );
			SelectTable( aRange.aStart.Tab(), sal_True );
		}
	}
}

void ScMarkData::FillRangeListWithMarks( ScRangeList* pList, sal_Bool bClear ) const
{
	if (!pList)
		return;

	if (bClear)
		pList->RemoveAll();

	//!		bei mehreren selektierten Tabellen mehrere Ranges eintragen !!!

	if ( bMultiMarked )
	{
		DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

		SCTAB nTab = aMultiRange.aStart.Tab();

		SCCOL nStartCol = aMultiRange.aStart.Col();
		SCCOL nEndCol = aMultiRange.aEnd.Col();
		for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
			if (pMultiSel[nCol].HasMarks())
			{
				SCROW nTop, nBottom;
				ScRange aRange( nCol, 0, nTab );
				ScMarkArrayIter aMarkIter( &pMultiSel[nCol] );
				while ( aMarkIter.Next( nTop, nBottom ) )
				{
					aRange.aStart.SetRow( nTop );
					aRange.aEnd.SetRow( nBottom );
					pList->Join( aRange );
				}
			}
	}

	if ( bMarked )
		pList->Append( aMarkRange );
}

void ScMarkData::ExtendRangeListTables( ScRangeList* pList ) const
{
	if (!pList)
		return;

	ScRangeList aOldList(*pList);
	pList->RemoveAll();					//!	oder die vorhandenen unten weglassen

	for (SCTAB nTab=0; nTab<=MAXTAB; nTab++)
		if (bTabMarked[nTab])
		{
			sal_uLong nCount = aOldList.Count();
			for (sal_uLong i=0; i<nCount; i++)
			{
				ScRange aRange = *aOldList.GetObject(i);
				aRange.aStart.SetTab(nTab);
				aRange.aEnd.SetTab(nTab);
				pList->Append( aRange );
			}
		}
}

SCCOLROW ScMarkData::GetMarkColumnRanges( SCCOLROW* pRanges )
{
	if (bMarked)
		MarkToMulti();

	if (!bMultiMarked)
		return 0;

	DBG_ASSERT(pMultiSel, "bMultiMarked, but pMultiSel == 0");

    const SCCOLROW nMultiStart = aMultiRange.aStart.Col();
    const SCCOLROW nMultiEnd = aMultiRange.aEnd.Col();
    if (nMultiStart == 0 && nMultiEnd == MAXCOL)
    {
        // One or more entire rows.
        pRanges[0] = 0;
        pRanges[1] = MAXCOL;
        return 1;
    }

	SCCOLROW nRangeCnt = 0;
	SCCOLROW nStart = nMultiStart;
	while (nStart <= nMultiEnd)
	{
		while (nStart < nMultiEnd && !pMultiSel[nStart].HasMarks())
			++nStart;
		if (pMultiSel[nStart].HasMarks())
		{
			SCCOLROW nEnd = nStart;
			while (nEnd < nMultiEnd && pMultiSel[nEnd].HasMarks())
				++nEnd;
			if (!pMultiSel[nEnd].HasMarks())
				--nEnd;
			pRanges[2*nRangeCnt  ] = nStart;
			pRanges[2*nRangeCnt+1] = nEnd;
			++nRangeCnt;
			nStart = nEnd+1;
		}
		else
			nStart = nMultiEnd+1;
	}

	return nRangeCnt;
}

SCCOLROW ScMarkData::GetMarkRowRanges( SCCOLROW* pRanges )
{
	if (bMarked)
		MarkToMulti();

	if (!bMultiMarked)
		return 0;

	DBG_ASSERT(pMultiSel, "bMultiMarked, but pMultiSel == 0");

    // Which rows are marked?

    // Optimized to not loop over MAXCOL*MAXROW as worst case, i.e. Ctrl+A

    const SCCOLROW nMultiStart = aMultiRange.aStart.Row();
    const SCCOLROW nMultiEnd = aMultiRange.aEnd.Row();

    sal_Bool*   bRowMarked = new sal_Bool[MAXROWCOUNT];
    memset( bRowMarked, 0, sizeof(sal_Bool) * MAXROWCOUNT);
	SCROW  nRow;
	SCCOL  nCol;

    SCROW nTop = -1, nBottom = -1;
    for (nCol = aMultiRange.aStart.Col(); nCol <= aMultiRange.aEnd.Col(); ++nCol)
    {
        ScMarkArrayIter aMarkIter( &pMultiSel[nCol] );
        while (aMarkIter.Next( nTop, nBottom ))
            for (nRow=nTop; nRow<=nBottom; nRow++)
                bRowMarked[nRow] = sal_True;
        if (nTop == nMultiStart && nBottom == nMultiEnd)
            break;  // for, all relevant rows marked
    }

    if (nTop == nMultiStart && nBottom == nMultiEnd)
    {
        pRanges[0] = nTop;
        pRanges[1] = nBottom;
        delete[] bRowMarked;
        return 1;
    }

    // Combine to ranges of rows.

    SCCOLROW nRangeCnt = 0;
    SCCOLROW nStart = nMultiStart;
    while (nStart <= nMultiEnd)
    {
        while (nStart < nMultiEnd && !bRowMarked[nStart])
            ++nStart;
        if (bRowMarked[nStart])
        {
            SCCOLROW nEnd = nStart;
            while (nEnd < nMultiEnd && bRowMarked[nEnd])
                ++nEnd;
            if (!bRowMarked[nEnd])
                --nEnd;
            pRanges[2*nRangeCnt  ] = nStart;
            pRanges[2*nRangeCnt+1] = nEnd;
            ++nRangeCnt;
            nStart = nEnd+1;
        }
        else
            nStart = nMultiEnd+1;
    }

	delete[] bRowMarked;
	return nRangeCnt;
}

sal_Bool ScMarkData::IsAllMarked( const ScRange& rRange ) const
{
	if ( !bMultiMarked )
		return sal_False;

	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

	SCCOL nStartCol = rRange.aStart.Col();
	SCROW nStartRow = rRange.aStart.Row();
	SCCOL nEndCol = rRange.aEnd.Col();
	SCROW nEndRow = rRange.aEnd.Row();
	sal_Bool bOk = sal_True;
	for (SCCOL nCol=nStartCol; nCol<=nEndCol && bOk; nCol++)
		if ( !pMultiSel[nCol].IsAllMarked( nStartRow, nEndRow ) )
			bOk = sal_False;

	return bOk;
}

SCsROW ScMarkData::GetNextMarked( SCCOL nCol, SCsROW nRow, sal_Bool bUp ) const
{
	if ( !bMultiMarked )
		return nRow;

	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

	return pMultiSel[nCol].GetNextMarked( nRow, bUp );
}

sal_Bool ScMarkData::HasMultiMarks( SCCOL nCol ) const
{
	if ( !bMultiMarked )
		return sal_False;

	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

	return pMultiSel[nCol].HasMarks();
}

sal_Bool ScMarkData::HasAnyMultiMarks() const
{
	if ( !bMultiMarked )
		return sal_False;

	DBG_ASSERT(pMultiSel, "bMultiMarked, aber pMultiSel == 0");

	for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
		if ( pMultiSel[nCol].HasMarks() )
			return sal_True;

	return sal_False;		// nix
}

void ScMarkData::InsertTab( SCTAB nTab )
{
	for (SCTAB i=MAXTAB; i>nTab; i--)
		bTabMarked[i] = bTabMarked[i-1];
	bTabMarked[nTab] = sal_False;
}

void ScMarkData::DeleteTab( SCTAB nTab )
{
	for (SCTAB i=nTab; i<MAXTAB; i++)
		bTabMarked[i] = bTabMarked[i+1];
	bTabMarked[MAXTAB] = sal_False;
}





