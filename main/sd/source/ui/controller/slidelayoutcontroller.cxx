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
#include "precompiled_sd.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/frame/status/FontHeight.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/DrawViewMode.hpp>

#include <memory>
#include <boost/scoped_ptr.hpp>

#include <vos/mutex.hxx>

#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>

#include <svl/languageoptions.hxx>

#include <svtools/ctrltool.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/toolbarmenu.hxx>
#include <svtools/valueset.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <sfx2/imagemgr.hxx>

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "sdresid.hxx"
#include "pres.hxx"
#include "slidelayoutcontroller.hxx"

using rtl::OUString;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;

namespace sd
{

extern ::rtl::OUString ImplRetrieveLabelFromCommand( const Reference< XFrame >& xFrame, const OUString& aCmdURL );

// -----------------------------------------------------------------------

class LayoutToolbarMenu : public svtools::ToolbarMenu
{
public:
    LayoutToolbarMenu( SlideLayoutController& rController, const Reference< XFrame >& xFrame, ::Window* pParent, const bool bInsertPage );
    virtual ~LayoutToolbarMenu();

protected:
	DECL_LINK( SelectHdl, void * );

private:
	SlideLayoutController& mrController;
    Reference< XFrame > mxFrame;
	bool mbInsertPage;
	ValueSet* mpLayoutSet1;
	ValueSet* mpLayoutSet2;
};

// -----------------------------------------------------------------------

struct snewfoil_value_info
{
    sal_uInt16 mnBmpResId;
    sal_uInt16 mnHCBmpResId;
    sal_uInt16 mnStrResId;
    WritingMode meWritingMode;
    AutoLayout maAutoLayout;
};

static snewfoil_value_info notes[] =
{
    {BMP_FOILN_01, BMP_FOILN_01_H, STR_AUTOLAYOUT_NOTES, WritingMode_LR_TB,
     AUTOLAYOUT_NOTES},
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static snewfoil_value_info handout[] =
{
    {BMP_FOILH_01, BMP_FOILH_01_H, STR_AUTOLAYOUT_HANDOUT1, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT1},
    {BMP_FOILH_02, BMP_FOILH_02_H, STR_AUTOLAYOUT_HANDOUT2, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT2},
    {BMP_FOILH_03, BMP_FOILH_03_H, STR_AUTOLAYOUT_HANDOUT3, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT3},
    {BMP_FOILH_04, BMP_FOILH_04_H, STR_AUTOLAYOUT_HANDOUT4, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT4},
    {BMP_FOILH_06, BMP_FOILH_06_H, STR_AUTOLAYOUT_HANDOUT6, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT6},
    {BMP_FOILH_09, BMP_FOILH_09_H, STR_AUTOLAYOUT_HANDOUT9, WritingMode_LR_TB,
     AUTOLAYOUT_HANDOUT9},
    {0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE},
};

static snewfoil_value_info standard[] =
{
    {BMP_LAYOUT_EMPTY, BMP_LAYOUT_EMPTY_H, STR_AUTOLAYOUT_NONE, WritingMode_LR_TB,        AUTOLAYOUT_NONE},
	{BMP_LAYOUT_HEAD03, BMP_LAYOUT_HEAD03_H, STR_AUTOLAYOUT_TITLE, WritingMode_LR_TB,       AUTOLAYOUT_TITLE},
    {BMP_LAYOUT_HEAD02, BMP_LAYOUT_HEAD02_H, STR_AUTOLAYOUT_CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_ENUM},   
	{BMP_LAYOUT_HEAD02A, BMP_LAYOUT_HEAD02A_H, STR_AUTOLAYOUT_2CONTENT, WritingMode_LR_TB,       AUTOLAYOUT_2TEXT},
	{BMP_LAYOUT_HEAD01, BMP_LAYOUT_HEAD01_H, STR_AUTOLAYOUT_ONLY_TITLE, WritingMode_LR_TB,  AUTOLAYOUT_ONLY_TITLE},    
	{BMP_LAYOUT_TEXTONLY, BMP_LAYOUT_TEXTONLY_H, STR_AUTOLAYOUT_ONLY_TEXT, WritingMode_LR_TB,   AUTOLAYOUT_ONLY_TEXT},
    {BMP_LAYOUT_HEAD03B, BMP_LAYOUT_HEAD03B_H, STR_AUTOLAYOUT_2CONTENT_CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_2OBJTEXT},
	{BMP_LAYOUT_HEAD03C, BMP_LAYOUT_HEAD03C_H, STR_AUTOLAYOUT_CONTENT_2CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_TEXT2OBJ},
	{BMP_LAYOUT_HEAD03A, BMP_LAYOUT_HEAD03A_H, STR_AUTOLAYOUT_2CONTENT_OVER_CONTENT,WritingMode_LR_TB, AUTOLAYOUT_2OBJOVERTEXT},    
	{BMP_LAYOUT_HEAD02B, BMP_LAYOUT_HEAD02B_H, STR_AUTOLAYOUT_CONTENT_OVER_CONTENT, WritingMode_LR_TB, AUTOLAYOUT_OBJOVERTEXT},
    {BMP_LAYOUT_HEAD04, BMP_LAYOUT_HEAD04_H, STR_AUTOLAYOUT_4CONTENT, WritingMode_LR_TB,        AUTOLAYOUT_4OBJ},   
	{BMP_LAYOUT_HEAD06, BMP_LAYOUT_HEAD06_H, STR_AUTOLAYOUT_6CONTENT, WritingMode_LR_TB,    AUTOLAYOUT_6CLIPART},
	{0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

static snewfoil_value_info v_standard[] =
{
	// vertical
    {BMP_LAYOUT_VERTICAL02, BMP_LAYOUT_VERTICAL02_H, STR_AL_VERT_TITLE_TEXT_CHART, WritingMode_TB_RL,AUTOLAYOUT_VERTICAL_TITLE_TEXT_CHART},
    {BMP_LAYOUT_VERTICAL01, BMP_LAYOUT_VERTICAL01_H, STR_AL_VERT_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_VERTICAL_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02, BMP_LAYOUT_HEAD02_H, STR_AL_TITLE_VERT_OUTLINE, WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE},
    {BMP_LAYOUT_HEAD02A, BMP_LAYOUT_HEAD02A_H, STR_AL_TITLE_VERT_OUTLINE_CLIPART,   WritingMode_TB_RL, AUTOLAYOUT_TITLE_VERTICAL_OUTLINE_CLIPART},
	{0, 0, 0, WritingMode_LR_TB, AUTOLAYOUT_NONE}
};

// -----------------------------------------------------------------------

static void fillLayoutValueSet( ValueSet* pValue, snewfoil_value_info* pInfo, const bool bHighContrast )
{
    Size aLayoutItemSize;
	for( ; pInfo->mnBmpResId; pInfo++ )
	{
		String aText( SdResId( pInfo->mnStrResId ) );
		BitmapEx aBmp( SdResId( (bHighContrast ? pInfo->mnHCBmpResId : pInfo->mnBmpResId) ) );

		pValue->InsertItem( static_cast<sal_uInt16>(pInfo->maAutoLayout)+1, aBmp, aText );

		aLayoutItemSize.Width() = std::max( aLayoutItemSize.Width(), aBmp.GetSizePixel().Width() );
		aLayoutItemSize.Height() = std::max( aLayoutItemSize.Height(), aBmp.GetSizePixel().Height() );
	}

	aLayoutItemSize = pValue->CalcItemSizePixel( aLayoutItemSize );
	pValue->SetSizePixel( pValue->CalcWindowSizePixel( aLayoutItemSize ) );
}

// -----------------------------------------------------------------------

LayoutToolbarMenu::LayoutToolbarMenu( SlideLayoutController& rController, const Reference< XFrame >& xFrame, ::Window* pParent, const bool bInsertPage )
: svtools::ToolbarMenu(xFrame, pParent, WB_CLIPCHILDREN )
, mrController( rController )
, mxFrame(xFrame)
, mbInsertPage( bInsertPage )
, mpLayoutSet1( 0 )
, mpLayoutSet2( 0 )
{
	DrawViewMode eMode = DrawViewMode_DRAW;

	// find out which view is running
	if( xFrame.is() ) try
	{
		Reference< XPropertySet > xControllerSet( xFrame->getController(), UNO_QUERY_THROW );
		xControllerSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DrawViewMode" ) ) ) >>= eMode;
	}
	catch( Exception& e )
	{
		(void)e;
		OSL_ASSERT(false);
	}

	const sal_Int32 LAYOUT_BORDER_PIX = 7;

	String aTitle1( SdResId( STR_GLUE_ESCDIR_HORZ ) );
	String aTitle2( SdResId( STR_GLUE_ESCDIR_VERT ) );
  
	const bool bHighContrast = GetSettings().GetStyleSettings().GetHighContrastMode();
	SvtLanguageOptions aLanguageOptions;
    const bool bVerticalEnabled = aLanguageOptions.IsVerticalTextEnabled();

	SetSelectHdl( LINK( this, LayoutToolbarMenu, SelectHdl ) );

	mpLayoutSet1 = createEmptyValueSetControl();
	mpLayoutSet1->SetSelectHdl( LINK( this, LayoutToolbarMenu, SelectHdl ) );

	snewfoil_value_info* pInfo = 0;
	sal_Int16 nColCount = 4;
	switch( eMode )
	{
	case DrawViewMode_DRAW: pInfo = &standard[0]; break;
	case DrawViewMode_HANDOUT: pInfo = &handout[0]; nColCount = 2; break;
	case DrawViewMode_NOTES: pInfo = &notes[0]; nColCount = 1; break;
	default: break;
	}

	mpLayoutSet1->SetColCount( nColCount );

	fillLayoutValueSet( mpLayoutSet1, pInfo, bHighContrast );

	Size aSize( mpLayoutSet1->GetOutputSizePixel() );
	aSize.Width() += (mpLayoutSet1->GetColCount() + 1) * LAYOUT_BORDER_PIX;
	aSize.Height() += (mpLayoutSet1->GetLineCount() +1) * LAYOUT_BORDER_PIX;
	mpLayoutSet1->SetOutputSizePixel( aSize );

	if( bVerticalEnabled && (eMode == DrawViewMode_DRAW) )
		appendEntry( -1, aTitle1 );
	appendEntry( 0, mpLayoutSet1 );

	if( bVerticalEnabled && (eMode == DrawViewMode_DRAW) )
	{
		mpLayoutSet2 = new ValueSet( this, WB_TABSTOP | WB_MENUSTYLEVALUESET | WB_FLATVALUESET | WB_NOBORDER | WB_NO_DIRECTSELECT );
	//	mpLayoutSet2->SetHelpId( HID_VALUESET_EXTRUSION_LIGHTING );

		mpLayoutSet2->SetSelectHdl( LINK( this, LayoutToolbarMenu, SelectHdl ) );
		mpLayoutSet2->SetColCount( 4 );
		mpLayoutSet2->EnableFullItemMode( sal_False );
		mpLayoutSet2->SetColor( GetControlBackground() );

		fillLayoutValueSet( mpLayoutSet2, &v_standard[0], bHighContrast );

		aSize = mpLayoutSet2->GetOutputSizePixel();
		aSize.Width() += (mpLayoutSet2->GetColCount() + 1) * LAYOUT_BORDER_PIX;
		aSize.Height() += (mpLayoutSet2->GetLineCount() + 1) * LAYOUT_BORDER_PIX;
		mpLayoutSet2->SetOutputSizePixel( aSize );

		appendEntry( -1, aTitle2 );
		appendEntry( 1, mpLayoutSet2 );
	}

	if( eMode == DrawViewMode_DRAW )
	{
		appendSeparator();

		OUString sSlotStr;
		Image aSlotImage;
		if( mxFrame.is() )
		{
			if( bInsertPage )
				sSlotStr = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DuplicatePage" ) );
			else
				sSlotStr = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:Undo" ) );
			aSlotImage = ::GetImage( mxFrame, sSlotStr, sal_False, bHighContrast );

			String sSlotTitle;
			if( bInsertPage )
				sSlotTitle = ImplRetrieveLabelFromCommand( mxFrame, sSlotStr );
			else
				sSlotTitle = String( SdResId( STR_RESET_LAYOUT ) );
			appendEntry( 2, sSlotTitle, aSlotImage);
		}
	}

	SetOutputSizePixel( getMenuSize() );
}

// -----------------------------------------------------------------------

LayoutToolbarMenu::~LayoutToolbarMenu()
{
}

// -----------------------------------------------------------------------

IMPL_LINK( LayoutToolbarMenu, SelectHdl, void *, pControl )
{
	if ( IsInPopupMode() )
		EndPopupMode();

	Sequence< PropertyValue > aArgs;

	AutoLayout eLayout = AUTOLAYOUT__END;

	OUString sCommandURL( mrController.getCommandURL() );

    if( pControl == mpLayoutSet1 )
    {
	    eLayout = static_cast< AutoLayout >(mpLayoutSet1->GetSelectItemId()-1);
    }
    else if( pControl == mpLayoutSet2 )
    {
	    eLayout = static_cast< AutoLayout >(mpLayoutSet2->GetSelectItemId()-1);
    }

	if( eLayout != AUTOLAYOUT__END )
	{
		aArgs = Sequence< PropertyValue >(1);
		aArgs[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "WhatLayout" ) );
		aArgs[0].Value <<= (sal_Int32)eLayout;
	}
	else if( mbInsertPage )
	{
		sCommandURL = OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DuplicatePage" ) );
	}

	mrController.dispatchCommand( sCommandURL, aArgs );

	return 0;
}

// ====================================================================

OUString SlideLayoutController_getImplementationName()
{
	return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sd.SlideLayoutController" ));
}

// --------------------------------------------------------------------

Sequence< OUString >  SlideLayoutController_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL SlideLayoutController_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
	return *new SlideLayoutController( rSMgr, OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:AssignLayout" )), false );
}

// --------------------------------------------------------------------

OUString InsertSlideController_getImplementationName()
{
	return OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sd.InsertSlideController" ));
}

// --------------------------------------------------------------------

Sequence< OUString >  InsertSlideController_getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.frame.ToolbarController" ));
    return aSNS;
}

// --------------------------------------------------------------------

Reference< XInterface > SAL_CALL InsertSlideController_createInstance( const Reference< XMultiServiceFactory >& rSMgr ) throw( RuntimeException )
{
	return *new SlideLayoutController( rSMgr, OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:InsertPage" )), true );
}

//========================================================================
// class SlideLayoutController
//========================================================================

SlideLayoutController::SlideLayoutController( const Reference< lang::XMultiServiceFactory >& rServiceManager, const rtl::OUString& sCommandURL, bool bInsertPage )
: svt::PopupWindowController( rServiceManager, Reference< frame::XFrame >(), sCommandURL )
, mbInsertPage( bInsertPage )
{
}

// --------------------------------------------------------------------

::Window* SlideLayoutController::createPopupWindow( ::Window* pParent )
{
    return new sd::LayoutToolbarMenu( *this, m_xFrame, pParent, mbInsertPage );
}

// --------------------------------------------------------------------
// XServiceInfo
// --------------------------------------------------------------------

OUString SAL_CALL SlideLayoutController::getImplementationName() throw( RuntimeException )
{
	if( mbInsertPage )
		return InsertSlideController_getImplementationName();
	else
		return SlideLayoutController_getImplementationName();
}

// --------------------------------------------------------------------

Sequence< OUString > SAL_CALL SlideLayoutController::getSupportedServiceNames(  ) throw( RuntimeException )
{
	if( mbInsertPage )
		return InsertSlideController_getSupportedServiceNames();
	else
	    return SlideLayoutController_getSupportedServiceNames();
}

}
