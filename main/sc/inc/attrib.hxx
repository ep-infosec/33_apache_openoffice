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



#ifndef SC_SCATTR_HXX
#define SC_SCATTR_HXX

#include <svl/poolitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include "scdllapi.h"
#include "global.hxx"
#include "address.hxx"

//------------------------------------------------------------------------

										// Flags fuer durch Merge verdeckte Zellen
										// und Control fuer Auto-Filter
#define SC_MF_HOR               0x0001
#define SC_MF_VER               0x0002
#define SC_MF_AUTO              0x0004  /// autofilter arrow
#define SC_MF_BUTTON            0x0008  /// field button for datapilot
#define SC_MF_SCENARIO          0x0010
#define SC_MF_BUTTON_POPUP      0x0020  /// dp button with popup arrow
#define SC_MF_HIDDEN_MEMBER     0x0040  /// dp field button with presence of hidden member 
#define SC_MF_DP_TABLE          0x0080  /// dp table output

#define SC_MF_ALL               0x00FF


class EditTextObject;
class SvxBorderLine;

sal_Bool SC_DLLPUBLIC ScHasPriority( const SvxBorderLine* pThis, const SvxBorderLine* pOther );

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScMergeAttr: public SfxPoolItem
{
	SCsCOL      nColMerge;
	SCsROW      nRowMerge;
public:
				TYPEINFO();
				ScMergeAttr();
				ScMergeAttr( SCsCOL nCol, SCsROW nRow = 0);
				ScMergeAttr( const ScMergeAttr& );
				~ScMergeAttr();

	virtual String          	GetValueText() const;

	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

			SCsCOL          GetColMerge() const {return nColMerge; }
			SCsROW          GetRowMerge() const {return nRowMerge; }

			sal_Bool			IsMerged() const { return nColMerge>1 || nRowMerge>1; }

	inline  ScMergeAttr& operator=(const ScMergeAttr& rMerge)
			{
				nColMerge = rMerge.nColMerge;
				nRowMerge = rMerge.nRowMerge;
				return *this;
			}
};

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScMergeFlagAttr: public SfxInt16Item
{
public:
			ScMergeFlagAttr();
			ScMergeFlagAttr(sal_Int16 nFlags);
			~ScMergeFlagAttr();

	sal_Bool	IsHorOverlapped() const		{ return ( GetValue() & SC_MF_HOR ) != 0;  }
	sal_Bool	IsVerOverlapped() const		{ return ( GetValue() & SC_MF_VER ) != 0;  }
	sal_Bool	IsOverlapped() const		{ return ( GetValue() & ( SC_MF_HOR | SC_MF_VER ) ) != 0; }

	sal_Bool	HasAutoFilter() const		{ return ( GetValue() & SC_MF_AUTO ) != 0; }
	sal_Bool	HasButton() const			{ return ( GetValue() & SC_MF_BUTTON ) != 0; }
    bool    HasDPTable() const          { return ( GetValue() & SC_MF_DP_TABLE ) != 0; }

	sal_Bool	IsScenario() const			{ return ( GetValue() & SC_MF_SCENARIO ) != 0; }
};

//------------------------------------------------------------------------
class SC_DLLPUBLIC ScProtectionAttr: public SfxPoolItem
{
	sal_Bool        bProtection;    // Zelle schuetzen
	sal_Bool        bHideFormula;   // Formel nicht Anzeigen
	sal_Bool        bHideCell;      // Zelle nicht Anzeigen
	sal_Bool        bHidePrint;     // Zelle nicht Ausdrucken
public:
							TYPEINFO();
							ScProtectionAttr();
							ScProtectionAttr(   sal_Bool bProtect,
												sal_Bool bHFormula = sal_False,
												sal_Bool bHCell = sal_False,
												sal_Bool bHPrint = sal_False);
							ScProtectionAttr( const ScProtectionAttr& );
							~ScProtectionAttr();

	virtual String          	GetValueText() const;
	virtual SfxItemPresentation GetPresentation(
									SfxItemPresentation ePres,
									SfxMapUnit eCoreMetric,
									SfxMapUnit ePresMetric,
									String& rText,
                                    const IntlWrapper* pIntl = 0 ) const;

	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

	virtual	sal_Bool			QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

