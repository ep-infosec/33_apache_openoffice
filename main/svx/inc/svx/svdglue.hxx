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



#ifndef _SVDGLUE_HXX
#define _SVDGLUE_HXX

class Window;
class OutputDevice;
class SvStream;
class SdrObject;

#include <tools/contnr.hxx>
#include <tools/gen.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDRESC_SMART  0x0000
#define SDRESC_LEFT   0x0001
#define SDRESC_RIGHT  0x0002
#define SDRESC_TOP    0x0004
#define SDRESC_BOTTOM 0x0008
#define SDRESC_LO    0x0010 /* ni */
#define SDRESC_LU    0x0020 /* ni */
#define SDRESC_RO    0x0040 /* ni */
#define SDRESC_RU    0x0080 /* ni */
#define SDRESC_HORZ  (SDRESC_LEFT|SDRESC_RIGHT)
#define SDRESC_VERT  (SDRESC_TOP|SDRESC_BOTTOM)
#define SDRESC_ALL   0x00FF

#define SDRHORZALIGN_CENTER   0x0000
#define SDRHORZALIGN_LEFT     0x0001
#define SDRHORZALIGN_RIGHT    0x0002
#define SDRHORZALIGN_DONTCARE 0x0010
#define SDRVERTALIGN_CENTER   0x0000
#define SDRVERTALIGN_TOP      0x0100
#define SDRVERTALIGN_BOTTOM   0x0200
#define SDRVERTALIGN_DONTCARE 0x1000

class SVX_DLLPUBLIC SdrGluePoint {
	// Bezugspunkt ist SdrObject::GetSnapRect().Center()
	// bNoPercent=FALSE: Position ist -5000..5000 (1/100)% bzw. 0..10000 (je nach Align)
	// bNoPercent=sal_True : Position ist in log Einh, rel zum Bezugspunkt
	Point    aPos;
	sal_uInt16   nEscDir;
	sal_uInt16   nId;
	sal_uInt16   nAlign;
	FASTBOOL bNoPercent:1;
	FASTBOOL bReallyAbsolute:1; // Temporaer zu setzen fuer Transformationen am Bezugsobjekt
	FASTBOOL bUserDefined:1; // #i38892#
public:
	SdrGluePoint(): nEscDir(SDRESC_SMART),nId(0),nAlign(0) { bNoPercent=sal_False; bReallyAbsolute=sal_False; bUserDefined=sal_True; }
	SdrGluePoint(const Point& rNewPos, FASTBOOL bNewPercent=sal_True, sal_uInt16 nNewAlign=0): aPos(rNewPos),nEscDir(SDRESC_SMART),nId(0),nAlign(nNewAlign) { bNoPercent=!bNewPercent; bReallyAbsolute=sal_False; bUserDefined=sal_True; }
	bool operator==(const SdrGluePoint& rCmpGP) const   { return aPos==rCmpGP.aPos && nEscDir==rCmpGP.nEscDir && nId==rCmpGP.nId && nAlign==rCmpGP.nAlign && bNoPercent==rCmpGP.bNoPercent && bReallyAbsolute==rCmpGP.bReallyAbsolute && bUserDefined==rCmpGP.bUserDefined; }
	bool operator!=(const SdrGluePoint& rCmpGP) const   { return !operator==(rCmpGP); }
	const Point& GetPos() const                             { return aPos; }
	void         SetPos(const Point& rNewPos)               { aPos=rNewPos; }
	sal_uInt16       GetEscDir() const                          { return nEscDir; }
	void         SetEscDir(sal_uInt16 nNewEsc)                  { nEscDir=nNewEsc; }
	sal_uInt16       GetId() const                              { return nId; }
	void         SetId(sal_uInt16 nNewId)                       { nId=nNewId; }
	bool         IsPercent() const                          { return !bNoPercent; }
	void         SetPercent(FASTBOOL bOn)                   { bNoPercent=!bOn; }
	// Temporaer zu setzen fuer Transformationen am Bezugsobjekt
	FASTBOOL     IsReallyAbsolute() const                   { return bReallyAbsolute; }
	void         SetReallyAbsolute(FASTBOOL bOn, const SdrObject& rObj);

