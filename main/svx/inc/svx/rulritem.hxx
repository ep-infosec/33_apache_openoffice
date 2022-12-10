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


#ifndef _SVX_RULRITEM_HXX
#define _SVX_RULRITEM_HXX

// include ---------------------------------------------------------------


#include <tools/gen.hxx>
#include <svl/poolitem.hxx>
#include "svx/svxdllapi.h"

// class SvxLongLRSpaceItem ----------------------------------------------

class SVX_DLLPUBLIC SvxLongLRSpaceItem : public SfxPoolItem
{
	long	lLeft;         // nLeft oder der neg. Erstzeileneinzug
	long	lRight;        // der unproblematische rechte Rand

  protected:

	virtual int 			 operator==( const SfxPoolItem& ) const;
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

	virtual String			 GetValueText() const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;

private:
	SVX_DLLPRIVATE const SvxLongLRSpaceItem& operator=(const SvxLongLRSpaceItem &); // n.i.

public:
	TYPEINFO();
	SvxLongLRSpaceItem(long lLeft, long lRight, sal_uInt16 nId);
	SvxLongLRSpaceItem(const SvxLongLRSpaceItem &);
    SvxLongLRSpaceItem();

	long    GetLeft() const { return lLeft; }
	long    GetRight() const { return lRight; }
	void    SetLeft(long lArgLeft) {lLeft=lArgLeft;}
	void    SetRight(long lArgRight) {lRight=lArgRight;}
};

// class SvxLongULSpaceItem ----------------------------------------------

class SVX_DLLPUBLIC SvxLongULSpaceItem : public SfxPoolItem
{
	long	lLeft;         // nLeft oder der neg. Erstzeileneinzug
	long	lRight;        // der unproblematische rechte Rand

  protected:

	virtual int 			 operator==( const SfxPoolItem& ) const;
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

	virtual String			 GetValueText() const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;

private:
	SVX_DLLPRIVATE const SvxLongULSpaceItem& operator=(const SvxLongULSpaceItem &); // n.i.

public:
	TYPEINFO();
	SvxLongULSpaceItem(long lUpper, long lLower, sal_uInt16 nId);
	SvxLongULSpaceItem(const SvxLongULSpaceItem &);
    SvxLongULSpaceItem();

	long    GetUpper() const { return lLeft; }
	long    GetLower() const { return lRight; }
	void    SetUpper(long lArgLeft) {lLeft=lArgLeft;}
	void    SetLower(long lArgRight) {lRight=lArgRight;}
};

// class SvxPagePosSizeItem ----------------------------------------------

class SVX_DLLPUBLIC SvxPagePosSizeItem : public SfxPoolItem
{
	Point aPos;
	long lWidth;
	long lHeight;
protected:
	virtual int 			 operator==( const SfxPoolItem& ) const;
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

	virtual String			 GetValueText() const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;

private:
	SVX_DLLPRIVATE const SvxPagePosSizeItem& operator=(const SvxPagePosSizeItem &); // n.i.
public:
	TYPEINFO();
	SvxPagePosSizeItem(const Point &rPos, long lWidth, long lHeight);
	SvxPagePosSizeItem(const SvxPagePosSizeItem &);
    SvxPagePosSizeItem();

	const Point &GetPos() const { return aPos; }
	long    GetWidth() const { return lWidth; }
	long    GetHeight() const { return lHeight; }
};

// struct SvxColumnDescription -------------------------------------------

struct SvxColumnDescription
{
    long nStart;                    /* Spaltenbeginn */
    long nEnd;                      /* Spaltenende */
	sal_Bool   bVisible;				   /* Sichtbarkeit */

    long nEndMin;         //min. possible position of end
    long nEndMax;         //max. possible position of end

	SvxColumnDescription():
        nStart(0), nEnd(0), bVisible(sal_True), nEndMin(0), nEndMax(0) {}

	SvxColumnDescription(const SvxColumnDescription &rCopy) :
        nStart(rCopy.nStart), nEnd(rCopy.nEnd),
        bVisible(rCopy.bVisible),
        nEndMin(rCopy.nEndMin), nEndMax(rCopy.nEndMax)
         {}

    SvxColumnDescription(long start, long end, sal_Bool bVis = sal_True):
        nStart(start), nEnd(end), 
        bVisible(bVis), 
        nEndMin(0), nEndMax(0) {}
    
    SvxColumnDescription(long start, long end, 
                        long endMin, long endMax, sal_Bool bVis = sal_True):
        nStart(start), nEnd(end), 
        bVisible(bVis), 
        nEndMin(endMin), nEndMax(endMax)
         {}
    
