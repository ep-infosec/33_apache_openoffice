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



#ifndef __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_
#define __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_

#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <svtools/statusbarcontroller.hxx>
#include <rtl/ustring.hxx>

#include "helper/mischelper.hxx"

#include <set>

class SvtLanguageTable;


// component helper namespace
namespace framework {

class LangSelectionStatusbarController : public svt::StatusbarController
{
    public:
        explicit LangSelectionStatusbarController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

        // XServiceInfo
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );

        // XStatusbarController
        virtual void SAL_CALL command( const ::com::sun::star::awt::Point& aPos,
                                       ::sal_Int32 nCommand,
                                       ::sal_Bool bMouseEvent,
                                       const ::com::sun::star::uno::Any& aData ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL click( const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);

    private:
        virtual ~LangSelectionStatusbarController() {}
        LangSelectionStatusbarController(LangSelectionStatusbarController &); // not defined
        void operator =(LangSelectionStatusbarController &); // not defined

	
        sal_Bool            m_bShowMenu;        // if the menu is to be displayed or not (depending on the selected object/text)
        sal_Int16           m_nScriptType;      // the flags for the different script types available in the selection, LATIN = 0x0001, ASIAN = 0x0002, COMPLEX = 0x0004
        ::rtl::OUString     m_aCurLang;         // the language of the current selection, "*" if there are more than one languages
        ::rtl::OUString     m_aKeyboardLang;    // the keyboard language
        ::rtl::OUString     m_aGuessedTextLang;     // the 'guessed' language for the selection, "" if none could be guessed
        LanguageGuessingHelper      m_aLangGuessHelper;
		
        void LangMenu( const ::com::sun::star::awt::Point& aPos ) throw (::com::sun::star::uno::RuntimeException);
};

} // framework namespace

#endif // __FRAMEWORK_UIELEMENT_LANGSELECTIONSTATUSBARCONTROLLER_HXX_
