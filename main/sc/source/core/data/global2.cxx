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

#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <tools/urlobj.hxx>
#include <unotools/charclass.hxx>
#include <stdlib.h>
#include <ctype.h>
#include <unotools/syslocale.hxx>

#include "global.hxx"
#include "rangeutl.hxx"
#include "rechead.hxx"
#include "compiler.hxx"
#include "paramisc.hxx"

#include "sc.hrc"
#include "globstr.hrc"

using ::std::vector;

// -----------------------------------------------------------------------




//------------------------------------------------------------------------
// struct ScImportParam:

ScImportParam::ScImportParam() :
	nCol1(0),
	nRow1(0),
	nCol2(0),
	nRow2(0),
	bImport(sal_False),
	bNative(sal_False),
	bSql(sal_True),
	nType(ScDbTable)
{
}

ScImportParam::ScImportParam( const ScImportParam& r ) :
	nCol1		(r.nCol1),
	nRow1		(r.nRow1),
	nCol2		(r.nCol2),
	nRow2		(r.nRow2),
	bImport		(r.bImport),
	aDBName		(r.aDBName),
	aStatement	(r.aStatement),
	bNative		(r.bNative),
	bSql		(r.bSql),
	nType		(r.nType)
{
}

ScImportParam::~ScImportParam()
{
}

//UNUSED2009-05 void ScImportParam::Clear()
//UNUSED2009-05 {
//UNUSED2009-05     nCol1 = nCol2 = 0;
//UNUSED2009-05     nRow1 = nRow2 = 0;
//UNUSED2009-05     bImport = sal_False;
//UNUSED2009-05     bNative = sal_False;
//UNUSED2009-05     bSql = sal_True;
//UNUSED2009-05     nType = ScDbTable;
//UNUSED2009-05     aDBName.Erase();
//UNUSED2009-05     aStatement.Erase();
//UNUSED2009-05 }

ScImportParam& ScImportParam::operator=( const ScImportParam& r )
{
	nCol1			= r.nCol1;
	nRow1			= r.nRow1;
	nCol2			= r.nCol2;
	nRow2			= r.nRow2;
	bImport			= r.bImport;
	aDBName			= r.aDBName;
	aStatement		= r.aStatement;
	bNative			= r.bNative;
	bSql			= r.bSql;
	nType			= r.nType;

	return *this;
}

sal_Bool ScImportParam::operator==( const ScImportParam& rOther ) const
{
	return(	nCol1		== rOther.nCol1 &&
			nRow1		== rOther.nRow1 &&
			nCol2		== rOther.nCol2 &&
			nRow2		== rOther.nRow2 &&
			bImport		== rOther.bImport &&
			aDBName		== rOther.aDBName &&
			aStatement	== rOther.aStatement &&
			bNative		== rOther.bNative &&
			bSql		== rOther.bSql &&
			nType		== rOther.nType );

	//!	nQuerySh und pConnection sind gleich ?
}


//------------------------------------------------------------------------
// struct ScQueryParam:

ScQueryEntry::ScQueryEntry() :
    bDoQuery(sal_False),
    bQueryByString(sal_False),
    bQueryByDate(false),
    nField(0),
    eOp(SC_EQUAL),
    eConnect(SC_AND),
    pStr(new String),
    nVal(0.0),
    pSearchParam(NULL),
    pSearchText(NULL)
{
}

ScQueryEntry::ScQueryEntry(const ScQueryEntry& r) :
    bDoQuery(r.bDoQuery),
    bQueryByString(r.bQueryByString),
    bQueryByDate(r.bQueryByDate),
    nField(r.nField),
    eOp(r.eOp),
    eConnect(r.eConnect),
    pStr(new String(*r.pStr)),
    nVal(r.nVal),
    pSearchParam(NULL),
    pSearchText(NULL)
{
}

