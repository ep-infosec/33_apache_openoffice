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
#include "precompiled_editeng.hxx"

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>

#include <svl/solar.hrc>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <svl/itempool.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>

#include <editeng/akrnitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <comphelper/processfactory.hxx>

static EditDLL* pDLL=0;

using namespace ::com::sun::star;

EditDLL* EditDLL::Get()
{
	if ( !pDLL )
		pDLL = new EditDLL;
	return pDLL;
}

GlobalEditData::GlobalEditData()
{
	ppDefItems = NULL;
	pStdRefDevice = NULL;
}

GlobalEditData::~GlobalEditData()
{
	// DefItems zerstoeren...
	// Oder einfach stehen lassen, da sowieso App-Ende?!
	if ( ppDefItems )
		SfxItemPool::ReleaseDefaults( ppDefItems, EDITITEMCOUNT, sal_True );
	delete pStdRefDevice;
}

SfxPoolItem** GlobalEditData::GetDefItems()
{
	if ( !ppDefItems )
	{
		ppDefItems = new SfxPoolItem*[EDITITEMCOUNT];

		// Absatzattribute:
		SvxNumRule aTmpNumRule( 0, 0, sal_False );

		ppDefItems[0]  = new SvxFrameDirectionItem( FRMDIR_HORI_LEFT_TOP, EE_PARA_WRITINGDIR );
		ppDefItems[1]  = new SvXMLAttrContainerItem( EE_PARA_XMLATTRIBS );
		ppDefItems[2]  = new SfxBoolItem( EE_PARA_HANGINGPUNCTUATION, sal_False );
		ppDefItems[3]  = new SfxBoolItem( EE_PARA_FORBIDDENRULES, sal_True );
		ppDefItems[4]  = new SvxScriptSpaceItem( sal_True, EE_PARA_ASIANCJKSPACING );
		ppDefItems[5]  = new SvxNumBulletItem( aTmpNumRule, EE_PARA_NUMBULLET );
		ppDefItems[6]  = new SfxBoolItem( EE_PARA_HYPHENATE, sal_False );
		ppDefItems[7]  = new SfxBoolItem( EE_PARA_BULLETSTATE, sal_True );
		ppDefItems[8]  = new SvxLRSpaceItem( EE_PARA_OUTLLRSPACE );
		ppDefItems[9]  = new SfxInt16Item( EE_PARA_OUTLLEVEL, -1 );
		ppDefItems[10]  = new SvxBulletItem( EE_PARA_BULLET );
		ppDefItems[11]  = new SvxLRSpaceItem( EE_PARA_LRSPACE );
		ppDefItems[12]  = new SvxULSpaceItem( EE_PARA_ULSPACE );
		ppDefItems[13]  = new SvxLineSpacingItem( 0, EE_PARA_SBL );
		ppDefItems[14]  = new SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST );
		ppDefItems[15]  = new SvxTabStopItem( 0, 0, SVX_TAB_ADJUST_LEFT, EE_PARA_TABS );

		// Zeichenattribute:
		ppDefItems[16]  = new SvxColorItem( Color( COL_AUTO ), EE_CHAR_COLOR );
		ppDefItems[17]  = new SvxFontItem( EE_CHAR_FONTINFO );
		ppDefItems[18] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT );
		ppDefItems[19] = new SvxCharScaleWidthItem( 100, EE_CHAR_FONTWIDTH );
		ppDefItems[20] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT );
		ppDefItems[21] = new SvxUnderlineItem( UNDERLINE_NONE, EE_CHAR_UNDERLINE );
		ppDefItems[22] = new SvxCrossedOutItem( STRIKEOUT_NONE, EE_CHAR_STRIKEOUT );
		ppDefItems[23] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC );
		ppDefItems[24] = new SvxContourItem( sal_False, EE_CHAR_OUTLINE );
		ppDefItems[25] = new SvxShadowedItem( sal_False, EE_CHAR_SHADOW );
		ppDefItems[26] = new SvxEscapementItem( 0, 100, EE_CHAR_ESCAPEMENT );
		ppDefItems[27] = new SvxAutoKernItem( sal_False, EE_CHAR_PAIRKERNING );
		ppDefItems[28] = new SvxKerningItem( 0, EE_CHAR_KERNING );
		ppDefItems[29] = new SvxWordLineModeItem( sal_False, EE_CHAR_WLM );
		ppDefItems[30] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE );
		ppDefItems[31] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CJK );
		ppDefItems[32] = new SvxLanguageItem( LANGUAGE_DONTKNOW, EE_CHAR_LANGUAGE_CTL );
		ppDefItems[33] = new SvxFontItem( EE_CHAR_FONTINFO_CJK );
		ppDefItems[34] = new SvxFontItem( EE_CHAR_FONTINFO_CTL );
		ppDefItems[35] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CJK );
		ppDefItems[36] = new SvxFontHeightItem( 240, 100, EE_CHAR_FONTHEIGHT_CTL );
		ppDefItems[37] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK );
 		ppDefItems[38] = new SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL );
		ppDefItems[39] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK );
		ppDefItems[40] = new SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL );
		ppDefItems[41] = new SvxEmphasisMarkItem( EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK );
		ppDefItems[42] = new SvxCharReliefItem( RELIEF_NONE, EE_CHAR_RELIEF );
		ppDefItems[43] = new SfxVoidItem( EE_CHAR_RUBI_DUMMY );
