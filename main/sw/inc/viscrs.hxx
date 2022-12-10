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


#ifndef _VISCRS_HXX
#define _VISCRS_HXX

#include <vcl/cursor.hxx>
#include "swcrsr.hxx"
#include "swrect.hxx"
#include "swregion.hxx"

class SwCrsrShell;
class SwShellCrsr;
class SwTxtInputFld;

// --------  Ab hier Klassen / Methoden fuer den nicht Text-Cursor ------

class SwVisCrsr
#ifdef SW_CRSR_TIMER
				: private Timer
#endif
{
	friend void _InitCore();
	friend void _FinitCore();

	sal_Bool bIsVisible : 1;
	sal_Bool bIsDragCrsr : 1;

#ifdef SW_CRSR_TIMER
	sal_Bool bTimerOn : 1;
#endif

	Cursor aTxtCrsr;
	const SwCrsrShell* pCrsrShell;

#ifdef SW_CRSR_TIMER
	virtual void Timeout();
#endif
	void _SetPosAndShow();

public:
	SwVisCrsr( const SwCrsrShell * pCShell );
	~SwVisCrsr();

	void Show();
	void Hide();

	sal_Bool IsVisible() const { return bIsVisible; }
    void SetDragCrsr( sal_Bool bFlag = sal_True ) { bIsDragCrsr = bFlag; }

#ifdef SW_CRSR_TIMER
	sal_Bool ChgTimerFlag( sal_Bool bTimerOn = sal_True );
#endif
};


// ------ Ab hier Klassen / Methoden fuer die Selectionen -------

// #i75172# predefines
namespace sdr { namespace overlay { class OverlayObject; }}
namespace sw { namespace overlay { class OverlayRangesOutline; }}

class SwSelPaintRects : public SwRects
{
	friend void _InitCore();
	friend void _FinitCore();

	static long nPixPtX, nPixPtY;
	static MapMode *pMapMode;

	// die Shell
	const SwCrsrShell* pCShell;

	virtual void Paint( const Rectangle& rRect );

	// #i75172#
	sdr::overlay::OverlayObject*	mpCursorOverlay;

	// #i75172# access to mpCursorOverlay for swapContent
	sdr::overlay::OverlayObject* getCursorOverlay() const { return mpCursorOverlay; }
	void setCursorOverlay(sdr::overlay::OverlayObject* pNew) { mpCursorOverlay = pNew; }

    bool mbShowTxtInputFldOverlay;
    sw::overlay::OverlayRangesOutline* mpTxtInputFldOverlay;

    void HighlightInputFld();

public:
	SwSelPaintRects( const SwCrsrShell& rCSh );
	virtual ~SwSelPaintRects();

	virtual void FillRects() = 0;

    // #i75172# in SwCrsrShell::CreateCrsr() the content of SwSelPaintRects is exchanged. To
	// make a complete swap access to mpCursorOverlay is needed there
	void swapContent(SwSelPaintRects& rSwap);

	void Show();
	void Hide();
	void Invalidate( const SwRect& rRect );

    inline void SetShowTxtInputFldOverlay( const bool bShow )
    {
        mbShowTxtInputFldOverlay = bShow;
    }

	const SwCrsrShell* GetShell() const { return pCShell; }
	// check current MapMode of the shell and set possibly the static members.
	// Optional set the parameters pX, pY
	static void Get1PixelInLogic( const ViewShell& rSh,
									long* pX = 0, long* pY = 0 );
};


class SwShellCrsr : public virtual SwCursor, public SwSelPaintRects
{
	// Dokument-Positionen der Start/End-Charakter einer SSelection
	Point aMkPt, aPtPt;
	const SwPosition* pPt;		// fuer Zuordung vom GetPoint() zum aPtPt

    using SwCursor::UpDown;

public:
	SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos );
	SwShellCrsr( const SwCrsrShell& rCrsrSh, const SwPosition &rPos,
					const Point& rPtPos, SwPaM* pRing = 0 );
	SwShellCrsr( SwShellCrsr& );
	virtual ~SwShellCrsr();

	virtual void FillRects();	// fuer Table- und normalen Crsr

	void Show();			// Update und zeige alle Selektionen an
	void Hide();	  		// verstecke alle Selektionen
	void Invalidate( const SwRect& rRect );

	const Point& GetPtPos() const	{ return( SwPaM::GetPoint() == pPt ? aPtPt : aMkPt ); }
		  Point& GetPtPos() 		{ return( SwPaM::GetPoint() == pPt ? aPtPt : aMkPt ); }
	const Point& GetMkPos() const 	{ return( SwPaM::GetMark() == pPt ? aPtPt : aMkPt ); }
		  Point& GetMkPos() 		{ return( SwPaM::GetMark() == pPt ? aPtPt : aMkPt ); }
	const Point& GetSttPos() const	{ return( SwPaM::Start() == pPt ? aPtPt : aMkPt ); }
		  Point& GetSttPos() 		{ return( SwPaM::Start() == pPt ? aPtPt : aMkPt ); }
	const Point& GetEndPos() const	{ return( SwPaM::End() == pPt ? aPtPt : aMkPt ); }
		  Point& GetEndPos() 		{ return( SwPaM::End() == pPt ? aPtPt : aMkPt ); }

	virtual void SetMark();

	virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
	virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

	sal_Bool UpDown( sal_Bool bUp, sal_uInt16 nCnt = 1 );

	// sal_True: an die Position kann der Cursor gesetzt werden
	virtual sal_Bool IsAtValidPos( sal_Bool bPoint = sal_True ) const;

#ifdef DBG_UTIL
// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//				am sichtbaren Cursor
	virtual sal_Bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ));
#endif

	virtual bool IsReadOnlyAvailable() const;

	DECL_FIXEDMEMPOOL_NEWDEL( SwShellCrsr )
};



class SwShellTableCrsr : public virtual SwShellCrsr, public virtual SwTableCursor
{
	// die Selection hat die gleiche Reihenfolge wie die
	// TabellenBoxen. D.h., wird aus dem einen Array an einer Position
	// etwas geloescht, dann muss es auch im anderen erfolgen!!

public:
	SwShellTableCrsr( const SwCrsrShell& rCrsrSh, const SwPosition& rPos );
	SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
					const SwPosition &rMkPos, const Point& rMkPt,
					const SwPosition &rPtPos, const Point& rPtPt );
	virtual ~SwShellTableCrsr();

	virtual void FillRects();	// fuer Table- und normalen Crsr

	// Pruefe, ob sich der SPoint innerhalb der Tabellen-SSelection befindet
	sal_Bool IsInside( const Point& rPt ) const;

	virtual void SetMark();
	virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

	// sal_True: an die Position kann der Cursor gesetzt werden
	virtual sal_Bool IsAtValidPos( sal_Bool bPoint = sal_True ) const;

#ifdef DBG_UTIL
// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//				am sichtbaren Cursor
	virtual sal_Bool IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ));
#endif
};



#endif	// _VISCRS_HXX