ScQueryEntry::~ScQueryEntry()
{
	delete pStr;
	if ( pSearchParam )
	{
		delete pSearchParam;
		delete pSearchText;
	}
}

ScQueryEntry& ScQueryEntry::operator=( const ScQueryEntry& r )
{
	bDoQuery		= r.bDoQuery;
	bQueryByString	= r.bQueryByString;
    bQueryByDate    = r.bQueryByDate;
	eOp				= r.eOp;
	eConnect		= r.eConnect;
	nField			= r.nField;
	nVal			= r.nVal;
	*pStr			= *r.pStr;
	if ( pSearchParam )
	{
		delete pSearchParam;
		delete pSearchText;
	}
	pSearchParam	= NULL;
	pSearchText		= NULL;

	return *this;
}

void ScQueryEntry::Clear()
{
	bDoQuery		= sal_False;
	bQueryByString	= sal_False;
    bQueryByDate    = false;
	eOp				= SC_EQUAL;
	eConnect		= SC_AND;
	nField			= 0;
	nVal			= 0.0;
	pStr->Erase();
	if ( pSearchParam )
	{
		delete pSearchParam;
		delete pSearchText;
	}
	pSearchParam	= NULL;
	pSearchText		= NULL;
}

sal_Bool ScQueryEntry::operator==( const ScQueryEntry& r ) const
{
	return bDoQuery			== r.bDoQuery
		&& bQueryByString	== r.bQueryByString
        && bQueryByDate     == r.bQueryByDate
		&& eOp				== r.eOp
		&& eConnect			== r.eConnect
		&& nField			== r.nField
		&& nVal				== r.nVal
		&& *pStr			== *r.pStr;
	//! pSearchParam und pSearchText nicht vergleichen
}

utl::TextSearch* ScQueryEntry::GetSearchTextPtr( sal_Bool bCaseSens )
{
	if ( !pSearchParam )
	{
		pSearchParam = new utl::SearchParam( *pStr, utl::SearchParam::SRCH_REGEXP,
			bCaseSens, sal_False, sal_False );
		pSearchText = new utl::TextSearch( *pSearchParam, *ScGlobal::pCharClass );
	}
	return pSearchText;
}

//------------------------------------------------------------------------
// struct ScSubTotalParam:

ScSubTotalParam::ScSubTotalParam()
{
	for ( sal_uInt16 i=0; i<MAXSUBTOTAL; i++ )
	{
		nSubTotals[i] = 0;
		pSubTotals[i] = NULL;
		pFunctions[i] = NULL;
	}

	Clear();
}

//------------------------------------------------------------------------

ScSubTotalParam::ScSubTotalParam( const ScSubTotalParam& r ) :
		nCol1(r.nCol1),nRow1(r.nRow1),nCol2(r.nCol2),nRow2(r.nRow2),
		bRemoveOnly(r.bRemoveOnly),bReplace(r.bReplace),bPagebreak(r.bPagebreak),bCaseSens(r.bCaseSens),
		bDoSort(r.bDoSort),bAscending(r.bAscending),bUserDef(r.bUserDef),nUserIndex(r.nUserIndex),
		bIncludePattern(r.bIncludePattern)
{
	for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
	{
		bGroupActive[i]	= r.bGroupActive[i];
		nField[i]		= r.nField[i];

		if ( (r.nSubTotals[i] > 0) && r.pSubTotals[i] && r.pFunctions[i] )
		{
			nSubTotals[i] = r.nSubTotals[i];
			pSubTotals[i] = new SCCOL	[r.nSubTotals[i]];
			pFunctions[i] = new ScSubTotalFunc	[r.nSubTotals[i]];

			for (SCCOL j=0; j<r.nSubTotals[i]; j++)
			{
				pSubTotals[i][j] = r.pSubTotals[i][j];
				pFunctions[i][j] = r.pFunctions[i][j];
			}
		}
		else
		{
			nSubTotals[i] = 0;
			pSubTotals[i] = NULL;
			pFunctions[i] = NULL;
		}
	}
}

