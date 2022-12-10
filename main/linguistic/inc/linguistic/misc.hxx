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



#ifndef _LINGUISTIC_MISC_HXX_
#define _LINGUISTIC_MISC_HXX_

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

#include <uno/lbnames.h>			// CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx>	// helper for implementations
#include <unotools/pathoptions.hxx>
#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <unotools/charclass.hxx>
#include <osl/thread.h>
#include <osl/mutex.hxx>
#include <linguistic/lngdllapi.h>

namespace com { namespace sun { namespace star { namespace beans {
	class XPropertySet;
	class XFastPropertySet;
}}}}

namespace com { namespace sun { namespace star { namespace frame {
	class XDesktop;
}}}}

class LocaleDataWrapper;

///////////////////////////////////////////////////////////////////////////
#define SN_GRAMMARCHECKER           "com.sun.star.linguistic2.Proofreader"
#define SN_GRAMMARCHECKINGITERATOR  "com.sun.star.linguistic2.ProofreadingIterator"
#define SN_SPELLCHECKER				"com.sun.star.linguistic2.SpellChecker"
#define SN_HYPHENATOR				"com.sun.star.linguistic2.Hyphenator"
#define SN_THESAURUS				"com.sun.star.linguistic2.Thesaurus"
#define SN_LINGU_SERVCICE_MANAGER	"com.sun.star.linguistic2.LinguServiceManager"
#define SN_LINGU_PROPERTIES			"com.sun.star.linguistic2.LinguProperties"
#define SN_DICTIONARY_LIST			"com.sun.star.linguistic2.DictionaryList"
#define SN_OTHER_LINGU				"com.sun.star.linguistic2.OtherLingu"
#define SN_DESKTOP					"com.sun.star.frame.Desktop"