	// #i38892#
	FASTBOOL     IsUserDefined() const                   { return bUserDefined; }
	void		 SetUserDefined(FASTBOOL bNew)			 { bUserDefined = bNew; }

	sal_uInt16       GetAlign() const                           { return nAlign; }
	void         SetAlign(sal_uInt16 nAlg)                      { nAlign=nAlg; }
	sal_uInt16       GetHorzAlign() const                       { return nAlign&0x00FF; }
	void         SetHorzAlign(sal_uInt16 nAlg)                  { nAlign=(nAlign&0xFF00)|(nAlg&0x00FF); }
	sal_uInt16       GetVertAlign() const                       { return nAlign&0xFF00; }
	void         SetVertAlign(sal_uInt16 nAlg)                  { nAlign=(nAlign&0x00FF)|(nAlg&0xFF00); }
	void         Draw(OutputDevice& rOut, const SdrObject* pObj) const;
	FASTBOOL     IsHit(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj) const;
	void         Invalidate(Window& rWin, const SdrObject* pObj) const;
	Point        GetAbsolutePos(const SdrObject& rObj) const;
	void         SetAbsolutePos(const Point& rNewPos, const SdrObject& rObj);
	long         GetAlignAngle() const;
	void         SetAlignAngle(long nWink);
	long         EscDirToAngle(sal_uInt16 nEsc) const;
	sal_uInt16       EscAngleToDir(long nWink) const;
	void         Rotate(const Point& rRef, long nWink, double sn, double cs, const SdrObject* pObj);
	void         Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj);
	void         Mirror(const Point& rRef1, const Point& rRef2, long nWink, const SdrObject* pObj);
	void         Shear (const Point& rRef, long nWink, double tn, FASTBOOL bVShear, const SdrObject* pObj);
};

#define SDRGLUEPOINT_NOTFOUND 0xFFFF

class SVX_DLLPUBLIC SdrGluePointList {
	Container aList;
protected:
	SdrGluePoint* GetObject(sal_uInt16 i) const { return (SdrGluePoint*)(aList.GetObject(i)); }
public:
	SdrGluePointList(): aList(1024,4,4) {}
	SdrGluePointList(const SdrGluePointList& rSrcList): aList(1024,4,4)     { *this=rSrcList; }
	~SdrGluePointList()                                                     { Clear(); }
	void                Clear();
	void                operator=(const SdrGluePointList& rSrcList);
	sal_uInt16              GetCount() const                                    { return sal_uInt16(aList.Count()); }
	// Beim Insert wird dem Objekt (also dem GluePoint) automatisch eine Id zugewiesen.
	// ReturnCode ist der Index des neuen GluePoints in der Liste
	sal_uInt16              Insert(const SdrGluePoint& rGP);
	void                Delete(sal_uInt16 nPos)                                 { delete (SdrGluePoint*)aList.Remove(nPos); }
	SdrGluePoint&       operator[](sal_uInt16 nPos)                             { return *GetObject(nPos); }
	const SdrGluePoint& operator[](sal_uInt16 nPos) const                       { return *GetObject(nPos); }
	sal_uInt16              FindGluePoint(sal_uInt16 nId) const;
	sal_uInt16              HitTest(const Point& rPnt, const OutputDevice& rOut, const SdrObject* pObj, FASTBOOL bBack=sal_False, FASTBOOL bNext=sal_False, sal_uInt16 nId0=0) const;
	void                Invalidate(Window& rWin, const SdrObject* pObj) const;
	// Temporaer zu setzen fuer Transformationen am Bezugsobjekt
	void                SetReallyAbsolute(FASTBOOL bOn, const SdrObject& rObj);
	void                Rotate(const Point& rRef, long nWink, double sn, double cs, const SdrObject* pObj);
	void                Mirror(const Point& rRef1, const Point& rRef2, const SdrObject* pObj);
	void                Mirror(const Point& rRef1, const Point& rRef2, long nWink, const SdrObject* pObj);
	void                Shear (const Point& rRef, long nWink, double tn, FASTBOOL bVShear, const SdrObject* pObj);
};


////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDGLUE_HXX

