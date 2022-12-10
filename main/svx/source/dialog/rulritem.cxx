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
#include "precompiled_svx.hxx"
#include <tools/string.hxx>

#include <svx/dialogs.hrc>
#include "svx/rulritem.hxx"
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/frame/status/LeftRightMargin.hpp>
#include <com/sun/star/frame/status/UpperLowerMargin.hpp>

//------------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxPagePosSizeItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxLongLRSpaceItem, SfxPoolItem);
TYPEINIT1_AUTOFACTORY(SvxLongULSpaceItem, SfxPoolItem);
TYPEINIT1(SvxColumnItem, SfxPoolItem);
TYPEINIT1(SvxObjectItem, SfxPoolItem);

//------------------------------------------------------------------------

int SvxLongLRSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
	return SfxPoolItem::operator==(rCmp) &&
		lLeft==((const SvxLongLRSpaceItem &)rCmp).lLeft &&
		lRight==((const SvxLongLRSpaceItem &)rCmp).lRight;
}


//------------------------------------------------------------------------

String SvxLongLRSpaceItem::GetValueText() const
{
	return String();
}

#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

sal_Bool SvxLongLRSpaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
	nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
	{
        case 0:
        {
            ::com::sun::star::frame::status::LeftRightMargin aLeftRightMargin;
            aLeftRightMargin.Left = bConvert ? TWIP_TO_MM100( lLeft ) : lLeft;
            aLeftRightMargin.Right = bConvert ? TWIP_TO_MM100( lRight ) : lRight;
            rVal <<= aLeftRightMargin;
            return sal_True;
        }

        case MID_LEFT: nVal = lLeft; break;
        case MID_RIGHT: nVal = lRight; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
	}

    if ( bConvert )
        nVal = TWIP_TO_MM100( nVal );

    rVal <<= nVal;
    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool SvxLongLRSpaceItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
	nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        ::com::sun::star::frame::status::LeftRightMargin aLeftRightMargin;
        if ( rVal >>= aLeftRightMargin )
        {
            lLeft    = bConvert ? MM100_TO_TWIP( aLeftRightMargin.Left ) : aLeftRightMargin.Left;
            lRight   = bConvert ? MM100_TO_TWIP( aLeftRightMargin.Right ) : aLeftRightMargin.Right;
            return sal_True;
        }
    }
    else if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = MM100_TO_TWIP( nVal );

        switch( nMemberId )
        {
            case MID_LEFT: lLeft = nVal; break;
            case MID_RIGHT: lRight = nVal; break;
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }

        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLongLRSpaceItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             /*rText*/, const IntlWrapper *
)	const
{
    
	return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxLongLRSpaceItem::Clone(SfxItemPool *) const
{
	return new SvxLongLRSpaceItem(*this);
}

//------------------------------------------------------------------------

SvxLongLRSpaceItem::SvxLongLRSpaceItem(long lL, long lR, sal_uInt16 nId)
	: SfxPoolItem(nId),
	lLeft(lL),
	lRight(lR)
{}

//------------------------------------------------------------------------

SvxLongLRSpaceItem::SvxLongLRSpaceItem() : 
    SfxPoolItem( 0 ),
    lLeft( 0 ),
    lRight( 0 )
{}

//------------------------------------------------------------------------

SvxLongLRSpaceItem::SvxLongLRSpaceItem(const SvxLongLRSpaceItem &rCpy)
	: SfxPoolItem(rCpy),
	lLeft(rCpy.lLeft),
	lRight(rCpy.lRight)
		{}

//------------------------------------------------------------------------

int SvxLongULSpaceItem::operator==( const SfxPoolItem& rCmp) const
{
	return SfxPoolItem::operator==(rCmp) &&
		lLeft==((const SvxLongULSpaceItem &)rCmp).lLeft &&
		lRight==((const SvxLongULSpaceItem &)rCmp).lRight;
}


//------------------------------------------------------------------------

String SvxLongULSpaceItem::GetValueText() const
{
	return String();
}

sal_Bool SvxLongULSpaceItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
	nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch( nMemberId )
	{
        case 0:
        {
            ::com::sun::star::frame::status::UpperLowerMargin aUpperLowerMargin;
            aUpperLowerMargin.Upper = bConvert ? TWIP_TO_MM100( lLeft ) : lLeft;
            aUpperLowerMargin.Lower = bConvert ? TWIP_TO_MM100( lRight ) : lRight;
            rVal <<= aUpperLowerMargin;
            return sal_True;
        }

        case MID_UPPER: nVal = lLeft; break;
        case MID_LOWER: nVal = lRight; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
	}

    if ( bConvert )
        nVal = TWIP_TO_MM100( nVal );

    rVal <<= nVal;
    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool SvxLongULSpaceItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
	sal_Bool bConvert = 0!=(nMemberId&CONVERT_TWIPS);
	nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        ::com::sun::star::frame::status::UpperLowerMargin aUpperLowerMargin;
        if ( rVal >>= aUpperLowerMargin )
        {
            lLeft    = bConvert ? MM100_TO_TWIP( aUpperLowerMargin.Upper ) : aUpperLowerMargin.Upper;
            lRight   = bConvert ? MM100_TO_TWIP( aUpperLowerMargin.Lower ) : aUpperLowerMargin.Lower;
            return sal_True;
        }
    }
    else if ( rVal >>= nVal )
    {
        if ( bConvert )
            nVal = MM100_TO_TWIP( nVal );

        switch( nMemberId )
        {
            case MID_UPPER: lLeft = nVal; break;
            case MID_LOWER: lRight = nVal; break;
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }

        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxLongULSpaceItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             /*rText*/, const IntlWrapper *
)   const
{
	return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxLongULSpaceItem::Clone(SfxItemPool *) const
{
	return new SvxLongULSpaceItem(*this);
}

//------------------------------------------------------------------------

SvxLongULSpaceItem::SvxLongULSpaceItem(long lL, long lR, sal_uInt16 nId)
	: SfxPoolItem(nId),
	lLeft(lL),
	lRight(lR)
{}

//------------------------------------------------------------------------

SvxLongULSpaceItem::SvxLongULSpaceItem(const SvxLongULSpaceItem &rCpy)
	: SfxPoolItem(rCpy),
	lLeft(rCpy.lLeft),
	lRight(rCpy.lRight)
		{}

//------------------------------------------------------------------------

SvxLongULSpaceItem::SvxLongULSpaceItem() : 
    SfxPoolItem( 0 ),
    lLeft( 0 ),
    lRight( 0 )
{}

//------------------------------------------------------------------------

int SvxPagePosSizeItem::operator==( const SfxPoolItem& rCmp) const
{
	return SfxPoolItem::operator==(rCmp) &&
		aPos == ((const SvxPagePosSizeItem &)rCmp).aPos &&
			lWidth == ((const SvxPagePosSizeItem &)rCmp).lWidth  &&
			lHeight == ((const SvxPagePosSizeItem &)rCmp).lHeight;
}

sal_Bool SvxPagePosSizeItem::QueryValue( ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
	nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal;
    switch ( nMemberId )
    {
        case 0 :
        {
            com::sun::star::awt::Rectangle aPagePosSize;
            aPagePosSize.X = aPos.X();
            aPagePosSize.Y = aPos.Y();
            aPagePosSize.Width = lWidth;
            aPagePosSize.Height = lHeight;
            rVal <<= aPagePosSize;
            return sal_True;
        }
    
        case MID_X: nVal = aPos.X(); break;
        case MID_Y: nVal = aPos.Y(); break;
        case MID_WIDTH: nVal = lWidth; break;
        case MID_HEIGHT: nVal = lHeight; break;

        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    rVal <<= nVal;
    return sal_True;
}

sal_Bool SvxPagePosSizeItem::PutValue( const ::com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
	nMemberId &= ~CONVERT_TWIPS;

    sal_Int32 nVal = 0;
    if ( nMemberId == 0 )
    {
        com::sun::star::awt::Rectangle aPagePosSize;
        if ( rVal >>= aPagePosSize )
        {
            aPos.X() = aPagePosSize.X;
            aPos.Y() = aPagePosSize.Y;
            lWidth   = aPagePosSize.Width;
            lHeight  = aPagePosSize.Height;
            return sal_True;
        }
        else
            return sal_False;
    }
    else if ( rVal >>= nVal )
    {
        switch ( nMemberId )
        {
            case MID_X: aPos.X() = nVal; break;
            case MID_Y: aPos.Y() = nVal; break;
            case MID_WIDTH: lWidth = nVal; break;
            case MID_HEIGHT: lHeight = nVal; break;
            
            default: DBG_ERROR("Wrong MemberId!"); return sal_False;
        }

        return sal_True;
    }

    return sal_False;
}

//------------------------------------------------------------------------

String SvxPagePosSizeItem::GetValueText() const
{
	return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxPagePosSizeItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             /*rText*/, const IntlWrapper *
)   const
{
	return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxPagePosSizeItem::Clone(SfxItemPool *) const
{
	return new SvxPagePosSizeItem(*this);
}

//------------------------------------------------------------------------

SvxPagePosSizeItem::SvxPagePosSizeItem(const Point &rP, long lW, long lH)
	: SfxPoolItem(SID_RULER_PAGE_POS),
	aPos(rP),
	lWidth(lW),
	lHeight(lH)
{}

//------------------------------------------------------------------------

SvxPagePosSizeItem::SvxPagePosSizeItem(const SvxPagePosSizeItem &rCpy)
	: SfxPoolItem(rCpy),
	aPos(rCpy.aPos),
	lWidth(rCpy.lWidth),
	lHeight(rCpy.lHeight)
		{}

//------------------------------------------------------------------------

SvxPagePosSizeItem::SvxPagePosSizeItem()
	: SfxPoolItem( 0 ),
	aPos( 0, 0 ),
	lWidth( 0 ),
	lHeight( 0 )
{}

//------------------------------------------------------------------------

void SvxColumnItem::DeleteAndDestroyColumns()
{
	for( sal_uInt16 i = aColumns.Count(); i>0; )
	{
		SvxColumnDescription *pTmp = (SvxColumnDescription *)aColumns[--i];
		aColumns.Remove( i );
		delete pTmp;
	}
}

//------------------------------------------------------------------------

int SvxColumnItem::operator==(const SfxPoolItem& rCmp) const
{
	if(!SfxPoolItem::operator==(rCmp) ||
	   nActColumn != ((const SvxColumnItem&)rCmp).nActColumn ||
	   nLeft != ((const SvxColumnItem&)rCmp).nLeft ||
	   nRight != ((const SvxColumnItem&)rCmp).nRight ||
	   bTable != ((const SvxColumnItem&)rCmp).bTable ||
	   Count() != ((const SvxColumnItem&)rCmp).Count())
		return sal_False;

	const sal_uInt16 nCount = ((const SvxColumnItem&)rCmp).Count();
	for(sal_uInt16 i = 0; i < nCount;++i) {
#if OSL_DEBUG_LEVEL > 1
		SvxColumnDescription *p1, *p2;
        p1 = (SvxColumnDescription *)aColumns[i];
        p2 = (SvxColumnDescription *)((const SvxColumnItem&)rCmp).aColumns[i];
#endif
		if( (*this)[i] != ((const SvxColumnItem&)rCmp)[i] )
			return sal_False;
	}
	return sal_True;
}

//------------------------------------------------------------------------

String SvxColumnItem::GetValueText() const
{
	return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxColumnItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             /*rText*/, const IntlWrapper *
)	const
{
	return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxColumnItem::Clone( SfxItemPool * ) const
{
	return new SvxColumnItem(*this);
}

//------------------------------------------------------------------------

SvxColumnItem::SvxColumnItem( sal_uInt16 nAct ) :

	SfxPoolItem( SID_RULER_BORDERS ),

    nLeft       ( 0 ),
	nRight		( 0 ),
    nActColumn  ( nAct ),
    bTable      ( sal_False ),
    bOrtho      (sal_True )

{
}

//------------------------------------------------------------------------

SvxColumnItem::SvxColumnItem( sal_uInt16 nActCol, sal_uInt16 left, sal_uInt16 right ) :

	SfxPoolItem( SID_RULER_BORDERS ),

    nLeft       ( left ),
	nRight		( right ),
    nActColumn  ( nActCol ),
    bTable      ( sal_True ),
    bOrtho      ( sal_True )
{
}

//------------------------------------------------------------------------

SvxColumnItem::SvxColumnItem( const SvxColumnItem& rCopy ) :

	SfxPoolItem( rCopy ),

      aColumns  ( (sal_uInt8)rCopy.Count() ),
      nLeft     ( rCopy.nLeft ),
	  nRight	( rCopy.nRight ),
      nActColumn( rCopy.nActColumn ),
      bTable    ( rCopy.bTable ),
      bOrtho    ( rCopy.bOrtho )

{
	const sal_uInt16 nCount = rCopy.Count();

	for ( sal_uInt16 i = 0; i < nCount; ++i )
		Append( rCopy[i] );
}

//------------------------------------------------------------------------

SvxColumnItem::~SvxColumnItem()
{
	DeleteAndDestroyColumns();
}

//------------------------------------------------------------------------

const SvxColumnItem &SvxColumnItem::operator=(const SvxColumnItem &rCopy)
{
	nLeft = rCopy.nLeft;
	nRight = rCopy.nRight;
	bTable = rCopy.bTable;
	nActColumn = rCopy.nActColumn;
	DeleteAndDestroyColumns();
	const sal_uInt16 nCount = rCopy.Count();
	for(sal_uInt16 i = 0; i < nCount;++i)
		Insert(rCopy[i], i);
	return *this;
}

//------------------------------------------------------------------------

sal_Bool SvxColumnItem::CalcOrtho() const
{
	const sal_uInt16 nCount = Count();
	DBG_ASSERT(nCount >= 2, "keine Spalten");
	if(nCount < 2)
		return sal_False;

	long nColWidth = (*this)[0].GetWidth();
	for(sal_uInt16 i = 1; i < nCount; ++i) {
		if( (*this)[i].GetWidth() != nColWidth)
			return sal_False;
	}
	//!! Breite Trenner
	return sal_True;
}

//------------------------------------------------------------------------

long SvxColumnItem::GetVisibleRight() const
{
	sal_uInt16 nIdx = 0;

	for ( sal_uInt16 i = 0; i < nActColumn; ++i )
	{
		if ( (*this)[i].bVisible )
			++nIdx;
	}
	return (*this)[nIdx].nEnd;
}

sal_Bool SvxColumnItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
	nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_COLUMNARRAY:
        {
            return sal_False;
        }
        case MID_RIGHT: rVal <<= nRight; break;
        case MID_LEFT: rVal <<= nLeft; break;
        case MID_ORTHO: rVal <<= (sal_Bool) bOrtho; break;
        case MID_ACTUAL: rVal <<= (sal_Int32) nActColumn; break;
        case MID_TABLE: rVal <<= (sal_Bool) bTable; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

sal_Bool SvxColumnItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
	nMemberId &= ~CONVERT_TWIPS;
    sal_Int32 nVal = 0;
    switch ( nMemberId )
    {
        case MID_COLUMNARRAY:
        {
            return sal_False;
        }
        case MID_RIGHT: rVal >>= nRight; break;
        case MID_LEFT: rVal >>= nLeft; break;
        case MID_ORTHO: rVal >>= nVal; bOrtho = (sal_Bool) nVal; break;
        case MID_ACTUAL: rVal >>= nVal; nActColumn = (sal_uInt16) nVal; break;
        case MID_TABLE: rVal >>= nVal; bTable = (sal_Bool) nVal; break;
        default: DBG_ERROR("Wrong MemberId!"); return sal_False;
    }

    return sal_True;
}

//------------------------------------------------------------------------

int SvxObjectItem::operator==( const SfxPoolItem& rCmp ) const
{
	return SfxPoolItem::operator==(rCmp) &&
	   nStartX == ((const SvxObjectItem&)rCmp).nStartX &&
	   nEndX == ((const SvxObjectItem&)rCmp).nEndX &&
	   nStartY == ((const SvxObjectItem&)rCmp).nStartY &&
	   nEndY == ((const SvxObjectItem&)rCmp).nEndY &&
	   bLimits == ((const SvxObjectItem&)rCmp).bLimits;
}

//------------------------------------------------------------------------

String SvxObjectItem::GetValueText() const
{
	return String();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxObjectItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    String&             /*rText*/, const IntlWrapper *
)   const
{
	return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

SfxPoolItem* SvxObjectItem::Clone(SfxItemPool *) const
{
	return new SvxObjectItem(*this);
}

//------------------------------------------------------------------------

SvxObjectItem::SvxObjectItem( long nSX, long nEX,
							  long nSY, long nEY, sal_Bool limits ) :

	SfxPoolItem( SID_RULER_OBJECT ),

	nStartX	( nSX ),
	nEndX	( nEX ),
	nStartY	( nSY ),
	nEndY	( nEY ),
	bLimits	( limits )

{
}

//------------------------------------------------------------------------

SvxObjectItem::SvxObjectItem( const SvxObjectItem& rCopy ) :

	SfxPoolItem( rCopy ),

	nStartX	( rCopy.nStartX ),
	nEndX	( rCopy.nEndX ),
	nStartY	( rCopy.nStartY ),
	nEndY	( rCopy.nEndY ),
	bLimits	( rCopy.bLimits )

{
}

sal_Bool SvxObjectItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
    {
        case MID_START_X : rVal <<= nStartX; break;
        case MID_START_Y : rVal <<= nStartY; break;
        case MID_END_X : rVal <<= nEndX; break;
        case MID_END_Y : rVal <<= nEndY; break;
        case MID_LIMIT : rVal <<= bLimits; break;
        default:
            DBG_ERROR( "Wrong MemberId" );
            return sal_False;
    }

    return sal_True;
}

sal_Bool SvxObjectItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet=sal_False;
    switch ( nMemberId )
    {
        case MID_START_X : bRet = (rVal >>= nStartX); break;
        case MID_START_Y : bRet = (rVal >>= nStartY); break;
        case MID_END_X : bRet = (rVal >>= nEndX); break;
        case MID_END_Y : bRet = (rVal >>= nEndY); break;
        case MID_LIMIT : bRet = (rVal >>= bLimits); break;
        default: DBG_ERROR( "Wrong MemberId" );
    }

    return bRet;
}

