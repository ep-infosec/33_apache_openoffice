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
#include "precompiled_framework.hxx"

#include <uielement/langselectionstatusbarcontroller.hxx>
#include <classes/fwkresid.hxx>
#include <services.h>
#include <classes/resource.hrc>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/status.hxx>
#include <toolkit/helper/convert.hxx>

#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>
#include <svtools/langtab.hxx>
#include "sal/types.h"
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/document/XDocumentLanguages.hpp>
#include <i18npool/mslangid.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <classes/fwkresid.hxx>
#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/gen.hxx>
#include <com/sun/star/awt/Command.hpp>
#include <svl/languageoptions.hxx>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <dispatch/uieventloghelper.hxx>

#include "helper/mischelper.hxx"

#include <rtl/ustrbuf.hxx>

#include <map>
#include <set>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::document;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;


namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   LangSelectionStatusbarController     	    ,
                                            OWeakObject                             ,
                                            SERVICENAME_STATUSBARCONTROLLER		    ,
											IMPLEMENTATIONNAME_LANGSELECTIONSTATUSBARCONTROLLER
										)

DEFINE_INIT_SERVICE                     (   LangSelectionStatusbarController, {} )

LangSelectionStatusbarController::LangSelectionStatusbarController( const uno::Reference< lang::XMultiServiceFactory >& xServiceManager ) :
    svt::StatusbarController( xServiceManager, uno::Reference< frame::XFrame >(), OUString(), 0 ),
	m_bShowMenu( sal_True ),
    m_nScriptType( LS_SCRIPT_LATIN | LS_SCRIPT_ASIAN | LS_SCRIPT_COMPLEX ),
    m_aLangGuessHelper( xServiceManager )
{
}

void SAL_CALL LangSelectionStatusbarController::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::initialize" );
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    svt::StatusbarController::initialize( aArguments );

    if ( m_xStatusbarItem.is() )
    {
        m_xStatusbarItem->setText( String( FwkResId( STR_LANGSTATUS_MULTIPLE_LANGUAGES ) ) );
    }
}

void LangSelectionStatusbarController::LangMenu(
    const ::com::sun::star::awt::Point& aPos )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::LangMenu" );
	if (!m_bShowMenu)
		return;

	//add context menu
    const static OUString s_sPopupMenu(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.PopupMenu"));
	Reference< awt::XPopupMenu > xPopupMenu( m_xServiceManager->createInstance( s_sPopupMenu ), UNO_QUERY );
    //sub menu that contains all items except the last two items: Separator + Set Language for Paragraph
	Reference< awt::XPopupMenu > subPopupMenu(m_xServiceManager->createInstance( s_sPopupMenu ), UNO_QUERY );

    SvtLanguageTable    aLanguageTable;

    // get languages to be displayed in the menu
    std::set< OUString > aLangItems;
    FillLangItems( aLangItems, aLanguageTable, m_xFrame, m_aLangGuessHelper, 
            m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedTextLang );

    //
    // add first few entries to main menu
    //
	sal_Int16 nItemId = static_cast< sal_Int16 >(MID_LANG_SEL_1);
    const OUString sAsterix(RTL_CONSTASCII_USTRINGPARAM("*"));  // multiple languages in current selection
    const OUString sEmpty;  // 'no language found' from language guessing
    std::map< sal_Int16, OUString > aLangMap;
    std::set< OUString >::const_iterator it;
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if ( rStr != OUString( aLanguageTable.GetString( LANGUAGE_NONE ) ) &&
             rStr != sAsterix &&
             rStr != sEmpty)
		{
            DBG_ASSERT( MID_LANG_SEL_1 <= nItemId && nItemId <= MID_LANG_SEL_9,
                    "nItemId outside of expected range!" );
	        xPopupMenu->insertItem( nItemId, rStr, 0, nItemId );
		    if ( rStr == m_aCurLang )
	        {
				//make a sign for the current language
                xPopupMenu->checkItem( nItemId, sal_True );
		    }
            aLangMap[ nItemId ] = rStr;
			++nItemId;
		}
    }
    xPopupMenu->insertItem( MID_LANG_SEL_NONE,  String( FwkResId( STR_LANGSTATUS_NONE )), 0, MID_LANG_SEL_NONE );
    xPopupMenu->insertItem( MID_LANG_SEL_RESET, String( FwkResId( STR_RESET_TO_DEFAULT_LANGUAGE )), 0, MID_LANG_SEL_RESET );
    xPopupMenu->insertItem( MID_LANG_SEL_MORE,  String( FwkResId( STR_LANGSTATUS_MORE )), 0, MID_LANG_SEL_MORE );

    //
    // add entries to submenu ('set language for paragraph')
    //
	nItemId = static_cast< sal_Int16 >(MID_LANG_PARA_1);
    for (it = aLangItems.begin(); it != aLangItems.end(); ++it)
    {
        const OUString & rStr( *it );
        if( rStr != OUString( aLanguageTable.GetString( LANGUAGE_NONE ) )&&
            rStr != sAsterix &&
            rStr != sEmpty)
		{
            DBG_ASSERT( MID_LANG_PARA_1 <= nItemId && nItemId <= MID_LANG_PARA_9,
                    "nItemId outside of expected range!" );
            subPopupMenu->insertItem( nItemId, rStr, 0, nItemId );
			aLangMap[nItemId] = rStr;
            ++nItemId;
		}
    }
    subPopupMenu->insertItem( MID_LANG_PARA_NONE,  String( FwkResId( STR_LANGSTATUS_NONE )), 0, MID_LANG_PARA_NONE );
    subPopupMenu->insertItem( MID_LANG_PARA_RESET, String( FwkResId( STR_RESET_TO_DEFAULT_LANGUAGE )), 0, MID_LANG_PARA_RESET );
    subPopupMenu->insertItem( MID_LANG_PARA_MORE,  String( FwkResId( STR_LANGSTATUS_MORE )), 0, MID_LANG_PARA_MORE );

    //
    // add last two entries to main menu
    //
	xPopupMenu->insertSeparator( MID_LANG_PARA_SEPERATOR );
    xPopupMenu->insertItem( MID_LANG_PARA_STRING, String( FwkResId( STR_SET_LANGUAGE_FOR_PARAGRAPH )), 0, MID_LANG_PARA_STRING );
	xPopupMenu->setPopupMenu( MID_LANG_PARA_STRING, subPopupMenu );


    //
    // now display the popup menu and execute every command ...
    //

	Reference< awt::XWindowPeer > xParent( m_xParentWindow, UNO_QUERY );
    com::sun::star::awt::Rectangle aRect( aPos.X, aPos.Y, 0, 0 );
	sal_Int16 nId = xPopupMenu->execute( xParent, aRect, com::sun::star::awt::PopupMenuDirection::EXECUTE_UP+16 );
    //click "More..."
    if ( nId && m_xFrame.is() )
    {
        OUStringBuffer aBuff;
        //set selected language as current language for selection
        const OUString aSelectedLang = aLangMap[nId];

		if (MID_LANG_SEL_1 <= nId && nId <= MID_LANG_SEL_9)
		{
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:LanguageStatus?Language:string=Current_") ));
			aBuff.append( aSelectedLang );
		}
		else if (nId == MID_LANG_SEL_NONE)
		{
            //set None as current language for selection
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:LanguageStatus?Language:string=Current_LANGUAGE_NONE") ));
		}
		else if (nId == MID_LANG_SEL_RESET)
		{
            // reset language attributes for selection
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:LanguageStatus?Language:string=Current_RESET_LANGUAGES") ));
        }
		else if (nId == MID_LANG_SEL_MORE)
		{
			//open the dialog "format/character" for current selection
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:FontDialog?Language:string=*") ));
		}
        else if (MID_LANG_PARA_1 <= nId && nId <= MID_LANG_PARA_9)
		{
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:LanguageStatus?Language:string=Paragraph_") ));
            aBuff.append( aSelectedLang );
		}
        else if (nId == MID_LANG_PARA_NONE)
        {
            //set None as language for current paragraph
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:LanguageStatus?Language:string=Paragraph_LANGUAGE_NONE") ));
        }
        else if (nId == MID_LANG_PARA_RESET)
        {
            // reset language attributes for paragraph
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:LanguageStatus?Language:string=Paragraph_RESET_LANGUAGES") ));
        }
        else if (nId == MID_LANG_PARA_MORE)
		{
			//open the dialog "format/character" for current paragraph
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( (".uno:FontDialogForParagraph") ));
		}

        const Sequence< beans::PropertyValue > aDummyArgs;
        execute( aBuff.makeStringAndClear(), aDummyArgs );
    }
}