//------------------------------------------------------------------------

void ScSubTotalParam::Clear()
{
	nCol1=nCol2= 0;
	nRow1=nRow2 = 0;
	nUserIndex = 0;
	bPagebreak=bCaseSens=bUserDef=bIncludePattern=bRemoveOnly = sal_False;
	bAscending=bReplace=bDoSort = sal_True;

	for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
	{
		bGroupActive[i]	= sal_False;
		nField[i]		= 0;

		if ( (nSubTotals[i] > 0) && pSubTotals[i] && pFunctions[i] )
		{
			for ( SCCOL j=0; j<nSubTotals[i]; j++ ) {
				pSubTotals[i][j] = 0;
				pFunctions[i][j] = SUBTOTAL_FUNC_NONE;
			}
		}
	}
}

//------------------------------------------------------------------------

ScSubTotalParam& ScSubTotalParam::operator=( const ScSubTotalParam& r )
{
	nCol1			= r.nCol1;
	nRow1			= r.nRow1;
	nCol2			= r.nCol2;
	nRow2			= r.nRow2;
	bRemoveOnly		= r.bRemoveOnly;
	bReplace		= r.bReplace;
	bPagebreak		= r.bPagebreak;
	bCaseSens		= r.bCaseSens;
	bDoSort			= r.bDoSort;
	bAscending		= r.bAscending;
	bUserDef		= r.bUserDef;
	nUserIndex		= r.nUserIndex;
	bIncludePattern	= r.bIncludePattern;

	for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
	{
		bGroupActive[i]	= r.bGroupActive[i];
		nField[i]		= r.nField[i];
		nSubTotals[i]	= r.nSubTotals[i];

		if ( pSubTotals[i] ) delete [] pSubTotals[i];
		if ( pFunctions[i] ) delete [] pFunctions[i];

		if ( r.nSubTotals[i] > 0 )
		{
			pSubTotals[i] = new SCCOL	[r.nSubTotals[i]];
			pFunctions[i] = new ScSubTotalFunc	[r.nSubTotals[i]];

			for (SCCOL j=0; j<r.nSubTotals[i]; j++)
			{
				pSubTotals[i][j] = r.pSubTotals[i][j];
				pFunctions[i][j] = r.pFunctions[i][j];
			}
		}
		else
		{
			nSubTotals[i] = 0;
			pSubTotals[i] = NULL;
			pFunctions[i] = NULL;
		}
	}

	return *this;
}

//------------------------------------------------------------------------

sal_Bool ScSubTotalParam::operator==( const ScSubTotalParam& rOther ) const
{
	sal_Bool bEqual =   (nCol1			== rOther.nCol1)
				 && (nRow1			== rOther.nRow1)
				 && (nCol2			== rOther.nCol2)
				 && (nRow2			== rOther.nRow2)
				 && (bRemoveOnly	== rOther.bRemoveOnly)
				 && (bReplace		== rOther.bReplace)
				 && (bPagebreak		== rOther.bPagebreak)
				 && (bDoSort		== rOther.bDoSort)
				 && (bCaseSens		== rOther.bCaseSens)
				 && (bAscending		== rOther.bAscending)
				 && (bUserDef		== rOther.bUserDef)
				 && (nUserIndex		== rOther.nUserIndex)
				 && (bIncludePattern== rOther.bIncludePattern);

	if ( bEqual )
	{
		bEqual = sal_True;
		for ( sal_uInt16 i=0; i<MAXSUBTOTAL && bEqual; i++ )
		{
			bEqual =   (bGroupActive[i]	== rOther.bGroupActive[i])
					&& (nField[i]		== rOther.nField[i])
					&& (nSubTotals[i]	== rOther.nSubTotals[i]);

			if ( bEqual && (nSubTotals[i] > 0) )
			{
				bEqual = (pSubTotals != NULL) && (pFunctions != NULL);

				for (SCCOL j=0; (j<nSubTotals[i]) && bEqual; j++)
				{
					bEqual =   bEqual
							&& (pSubTotals[i][j] == rOther.pSubTotals[i][j])
							&& (pFunctions[i][j] == rOther.pFunctions[i][j]);
				}
			}
		}
	}

	return bEqual;
}

