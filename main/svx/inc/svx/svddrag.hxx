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



#ifndef _SVDDRAG_HXX
#define _SVDDRAG_HXX


#include <tools/contnr.hxx>
#include <tools/gen.hxx>
#include <tools/fract.hxx>
#include "svx/svxdllapi.h"

// Statushalter fuer objektspeziefisches Draggen. Damit das Model
// Statusfrei bleibt werden die Statusdaten an der View gehalten
// und dem Objekt zu gegebener Zeit als Parameter uebergeben.
// Ausserdem auch Statushalter fuer den Vorgang der Interaktiven
// Objekterzeugung. pHdl ist in diesem Fall NULL.
class SdrHdl;
class SdrView;
class SdrPageView;
class SdrDragMethod;

struct SVX_DLLPUBLIC SdrDragStatUserData
{
};

class SVX_DLLPUBLIC SdrDragStat {
protected:
	SdrHdl*  pHdl;      // Der Handle an dem der User zottelt
	SdrView* pView;
	SdrPageView* pPageView;
	Container aPnts;    // Alle bisherigen Punkte: [0]=Start, [Count()-2]=Prev
	Point     aRef1;     // Referenzpunkt: Resize-Fixpunkt, (Drehachse,
	Point     aRef2;     // Spiegelachse, ...)
	Point     aPos0;     // Position beim letzten Event
	Point     aRealPos0; // Position beim letzten Event
	Point     aRealNow;  // Aktuelle Dragposition ohne Snap, Ortho und Limit
	Point     aRealLast; // RealPos des letzten Punkts (fuer MinMoved)
	Rectangle aActionRect;

	// Reserve fuer kompatible Erweiterungen, die sonst inkompatibel wuerden.
	Point     aReservePoint1;
	Point     aReservePoint2;
	Point     aReservePoint3;
	Point     aReservePoint4;
	Rectangle aReserveRect1;
	Rectangle aReserveRect2;
	FASTBOOL  bEndDragChangesAttributes;
	FASTBOOL  bEndDragChangesGeoAndAttributes;
	FASTBOOL  bMouseIsUp;
	FASTBOOL  aReserveBool3;
	FASTBOOL  aReserveBool4;
	long      aReserveLong1;
	long      aReserveLong2;
	long      aReserveLong3;
	long      aReserveLong4;
	void*     aReservePtr1;
	void*     aReservePtr2;
	void*     aReservePtr3;
	void*     aReservePtr4;

	FASTBOOL  bShown;    // Xor sichrbar?
	sal_uInt16    nMinMov;   // Soviel muss erstmal minimal bewegt werden
	FASTBOOL  bMinMoved; // MinMove durchbrochen?

	FASTBOOL  bHorFixed; // nur Vertikal draggen
	FASTBOOL  bVerFixed; // nur Horizontal draggen
	FASTBOOL  bWantNoSnap; // sal_True=Fuer die Entscheidung ob fuer pObj->MovCreate() NoSnapPos verwendet
						  // werden soll. Entsprechend wird auch NoSnapPos in den Buffer geschrieben.
	FASTBOOL  bOrtho4;
	FASTBOOL  bOrtho8;

	SdrDragMethod* pDragMethod;

protected:
	void Clear(FASTBOOL bLeaveOne);
	Point& Pnt(sal_uIntPtr nNum)                           { return *((Point*)aPnts.GetObject(nNum)); }
//public:
	SdrDragStatUserData*    pUser;     // Userdata
public:
	SdrDragStat(): aPnts(1024,16,16)                 { pUser=NULL; Reset(); }
	~SdrDragStat()                                   { Clear(sal_False); }
	void         Reset();
	SdrView*     GetView() const                     { return pView; }
	void         SetView(SdrView* pV)                { pView=pV; }
	SdrPageView* GetPageView() const                 { return pPageView; }
	void         SetPageView(SdrPageView* pPV)       { pPageView=pPV; }
	const Point& GetPoint(sal_uIntPtr nNum) const          { return *((Point*)aPnts.GetObject(nNum)); }
	sal_uIntPtr        GetPointAnz() const                 { return aPnts.Count(); }
	const Point& GetStart() const                    { return GetPoint(0); }
	Point&       Start()                             { return Pnt(0); }
	const Point& GetPrev() const                     { return GetPoint(GetPointAnz()-(GetPointAnz()>=2 ? 2:1)); }
	Point& Prev()                                    { return Pnt(GetPointAnz()-(GetPointAnz()>=2 ? 2:1)); }
	const Point& GetPos0() const                     { return aPos0;  }
	Point&       Pos0()                              { return aPos0;  }
	const Point& GetNow() const                      { return GetPoint(GetPointAnz()-1); }
	Point&       Now()                               { return Pnt(GetPointAnz()-1); }
	const Point& GetRealNow() const                  { return aRealNow; }
	Point&       RealNow()                           { return aRealNow; }
	const Point& GetRef1() const                     { return aRef1;  }
	Point&       Ref1()                              { return aRef1;  }
	const Point& GetRef2() const                     { return aRef2;  }
	Point&       Ref2()                              { return aRef2;  }
	const        SdrHdl* GetHdl() const              { return pHdl;   }
	void         SetHdl(SdrHdl* pH)                  { pHdl=pH;       }
	SdrDragStatUserData* GetUser() const             { return pUser;  }
	void SetUser(SdrDragStatUserData* pU)            { pUser=pU; }
	FASTBOOL     IsShown() const                     { return bShown; }
	void         SetShown(FASTBOOL bOn)              { bShown=bOn; }

