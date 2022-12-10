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


#ifndef _TBLSEL_HXX
#define _TBLSEL_HXX

#include <svl/svarray.hxx>
#include <swtable.hxx>
#include <swrect.hxx>
#include "swdllapi.h"

class SwCrsrShell;
class SwCursor;
class SwTableCursor;
class SwFrm;
class SwTabFrm;
class SwTableBox;
class SwTableLine;
class SwLayoutFrm;
class SwPaM;
class SwNode;
class SwTable;
class SwUndoTblMerge;
class SwCellFrm;

SV_DECL_PTRARR( SwCellFrms, SwCellFrm*, 16, 16 )
SV_DECL_PTRARR_SORT( SwSelBoxes, SwTableBoxPtr, 10, 20 )


//Sucht alle Boxen zusammen, die in der Tabelle selektiert sind.
//Je nach enum-Parameter wird die Selektion in der angegebenen Richtung
//erweitert.
//Die Boxen werden ueber das Layout zusammengsucht, es wird auch bei
//aufgespaltenen Tabellen korrekt gearbeitet (siehe: MakeSelUnions()).
typedef sal_uInt16 SwTblSearchType;
namespace nsSwTblSearchType
{
	const SwTblSearchType TBLSEARCH_NONE = 0x1;       // keine Erweiterung
	const SwTblSearchType TBLSEARCH_ROW	 = 0x2;       // erweiter auf Zeilen
	const SwTblSearchType TBLSEARCH_COL  = 0x3;       // erweiter auf Spalten

	// als Flag zu den anderen Werten!!
	const SwTblSearchType TBLSEARCH_PROTECT	= 0x8;		// auch geschuetzte Boxen einsammeln
	const SwTblSearchType TBLSEARCH_NO_UNION_CORRECT = 0x10; // die zusammenges. Union nicht korrigieren
}

SW_DLLPUBLIC void GetTblSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
				const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

void GetTblSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
				const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

//wie vor, jedoch wird nicht von der Selektion sondern von den
//Start- EndFrms ausgegangen.
void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, SwCellFrms* pCells,
                const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

// Desgleichen nocheinmal direkt per PaM's
void GetTblSelCrs( const SwCrsrShell& rShell, SwSelBoxes& rBoxes );
void GetTblSelCrs( const SwTableCursor& rTblCrsr, SwSelBoxes& rBoxes );

// suche fuer eine AutoSumme die beteiligten Boxen zusammen
sal_Bool GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

// check if the SelBoxes contains protected Boxes
sal_Bool HasProtectedCells( const SwSelBoxes& rBoxes );

// teste, ob die Selektion ausgeglichen ist
SV_DECL_PTRARR( SwChartBoxes, SwTableBoxPtr, 16, 16)
SV_DECL_PTRARR_DEL( SwChartLines, SwChartBoxes*, 25, 50)

sal_Bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd,
					SwChartLines* pGetCLines = 0 );

// teste ob die Celle in die SSelection gehoert
// (wurde eine Funktion, damit GetTblSel() und MakeTblCrsr() immer
// das "gleiche Verstaendnis" fuer die Selektion haben)
sal_Bool IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell );

// bestimme die Boxen, die zusammen gefasst werden sollen.
// Dabei wird auf Layout Basis das Rechteck "angepasst". D.H. es
// werden Boxen zugefuegt wenn welche an den Seiten ueberlappen
// Zusaetzlich wird die neue Box erzeugt und mit dem entsprechenden
// Inhalt gefuellt.
void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
				  SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo = 0 );

// teste ob die selektierten Boxen ein gueltiges Merge erlauben
sal_uInt16 CheckMergeSel( const SwPaM& rPam );
sal_uInt16 CheckMergeSel( const SwSelBoxes& rBoxes );

sal_Bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam );

// teste ob ein Split oder InsertCol dazu fuehrt, das eine Box
// kleiner als MINLAY wird.
sal_Bool CheckSplitCells( const SwCrsrShell& rShell, sal_uInt16 nDiv,
						const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );
sal_Bool CheckSplitCells( const SwCursor& rCrsr, sal_uInt16 nDiv,
						const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

//Fuer das Arbeiten auf TabSelektion auch fuer aufgespaltene Tabellen.
class SwSelUnion
{
	SwRect   aUnion;		//Das die Sel umschliessende Rechteck.
	SwTabFrm *pTable;		//Die (Follow-)Table zu der Union.

public:
	SwSelUnion( const SwRect &rRect, SwTabFrm *pTab ) :
		aUnion( rRect ), pTable( pTab ) {}

	const SwRect&	GetUnion() const { return aUnion; }
		  SwRect&	GetUnion()		 { return aUnion; }
	const SwTabFrm *GetTable() const { return pTable; }
		  SwTabFrm *GetTable()		 { return pTable; }
};

SV_DECL_PTRARR_DEL( SwSelUnions, SwSelUnion*, 10, 20 )

//Ermittelt die von einer Tabellenselektion betroffenen Tabellen und die
//Union-Rechteckte der Selektionen - auch fuer aufgespaltene Tabellen.
//Wenn ein Parameter != nsSwTblSearchType::TBLSEARCH_NONE uebergeben wird, so wird die
//Selektion in der angegebenen Richtung erweitert.
void MakeSelUnions( SwSelUnions&, const SwLayoutFrm *pStart,
					const SwLayoutFrm *pEnd,
					const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );


// -------------------------------------------------------------------
// Diese Klassen kopieren die aktuelle Tabellen-Selektion (rBoxes)
// unter Beibehaltung der Tabellen-Strubktur in eine eigene Struktur

class _FndBox;
class _FndLine;

SV_DECL_PTRARR_DEL( _FndBoxes, _FndBox*, 10, 20 )
SV_DECL_PTRARR_DEL( _FndLines, _FndLine*,10, 20 )

class _FndBox
{
	SwTableBox* pBox;
	_FndLines aLines;
	_FndLine* pUpper;

	SwTableLine *pLineBefore;	//Zum Loeschen/Restaurieren des Layouts.
	SwTableLine *pLineBehind;

public:
	_FndBox( SwTableBox* pB, _FndLine* pFL ) :
		pBox(pB), pUpper(pFL), pLineBefore( 0 ), pLineBehind( 0 ) {}

	const _FndLines&	GetLines() const	{ return aLines; }
		_FndLines& 		GetLines() 			{ return aLines; }
	const SwTableBox* 	GetBox() const 		{ return pBox; }
		SwTableBox* 	GetBox() 			{ return pBox; }
	const _FndLine* 	GetUpper() const 	{ return pUpper; }
		_FndLine* 		GetUpper() 			{ return pUpper; }

	void SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable );
	void SetTableLines( const SwTable &rTable );
	//Solution:Add an input param to identify if acc table should be disposed
	//void DelFrms ( SwTable &rTable );
	void DelFrms ( SwTable &rTable,sal_Bool bAccTableDispose = sal_False );
	void MakeFrms( SwTable &rTable );
	void MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
									   const sal_Bool bBehind );
	sal_Bool AreLinesToRestore( const SwTable &rTable ) const;

    void ClearLineBehind() { pLineBehind = 0; }
};


class _FndLine
{
	SwTableLine* pLine;
	_FndBoxes aBoxes;
	_FndBox* pUpper;
public:
	_FndLine(SwTableLine* pL, _FndBox* pFB=0) : pLine(pL), pUpper(pFB) {}
	const _FndBoxes& 	GetBoxes() const 	{ return aBoxes; }
		_FndBoxes& 		GetBoxes() 			{ return aBoxes; }
	const SwTableLine* 	GetLine() const 	{ return pLine; }
		SwTableLine* 	GetLine() 			{ return pLine; }
	const _FndBox*	 	GetUpper() const 	{ return pUpper; }
		_FndBox* 		GetUpper() 			{ return pUpper; }

	void SetUpper( _FndBox* pUp ) { pUpper = pUp; }
};


struct _FndPara
{
	const SwSelBoxes& rBoxes;
	_FndLine* pFndLine;
	_FndBox* pFndBox;

	_FndPara( const SwSelBoxes& rBxs, _FndBox* pFB )
		: rBoxes(rBxs), pFndLine(0), pFndBox(pFB) {}
	_FndPara( const _FndPara& rPara, _FndBox* pFB )
		: rBoxes(rPara.rBoxes), pFndLine(rPara.pFndLine), pFndBox(pFB) {}
	_FndPara( const _FndPara& rPara, _FndLine* pFL )
		: rBoxes(rPara.rBoxes), pFndLine(pFL), pFndBox(rPara.pFndBox) {}
};

sal_Bool _FndBoxCopyCol( const SwTableBox*& rpBox, void* pPara );
SW_DLLPUBLIC sal_Bool _FndLineCopyCol( const SwTableLine*& rpLine, void* pPara );


#endif	//  _TBLSEL_HXX