//------------------------------------------------------------------------

void ScSubTotalParam::SetSubTotals( sal_uInt16					nGroup,
									const SCCOL*			ptrSubTotals,
									const ScSubTotalFunc*	ptrFunctions,
									sal_uInt16					nCount )
{
	DBG_ASSERT( (nGroup <= MAXSUBTOTAL),
				"ScSubTotalParam::SetSubTotals(): nGroup > MAXSUBTOTAL!" );
	DBG_ASSERT( ptrSubTotals,
				"ScSubTotalParam::SetSubTotals(): ptrSubTotals == NULL!" );
	DBG_ASSERT( ptrFunctions,
				"ScSubTotalParam::SetSubTotals(): ptrFunctions == NULL!" );
	DBG_ASSERT( (nCount > 0),
				"ScSubTotalParam::SetSubTotals(): nCount <= 0!" );

	if ( ptrSubTotals && ptrFunctions && (nCount > 0) && (nGroup <= MAXSUBTOTAL) )
	{
		// 0 wird als 1 aufgefasst, sonst zum Array-Index dekrementieren
		if (nGroup != 0)
			nGroup--;

		delete [] pSubTotals[nGroup];
		delete [] pFunctions[nGroup];

		pSubTotals[nGroup] = new SCCOL		[nCount];
		pFunctions[nGroup] = new ScSubTotalFunc	[nCount];
		nSubTotals[nGroup] = static_cast<SCCOL>(nCount);

		for ( sal_uInt16 i=0; i<nCount; i++ )
		{
			pSubTotals[nGroup][i] = ptrSubTotals[i];
			pFunctions[nGroup][i] = ptrFunctions[i];
		}
	}
}

//------------------------------------------------------------------------
// struct ScConsolidateParam:

ScConsolidateParam::ScConsolidateParam() :
	ppDataAreas( NULL )
{
	Clear();
}

//------------------------------------------------------------------------

ScConsolidateParam::ScConsolidateParam( const ScConsolidateParam& r ) :
		nCol(r.nCol),nRow(r.nRow),nTab(r.nTab),
		eFunction(r.eFunction),nDataAreaCount(0),
		ppDataAreas( NULL ),
		bByCol(r.bByCol),bByRow(r.bByRow),bReferenceData(r.bReferenceData)
{
	if ( r.nDataAreaCount > 0 )
	{
		nDataAreaCount = r.nDataAreaCount;
		ppDataAreas = new ScArea*[nDataAreaCount];
		for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
			ppDataAreas[i] = new ScArea( *(r.ppDataAreas[i]) );
	}
}

//------------------------------------------------------------------------

__EXPORT ScConsolidateParam::~ScConsolidateParam()
{
	ClearDataAreas();
}

//------------------------------------------------------------------------

void __EXPORT ScConsolidateParam::ClearDataAreas()
{
	if ( ppDataAreas )
	{
		for ( sal_uInt16 i=0; i<nDataAreaCount; i++ )
			delete ppDataAreas[i];
		delete [] ppDataAreas;
		ppDataAreas = NULL;
	}
	nDataAreaCount = 0;
}

//------------------------------------------------------------------------

void __EXPORT ScConsolidateParam::Clear()
{
	ClearDataAreas();

	nCol = 0;
	nRow = 0;
	nTab = 0;
	bByCol = bByRow = bReferenceData	= sal_False;
	eFunction							= SUBTOTAL_FUNC_SUM;
}

//------------------------------------------------------------------------

