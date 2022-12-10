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

#include <svl/srchitem.hxx>

#include <unotools/searchopt.hxx>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <svl/memberid.hrc>
#include <i18npool/mslangid.hxx>

using namespace utl;
using namespace com::sun::star::beans;
using namespace com::sun::star::i18n;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;

#define CFG_ROOT_NODE       "Office.Common/SearchOptions"

#define SRCH_PARAMS         11
#define SRCH_PARA_OPTIONS   "Options"
#define SRCH_PARA_FAMILY    "Family"
#define SRCH_PARA_COMMAND   "Command"
#define SRCH_PARA_CELLTYPE  "CellType"
#define SRCH_PARA_APPFLAG   "AppFlag"
#define SRCH_PARA_ROWDIR    "RowDirection"
#define SRCH_PARA_ALLTABLES "AllTables"
#define SRCH_PARA_BACKWARD  "Backward"
#define SRCH_PARA_PATTERN   "Pattern"
#define SRCH_PARA_CONTENT   "Content"
#define SRCH_PARA_ASIANOPT  "AsianOptions"

// STATIC DATA -----------------------------------------------------------

TYPEINIT1_FACTORY(SvxSearchItem, SfxPoolItem, new SvxSearchItem(0));

// -----------------------------------------------------------------------

static Sequence< ::rtl::OUString > lcl_GetNotifyNames()
{
	// names of transliteration relevant properties
	static const char* aTranslitNames[] =
	{
		"IsMatchCase",							//  0
		"Japanese/IsMatchFullHalfWidthForms",	//  1
		"Japanese/IsMatchHiraganaKatakana",		//  2
		"Japanese/IsMatchContractions",			//  3
		"Japanese/IsMatchMinusDashCho-on",		//  4
		"Japanese/IsMatchRepeatCharMarks",		//  5
		"Japanese/IsMatchVariantFormKanji",		//  6
		"Japanese/IsMatchOldKanaForms",			//  7
		"Japanese/IsMatch_DiZi_DuZu",			//  8
		"Japanese/IsMatch_BaVa_HaFa",			//  9
		"Japanese/IsMatch_TsiThiChi_DhiZi",		// 10
		"Japanese/IsMatch_HyuIyu_ByuVyu",		// 11
		"Japanese/IsMatch_SeShe_ZeJe",			// 12
		"Japanese/IsMatch_IaIya",				// 13
		"Japanese/IsMatch_KiKu",				// 14
		"Japanese/IsIgnorePunctuation",			// 15
		"Japanese/IsIgnoreWhitespace",			// 16
		"Japanese/IsIgnoreProlongedSoundMark",	// 17
		"Japanese/IsIgnoreMiddleDot"			// 18
	};

    const int nCount = sizeof( aTranslitNames ) / sizeof( aTranslitNames[0] );
    Sequence< ::rtl::OUString > aNames( nCount );
    ::rtl::OUString* pNames = aNames.getArray();
	for (sal_Int32 i = 0;  i < nCount;  ++i)
        pNames[i] = ::rtl::OUString::createFromAscii( aTranslitNames[i] );

	return aNames;
}

