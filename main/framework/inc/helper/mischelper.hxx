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



#ifndef __MISC_HELPER_HXX_
#define __MISC_HELPER_HXX_

#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <i18npool/lang.h>
#include <svl/languageoptions.hxx>
#include <rtl/ustring.hxx>
#include <fwidllapi.h>

#include <set>

class SvtLanguageTable;


// flags for script types in use within selection
#define LS_SCRIPT_LATIN     0x0001
#define LS_SCRIPT_ASIAN     0x0002
#define LS_SCRIPT_COMPLEX   0x0004


namespace framework
{

// menu ids for language status bar control
enum LangMenuIDs
{
    MID_LANG_SEL_1 = 1,     // need to start with 1 since xPopupMenu->execute will return 0 if the menu is cancelled
    MID_LANG_SEL_2,
    MID_LANG_SEL_3,
    MID_LANG_SEL_4,
    MID_LANG_SEL_5,
    MID_LANG_SEL_6,
    MID_LANG_SEL_7,
    MID_LANG_SEL_8,
    MID_LANG_SEL_9,
    MID_LANG_SEL_NONE,
    MID_LANG_SEL_RESET,
    MID_LANG_SEL_MORE,

    MID_LANG_PARA_SEPERATOR,
    MID_LANG_PARA_STRING,

    MID_LANG_PARA_1,
    MID_LANG_PARA_2,
    MID_LANG_PARA_3,
    MID_LANG_PARA_4,
    MID_LANG_PARA_5,
    MID_LANG_PARA_6,
    MID_LANG_PARA_7,
    MID_LANG_PARA_8,
    MID_LANG_PARA_9,
    MID_LANG_PARA_NONE,
    MID_LANG_PARA_RESET,
    MID_LANG_PARA_MORE,
};
    
    
inline bool IsScriptTypeMatchingToLanguage( sal_Int16 nScriptType, LanguageType nLang )
{
    return 0 != (nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

    
class FWI_DLLPUBLIC LanguageGuessingHelper
{
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing >    m_xLanguageGuesser;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
    
public:
    LanguageGuessingHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceManager) : m_xServiceManager(_xServiceManager){}

    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XLanguageGuessing >  GetGuesser() const;
};

FWI_DLLPUBLIC ::rtl::OUString RetrieveLabelFromCommand( const ::rtl::OUString& aCmdURL
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&    _xServiceFactory
            ,::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >&        _xUICommandLabels
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame
            ,::rtl::OUString& _rModuleIdentifier
            ,sal_Bool& _rIni
            ,const sal_Char* _pName);

FWI_DLLPUBLIC void FillLangItems( std::set< ::rtl::OUString > &rLangItems,
        const SvtLanguageTable &rLanguageTable,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > &rxFrame,
        const LanguageGuessingHelper & rLangGuessHelper,
        sal_Int16               nScriptType,
        const ::rtl::OUString & rCurLang,
        const ::rtl::OUString & rKeyboardLang,
        const ::rtl::OUString & rGuessedTextLang );

} // namespace framework

#endif // __MISC_HELPER_HXX_

