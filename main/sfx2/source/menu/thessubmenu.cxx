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
#include "precompiled_sfx2.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>

#include <comphelper/processfactory.hxx>
#include <svl/stritem.hxx>
#include <tools/debug.hxx>
#include <vcl/graph.hxx>
#include <svtools/filter.hxx>


#include <vector>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include "thessubmenu.hxx"


using namespace ::com::sun::star;
using ::rtl::OUString;


// STATIC DATA -----------------------------------------------------------

SFX_IMPL_MENU_CONTROL(SfxThesSubMenuControl, SfxStringItem);

////////////////////////////////////////////////////////////


/*
	Ctor; setzt Select-Handler am Menu und traegt Menu
	in seinen Parent ein.
 */
SfxThesSubMenuControl::SfxThesSubMenuControl( sal_uInt16 nSlotId, Menu &rMenu, SfxBindings &rBindings )
    : SfxMenuControl( nSlotId, rBindings ),
    pMenu(new PopupMenu),
    rParent(rMenu)
{
	rMenu.SetPopupMenu(nSlotId, pMenu);
	pMenu->SetSelectHdl(LINK(this, SfxThesSubMenuControl, MenuSelect));
	pMenu->Clear();
	rParent.EnableItem( GetId(), sal_False );
}


SfxThesSubMenuControl::~SfxThesSubMenuControl()
{
	delete pMenu;
}


/*
	Statusbenachrichtigung;
	Ist die Funktionalit"at disabled, wird der entsprechende
	Menueeintrag im Parentmenu disabled, andernfalls wird er enabled.
 */
void SfxThesSubMenuControl::StateChanged( 
    sal_uInt16 /*nSID*/, 
    SfxItemState eState,
	const SfxPoolItem* /*pState*/ )
{
	rParent.EnableItem(GetId(), SFX_ITEM_AVAILABLE == eState );
}


/*
	Select-Handler des Menus;
	das selektierte Verb mit ausgef"uhrt,
 */
IMPL_LINK_INLINE_START( SfxThesSubMenuControl, MenuSelect, Menu *, pSelMenu )
{
	const sal_uInt16 nSlotId = pSelMenu->GetCurItemId();
    if( nSlotId )
        GetBindings().Execute(nSlotId);
	return 1;
}
IMPL_LINK_INLINE_END( SfxThesSubMenuControl, MenuSelect, Menu *, pSelMenu )


PopupMenu* SfxThesSubMenuControl::GetPopup() const
{
	return pMenu;
}


////////////////////////////////////////////////////////////

OUString SfxThesSubMenuHelper::GetText( 
    const String &rLookUpString, 
    xub_StrLen nDelimPos )
{
    return OUString( rLookUpString.Copy( 0, nDelimPos ) );
}

    
void SfxThesSubMenuHelper::GetLocale( 
    lang::Locale /*out */ &rLocale, 
    const String &rLookUpString, 
    xub_StrLen nDelimPos  )
{
    String aIsoLang( rLookUpString.Copy( nDelimPos + 1) );
    const xub_StrLen nPos = aIsoLang.Search( '-' );
    if (nPos != STRING_NOTFOUND)
    {
        rLocale.Language    = aIsoLang.Copy( 0, nPos );
        rLocale.Country     = aIsoLang.Copy( nPos + 1 );
        rLocale.Variant     = String::EmptyString();
    }
}

    
SfxThesSubMenuHelper::SfxThesSubMenuHelper()
{
    try
    {
        uno::Reference< lang::XMultiServiceFactory >  xMSF( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        m_xLngMgr = uno::Reference< linguistic2::XLinguServiceManager >( xMSF->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.linguistic2.LinguServiceManager" ))), uno::UNO_QUERY_THROW );
        m_xThesarus = m_xLngMgr->getThesaurus();
    }
    catch (uno::Exception &e)    
    {
        (void) e;
        DBG_ASSERT( 0, "failed to get thesaurus" );
    }    
}

    
SfxThesSubMenuHelper::~SfxThesSubMenuHelper()
{
}    


bool SfxThesSubMenuHelper::IsSupportedLocale( const lang::Locale & rLocale ) const
{
    return m_xThesarus.is() && m_xThesarus->hasLocale( rLocale );
}
    

bool SfxThesSubMenuHelper::GetMeanings( 
    std::vector< OUString > & rSynonyms, 
    const OUString & rWord,
    const lang::Locale & rLocale, 
    sal_Int16 nMaxSynonms )
{
    bool bHasMoreSynonyms = false;
    rSynonyms.clear();
    if (IsSupportedLocale( rLocale ) && rWord.getLength() && nMaxSynonms > 0)
    {
        try
        {
            // get all meannings
            const uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeaningSeq( 
                    m_xThesarus->queryMeanings( rWord, rLocale, uno::Sequence< beans::PropertyValue >() ));
            const uno::Reference< linguistic2::XMeaning > *pxMeaning = aMeaningSeq.getConstArray();
            const sal_Int32 nMeanings = aMeaningSeq.getLength();
        
            // iterate over all meanings until nMaxSynonms are found or all meanings are processed
            sal_Int32 nCount = 0;
            sal_Int32 i = 0;
            for ( ;  i < nMeanings && nCount < nMaxSynonms;  ++i)
            {
                const uno::Sequence< OUString > aSynonymSeq( pxMeaning[i]->querySynonyms() );
                const OUString *pSynonyms = aSynonymSeq.getConstArray();
                const sal_Int32 nSynonyms = aSynonymSeq.getLength();
                sal_Int32 k = 0;
                for ( ;  k < nSynonyms && nCount < nMaxSynonms;  ++k)
                {
                    rSynonyms.push_back( pSynonyms[k] );
                    ++nCount;
                }
                bHasMoreSynonyms = k < nSynonyms;    // any synonym from this meaning skipped?
            }
            
            bHasMoreSynonyms |= i < nMeanings;   // any meaning skipped?
        }
        catch (uno::Exception &e)
        {
            (void) e;
            DBG_ASSERT( 0, "failed to get synonyms" );
        }    
    }
    return bHasMoreSynonyms;
}
    

String SfxThesSubMenuHelper::GetThesImplName( const lang::Locale &rLocale ) const
{
    String aRes;
    DBG_ASSERT( m_xLngMgr.is(), "LinguServiceManager missing" );
    if (m_xLngMgr.is())
    {
        uno::Sequence< OUString > aServiceNames = m_xLngMgr->getConfiguredServices(
                OUString::createFromAscii("com.sun.star.linguistic2.Thesaurus"), rLocale );
        // there should be at most one thesaurus configured for each language
        DBG_ASSERT( aServiceNames.getLength() <= 1, "more than one thesaurus found. Should not be possible" );
        if (aServiceNames.getLength() == 1)
            aRes = aServiceNames[0];
    }    
    return aRes;
}    

////////////////////////////////////////////////////////////