#ifndef SVX_LIGHT
		ppDefItems[44] = new SvXMLAttrContainerItem( EE_CHAR_XMLATTRIBS );
#else
        // no need to have alien attributes persistent
		ppDefItems[44] = new SfxVoidItem( EE_CHAR_XMLATTRIBS );
#endif // #ifndef SVX_LIGHT
		ppDefItems[45] = new SvxOverlineItem( UNDERLINE_NONE, EE_CHAR_OVERLINE );

		// Features
		ppDefItems[46] = new SfxVoidItem( EE_FEATURE_TAB );
		ppDefItems[47] = new SfxVoidItem( EE_FEATURE_LINEBR );
		ppDefItems[48] = new SvxCharSetColorItem( Color( COL_RED ), RTL_TEXTENCODING_DONTKNOW, EE_FEATURE_NOTCONV );
		ppDefItems[49] = new SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );

		DBG_ASSERT( EDITITEMCOUNT == 50, "ITEMCOUNT geaendert, DefItems nicht angepasst!" );

		// Init DefFonts:
		GetDefaultFonts( *(SvxFontItem*)ppDefItems[EE_CHAR_FONTINFO - EE_ITEMS_START],
						 *(SvxFontItem*)ppDefItems[EE_CHAR_FONTINFO_CJK - EE_ITEMS_START],
						 *(SvxFontItem*)ppDefItems[EE_CHAR_FONTINFO_CTL - EE_ITEMS_START] );
	}

	return ppDefItems;
}

vos::ORef<SvxForbiddenCharactersTable> GlobalEditData::GetForbiddenCharsTable()
{
	if ( !xForbiddenCharsTable.isValid() )
	{
		::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
		xForbiddenCharsTable = new SvxForbiddenCharactersTable( xMSF );
	}
	return xForbiddenCharsTable;
}

uno::Reference< linguistic2::XLanguageGuessing > GlobalEditData::GetLanguageGuesser()
{
    if (!xLanguageGuesser.is())
    {
        uno::Reference< lang::XMultiServiceFactory > xMgr ( comphelper::getProcessServiceFactory() );
        if (xMgr.is())
        {
            xLanguageGuesser = uno::Reference< linguistic2::XLanguageGuessing >(
                    xMgr->createInstance(
                        rtl::OUString::createFromAscii( "com.sun.star.linguistic2.LanguageGuessing" ) ),
                        uno::UNO_QUERY );
        }
    }
    return xLanguageGuesser;
}

OutputDevice* GlobalEditData::GetStdRefDevice()
{
	if ( !pStdRefDevice )
	{
		pStdRefDevice = new VirtualDevice;
		pStdRefDevice->SetMapMode( MAP_TWIP );
	}
	return pStdRefDevice;
}

EditResId::EditResId( sal_uInt16 nId ):
	ResId( nId, *EE_DLL()->GetResMgr() )
{
}

EditDLL::EditDLL()
{
	pGlobalData = new GlobalEditData;
	ByteString aResMgrName( "editeng" );
	pResMgr = ResMgr::CreateResMgr(
        aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
}

EditDLL::~EditDLL()
{
	delete pResMgr;
	delete pGlobalData;
}
