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
#include "precompiled_unotools.hxx"
#ifndef GCC
#endif

#include <unotools/syslocale.hxx>
#include <tools/string.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <i18npool/mslangid.hxx>
#include <rtl/tencinfo.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>

using namespace osl;
using namespace com::sun::star;


SvtSysLocale_Impl*  SvtSysLocale::pImpl = NULL;
sal_Int32           SvtSysLocale::nRefCount = 0;


class SvtSysLocale_Impl : public utl::ConfigurationListener
{
public:
        SvtSysLocaleOptions     aSysLocaleOptions;
        LocaleDataWrapper*      pLocaleData;
        CharClass*              pCharClass;

                                	SvtSysLocale_Impl();
    virtual                     	~SvtSysLocale_Impl();

    CharClass*                  	GetCharClass();
	virtual void 					ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );
};

// -----------------------------------------------------------------------

SvtSysLocale_Impl::SvtSysLocale_Impl() : pCharClass(NULL)
{
    pLocaleData = new LocaleDataWrapper( ::comphelper::getProcessServiceFactory(), aSysLocaleOptions.GetRealLocale() );

	// listen for further changes
    aSysLocaleOptions.AddListener( this );  
}


SvtSysLocale_Impl::~SvtSysLocale_Impl()
{
    aSysLocaleOptions.RemoveListener( this );
    delete pCharClass;
    delete pLocaleData;
}

CharClass* SvtSysLocale_Impl::GetCharClass()
{
    if ( !pCharClass )
        pCharClass = new CharClass(::comphelper::getProcessServiceFactory(), aSysLocaleOptions.GetRealLocale() );
    return pCharClass;
}

void SvtSysLocale_Impl::ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 nHint )
{
    MutexGuard aGuard( SvtSysLocale::GetMutex() );
	if ( nHint & SYSLOCALEOPTIONS_HINT_LOCALE )
	{
		com::sun::star::lang::Locale aLocale( aSysLocaleOptions.GetRealLocale() );
		pLocaleData->setLocale( aLocale );
		GetCharClass()->setLocale( aLocale );
	}
}

// ====================================================================

SvtSysLocale::SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    if ( !pImpl )
        pImpl = new SvtSysLocale_Impl;
    ++nRefCount;
}


SvtSysLocale::~SvtSysLocale()
{
    MutexGuard aGuard( GetMutex() );
    if ( !--nRefCount )
	{
        delete pImpl;
        pImpl = NULL;
	}
}


// static
Mutex& SvtSysLocale::GetMutex()
{
    static Mutex* pMutex = NULL;
    if( !pMutex )
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            // #i77768# Due to a static reference in the toolkit lib
            // we need a mutex that lives longer than the svl library.
            // Otherwise the dtor would use a destructed mutex!!
            pMutex = new Mutex;
        }
    }
    return *pMutex;
}


const LocaleDataWrapper& SvtSysLocale::GetLocaleData() const
{
    return *(pImpl->pLocaleData);
}


const LocaleDataWrapper* SvtSysLocale::GetLocaleDataPtr() const
{
    return pImpl->pLocaleData;
}


const CharClass& SvtSysLocale::GetCharClass() const
{
    return *(pImpl->GetCharClass());
}


const CharClass* SvtSysLocale::GetCharClassPtr() const
{
    return pImpl->GetCharClass();
}

SvtSysLocaleOptions& SvtSysLocale::GetOptions() const
{
	return pImpl->aSysLocaleOptions;
}

com::sun::star::lang::Locale SvtSysLocale::GetLocale() const
{
	return pImpl->aSysLocaleOptions.GetRealLocale();
}

LanguageType SvtSysLocale::GetLanguage() const
{
	return pImpl->aSysLocaleOptions.GetRealLanguage();
}

com::sun::star::lang::Locale SvtSysLocale::GetUILocale() const
{
	return pImpl->aSysLocaleOptions.GetRealUILocale();
}

LanguageType SvtSysLocale::GetUILanguage() const
{
	return pImpl->aSysLocaleOptions.GetRealUILanguage();
}

//------------------------------------------------------------------------

// static
rtl_TextEncoding SvtSysLocale::GetBestMimeEncoding()
{
    const sal_Char* pCharSet = rtl_getBestMimeCharsetFromTextEncoding(
            gsl_getSystemTextEncoding() );
    if ( !pCharSet )
    {   
        // If the system locale is unknown to us, e.g. LC_ALL=xx, match the UI
        // language if possible.
        ::com::sun::star::lang::Locale aLocale( SvtSysLocale().GetUILocale() );
        rtl_Locale * pLocale = rtl_locale_register( aLocale.Language.getStr(),
                aLocale.Country.getStr(), aLocale.Variant.getStr() );
        rtl_TextEncoding nEnc = osl_getTextEncodingFromLocale( pLocale );
        pCharSet = rtl_getBestMimeCharsetFromTextEncoding( nEnc );
    }
    rtl_TextEncoding nRet;
    if ( pCharSet )
        nRet = rtl_getTextEncodingFromMimeCharset( pCharSet );
    else
        nRet = RTL_TEXTENCODING_UTF8;
    return nRet;
}