namespace linguistic
{

// ascii to OUString conversion
#define A2OU(x) ::rtl::OUString::createFromAscii( x )

/// Flags to be used with the multi-path related functions
/// @see GetDictionaryPaths, GetLinguisticPaths
#define PATH_FLAG_INTERNAL  0x01
#define PATH_FLAG_USER      0x02
#define PATH_FLAG_WRITABLE  0x04
#define PATH_FLAG_ALL       (PATH_FLAG_INTERNAL | PATH_FLAG_USER | PATH_FLAG_WRITABLE)


// AddEntryToDic return values
#define DIC_ERR_NONE        0
#define DIC_ERR_FULL        1
#define DIC_ERR_READONLY    2
#define DIC_ERR_UNKNOWN     3
#define DIC_ERR_NOT_EXISTS  4

///////////////////////////////////////////////////////////////////////////

LNG_DLLPUBLIC ::osl::Mutex& GetLinguMutex();

LocaleDataWrapper & GetLocaleDataWrapper( sal_Int16 nLang );

///////////////////////////////////////////////////////////////////////////

rtl_TextEncoding GetTextEncoding( sal_Int16 nLanguage );

inline ::rtl::OUString BS2OU(const ByteString &rText, rtl_TextEncoding nEnc)
{
    return ::rtl::OUString( rText.GetBuffer(), rText.Len(), nEnc );
}

inline ByteString OU2BS(const ::rtl::OUString &rText, rtl_TextEncoding nEnc)
{
    return ByteString( rText.getStr(), nEnc );
}

rtl::OUString StripTrailingChars( rtl::OUString &rTxt, sal_Unicode cChar );

///////////////////////////////////////////////////////////////////////////

sal_Int32 LevDistance( const rtl::OUString &rTxt1, const rtl::OUString &rTxt2 );

///////////////////////////////////////////////////////////////////////////

::com::sun::star::lang::Locale
	CreateLocale( LanguageType eLang );

LNG_DLLPUBLIC LanguageType
 	LocaleToLanguage( const ::com::sun::star::lang::Locale& rLocale );

::com::sun::star::lang::Locale&
	LanguageToLocale( ::com::sun::star::lang::Locale& rLocale, LanguageType eLang );

::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >
    LangSeqToLocaleSeq( const ::com::sun::star::uno::Sequence< sal_Int16 > &rLangSeq );

::com::sun::star::uno::Sequence< sal_Int16 >
    LocaleSeqToLangSeq( ::com::sun::star::uno::Sequence<
        ::com::sun::star::lang::Locale > &rLocaleSeq );

///////////////////////////////////////////////////////////////////////////

// checks if file pointed to by rURL is readonly
// and may also check return if such a file exists or not
sal_Bool    IsReadOnly( const String &rURL, sal_Bool *pbExist = 0 );

// checks if a file with the given URL exists
sal_Bool    FileExists( const String &rURL );

#ifdef TL_OUTDATED
// returns complete file URL for given filename that is to be searched in
// the specified path
String  GetFileURL( SvtPathOptions::Pathes ePath, const String &rFileName );

String  GetModulePath( SvtPathOptions::Pathes ePath, sal_Bool bAddAccessDelim = sal_True );
#endif

///////////////////////////////////////////////////////////////////////////

::rtl::OUString     GetDictionaryWriteablePath();
::com::sun::star::uno::Sequence< ::rtl::OUString > GetDictionaryPaths( sal_Int16 nPathFlags = PATH_FLAG_ALL );
::com::sun::star::uno::Sequence< ::rtl::OUString > GetLinguisticPaths( sal_Int16 nPathFlags = PATH_FLAG_ALL );

/// @returns an URL for a new and writable dictionary rDicName.
///     The URL will point to the path given by 'GetDictionaryWriteablePath'
LNG_DLLPUBLIC String  GetWritableDictionaryURL( const String &rDicName );

// looks for the specified file in the list of paths.
// In case of multiple occurrences only the first found is returned.
String     SearchFileInPaths( const String &rFile, const ::com::sun::star::uno::Sequence< ::rtl::OUString > &rPaths );


///////////////////////////////////////////////////////////////////////////

LNG_DLLPUBLIC sal_Int32 GetPosInWordToCheck( const rtl::OUString &rTxt, sal_Int32 nPos );

::com::sun::star::uno::Reference<
	::com::sun::star::linguistic2::XHyphenatedWord >
			RebuildHyphensAndControlChars( const rtl::OUString &rOrigWord,
				::com::sun::star::uno::Reference<
					::com::sun::star::linguistic2::XHyphenatedWord > &rxHyphWord );

///////////////////////////////////////////////////////////////////////////

LNG_DLLPUBLIC sal_Bool        IsUpper( const String &rText, xub_StrLen nPos, xub_StrLen nLen, sal_Int16 nLanguage );
LNG_DLLPUBLIC sal_Bool        IsLower( const String &rText, xub_StrLen nPos, xub_StrLen nLen, sal_Int16 nLanguage );

inline sal_Bool        IsUpper( const String &rText, sal_Int16 nLanguage )     { return IsUpper( rText, 0, rText.Len(), nLanguage ); }
inline sal_Bool        IsLower( const String &rText, sal_Int16 nLanguage )     { return IsLower( rText, 0, rText.Len(), nLanguage ); }

String      ToLower( const String &rText, sal_Int16 nLanguage );
String      ToUpper( const String &rText, sal_Int16 nLanguage );
String      ToTitle( const String &rText, sal_Int16 nLanguage );
sal_Unicode	ToLower( const sal_Unicode cChar, sal_Int16 nLanguage );
sal_Unicode	ToUpper( const sal_Unicode cChar, sal_Int16 nLanguage );
LNG_DLLPUBLIC sal_Bool		HasDigits( const ::rtl::OUString &rText );
LNG_DLLPUBLIC sal_Bool		IsNumeric( const String &rText );

///////////////////////////////////////////////////////////////////////////

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > GetOneInstanceService( const char *pServiceName );
LNG_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > GetLinguProperties();
::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSearchableDictionaryList > GetSearchableDictionaryList();
::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryList > GetDictionaryList();
::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary > GetIgnoreAllList();

///////////////////////////////////////////////////////////////////////////

sal_Bool IsUseDicList( const ::com::sun::star::beans::PropertyValues &rProperties,
		const ::com::sun::star::uno::Reference<
			::com::sun::star::beans::XPropertySet > &rxPropSet );

sal_Bool IsIgnoreControlChars( const ::com::sun::star::beans::PropertyValues &rProperties,
		const ::com::sun::star::uno::Reference<
			::com::sun::star::beans::XPropertySet > &rxPropSet );

::com::sun::star::uno::Reference<
	::com::sun::star::linguistic2::XDictionaryEntry >
		SearchDicList(
            const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryList >& rDicList,
			const ::rtl::OUString& rWord, sal_Int16 nLanguage,
			sal_Bool bSearchPosDics, sal_Bool bSearchSpellEntry );

LNG_DLLPUBLIC sal_uInt8 AddEntryToDic(
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionary >  &rxDic,
    const ::rtl::OUString &rWord, sal_Bool bIsNeg,
    const ::rtl::OUString &rRplcTxt, sal_Int16 nRplcLang,
    sal_Bool bStripDot = sal_True );

LNG_DLLPUBLIC sal_Bool SaveDictionaries( const ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XDictionaryList > &xDicList );

///////////////////////////////////////////////////////////////////////////
//
// AppExitLstnr:
// virtual base class that calls it AtExit function when the application
// (ie the Desktop) is about to terminate
//

class AppExitListener :
	public cppu::WeakImplHelper1
	<
		::com::sun::star::frame::XTerminateListener
	>
{
	::com::sun::star::uno::Reference<
		::com::sun::star::frame::XDesktop >		xDesktop;

public:
	AppExitListener();
	virtual ~AppExitListener();

	virtual	void	AtExit() = 0;

	void			Activate();
	void			Deactivate();

	// XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);
};

///////////////////////////////////////////////////////////////////////////

}	// namespace linguistic

#endif