			sal_Bool            GetProtection() const { return bProtection; }
			sal_Bool            SetProtection( sal_Bool bProtect);
			sal_Bool            GetHideFormula() const { return bHideFormula; }
			sal_Bool            SetHideFormula( sal_Bool bHFormula);
			sal_Bool            GetHideCell() const { return bHideCell; }
			sal_Bool            SetHideCell( sal_Bool bHCell);
			sal_Bool            GetHidePrint() const { return bHidePrint; }
			sal_Bool            SetHidePrint( sal_Bool bHPrint);
	inline  ScProtectionAttr& operator=(const ScProtectionAttr& rProtection)
			{
				bProtection = rProtection.bProtection;
				bHideFormula = rProtection.bHideFormula;
				bHideCell = rProtection.bHideCell;
				bHidePrint = rProtection.bHidePrint;
				return *this;
			}
};


//----------------------------------------------------------------------------
// ScRangeItem: verwaltet einen Tabellenbereich

#define SCR_INVALID		0x01
#define SCR_ALLTABS		0x02
#define SCR_TONEWTAB	0x04

class ScRangeItem : public SfxPoolItem
{
public:
			TYPEINFO();

			inline	ScRangeItem( const sal_uInt16 nWhich );
			inline	ScRangeItem( const sal_uInt16   nWhich,
								 const ScRange& rRange,
								 const sal_uInt16 	nNewFlags = 0 );
			inline	ScRangeItem( const ScRangeItem& rCpy );

	inline ScRangeItem& operator=( const ScRangeItem &rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual int 				operator==( const SfxPoolItem& ) const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
												 SfxMapUnit eCoreMetric,
												 SfxMapUnit ePresMetric,
												 String &rText,
                                                 const IntlWrapper* pIntl = 0 ) const;
	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;

	const ScRange&	GetRange() const 				{ return aRange;  }
	void			SetRange( const ScRange& rNew )	{ aRange = rNew; }

	sal_uInt16			GetFlags() const 				{ return nFlags;  }
	void			SetFlags( sal_uInt16 nNew )	 		{ nFlags = nNew; }

private:
	ScRange aRange;
	sal_uInt16	nFlags;
};

inline ScRangeItem::ScRangeItem( const sal_uInt16 nWhichP )
    :   SfxPoolItem( nWhichP ), nFlags( SCR_INVALID ) // == ungueltige Area
{
}

inline ScRangeItem::ScRangeItem( const sal_uInt16   nWhichP,
								 const ScRange& rRange,
								 const sal_uInt16	nNew )
    : SfxPoolItem( nWhichP ), aRange( rRange ), nFlags( nNew )
{
}

inline ScRangeItem::ScRangeItem( const ScRangeItem& rCpy )
	: SfxPoolItem( rCpy.Which() ), aRange( rCpy.aRange ), nFlags( rCpy.nFlags )
{}

inline ScRangeItem& ScRangeItem::operator=( const ScRangeItem &rCpy )
{
	aRange = rCpy.aRange;
	return *this;
}

//----------------------------------------------------------------------------
// ScTableListItem: verwaltet eine Liste von Tabellen
//----------------------------------------------------------------------------
class ScTableListItem : public SfxPoolItem
{
public:
	TYPEINFO();

	inline	ScTableListItem( const sal_uInt16 nWhich );
			ScTableListItem( const ScTableListItem& rCpy );
//UNUSED2008-05  ScTableListItem( const sal_uInt16 nWhich, const List& rList );
			~ScTableListItem();

	ScTableListItem& operator=( const ScTableListItem &rCpy );

	// "pure virtual Methoden" vom SfxPoolItem
	virtual int 				operator==( const SfxPoolItem& ) const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
												 SfxMapUnit eCoreMetric,
												 SfxMapUnit ePresMetric,
												 String &rText,
                                                 const IntlWrapper* pIntl = 0 ) const;
	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;

//UNUSED2009-05 sal_Bool	GetTableList( List& aList ) const;
//UNUSED2009-05 void	SetTableList( const List& aList );

public:
	sal_uInt16  nCount;
	SCTAB*  pTabArr;
};

inline ScTableListItem::ScTableListItem( const sal_uInt16 nWhichP )
    : SfxPoolItem(nWhichP), nCount(0), pTabArr(NULL)
{}

//----------------------------------------------------------------------------
// Seitenformat-Item: Kopf-/Fusszeileninhalte

#define SC_HF_LEFTAREA   1
#define SC_HF_CENTERAREA 2
#define SC_HF_RIGHTAREA  3

class SC_DLLPUBLIC ScPageHFItem : public SfxPoolItem
{
	EditTextObject* pLeftArea;
	EditTextObject* pCenterArea;
	EditTextObject* pRightArea;

public:
				TYPEINFO();
				ScPageHFItem( sal_uInt16 nWhich );
				ScPageHFItem( const ScPageHFItem& rItem );
				~ScPageHFItem();

