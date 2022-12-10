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



#ifndef SC_DBCOLECT_HXX
#define SC_DBCOLECT_HXX

#include "scdllapi.h"
#include "collect.hxx"
#include "global.hxx"		// MAXQUERY
#include "sortparam.hxx"	// MAXSORT
#include "refreshtimer.hxx"
#include "address.hxx"
#include "scdllapi.h"

//------------------------------------------------------------------------

class ScDocument;

//------------------------------------------------------------------------

class ScDBData : public ScDataObject, public ScRefreshTimer
{
friend class ScDBCollection;
private:
	// DBParam
	String			aName;
	SCTAB			nTable;
	SCCOL      	    nStartCol;
	SCROW			nStartRow;
	SCCOL			nEndCol;
	SCROW			nEndRow;
	sal_Bool			bByRow;
	sal_Bool			bHasHeader;
	sal_Bool			bDoSize;
	sal_Bool			bKeepFmt;
	sal_Bool			bStripData;
	// SortParam
	sal_Bool			bSortCaseSens;
	sal_Bool			bIncludePattern;
	sal_Bool			bSortInplace;
	sal_Bool			bSortUserDef;
	sal_uInt16			nSortUserIndex;
	SCTAB			nSortDestTab;
	SCCOL			nSortDestCol;
	SCROW			nSortDestRow;
	sal_Bool			bDoSort[MAXSORT];
	SCCOLROW		nSortField[MAXSORT];
	sal_Bool			bAscending[MAXSORT];
	::com::sun::star::lang::Locale aSortLocale;
	String			aSortAlgorithm;
	// QueryParam
	sal_Bool			bQueryInplace;
	sal_Bool			bQueryCaseSens;
	sal_Bool			bQueryRegExp;
	sal_Bool			bQueryDuplicate;
	SCTAB			nQueryDestTab;
	SCCOL			nQueryDestCol;
	SCROW			nQueryDestRow;
	sal_Bool			bDoQuery[MAXQUERY];
	SCCOLROW		nQueryField[MAXQUERY];
	ScQueryOp		eQueryOp[MAXQUERY];
	sal_Bool			bQueryByString[MAXQUERY];
    bool            bQueryByDate[MAXQUERY];
	String*			pQueryStr[MAXQUERY];
	double			nQueryVal[MAXQUERY];
	ScQueryConnect  eQueryConnect[MAXQUERY];
	sal_Bool			bIsAdvanced;		// sal_True if created by advanced filter
	ScRange			aAdvSource;			// source range
	// SubTotalParam
	sal_Bool			bSubRemoveOnly;
	sal_Bool			bSubReplace;
	sal_Bool			bSubPagebreak;
	sal_Bool			bSubCaseSens;
	sal_Bool			bSubDoSort;
	sal_Bool			bSubAscending;
	sal_Bool			bSubIncludePattern;
	sal_Bool			bSubUserDef;
	sal_uInt16			nSubUserIndex;
	sal_Bool			bDoSubTotal[MAXSUBTOTAL];
	SCCOL			nSubField[MAXSUBTOTAL];
	SCCOL			nSubTotals[MAXSUBTOTAL];
	SCCOL*			pSubTotals[MAXSUBTOTAL];
	ScSubTotalFunc*	pFunctions[MAXSUBTOTAL];
	// Datenbank-Import
	sal_Bool			bDBImport;
	String			aDBName;
	String			aDBStatement;
	sal_Bool			bDBNative;
	sal_Bool			bDBSelection;		// nicht im Param: Wenn Selektion, Update sperren
	sal_Bool			bDBSql;				// aDBStatement ist SQL und kein Name
	sal_uInt8			nDBType;			// enum DBObject (bisher nur dbTable, dbQuery)

	sal_uInt16			nIndex; 			// eindeutiger Index fuer Formeln
	sal_Bool			bAutoFilter;		// AutoFilter? (nicht gespeichert)
	sal_Bool			bModified;			// wird bei UpdateReference gesetzt/geloescht

    using ScRefreshTimer::operator==;

public:
			SC_DLLPUBLIC ScDBData(const String& rName,
					 SCTAB nTab,
					 SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
					 sal_Bool bByR = sal_True, sal_Bool bHasH = sal_True);
			ScDBData(const ScDBData& rData);
			~ScDBData();

	virtual	ScDataObject*	Clone() const;

			ScDBData&	operator= (const ScDBData& rData);

			sal_Bool		operator== (const ScDBData& rData) const;