// -----------------------------------------------------------------------
SvxSearchItem::SvxSearchItem( const sal_uInt16 nId ) :

	SfxPoolItem( nId ),
    ConfigItem( ::rtl::OUString::createFromAscii( CFG_ROOT_NODE ) ),

    aSearchOpt      (   SearchAlgorithms_ABSOLUTE,
                        SearchFlags::LEV_RELAXED,
                        ::rtl::OUString(),
                        ::rtl::OUString(),
  						Locale(),
  						2, 2, 2,
  						TransliterationModules_IGNORE_CASE ),
	eFamily			( SFX_STYLE_FAMILY_PARA ),
	nCommand		( 0 ),
	nCellType		( SVX_SEARCHIN_FORMULA ),
	nAppFlag		( SVX_SEARCHAPP_WRITER ),
	bRowDirection	( sal_True ),
	bAllTables		( sal_False ),
	bNotes			( sal_False),
	bBackward		( sal_False ),
	bPattern		( sal_False ),
	bContent		( sal_False ),
	bAsianOptions	( sal_False )
{
	EnableNotification( lcl_GetNotifyNames() );

	SvtSearchOptions aOpt;

	bBackward 		= aOpt.IsBackwards();
	bAsianOptions	= aOpt.IsUseAsianOptions();
	bNotes = aOpt.IsNotes();

	if (aOpt.IsUseRegularExpression())
		aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
	if (aOpt.IsSimilaritySearch())
		aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
	if (aOpt.IsWholeWordsOnly())
		aSearchOpt.searchFlag |= SearchFlags::NORM_WORD_ONLY;

	sal_Int32 &rFlags = aSearchOpt.transliterateFlags;

	if (!aOpt.IsMatchCase())
		rFlags |= TransliterationModules_IGNORE_CASE;
    if ( aOpt.IsMatchFullHalfWidthForms())
		rFlags |= TransliterationModules_IGNORE_WIDTH;
    if ( bAsianOptions )
    {
        if ( aOpt.IsMatchHiraganaKatakana())
            rFlags |= TransliterationModules_IGNORE_KANA;
        if ( aOpt.IsMatchContractions())
            rFlags |= TransliterationModules_ignoreSize_ja_JP;
        if ( aOpt.IsMatchMinusDashChoon())
            rFlags |= TransliterationModules_ignoreMinusSign_ja_JP;
        if ( aOpt.IsMatchRepeatCharMarks())
            rFlags |= TransliterationModules_ignoreIterationMark_ja_JP;
        if ( aOpt.IsMatchVariantFormKanji())
            rFlags |= TransliterationModules_ignoreTraditionalKanji_ja_JP;
        if ( aOpt.IsMatchOldKanaForms())
            rFlags |= TransliterationModules_ignoreTraditionalKana_ja_JP;
        if ( aOpt.IsMatchDiziDuzu())
            rFlags |= TransliterationModules_ignoreZiZu_ja_JP;
        if ( aOpt.IsMatchBavaHafa())
            rFlags |= TransliterationModules_ignoreBaFa_ja_JP;
        if ( aOpt.IsMatchTsithichiDhizi())
            rFlags |= TransliterationModules_ignoreTiJi_ja_JP;
        if ( aOpt.IsMatchHyuiyuByuvyu())
            rFlags |= TransliterationModules_ignoreHyuByu_ja_JP;
        if ( aOpt.IsMatchSesheZeje())
            rFlags |= TransliterationModules_ignoreSeZe_ja_JP;
        if ( aOpt.IsMatchIaiya())
            rFlags |= TransliterationModules_ignoreIandEfollowedByYa_ja_JP;
        if ( aOpt.IsMatchKiku())
            rFlags |= TransliterationModules_ignoreKiKuFollowedBySa_ja_JP;
        if ( aOpt.IsIgnorePunctuation())
            rFlags |= TransliterationModules_ignoreSeparator_ja_JP;
        if ( aOpt.IsIgnoreWhitespace())
            rFlags |= TransliterationModules_ignoreSpace_ja_JP;
        if ( aOpt.IsIgnoreProlongedSoundMark())
            rFlags |= TransliterationModules_ignoreProlongedSoundMark_ja_JP;
        if ( aOpt.IsIgnoreMiddleDot())
            rFlags |= TransliterationModules_ignoreMiddleDot_ja_JP;
    }

}

// -----------------------------------------------------------------------

SvxSearchItem::SvxSearchItem( const SvxSearchItem& rItem ) :

	SfxPoolItem	( rItem ),
    ConfigItem( ::rtl::OUString::createFromAscii( CFG_ROOT_NODE ) ),

	aSearchOpt		( rItem.aSearchOpt ),
	eFamily			( rItem.eFamily ),
	nCommand		( rItem.nCommand ),
	nCellType		( rItem.nCellType ),
	nAppFlag		( rItem.nAppFlag ),
	bRowDirection	( rItem.bRowDirection ),
	bAllTables		( rItem.bAllTables ),
	bNotes			( rItem.bNotes),
	bBackward		( rItem.bBackward ),
	bPattern		( rItem.bPattern ),
	bContent		( rItem.bContent ),
	bAsianOptions	( rItem.bAsianOptions )
{
	EnableNotification( lcl_GetNotifyNames() );
}

