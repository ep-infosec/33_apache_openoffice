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
#include "precompiled_svl.hxx"

// include ---------------------------------------------------------------

#define _DATETIMEITEM_CXX
#include <svl/dateitem.hxx>
#include <svl/svldata.hxx>
#include <svl/svl.hrc>

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/lang/Locale.hpp>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxDateTimeItem)


// -----------------------------------------------------------------------

TYPEINIT1(SfxDateTimeItem, SfxPoolItem);

// -----------------------------------------------------------------------

SfxDateTimeItem::SfxDateTimeItem( sal_uInt16 which ) :
	SfxPoolItem( which )
{
	DBG_CTOR(SfxDateTimeItem, 0);
}

// -----------------------------------------------------------------------

SfxDateTimeItem::SfxDateTimeItem( sal_uInt16 which, const DateTime& rDT ) :
	SfxPoolItem( which ),
	aDateTime( rDT )

{
	DBG_CTOR(SfxDateTimeItem, 0);
}

// -----------------------------------------------------------------------

SfxDateTimeItem::SfxDateTimeItem( const SfxDateTimeItem& rItem ) :
	SfxPoolItem( rItem ),
	aDateTime( rItem.aDateTime )
{
	DBG_CTOR(SfxDateTimeItem, 0);
}

// -----------------------------------------------------------------------

int SfxDateTimeItem::operator==( const SfxPoolItem& rItem ) const
{
	DBG_CHKTHIS(SfxDateTimeItem, 0);
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );
	return ( ( (SfxDateTimeItem&)rItem ).aDateTime == aDateTime );
}

// -----------------------------------------------------------------------