ScConsolidateParam& __EXPORT ScConsolidateParam::operator=( const ScConsolidateParam& r )
{
	nCol			= r.nCol;
	nRow			= r.nRow;
	nTab			= r.nTab;
	bByCol			= r.bByCol;
	bByRow			= r.bByRow;
	bReferenceData	= r.bReferenceData;
	eFunction		= r.eFunction;
	SetAreas( r.ppDataAreas, r.nDataAreaCount );

	return *this;
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScConsolidateParam::operator==( const ScConsolidateParam& r ) const
{
	sal_Bool bEqual = 	(nCol			== r.nCol)
				 &&	(nRow			== r.nRow)
				 && (nTab			== r.nTab)
				 && (bByCol			== r.bByCol)
				 && (bByRow			== r.bByRow)
				 && (bReferenceData	== r.bReferenceData)
				 && (nDataAreaCount	== r.nDataAreaCount)
				 && (eFunction		== r.eFunction);

	if ( nDataAreaCount == 0 )
		bEqual = bEqual && (ppDataAreas == NULL) && (r.ppDataAreas == NULL);
	else
		bEqual = bEqual && (ppDataAreas != NULL) && (r.ppDataAreas != NULL);

	if ( bEqual && (nDataAreaCount > 0) )
		for ( sal_uInt16 i=0; i<nDataAreaCount && bEqual; i++ )
			bEqual = *(ppDataAreas[i]) == *(r.ppDataAreas[i]);

	return bEqual;
}

//------------------------------------------------------------------------

void __EXPORT ScConsolidateParam::SetAreas( ScArea* const* ppAreas, sal_uInt16 nCount )
{
	ClearDataAreas();
	if ( ppAreas && nCount > 0 )
	{
		ppDataAreas = new ScArea*[nCount];
		for ( sal_uInt16 i=0; i<nCount; i++ )
			ppDataAreas[i] = new ScArea( *(ppAreas[i]) );
		nDataAreaCount = nCount;
	}
}

//------------------------------------------------------------------------
// struct ScSolveParam

ScSolveParam::ScSolveParam()
	:	pStrTargetVal( NULL )
{
}

//------------------------------------------------------------------------

ScSolveParam::ScSolveParam( const ScSolveParam& r )
	:	aRefFormulaCell	( r.aRefFormulaCell ),
		aRefVariableCell( r.aRefVariableCell ),
		pStrTargetVal	( r.pStrTargetVal
							? new String(*r.pStrTargetVal)
							: NULL )
{
}

//------------------------------------------------------------------------

ScSolveParam::ScSolveParam( const ScAddress& rFormulaCell,
							const ScAddress& rVariableCell,
							const String& 	rTargetValStr )
	:	aRefFormulaCell	( rFormulaCell ),
		aRefVariableCell( rVariableCell ),
		pStrTargetVal	( new String(rTargetValStr) )
{
}

//------------------------------------------------------------------------

ScSolveParam::~ScSolveParam()
{
	delete pStrTargetVal;
}

//------------------------------------------------------------------------

ScSolveParam& __EXPORT ScSolveParam::operator=( const ScSolveParam& r )
{
	delete pStrTargetVal;

	aRefFormulaCell  = r.aRefFormulaCell;
	aRefVariableCell = r.aRefVariableCell;
	pStrTargetVal    = r.pStrTargetVal
							? new String(*r.pStrTargetVal)
							: NULL;
	return *this;
}

//------------------------------------------------------------------------

sal_Bool ScSolveParam::operator==( const ScSolveParam& r ) const
{
	sal_Bool bEqual = 	(aRefFormulaCell  == r.aRefFormulaCell)
				 &&	(aRefVariableCell == r.aRefVariableCell);

	if ( bEqual )
	{
		if ( !pStrTargetVal && !r.pStrTargetVal )
			bEqual = sal_True;
		else if ( !pStrTargetVal || !r.pStrTargetVal )
			bEqual = sal_False;
		else if ( pStrTargetVal && r.pStrTargetVal )
			bEqual = ( *pStrTargetVal == *(r.pStrTargetVal) );
	}

	return bEqual;
}


//------------------------------------------------------------------------
// struct ScTabOpParam

ScTabOpParam::ScTabOpParam( const ScTabOpParam& r )
	:	aRefFormulaCell	( r.aRefFormulaCell ),
		aRefFormulaEnd	( r.aRefFormulaEnd ),
		aRefRowCell		( r.aRefRowCell ),
		aRefColCell		( r.aRefColCell ),
		nMode			( r.nMode )
{
}

//------------------------------------------------------------------------

ScTabOpParam::ScTabOpParam( const ScRefAddress& rFormulaCell,
							const ScRefAddress& rFormulaEnd,
							const ScRefAddress& rRowCell,
							const ScRefAddress& rColCell,
								  sal_uInt8		 nMd)
	:	aRefFormulaCell	( rFormulaCell ),
		aRefFormulaEnd	( rFormulaEnd ),
		aRefRowCell		( rRowCell ),
		aRefColCell		( rColCell ),
		nMode			( nMd )
{
}

//------------------------------------------------------------------------

ScTabOpParam& ScTabOpParam::operator=( const ScTabOpParam& r )
{
	aRefFormulaCell  = r.aRefFormulaCell;
	aRefFormulaEnd   = r.aRefFormulaEnd;
	aRefRowCell 	 = r.aRefRowCell;
	aRefColCell 	 = r.aRefColCell;
	nMode		     = r.nMode;
	return *this;
}

//------------------------------------------------------------------------

sal_Bool __EXPORT ScTabOpParam::operator==( const ScTabOpParam& r ) const
{
	return (		(aRefFormulaCell == r.aRefFormulaCell)
				 &&	(aRefFormulaEnd	 == r.aRefFormulaEnd)
				 &&	(aRefRowCell	 == r.aRefRowCell)
				 &&	(aRefColCell	 == r.aRefColCell)
				 && (nMode 			 == r.nMode) );
}

String ScGlobal::GetAbsDocName( const String& rFileName,
								SfxObjectShell* pShell )
{
	String aAbsName;
	if ( !pShell->HasName() )
	{	// maybe relative to document path working directory
		INetURLObject aObj;
		SvtPathOptions aPathOpt;
		aObj.SetSmartURL( aPathOpt.GetWorkPath() );
		aObj.setFinalSlash();		// it IS a path
		bool bWasAbs = true;
		aAbsName = aObj.smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
		//	returned string must be encoded because it's used directly to create SfxMedium
	}
	else
	{
		const SfxMedium* pMedium = pShell->GetMedium();
		if ( pMedium )
		{
			bool bWasAbs = true;
			aAbsName = pMedium->GetURLObject().smartRel2Abs( rFileName, bWasAbs ).GetMainURL(INetURLObject::NO_DECODE);
		}
		else
		{	// This can't happen, but ...
			// just to be sure to have the same encoding
			INetURLObject aObj;
			aObj.SetSmartURL( aAbsName );
			aAbsName = aObj.GetMainURL(INetURLObject::NO_DECODE);
		}
	}
	return aAbsName;
}


String ScGlobal::GetDocTabName( const String& rFileName,
								const String& rTabName )
{
	String aDocTab( '\'' );
	aDocTab += rFileName;
	xub_StrLen nPos = 1;
	while( (nPos = aDocTab.Search( '\'', nPos ))
			!= STRING_NOTFOUND )
	{	// escape Quotes
		aDocTab.Insert( '\\', nPos );
		nPos += 2;
	}
	aDocTab += '\'';
	aDocTab += SC_COMPILER_FILE_TAB_SEP;
	aDocTab += rTabName;  	// "'Doc'#Tab"
	return aDocTab;
}