    int operator==(const SvxColumnDescription &rCmp) const {
		return nStart == rCmp.nStart &&
            bVisible == rCmp.bVisible &&
            nEnd == rCmp.nEnd &&
            nEndMin == rCmp.nEndMin &&
                nEndMax == rCmp.nEndMax;
    }
	int operator!=(const SvxColumnDescription &rCmp) const {
		return !operator==(rCmp);
	}
    long GetWidth() const { return nEnd - nStart; }
};

// class SvxColumnItem ---------------------------------------------------

typedef SvPtrarr SvxColumns;

class SVX_DLLPUBLIC SvxColumnItem : public SfxPoolItem
{
	SvxColumns aColumns;// Spaltenarray
	long	nLeft,		// Linker Rand bei Tabelle
		   nRight;		// Rechter Rand bei Tabelle; bei Spalten immer gleich
						// zum umgebenden Rahmen
	sal_uInt16 nActColumn;	// die aktuelle Spalte
	sal_Bool    bTable;		// Tabelle?
	sal_Bool	bOrtho;     // Gleichverteilte Spalten

	void DeleteAndDestroyColumns();

protected:
	virtual int 			 operator==( const SfxPoolItem& ) const;

	virtual String			 GetValueText() const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
public:
	TYPEINFO();
	// rechter Rand des umgebenden Rahmens
	// nLeft, nRight jeweils der Abstand zum umgebenden Rahmen
	SvxColumnItem(sal_uInt16 nAct = 0); // Spalten
	SvxColumnItem(sal_uInt16 nActCol,
				  sal_uInt16 nLeft, sal_uInt16 nRight = 0);	// Tabelle mit Raendern
	SvxColumnItem(const	SvxColumnItem &);
	~SvxColumnItem();

	const SvxColumnItem &operator=(const SvxColumnItem &);

	sal_uInt16 Count() const { return aColumns.Count(); }
	SvxColumnDescription &operator[](sal_uInt16 i)
		{ return *(SvxColumnDescription*)aColumns[i]; }
	const SvxColumnDescription &operator[](sal_uInt16 i) const
		{ return *(SvxColumnDescription*)aColumns[i]; }
	void Insert(const SvxColumnDescription &rDesc, sal_uInt16 nPos) {
		SvxColumnDescription* pDesc = new SvxColumnDescription(rDesc);
		aColumns.Insert(pDesc, nPos);
	}
	void   Append(const SvxColumnDescription &rDesc) { Insert(rDesc, Count()); }
	void   SetLeft(long left) { nLeft = left; }
	void   SetRight(long right) { nRight = right; }
	void   SetActColumn(sal_uInt16 nCol) { nActColumn = nCol; }

	sal_uInt16 GetActColumn() const { return nActColumn; }
	sal_Bool   IsFirstAct() const { return nActColumn == 0; }
	sal_Bool   IsLastAct() const { return nActColumn == Count()-1; }
	long GetLeft() { return nLeft; }
	long GetRight() { return nRight; }

	sal_Bool   IsTable() const { return bTable; }

	sal_Bool   CalcOrtho() const;
	void   SetOrtho(sal_Bool bVal) { bOrtho = bVal; }
	sal_Bool   IsOrtho () const { return sal_False ; }

	sal_Bool IsConsistent() const  { return nActColumn < aColumns.Count(); }
	long   GetVisibleRight() const;// rechter sichtbare Rand der aktuellen Spalte
};

// class SvxObjectItem ---------------------------------------------------

class SVX_DLLPUBLIC SvxObjectItem : public SfxPoolItem
{
private:
	long   nStartX;					   /* Beginn in X-Richtung */
	long   nEndX;					   /* Ende in X-Richtung */
	long   nStartY;                    /* Beginn in Y-Richtung */
	long   nEndY;                      /* Ende in Y-Richtung */
	sal_Bool   bLimits;					   /* Grenzwertkontrolle durch die Applikation */
protected:
	virtual int 			 operator==( const SfxPoolItem& ) const;

	virtual String			 GetValueText() const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*	 Clone( SfxItemPool *pPool = 0 ) const;
	virtual	sal_Bool        	 QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			 PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
private:
	SVX_DLLPRIVATE const SvxObjectItem &operator=(const SvxObjectItem &); // n.i.
public:
	TYPEINFO();
	SvxObjectItem(long nStartX, long nEndX,
				  long nStartY, long nEndY,
				  sal_Bool bLimits = sal_False);
	SvxObjectItem(const SvxObjectItem &);

	sal_Bool   HasLimits() const { return bLimits; }

	long   GetStartX() const { return nStartX; }
	long   GetEndX() const { return nEndX; }
	long   GetStartY() const { return nStartY; }
	long   GetEndY() const { return nEndY; }

	void   SetStartX(long l) { nStartX = l; }
	void   SetEndX(long l) { nEndX = l; }
	void   SetStartY(long l) { nStartY = l; }
	void   SetEndY(long l) { nEndY = l; }
};


#endif

