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



#ifndef _SVDDRGM1_HXX
#define _SVDDRGM1_HXX

#include <svx/xpoly.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svddrgv.hxx>
#include <svx/svddrgmt.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predeclarations

class SdrDragView;
class SdrDragStat;

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMovHdl

class SdrDragMovHdl : public SdrDragMethod
{
private:
	bool					bMirrObjShown;

protected:
    // define nothing, overload to do so
    virtual void createSdrDragEntries();

public:
	TYPEINFO();
	SdrDragMovHdl(SdrDragView& rNewView);

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual void CancelSdrDrag();
	virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragRotate

class SdrDragRotate : public SdrDragMethod
{
private:
	double						nSin;
	double						nCos;
	long						nWink0;
	long						nWink;
	bool    					bRight;

public:
	TYPEINFO();
	SdrDragRotate(SdrDragView& rNewView);

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual Pointer GetSdrDragPointer() const;
	
	virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragShear

class SdrDragShear : public SdrDragMethod
{
private:
	Fraction					aFact;
	long						nWink0;
	long						nWink;
	double						nTan;
	bool    					bVertical;   // Vertikales verzerren
	bool    					bResize;     // Shear mit Resize
	bool    					bUpSideDown; // Beim Shear/Slant gespiegelt
	bool    					bSlant;

public:
	TYPEINFO();
	SdrDragShear(SdrDragView& rNewView,bool bSlant1);

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual Pointer GetSdrDragPointer() const;
	
	virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragMirror

class SdrDragMirror : public SdrDragMethod
{
private:
	Point						aDif;
	long						nWink;
	bool    					bMirrored;
	bool    					bSide0;

	bool ImpCheckSide(const Point& rPnt) const;

public:
	TYPEINFO();
	SdrDragMirror(SdrDragView& rNewView);

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual Pointer GetSdrDragPointer() const;
	
	virtual basegfx::B2DHomMatrix getCurrentTransformation();
    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragGradient

class SdrDragGradient : public SdrDragMethod
{
private:
	// Handles to work on
	SdrHdlGradient*				pIAOHandle;

	// is this for gradient (or for transparence) ?
	unsigned					bIsGradient : 1;

public:
	TYPEINFO();
	SdrDragGradient(SdrDragView& rNewView, bool bGrad = true);

	bool IsGradient() const { return bIsGradient; }

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual Pointer GetSdrDragPointer() const;
	virtual void CancelSdrDrag();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragCrook

class SdrDragCrook : public SdrDragMethod
{
private:
	Rectangle					aMarkRect;
	Point						aMarkCenter;
	Point						aCenter;
	Point						aStart;
	Fraction					aFact;
	Point						aRad;
	bool    					bContortionAllowed;
	bool    					bNoContortionAllowed;
	bool    					bContortion;
	bool    					bResizeAllowed;
	bool    					bResize;
	bool    					bRotateAllowed;
	bool    					bRotate;
	bool    					bVertical;
	bool    					bValid;
	bool    					bLft;
	bool    					bRgt;
	bool    					bUpr;
	bool    					bLwr;
	bool    					bAtCenter;
	long						nWink;
	long						nMarkSize;
	SdrCrookMode				eMode;

	// helpers for applyCurrentTransformationToPolyPolygon
	void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);
	void _MovCrookPoint(Point& rPnt, Point* pC1, Point* pC2);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries();

public:
	TYPEINFO();
	SdrDragCrook(SdrDragView& rNewView);

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual Pointer GetSdrDragPointer() const;

	virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
	virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragDistort

class SdrDragDistort : public SdrDragMethod
{
private:
	Rectangle					aMarkRect;
	XPolygon					aDistortedRect;
	sal_uInt16					nPolyPt;
	bool    					bContortionAllowed;
	bool    					bNoContortionAllowed;
	bool    					bContortion;

	// helper for applyCurrentTransformationToPolyPolygon
	void _MovAllPoints(basegfx::B2DPolyPolygon& rTarget);

protected:
    // needs to add drag geometry to the default
    virtual void createSdrDragEntries();

public:
	TYPEINFO();
	SdrDragDistort(SdrDragView& rNewView);

	virtual void TakeSdrDragComment(String& rStr) const;
	virtual bool BeginSdrDrag();
	virtual void MoveSdrDrag(const Point& rPnt);
	virtual bool EndSdrDrag(bool bCopy);
	virtual Pointer GetSdrDragPointer() const;

    virtual void applyCurrentTransformationToSdrObject(SdrObject& rTarget);
	virtual void applyCurrentTransformationToPolyPolygon(basegfx::B2DPolyPolygon& rTarget);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//   SdrDragCrop

// derive from SdrDragObjOwn to have handles aligned to object when it
// is sheared or rotated
class SdrDragCrop : public SdrDragObjOwn
{
public:
    TYPEINFO();
    SdrDragCrop(SdrDragView& rNewView);

    virtual void TakeSdrDragComment(String& rStr) const;
    virtual bool BeginSdrDrag();
    virtual bool EndSdrDrag(bool bCopy);
    virtual Pointer GetSdrDragPointer() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //_SVDDRGM1_HXX

// eof