// -----------------------------------------------------------------------

SvxSearchItem::~SvxSearchItem()
{
}

// -----------------------------------------------------------------------
SfxPoolItem* SvxSearchItem::Clone( SfxItemPool *) const
{
	return new SvxSearchItem(*this);
}

// -----------------------------------------------------------------------

//! used below
static sal_Bool operator == ( const SearchOptions& rItem1, const SearchOptions& rItem2 )
{
	return rItem1.algorithmType 		== rItem2.algorithmType	&&
		   rItem1.searchFlag 			== rItem2.searchFlag	&&
		   rItem1.searchString 			== rItem2.searchString	&&
		   rItem1.replaceString 		== rItem2.replaceString	&&
		   //rItem1.Locale 				== rItem2.Locale		&&
		   rItem1.changedChars 			== rItem2.changedChars	&&
		   rItem1.deletedChars 			== rItem2.deletedChars	&&
		   rItem1.insertedChars 		== rItem2.insertedChars	&&
		   rItem1.transliterateFlags	== rItem2.transliterateFlags;
}


int SvxSearchItem::operator==( const SfxPoolItem& rItem ) const
{
	DBG_ASSERT( SfxPoolItem::operator==( rItem ), "unequal which or type" );
	const SvxSearchItem &rSItem = (SvxSearchItem &) rItem;
	return ( nCommand 		== rSItem.nCommand )		&&
		   ( bBackward 		== rSItem.bBackward )		&&
		   ( bPattern 		== rSItem.bPattern )		&&
		   ( bContent 		== rSItem.bContent )		&&
		   ( eFamily 		== rSItem.eFamily )			&&
		   ( bRowDirection 	== rSItem.bRowDirection )	&&
		   ( bAllTables 	== rSItem.bAllTables )		&&
		   ( nCellType 		== rSItem.nCellType )		&&
		   ( nAppFlag 		== rSItem.nAppFlag )		&&
		   ( bAsianOptions	== rSItem.bAsianOptions )	&&
		   ( aSearchOpt     == rSItem.aSearchOpt )		&&
		   ( bNotes			== rSItem.bNotes );
}


//------------------------------------------------------------------------

SfxItemPresentation SvxSearchItem::GetPresentation
(
	SfxItemPresentation ,
	SfxMapUnit			,
	SfxMapUnit			,
	XubString& 			,
    const IntlWrapper *
)	const
{
	return SFX_ITEM_PRESENTATION_NONE;
}