void SAL_CALL LangSelectionStatusbarController::command(
    const ::com::sun::star::awt::Point& aPos,
    ::sal_Int32 nCommand,
    ::sal_Bool /*bMouseEvent*/,
    const ::com::sun::star::uno::Any& /*aData*/ )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::command" );
	if ( nCommand & ::awt::Command::CONTEXTMENU )
	{
        LangMenu( aPos );
	}
}

void SAL_CALL LangSelectionStatusbarController::click(
    const ::com::sun::star::awt::Point& aPos )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::click" );
    LangMenu( aPos );
}

// XStatusListener
void SAL_CALL LangSelectionStatusbarController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    // This function will be called when observed data changes,
    // for example the selection or keyboard language.
    // - It displays the language in use in the status bar
    // - and it stores the relevant data for creating the menu
    //   at some later point in the member variables
    //      m_nScriptType, m_aCurLang, m_aKeyboardLang, m_aGuessedText

    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "LangSelectionStatusbarController::statusChanged" );
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    if ( m_bDisposed )
        return;

	m_bShowMenu = sal_True;
	m_nScriptType = LS_SCRIPT_LATIN | LS_SCRIPT_ASIAN | LS_SCRIPT_COMPLEX;  //set the default value

    if ( m_xStatusbarItem.is() )
    {
        OUString aStrValue;
        Sequence< OUString > aSeq;

        if ( Event.State >>= aStrValue )
            m_xStatusbarItem->setText( aStrValue );
        else if ( Event.State >>= aSeq )
        {
            if ( aSeq.getLength() == 4 )
            {
				OUString aStatusText = aSeq[0];
				if ( 0 == aStatusText.compareToAscii( RTL_CONSTASCII_STRINGPARAM("*") ))
                {
                    aStatusText = String( FwkResId( STR_LANGSTATUS_MULTIPLE_LANGUAGES ) );
                }
                m_xStatusbarItem->setText( aStatusText );

                // Retrieve all other values from the sequence and
                // store it members!
				m_aCurLang      = aSeq[0];
                m_nScriptType   = static_cast< sal_Int16 >( aSeq[1].toInt32() );
	            m_aKeyboardLang = aSeq[2];
                m_aGuessedTextLang  = aSeq[3];
            }
        }
        else if ( !Event.State.hasValue() )
		{
            m_xStatusbarItem->setText( OUString() );
			m_bShowMenu = sal_False;	// no language -> no menu
		}
    }
}

}