	virtual String          GetValueText() const;
	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

	virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

	virtual	sal_Bool			QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
	virtual	sal_Bool			PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

	const EditTextObject* GetLeftArea() const		{ return pLeftArea; }
	const EditTextObject* GetCenterArea() const		{ return pCenterArea; }
	const EditTextObject* GetRightArea() const		{ return pRightArea; }

	void SetLeftArea( const EditTextObject& rNew );
	void SetCenterArea( const EditTextObject& rNew );
	void SetRightArea( const EditTextObject& rNew );

	//Set mit Uebereignung der Pointer, nArea siehe defines oben
	void SetArea( EditTextObject *pNew, int nArea );
};


//----------------------------------------------------------------------------
// Seitenformat-Item: Kopf-/Fusszeileninhalte

class SC_DLLPUBLIC ScViewObjectModeItem: public SfxEnumItem
{
public:
				TYPEINFO();

				ScViewObjectModeItem( sal_uInt16 nWhich );
				ScViewObjectModeItem( sal_uInt16 nWhich, ScVObjMode eMode );
				~ScViewObjectModeItem();

	virtual sal_uInt16				GetValueCount() const;
	virtual String				GetValueText( sal_uInt16 nVal ) const;
	virtual SfxPoolItem*		Clone( SfxItemPool *pPool = 0 ) const;
	virtual SfxPoolItem*		Create(SvStream &, sal_uInt16) const;
	virtual sal_uInt16				GetVersion( sal_uInt16 nFileVersion ) const;
	virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
												 SfxMapUnit eCoreMetric,
												 SfxMapUnit ePresMetric,
												 String& rText,
                                                 const IntlWrapper* pIntl = 0 ) const;
};

//----------------------------------------------------------------------------
//

class ScDoubleItem : public SfxPoolItem
{
public:
				TYPEINFO();
				ScDoubleItem( sal_uInt16 nWhich, double nVal=0 );
				ScDoubleItem( const ScDoubleItem& rItem );
				~ScDoubleItem();

	virtual String          GetValueText() const;
	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

	virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

	double GetValue() const		{ return nValue; }

	void SetValue( const double nVal ) { nValue = nVal;}

private:
	double	nValue;
};


// ============================================================================

/** Member ID for "page scale to width" value in QueryValue() and PutValue(). */
const sal_uInt8 SC_MID_PAGE_SCALETO_WIDTH    = 1;
/** Member ID for "page scale to height" value in QueryValue() and PutValue(). */
const sal_uInt8 SC_MID_PAGE_SCALETO_HEIGHT   = 2;


/** Contains the "scale to width/height" attribute in page styles. */
class SC_DLLPUBLIC ScPageScaleToItem : public SfxPoolItem
{
public:
                                TYPEINFO();

    /** Default c'tor sets the width and height to 0. */
    explicit                    ScPageScaleToItem();
    explicit                    ScPageScaleToItem( sal_uInt16 nWidth, sal_uInt16 nHeight );

    virtual                     ~ScPageScaleToItem();

    virtual ScPageScaleToItem*  Clone( SfxItemPool* = 0 ) const;

    virtual int                 operator==( const SfxPoolItem& rCmp ) const;

    inline sal_uInt16           GetWidth() const { return mnWidth; }
    inline sal_uInt16           GetHeight() const { return mnHeight; }
    inline bool                 IsValid() const { return mnWidth || mnHeight; }

    inline void                 SetWidth( sal_uInt16 nWidth ) { mnWidth = nWidth; }
    inline void                 SetHeight( sal_uInt16 nHeight ) { mnHeight = nHeight; }
    inline void                 Set( sal_uInt16 nWidth, sal_uInt16 nHeight )
                                    { mnWidth = nWidth; mnHeight = nHeight; }
    inline void                 SetInvalid() { mnWidth = mnHeight = 0; }

    virtual SfxItemPresentation GetPresentation(
                                    SfxItemPresentation ePresentation,
                                    SfxMapUnit, SfxMapUnit,
                                    XubString& rText,
                                    const IntlWrapper* = 0 ) const;

    virtual sal_Bool                QueryValue( ::com::sun::star::uno::Any& rAny, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool                PutValue( const ::com::sun::star::uno::Any& rAny, sal_uInt8 nMemberId = 0 );

private:
    sal_uInt16                  mnWidth;
    sal_uInt16                  mnHeight;
};

// ============================================================================

#endif