void SvxSearchItem::GetFromDescriptor( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor >& rDescr )
{
	SetSearchString( rDescr->getSearchString() );
	::com::sun::star::uno::Any aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchWords") ) );
	sal_Bool bTemp = false;
	aAny >>= bTemp ;
	SetWordOnly( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchCaseSensitive") ) );
	aAny >>= bTemp ;
	SetExact( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchBackwards") ) );
	aAny >>= bTemp ;
	SetBackward( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchInSelection") ) );
	aAny >>= bTemp ;
	SetSelection( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchRegularExpression") ) );
	aAny >>= bTemp ;
	SetRegExp( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarity") ) );
	aAny >>= bTemp ;
	SetLevenshtein( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityRelax") ) );
	aAny >>= bTemp ;
	SetLEVRelaxed( bTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityExchange") ) );
	sal_Int16 nTemp = 0;
	aAny >>= nTemp ;
	SetLEVOther( nTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityRemove") ) );
	aAny >>= nTemp ;
	SetLEVShorter( nTemp );
	aAny = rDescr->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityAdd") ) );
	aAny >>= nTemp ;
	SetLEVLonger( nTemp );
}

void SvxSearchItem::SetToDescriptor( ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > & rDescr )
{
	rDescr->setSearchString( GetSearchString() );
	::com::sun::star::uno::Any aAny;
	aAny <<= GetWordOnly() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchWords") ), aAny );
	aAny <<= GetExact() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchCaseSensitive") ), aAny );
	aAny <<= GetBackward() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchBackwards") ), aAny );
	aAny <<= GetSelection() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchInSelection") ), aAny );
	aAny <<= GetRegExp() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchRegularExpression") ), aAny );
	aAny <<= IsLevenshtein() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarity") ), aAny );
	aAny <<= IsLEVRelaxed() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityRelax") ), aAny );
	aAny <<= GetLEVOther() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityExchange") ), aAny );
	aAny <<= GetLEVShorter() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityRemove") ), aAny );
	aAny <<= GetLEVLonger() ;
	rDescr->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("SearchSimilarityAdd") ), aAny );
}


void SvxSearchItem::Notify( const Sequence< ::rtl::OUString > & )
{
	// applies transliteration changes in the configuration database
	// to the current SvxSearchItem
	SetTransliterationFlags( SvtSearchOptions().GetTransliterationFlags() );
}

void SvxSearchItem::Commit()
{
}

void SvxSearchItem::SetMatchFullHalfWidthForms( sal_Bool bVal )
{
    if (bVal)
		aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_WIDTH;
	else
		aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_WIDTH;
}


void SvxSearchItem::SetWordOnly( sal_Bool bVal )
{
	if (bVal)
		aSearchOpt.searchFlag |=  SearchFlags::NORM_WORD_ONLY;
	else
		aSearchOpt.searchFlag &= ~SearchFlags::NORM_WORD_ONLY;
}


void SvxSearchItem::SetExact( sal_Bool bVal )
{
	if (!bVal)
		aSearchOpt.transliterateFlags |=  TransliterationModules_IGNORE_CASE;
	else
		aSearchOpt.transliterateFlags &= ~TransliterationModules_IGNORE_CASE;
}


void SvxSearchItem::SetSelection( sal_Bool bVal )
{
	if (bVal)
	{
		aSearchOpt.searchFlag |=  (SearchFlags::REG_NOT_BEGINOFLINE |
								   SearchFlags::REG_NOT_ENDOFLINE);
	}
	else
	{
		aSearchOpt.searchFlag &= ~(SearchFlags::REG_NOT_BEGINOFLINE |
								   SearchFlags::REG_NOT_ENDOFLINE);
	}
}


void SvxSearchItem::SetRegExp( sal_Bool bVal )
{
    if ( bVal )
		aSearchOpt.algorithmType = SearchAlgorithms_REGEXP;
    else if ( SearchAlgorithms_REGEXP == aSearchOpt.algorithmType )
		aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
}


void SvxSearchItem::SetLEVRelaxed( sal_Bool bVal )
{
	if (bVal)
		aSearchOpt.searchFlag |=  SearchFlags::LEV_RELAXED;
	else
		aSearchOpt.searchFlag &= ~SearchFlags::LEV_RELAXED;
}


void SvxSearchItem::SetLevenshtein( sal_Bool bVal )
{
    if ( bVal )
		aSearchOpt.algorithmType = SearchAlgorithms_APPROXIMATE;
    else if ( SearchAlgorithms_APPROXIMATE == aSearchOpt.algorithmType )
        aSearchOpt.algorithmType = SearchAlgorithms_ABSOLUTE;
}


void SvxSearchItem::SetTransliterationFlags( sal_Int32 nFlags )
{
	aSearchOpt.transliterateFlags = nFlags;
}

sal_Bool SvxSearchItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId ) const
{
    nMemberId &= ~CONVERT_TWIPS;
    switch ( nMemberId )
	{
        case 0 :
        {
            Sequence< PropertyValue > aSeq( SRCH_PARAMS );
            aSeq[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_OPTIONS ) );
            aSeq[0].Value <<= aSearchOpt;
            aSeq[1].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_FAMILY ));
            aSeq[1].Value <<= sal_Int16( eFamily );
            aSeq[2].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_COMMAND ));
            aSeq[2].Value <<= nCommand;
            aSeq[3].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_CELLTYPE ));
            aSeq[3].Value <<= nCellType;
            aSeq[4].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_APPFLAG ));
            aSeq[4].Value <<= nAppFlag;
            aSeq[5].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_ROWDIR ));
            aSeq[5].Value <<= bRowDirection;
            aSeq[6].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_ALLTABLES ));
            aSeq[6].Value <<= bAllTables;
            aSeq[7].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_BACKWARD ));
            aSeq[7].Value <<= bBackward;
            aSeq[8].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_PATTERN ));
            aSeq[8].Value <<= bPattern;
            aSeq[9].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_CONTENT ));
            aSeq[9].Value <<= bContent;
            aSeq[10].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( SRCH_PARA_ASIANOPT ));
            aSeq[10].Value <<= bAsianOptions;
            rVal <<= aSeq;
        }
        break;
        case MID_SEARCH_COMMAND:
            rVal <<= (sal_Int16) nCommand; break;
        case MID_SEARCH_STYLEFAMILY:
            rVal <<= (sal_Int16) eFamily; break;
        case MID_SEARCH_CELLTYPE:
            rVal <<= (sal_Int32) nCellType; break;
        case MID_SEARCH_ROWDIRECTION:
            rVal <<= (sal_Bool) bRowDirection; break;
        case MID_SEARCH_ALLTABLES:
            rVal <<= (sal_Bool) bAllTables; break;
        case MID_SEARCH_BACKWARD:
            rVal <<= (sal_Bool) bBackward; break;
        case MID_SEARCH_PATTERN:
            rVal <<= (sal_Bool) bPattern; break;
        case MID_SEARCH_CONTENT:
            rVal <<= (sal_Bool) bContent; break;
        case MID_SEARCH_ASIANOPTIONS:
            rVal <<= (sal_Bool) bAsianOptions; break;
        case MID_SEARCH_ALGORITHMTYPE:
            rVal <<= (sal_Int16) aSearchOpt.algorithmType; break;
        case MID_SEARCH_FLAGS:
            rVal <<= aSearchOpt.searchFlag; break;
        case MID_SEARCH_SEARCHSTRING:
            rVal <<= aSearchOpt.searchString; break;
        case MID_SEARCH_REPLACESTRING:
            rVal <<= aSearchOpt.replaceString; break;
        case MID_SEARCH_CHANGEDCHARS:
            rVal <<= aSearchOpt.changedChars; break;
        case MID_SEARCH_DELETEDCHARS:
            rVal <<= aSearchOpt.deletedChars; break;
        case MID_SEARCH_INSERTEDCHARS:
            rVal <<= aSearchOpt.insertedChars; break;
        case MID_SEARCH_TRANSLITERATEFLAGS:
            rVal <<= aSearchOpt.transliterateFlags; break;
        case MID_SEARCH_LOCALE:
        {
            sal_Int16 nLocale;
            if (aSearchOpt.Locale.Language.getLength() || aSearchOpt.Locale.Country.getLength() )
                nLocale = MsLangId::convertLocaleToLanguage( aSearchOpt.Locale );
			else
                nLocale = LANGUAGE_NONE;
            rVal <<= nLocale;
            break;
        }

        default:
            DBG_ERRORFILE( "SvxSearchItem::QueryValue(): Unknown MemberId" );
            return sal_False;
	}

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool SvxSearchItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId )
{
    nMemberId &= ~CONVERT_TWIPS;
    sal_Bool bRet = sal_False;
    sal_Int32 nInt = 0;
    switch ( nMemberId )
	{
        case 0 :
        {
            Sequence< PropertyValue > aSeq;
            if ( ( rVal >>= aSeq ) && ( aSeq.getLength() == SRCH_PARAMS ) )
            {
                sal_Int16 nConvertedCount( 0 );
                for ( sal_Int32 i = 0; i < aSeq.getLength(); ++i )
                {
                    if ( aSeq[i].Name.equalsAscii( SRCH_PARA_OPTIONS ) )
                    {
                        if ( ( aSeq[i].Value >>= aSearchOpt ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_FAMILY ) )
                    {
                        sal_uInt16 nTemp( 0 );
                        if ( ( aSeq[i].Value >>= nTemp ) == sal_True )
                        {
                            eFamily = SfxStyleFamily( nTemp );
                            ++nConvertedCount;
                        }
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_COMMAND ) )
                    {
                        if ( ( aSeq[i].Value >>= nCommand ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_CELLTYPE ) )
                    {
                        if ( ( aSeq[i].Value >>= nCellType ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_APPFLAG ) )
                    {
                        if ( ( aSeq[i].Value >>= nAppFlag ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_ROWDIR ) )
                    {
                        if ( ( aSeq[i].Value >>= bRowDirection ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_ALLTABLES ) )
                    {
                        if ( ( aSeq[i].Value >>= bAllTables ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_BACKWARD ) )
                    {
                        if ( ( aSeq[i].Value >>= bBackward ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_PATTERN ) )
                    {
                        if ( ( aSeq[i].Value >>= bPattern ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_CONTENT ) )
                    {
                        if ( ( aSeq[i].Value >>= bContent ) == sal_True )
                            ++nConvertedCount;
                    }
                    else if ( aSeq[i].Name.equalsAscii( SRCH_PARA_ASIANOPT ) )
                    {
                        if ( ( aSeq[i].Value >>= bAsianOptions ) == sal_True )
                            ++nConvertedCount;
                    }
                }

                bRet = ( nConvertedCount == SRCH_PARAMS );
            }
            break;
        }
        case MID_SEARCH_COMMAND:
            bRet = (rVal >>= nInt); nCommand = (sal_uInt16) nInt; break;
        case MID_SEARCH_STYLEFAMILY:
            bRet = (rVal >>= nInt); eFamily =  (SfxStyleFamily) (sal_Int16) nInt; break;
        case MID_SEARCH_CELLTYPE:
            bRet = (rVal >>= nInt); nCellType = (sal_uInt16) nInt; break;
        case MID_SEARCH_ROWDIRECTION:
            bRet = (rVal >>= bRowDirection); break;
        case MID_SEARCH_ALLTABLES:
            bRet = (rVal >>= bAllTables); break;
        case MID_SEARCH_BACKWARD:
            bRet = (rVal >>= bBackward); break;
        case MID_SEARCH_PATTERN:
            bRet = (rVal >>= bPattern); break;
        case MID_SEARCH_CONTENT:
            bRet = (rVal >>= bContent); break;
        case MID_SEARCH_ASIANOPTIONS:
            bRet = (rVal >>= bAsianOptions); break;
        case MID_SEARCH_ALGORITHMTYPE:
            bRet = (rVal >>= nInt); aSearchOpt.algorithmType = (SearchAlgorithms)(sal_Int16)nInt; break;
        case MID_SEARCH_FLAGS:
            bRet = (rVal >>= aSearchOpt.searchFlag); break;
        case MID_SEARCH_SEARCHSTRING:
            bRet = (rVal >>= aSearchOpt.searchString); break;
        case MID_SEARCH_REPLACESTRING:
            bRet = (rVal >>= aSearchOpt.replaceString); break;
        case MID_SEARCH_CHANGEDCHARS:
            bRet = (rVal >>= aSearchOpt.changedChars); break;
        case MID_SEARCH_DELETEDCHARS:
            bRet = (rVal >>= aSearchOpt.deletedChars); break;
        case MID_SEARCH_INSERTEDCHARS:
            bRet = (rVal >>= aSearchOpt.insertedChars); break;
        case MID_SEARCH_TRANSLITERATEFLAGS:
            bRet = (rVal >>= aSearchOpt.transliterateFlags); break;
        case MID_SEARCH_LOCALE:
        {
            bRet = (rVal >>= nInt);
            if ( bRet )
            {
                if ( nInt == LANGUAGE_NONE )
                {
                    aSearchOpt.Locale = ::com::sun::star::lang::Locale();
                }
                else
                {
                    MsLangId::convertLanguageToLocale( (sal_Int16) nInt, aSearchOpt.Locale );
                }
            }
            break;
        }
		default:
            DBG_ERROR( "Unknown MemberId" );
	}

    return bRet;
}