			const String& GetName() const				{ return aName; }
			void		GetName(String& rName) const	{ rName = aName; }
			void		SetName(const String& rName)	{ aName = rName; }
			void		GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2) const;
			SC_DLLPUBLIC void		GetArea(ScRange& rRange) const;
			void		SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

            bool IsInternalUnnamed() const;
            bool IsInternalForAutoFilter() const;

            void		MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
			sal_Bool		IsByRow() const 				{ return bByRow; }
			void		SetByRow(sal_Bool bByR) 			{ bByRow = bByR; }
			sal_Bool		HasHeader() const 				{ return bHasHeader; }
			void		SetHeader(sal_Bool bHasH) 			{ bHasHeader = bHasH; }
            void        SetIndex(sal_uInt16 nInd)           { nIndex = nInd; }
			sal_uInt16		GetIndex() const				{ return nIndex; }
			sal_Bool		IsDoSize() const				{ return bDoSize; }
			void		SetDoSize(sal_Bool bSet)			{ bDoSize = bSet; }
			sal_Bool		IsKeepFmt() const				{ return bKeepFmt; }
			void		SetKeepFmt(sal_Bool bSet)			{ bKeepFmt = bSet; }
			sal_Bool		IsStripData() const				{ return bStripData; }
			void		SetStripData(sal_Bool bSet)			{ bStripData = bSet; }

//UNUSED2008-05  sal_Bool		IsBeyond(SCROW nMaxRow) const;

			String		GetSourceString() const;
			String		GetOperations() const;

			void		GetSortParam(ScSortParam& rSortParam) const;
			void		SetSortParam(const ScSortParam& rSortParam);

			SC_DLLPUBLIC void		GetQueryParam(ScQueryParam& rQueryParam) const;
			SC_DLLPUBLIC void		SetQueryParam(const ScQueryParam& rQueryParam);
			SC_DLLPUBLIC sal_Bool		GetAdvancedQuerySource(ScRange& rSource) const;
			SC_DLLPUBLIC void		SetAdvancedQuerySource(const ScRange* pSource);

			void		GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const;
			void		SetSubTotalParam(const ScSubTotalParam& rSubTotalParam);

			void		GetImportParam(ScImportParam& rImportParam) const;
			void		SetImportParam(const ScImportParam& rImportParam);

			sal_Bool		IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly) const;
			sal_Bool		IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

			sal_Bool		HasImportParam() const	 { return bDBImport; }
			sal_Bool		HasQueryParam() const	 { return bDoQuery[0]; }
			sal_Bool		HasSortParam() const	 { return bDoSort[0]; }
			sal_Bool		HasSubTotalParam() const { return bDoSubTotal[0]; }

			sal_Bool		HasImportSelection() const		{ return bDBSelection; }
			void		SetImportSelection(sal_Bool bSet)	{ bDBSelection = bSet; }

			sal_Bool		HasAutoFilter() const 		{ return bAutoFilter; }
			void		SetAutoFilter(sal_Bool bSet)	{ bAutoFilter = bSet; }

			sal_Bool		IsModified() const			{ return bModified; }
			void		SetModified(sal_Bool bMod)		{ bModified = bMod; }
};


//------------------------------------------------------------------------
class SC_DLLPUBLIC ScDBCollection : public ScSortedCollection
{

private:
	Link		aRefreshHandler;
	ScDocument* pDoc;
	sal_uInt16 nEntryIndex;			// Zaehler fuer die eindeutigen Indizes

public:
	ScDBCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4, sal_Bool bDup = sal_False, ScDocument* pDocument = NULL) :
					ScSortedCollection	( nLim, nDel, bDup ),
					pDoc				( pDocument ),
					nEntryIndex			( SC_START_INDEX_DB_COLL )	// oberhalb der Namen
					{}
	ScDBCollection(const ScDBCollection& rScDBCollection) :
					ScSortedCollection	( rScDBCollection ),
					pDoc 				( rScDBCollection.pDoc ),
					nEntryIndex			( rScDBCollection.nEntryIndex)
					{}

	virtual	ScDataObject*	Clone() const { return new ScDBCollection(*this); }
			ScDBData*	operator[]( const sal_uInt16 nIndex) const {return (ScDBData*)At(nIndex);}
	virtual	short		Compare(ScDataObject* pKey1, ScDataObject* pKey2) const;
	virtual	sal_Bool		IsEqual(ScDataObject* pKey1, ScDataObject* pKey2) const;
			ScDBData*	GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, sal_Bool bStartOnly) const;
            ScDBData* GetDBAtArea(
                const SCTAB nTab,
                const SCCOL nCol1,
                const SCROW nRow1,
                const SCCOL nCol2,
                const SCROW nRow2 ) const;
			ScDBData*       GetFilterDBAtTable(SCTAB nTab) const;

	sal_Bool	SearchName( const String& rName, sal_uInt16& rIndex ) const;

    void    DeleteOnTab( SCTAB nTab );
	void	UpdateReference(UpdateRefMode eUpdateRefMode,
								SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
								SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
								SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
	void	UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos );

	ScDBData* FindIndex(sal_uInt16 nIndex);
	sal_uInt16 	GetEntryIndex()					{ return nEntryIndex; }
	void 	SetEntryIndex(sal_uInt16 nInd)		{ nEntryIndex = nInd; }
	virtual sal_Bool Insert(ScDataObject* pScDataObject);

	void			SetRefreshHandler( const Link& rLink )
						{ aRefreshHandler = rLink; }
	const Link&		GetRefreshHandler() const	{ return aRefreshHandler; }
    String      GetNewDefaultDBName();
    /*sal_Bool        IsFiltered(SCTAB nTab, SCROW nRow);*/
};

#endif
