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

#include "precompiled_svx.hxx"
#include "ParaNumberingControl.hxx"
#include "ParaPropertyPanel.hrc"
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <unotools/viewoptions.hxx>
#include <editeng/kernitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <svtools/unitconv.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/nbdtmg.hxx>
#include <svx/nbdtmgfact.hxx>
#include <editeng/unolingu.hxx>
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::text;

namespace svx { namespace sidebar {

Reference<XDefaultNumberingProvider> lcl_GetNumberingProvider()
{
    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xI = xMSF->createInstance(
        ::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
    Reference<XDefaultNumberingProvider> xRet(xI, UNO_QUERY);

    return xRet;
}

ParaNumberingControl::ParaNumberingControl(
    Window* pParent, 
    svx::sidebar::ParaPropertyPanel& rPanel )
    : PopupControl( pParent,SVX_RES(RID_POPUPPANEL_PARAPAGE_NUMBERING) )
    , maNumberVS( this,SVX_RES(VS_NUMBERING) )
    , maMoreButton( this,SVX_RES(CB_NUMBERING_MORE) )
    , mrParaPropertyPanel( rPanel )
    , mpBindings( mrParaPropertyPanel.GetBindings() )
{
    FreeResource();

    maNumberVS.SetStyle( maNumberVS.GetStyle() | WB_NO_DIRECTSELECT );
    maNumberVS.SetExtraSpacing( NUM_IMAGE_SPACING );
    maNumberVS.SetItemWidth(NUM_IMAGE_WIDTH);
    maNumberVS.SetItemHeight(NUM_IMAGE_HEIGHT);

    Reference<XDefaultNumberingProvider> xDefNum = lcl_GetNumberingProvider();
    if(xDefNum.is())
    {
        Sequence< Sequence< PropertyValue > > aNumberings;
        LanguageType eLang = GetSettings().GetLanguage();
        Locale aLocale = SvxCreateLocale(eLang);
        try
        {
            aNumberings = xDefNum->getDefaultContinuousNumberingLevels( aLocale );
        }
        catch(Exception&)
        {
        }
        Reference<XNumberingFormatter> xFormat(xDefNum, UNO_QUERY);
        maNumberVS.SetNumberingSettings(aNumberings, xFormat, aLocale);
    }
    
    maNumberVS.Show();
    maNumberVS.SetSelectHdl( LINK(this, ParaNumberingControl, NumSelectHdl_Impl) );

    maNumberVS.SetColor( GetSettings().GetStyleSettings().GetHighContrastMode()
                         ? GetSettings().GetStyleSettings().GetMenuColor()
                         : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ) );
    maNumberVS.SetBackground( GetSettings().GetStyleSettings().GetHighContrastMode()
                              ? GetSettings().GetStyleSettings().GetMenuColor()
                              : sfx2::sidebar::Theme::GetColor( sfx2::sidebar::Theme::Paint_PanelBackground ) );

    maMoreButton.SetClickHdl(LINK(this, ParaNumberingControl, MoreButtonClickHdl_Impl));
}


ParaNumberingControl::~ParaNumberingControl()
{
}


IMPL_LINK(ParaNumberingControl, NumSelectHdl_Impl, ValueSet*, EMPTYARG)
{
    const sal_uInt16 nIdx = maNumberVS.GetSelectItemId();
    SfxUInt16Item aItem( FN_SVX_SET_NUMBER, nIdx );
    if (mpBindings)
        mpBindings->GetDispatcher()->Execute( FN_SVX_SET_NUMBER, SFX_CALLMODE_RECORD, &aItem, 0L );

    mrParaPropertyPanel.EndNumberingPopupMode();

    return 0;
}


IMPL_LINK(ParaNumberingControl, MoreButtonClickHdl_Impl, void*, EMPTYARG)
{
    if (mpBindings)
        mpBindings->GetDispatcher()->Execute( SID_OUTLINE_BULLET, SFX_CALLMODE_ASYNCHRON );

    mrParaPropertyPanel.EndNumberingPopupMode();

    return 0;
}


void ParaNumberingControl::UpdateValueSet()
{
    maNumberVS.StateChanged(STATE_CHANGE_STYLE);
    maNumberVS.StateChanged(STATE_CHANGE_INITSHOW);

    const sal_uInt16 nTypeIndex = mrParaPropertyPanel.GetNumTypeIndex();
    if ( nTypeIndex != (sal_uInt16)0xFFFF )
        maNumberVS.SelectItem( nTypeIndex );
    else
    {
        maNumberVS.SelectItem(0);
    }
    maMoreButton.GrabFocus();
}

}} // end of namespace sidebar