int SfxDateTimeItem::Compare( const SfxPoolItem& rItem ) const
{
	DBG_CHKTHIS(SfxDateTimeItem, 0);
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal type" );

	// da X.Compare( Y ) am String einem Compare( Y, X ) entspricht,
	// vergleichen wir hier Y mit X
	if ( ( (const SfxDateTimeItem&)rItem ).aDateTime < aDateTime )
		return -1;
	else if ( ( (const SfxDateTimeItem&)rItem ).aDateTime == aDateTime )
		return 0;
	else
		return 1;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeItem::Create( SvStream& rStream, sal_uInt16 ) const
{
	DBG_CHKTHIS(SfxDateTimeItem, 0);
	sal_uInt32 nDate = 0;
	sal_Int32 nTime = 0;
	rStream >> nDate;
	rStream >> nTime;
	DateTime aDT(nDate, nTime);
	return new SfxDateTimeItem( Which(), aDT );
}

// -----------------------------------------------------------------------

SvStream& SfxDateTimeItem::Store( SvStream& rStream, sal_uInt16 ) const
{
	DBG_CHKTHIS(SfxDateTimeItem, 0);
	rStream << aDateTime.GetDate();
	rStream << aDateTime.GetTime();
	return rStream;
}

// -----------------------------------------------------------------------

SfxPoolItem* SfxDateTimeItem::Clone( SfxItemPool* ) const
{
	DBG_CHKTHIS(SfxDateTimeItem, 0);
	return new SfxDateTimeItem( *this );
}

// -----------------------------------------------------------------------

SfxItemPresentation SfxDateTimeItem::GetPresentation
(
	SfxItemPresentation 	/*ePresentation*/,
	SfxMapUnit				/*eCoreMetric*/,
	SfxMapUnit				/*ePresentationMetric*/,
	XubString& 				rText,
    const IntlWrapper *   pIntlWrapper
)	const
{
	DBG_CHKTHIS(SfxDateTimeItem, 0);
	if (aDateTime.IsValid())
        if (pIntlWrapper)
		{
            rText = pIntlWrapper->getLocaleData()->getDate(aDateTime);
			rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
            rText += pIntlWrapper->getLocaleData()->getTime(aDateTime);
		}
		else
		{
			DBG_WARNING("SfxDateTimeItem::GetPresentation():"
                         " Using default en_US IntlWrapper");
            const IntlWrapper aIntlWrapper(
                ::comphelper::getProcessServiceFactory(), LANGUAGE_ENGLISH_US );
            rText = aIntlWrapper.getLocaleData()->getDate(aDateTime);
			rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
            rText += aIntlWrapper.getLocaleData()->getTime(aDateTime);
		}
	else
		rText.Erase();
	return SFX_ITEM_PRESENTATION_NAMELESS;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool SfxDateTimeItem::PutValue( const com::sun::star::uno::Any& rVal,
						   		sal_uInt8 nMemberId )
{
	nMemberId &= ~CONVERT_TWIPS;
	com::sun::star::util::DateTime aValue;
	if ( rVal >>= aValue )
	{
		aDateTime = DateTime( Date( aValue.Day,
			  						aValue.Month,
			  						aValue.Year ),
							  Time( aValue.Hours,
			  						aValue.Minutes,
			  						aValue.Seconds,
			  						aValue.HundredthSeconds ) );
		return sal_True;
	}

	DBG_ERROR( "SfxDateTimeItem::PutValue - Wrong type!" );
	return sal_False;
}

//----------------------------------------------------------------------------
// virtual
sal_Bool SfxDateTimeItem::QueryValue( com::sun::star::uno::Any& rVal,
							 	  sal_uInt8 nMemberId ) const
{
	nMemberId &= ~CONVERT_TWIPS;
	com::sun::star::util::DateTime aValue( aDateTime.Get100Sec(),
    									   aDateTime.GetSec(),
   										   aDateTime.GetMin(),
    									   aDateTime.GetHour(),
    									   aDateTime.GetDay(),
    									   aDateTime.GetMonth(),
    									   aDateTime.GetYear() );
	rVal <<= aValue;
	return sal_True;
}

// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------

TYPEINIT1(SfxColumnDateTimeItem, SfxDateTimeItem);


SfxColumnDateTimeItem::SfxColumnDateTimeItem( sal_uInt16 which ) :
	SfxDateTimeItem( which )
{}

SfxColumnDateTimeItem::SfxColumnDateTimeItem( sal_uInt16 which, const DateTime& rDT ) :
	SfxDateTimeItem( which, rDT )
{}

SfxColumnDateTimeItem::SfxColumnDateTimeItem( const SfxDateTimeItem& rCpy ) :
	SfxDateTimeItem( rCpy )
{}

SfxPoolItem* SfxColumnDateTimeItem::Clone( SfxItemPool* ) const
{
	return new SfxColumnDateTimeItem( *this );
}

SfxItemPresentation SfxColumnDateTimeItem::GetPresentation
(
	SfxItemPresentation 	/*ePresentation*/,
	SfxMapUnit				/*eCoreMetric*/,
	SfxMapUnit				/*ePresentationMetric*/,
	XubString& 				rText,
    const IntlWrapper *   pIntlWrapper
)	const
{
    DBG_ASSERT(pIntlWrapper,
			   "SfxColumnDateTimeItem::GetPresentation():"
                " Using default en_US IntlWrapper");

    ::com::sun::star::lang::Locale aLocale;
	if (GetDateTime() == DateTime(Date(1, 2, 3), Time(3, 2, 1)))
	{
		rText = String(SvtSimpleResId(STR_COLUM_DT_AUTO,
                                pIntlWrapper ?
                                 pIntlWrapper->getLocale() :
								 aLocale));
	}
    else if (pIntlWrapper)
	{
        rText = pIntlWrapper->getLocaleData()->getDate(GetDateTime());
		rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += pIntlWrapper->getLocaleData()->getTime(GetDateTime());
	}
	else
	{
        const IntlWrapper aIntlWrapper(
            ::comphelper::getProcessServiceFactory(), LANGUAGE_ENGLISH_US );
        rText = aIntlWrapper.getLocaleData()->getDate(GetDateTime());
		rText.AppendAscii(RTL_CONSTASCII_STRINGPARAM(", "));
        rText += aIntlWrapper.getLocaleData()->getTime(GetDateTime());
	}
	return SFX_ITEM_PRESENTATION_NAMELESS;
}