	FASTBOOL     IsMinMoved() const                  { return bMinMoved; }
	void         SetMinMoved()                       { bMinMoved=sal_True; }
	void         ResetMinMoved()                     { bMinMoved=sal_False; }
	void         SetMinMove(sal_uInt16 nDist)            { nMinMov=nDist; if (nMinMov<1) nMinMov=1; }
	sal_uInt16       GetMinMove() const                  { return nMinMov; }

	FASTBOOL     IsHorFixed() const                  { return bHorFixed; }
	void         SetHorFixed(FASTBOOL bOn)           { bHorFixed=bOn; }
	FASTBOOL     IsVerFixed() const                  { return bVerFixed; }
	void         SetVerFixed(FASTBOOL bOn)           { bVerFixed=bOn; }

	// Hier kann das Obj sagen: "Ich will keinen Koordinatenfang!"
	// z.B. fuer den Winkel des Kreisbogen...
	FASTBOOL     IsNoSnap() const                     { return bWantNoSnap; }
	void         SetNoSnap(FASTBOOL bOn=sal_True)         { bWantNoSnap=bOn; }

	// Und hier kann das Obj sagen welches Ortho (wenn ueberhaupt eins)
	// sinnvoll auf ihm angewendet werden kann.
	// Ortho4 bedeutet Ortho in 4 Richtungen (fuer Rect und Cirt)
	FASTBOOL     IsOrtho4Possible() const             { return bOrtho4; }
	void         SetOrtho4Possible(FASTBOOL bOn=sal_True) { bOrtho4=bOn; }
	// Ortho8 bedeutet Ortho in 8 Richtungen (fuer Linien)
	FASTBOOL     IsOrtho8Possible() const             { return bOrtho8; }
	void         SetOrtho8Possible(FASTBOOL bOn=sal_True) { bOrtho8=bOn; }

	// Wird vom gedraggten Objekt gesetzt
	FASTBOOL     IsEndDragChangesAttributes() const   { return bEndDragChangesAttributes; }
	void         SetEndDragChangesAttributes(FASTBOOL bOn) { bEndDragChangesAttributes=bOn; }
	FASTBOOL     IsEndDragChangesGeoAndAttributes() const   { return bEndDragChangesGeoAndAttributes; }
	void         SetEndDragChangesGeoAndAttributes(FASTBOOL bOn) { bEndDragChangesGeoAndAttributes=bOn; }

	// Wird von der View gesetzt und kann vom Obj ausgewertet werden
	FASTBOOL     IsMouseDown() const                  { return !bMouseIsUp; }
	void         SetMouseDown(FASTBOOL bDown)         { bMouseIsUp=!bDown; }

	Point KorregPos(const Point& rNow, const Point& rPrev) const;
	void  Reset(const Point& rPnt);
	void  NextMove(const Point& rPnt);
	void  NextPoint(FASTBOOL bSaveReal=sal_False);
	void  PrevPoint();
	FASTBOOL CheckMinMoved(const Point& rPnt);
	long  GetDX() const                     { return GetNow().X()-GetPrev().X(); }
	long  GetDY() const                     { return GetNow().Y()-GetPrev().Y(); }
	Fraction GetXFact() const;
	Fraction GetYFact() const;

	SdrDragMethod* GetDragMethod() const               { return pDragMethod; }
	void           SetDragMethod(SdrDragMethod* pMth)  { pDragMethod=pMth; }

	const Rectangle& GetActionRect() const             { return aActionRect; }
	void             SetActionRect(const Rectangle& rR) { aActionRect=rR; }

	// Unter Beruecksichtigung von 1stPointAsCenter
	void TakeCreateRect(Rectangle& rRect) const;
};

#endif //_SVDDRAG_HXX

