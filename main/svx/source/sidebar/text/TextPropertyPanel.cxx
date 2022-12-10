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

#include "TextPropertyPanel.hrc"
#include "TextPropertyPanel.hxx"
#include "SvxSBFontNameBox.hxx"

#include "svx/dialmgr.hxx"

#include <editeng/brshitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <rtl/ref.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/ControllerFactory.hxx>
#include <sfx2/sidebar/Layouter.hxx>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/SidebarToolBox.hxx>
#include "sfx2/imagemgr.hxx"
#include <svtools/ctrltool.hxx>
#include <svtools/unitconv.hxx>

#include <vcl/gradient.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include "TextCharacterSpacingControl.hxx"
#include "TextCharacterSpacingPopup.hxx"
#include "TextUnderlineControl.hxx"
#include "TextUnderlinePopup.hxx"
#include <svx/sidebar/ColorControl.hxx>
#include <svx/sidebar/PopupContainer.hxx>

#include <boost/bind.hpp>

using namespace css;
using namespace cssu;
using namespace ::sfx2::sidebar;
using ::sfx2::sidebar::Theme;
using ::sfx2::sidebar::ControlFactory;
using ::sfx2::sidebar::Layouter;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))




namespace svx { namespace sidebar {

#undef HAS_IA2



PopupControl* TextPropertyPanel::CreateCharacterSpacingControl (PopupContainer* pParent)
{
    return new TextCharacterSpacingControl(pParent, *this, mpBindings);
}

PopupControl* TextPropertyPanel::CreateUnderlinePopupControl (PopupContainer* pParent)
{
	return new TextUnderlineControl(pParent, *this, mpBindings);
}

namespace 
{
    Color GetAutomaticColor(void)
    {
        return COL_AUTO;
    }
} // end of anonymous namespace

long TextPropertyPanel::GetSelFontSize()
{
    long nH = 240;
    SfxMapUnit eUnit = maSpacingControl.GetCoreMetric();
    if (mpHeightItem)
        nH = LogicToLogic(  mpHeightItem->GetHeight(), (MapUnit)eUnit, MAP_TWIP );
    return nH;
} 


TextPropertyPanel* TextPropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const ::sfx2::sidebar::EnumContext& rContext)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to TextPropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to TextPropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to TextPropertyPanel::Create"), NULL, 2);
    
    return new TextPropertyPanel(
        pParent,
        rxFrame,
        pBindings,
        rContext);
}


::sfx2::sidebar::ControllerItem& TextPropertyPanel::GetSpaceController()
{
	return maSpacingControl;
}

TextPropertyPanel::TextPropertyPanel (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings,
    const ::sfx2::sidebar::EnumContext& rContext)
    :   Control(pParent, SVX_RES(RID_SIDEBAR_TEXT_PANEL)),
        mpFontNameBox (new SvxSBFontNameBox(this, SVX_RES(CB_SBFONT_FONT))),
        maFontSizeBox		(this, SVX_RES(MB_SBFONT_FONTSIZE)),
        mpToolBoxFontBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxFont(ControlFactory::CreateToolBox(
                mpToolBoxFontBackground.get(),
                SVX_RES(TB_FONT))),
        mpToolBoxIncDecBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxIncDec(ControlFactory::CreateToolBox(
                mpToolBoxIncDecBackground.get(),
                SVX_RES(TB_INCREASE_DECREASE))),
        mpToolBoxScriptBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxScript(ControlFactory::CreateToolBox(
                mpToolBoxScriptBackground.get(),
                SVX_RES(TB_SCRIPT))),
        mpToolBoxScriptSwBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxScriptSw(ControlFactory::CreateToolBox(
                mpToolBoxScriptSwBackground.get(),
                SVX_RES(TB_SCRIPT_SW))),
        mpToolBoxSpacingBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxSpacing(ControlFactory::CreateToolBox(
                mpToolBoxSpacingBackground.get(),
                SVX_RES(TB_SPACING))),
        mpToolBoxFontColorBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxFontColor(ControlFactory::CreateToolBox(
                mpToolBoxFontColorBackground.get(),
                SVX_RES(TB_FONTCOLOR),
                rxFrame)),
        mpToolBoxFontColorBackgroundSW(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxFontColorSW(ControlFactory::CreateToolBox(
                mpToolBoxFontColorBackgroundSW.get(),
                SVX_RES(TB_FONTCOLOR_SW),
                rxFrame)),
        mpToolBoxHighlightBackground(ControlFactory::CreateToolBoxBackground(this)),
        mpToolBoxHighlight(ControlFactory::CreateToolBox(
                mpToolBoxHighlightBackground.get(),
                SVX_RES(TB_HIGHLIGHT),
                rxFrame)),
        mpFontColorUpdater(),
        mpHighlightUpdater(),

        maFontNameControl	(SID_ATTR_CHAR_FONT,		*pBindings, *this, A2S("CharFontName"), rxFrame),
        maFontSizeControl	(SID_ATTR_CHAR_FONTHEIGHT,	*pBindings, *this, A2S("FontHeight"),   rxFrame),
        maWeightControl		(SID_ATTR_CHAR_WEIGHT,		*pBindings, *this, A2S("Bold"),         rxFrame),
        maItalicControl		(SID_ATTR_CHAR_POSTURE,		*pBindings, *this, A2S("Italic"),       rxFrame),
        maUnderlineControl	(SID_ATTR_CHAR_UNDERLINE,	*pBindings, *this, A2S("Underline"),    rxFrame),
        maStrikeControl		(SID_ATTR_CHAR_STRIKEOUT,	*pBindings, *this, A2S("Strikeout"),    rxFrame),
        maShadowControl		(SID_ATTR_CHAR_SHADOWED,	*pBindings, *this, A2S("Shadowed"),     rxFrame),
        maScriptControlSw	(SID_ATTR_CHAR_ESCAPEMENT,	*pBindings, *this, A2S("Escapement"),   rxFrame),
        maSuperScriptControl(SID_SET_SUPER_SCRIPT,		*pBindings, *this, A2S("SuperScript"),  rxFrame),
        maSubScriptControl	(SID_SET_SUB_SCRIPT,		*pBindings, *this, A2S("SubScript"),    rxFrame),
        maSpacingControl	(SID_ATTR_CHAR_KERNING,		*pBindings, *this, A2S("Spacing"),      rxFrame),
        maSDFontGrow		(SID_GROW_FONT_SIZE,		*pBindings, *this, A2S("Grow"),         rxFrame),
        maSDFontShrink		(SID_SHRINK_FONT_SIZE,		*pBindings, *this, A2S("Shrink"),       rxFrame),

        mpFontList			(NULL),
        mbMustDelete		(false),
        mbFocusOnFontSizeCtrl(false),
        maCharSpacePopup(this, ::boost::bind(&TextPropertyPanel::CreateCharacterSpacingControl, this, _1)),
        maUnderlinePopup(this, ::boost::bind(&TextPropertyPanel::CreateUnderlinePopupControl, this, _1)),
        mxFrame(rxFrame),
        maContext(),
        mpBindings(pBindings),
        maLayouter(*this)
{
	Initialize();

	FreeResource();

    UpdateFontColorToolbox(rContext);

    // Setup the grid layouter.
    maLayouter.GetCell(0,0).SetControl(*mpFontNameBox).SetMinimumWidth(Layouter::MapWidth(*this,FONTNAME_WIDTH));
    maLayouter.GetCell(0,2).SetControl(maFontSizeBox).SetFixedWidth();

    maLayouter.GetCell(1,0).SetControl(*mpToolBoxFontBackground).SetFixedWidth();
    maLayouter.GetCell(1,2).SetControl(*mpToolBoxIncDecBackground).SetFixedWidth();

    maLayouter.GetColumn(0)
        .SetWeight(1)
        .SetLeftPadding(Layouter::MapWidth(*this,SECTIONPAGE_MARGIN_HORIZONTAL));
    maLayouter.GetColumn(1)
        .SetWeight(0)
        .SetMinimumWidth(Layouter::MapWidth(*this, CONTROL_SPACING_HORIZONTAL));
    maLayouter.GetColumn(2)
        .SetWeight(0)
        .SetRightPadding(Layouter::MapWidth(*this,SECTIONPAGE_MARGIN_HORIZONTAL));
}




TextPropertyPanel::~TextPropertyPanel (void)
{
    if(mbMustDelete)
        delete mpFontList;

    // Destroy the toolbox windows.
    mpToolBoxIncDec.reset();
    mpToolBoxFont.reset();
    mpToolBoxFontColor.reset();
    mpToolBoxFontColorSW.reset();
    mpToolBoxScript.reset();
    mpToolBoxScriptSw.reset();
    mpToolBoxSpacing.reset();
    mpToolBoxHighlight.reset();

    // Destroy the background windows of the toolboxes.
    mpToolBoxIncDecBackground.reset();
    mpToolBoxFontBackground.reset();
    mpToolBoxFontColorBackground.reset();
    mpToolBoxFontColorBackgroundSW.reset();
    mpToolBoxScriptBackground.reset();
    mpToolBoxScriptSwBackground.reset();
    mpToolBoxSpacingBackground.reset();
    mpToolBoxHighlightBackground.reset();
}




void TextPropertyPanel::SetSpacing(long nKern)
{
	mlKerning = nKern;
}


void TextPropertyPanel::HandleContextChange (
    const ::sfx2::sidebar::EnumContext aContext)
{
    if (maContext == aContext)
    {
        // Nothing to do.
        return;
    }

    maContext = aContext;
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_Calc, Context_Cell):
        case CombinedEnumContext(Application_Calc, Context_Pivot):
            mpToolBoxScriptSw->Hide();
            mpToolBoxHighlight->Hide();
            mpToolBoxScript->Disable();
            mpToolBoxSpacing->Disable();
            break;

        case CombinedEnumContext(Application_Calc, Context_EditCell):
        case CombinedEnumContext(Application_Calc, Context_DrawText):
            mpToolBoxScriptSw->Hide();
            mpToolBoxHighlight->Hide();
            mpToolBoxScript->Enable();
            mpToolBoxSpacing->Enable();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_Text):
        case CombinedEnumContext(Application_WriterVariants, Context_Table):
            mpToolBoxScriptSw->Show();
            mpToolBoxScript->Hide();
            mpToolBoxHighlight->Show();
            mpToolBoxSpacing->Show();
            break;

        case CombinedEnumContext(Application_WriterVariants, Context_DrawText):
        case CombinedEnumContext(Application_WriterVariants, Context_Annotation):
            mpToolBoxScriptSw->Show();
            mpToolBoxScript->Hide();
            mpToolBoxSpacing->Show();
            mpToolBoxHighlight->Hide();
            break;

        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            mpToolBoxScriptSw->Hide();
            mpToolBoxScript->Show();
            mpToolBoxSpacing->Show();
            mpToolBoxHighlight->Hide();
            break;

        default:
            break;
    }

    UpdateFontColorToolbox(aContext);
}




void TextPropertyPanel::UpdateFontColorToolbox (
    const ::sfx2::sidebar::EnumContext /* aContext */)
{
    bool bIsWriterFontColor (false);
    if (maContext.GetApplication_DI() == sfx2::sidebar::EnumContext::Application_WriterVariants)
        if (maContext.GetContext() != sfx2::sidebar::EnumContext::Context_DrawText)
            bIsWriterFontColor = true;
    if (bIsWriterFontColor)
    {
        mpToolBoxFontColor->Hide();
        mpToolBoxFontColorSW->Show();
    }
    else
    {
        mpToolBoxFontColor->Show();
        mpToolBoxFontColorSW->Hide();
    }
}




void TextPropertyPanel::DataChanged (const DataChangedEvent& rEvent)
{
    (void)rEvent;
    
    SetupToolboxItems();
}




void TextPropertyPanel::Initialize (void)
{
    //<<modify fill font list
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    
    if (pDocSh != NULL)
        pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST );
    if (pItem != NULL)
        mpFontList = ( (SvxFontListItem*)pItem )->GetFontList();
    else
    {
        mpFontList = new FontList( Application::GetDefaultDevice() );
        mbMustDelete = true;
    }

    mpFontNameBox->SetAccessibleName(mpFontNameBox->GetQuickHelpText());
    const FontInfo aFontInfo (mpFontList->Get( String::CreateFromAscii( "" ), String::CreateFromAscii( "" )));
    maFontSizeBox.Fill(&aFontInfo,mpFontList);
    maFontSizeBox.SetAccessibleName(maFontSizeBox.GetQuickHelpText());

    //toolbox
    SetupToolboxItems();
    InitToolBoxIncDec();
    InitToolBoxFont();
    InitToolBoxScript();
    InitToolBoxSpacing();

#ifdef HAS_IA2
    mpFontNameBox->SetAccRelationLabeledBy(&mpFontNameBox);
    mpFontNameBox->SetMpSubEditAccLableBy(&mpFontNameBox);
    maFontSizeBox.SetAccRelationLabeledBy(&maFontSizeBox);
    maFontSizeBox.SetMpSubEditAccLableBy(&maFontSizeBox);
    mpToolBoxFont.SetAccRelationLabeledBy(&mpToolBoxFont);
    mpToolBoxIncDec.SetAccRelationLabeledBy(&mpToolBoxIncDec);
    mpToolBoxFontColor.SetAccRelationLabeledBy(&mpToolBoxFontColor);
    mpToolBoxScript.SetAccRelationLabeledBy(&mpToolBoxScript);
    mpToolBoxScriptSw.SetAccRelationLabeledBy(&mpToolBoxScriptSw);
    mpToolBoxSpacing.SetAccRelationLabeledBy(&mpToolBoxSpacing);
    mpToolBoxHighlight.SetAccRelationLabeledBy(&mpToolBoxHighlight);
#endif

    //init state
    mpHeightItem = NULL;
    meWeight = WEIGHT_NORMAL;
    meItalic = ITALIC_NONE;
    mbShadow = false;
    meStrike = STRIKEOUT_NONE;
    mbPostureAvailable = true;
    mbWeightAvailable = true;
    meUnderline = UNDERLINE_NONE;
    meUnderlineColor = COL_AUTO;
    meEscape = SVX_ESCAPEMENT_OFF;
    mbSuper = false;
    mbSub = false;
    mbKernAvailable = true;
    mbKernLBAvailable = true;
    mlKerning = 0;
    
    //set handler
    mpFontNameBox->SetBindings(mpBindings);
    Link aLink = LINK(this, TextPropertyPanel, FontSelHdl);
    mpFontNameBox->SetSelectHdl(aLink);
    aLink = LINK(this, TextPropertyPanel, FontSizeModifyHdl);
    maFontSizeBox.SetModifyHdl(aLink);
    aLink = LINK(this, TextPropertyPanel, FontSizeSelHdl);
    maFontSizeBox.SetSelectHdl(aLink);
    aLink = LINK(this, TextPropertyPanel, FontSizeLoseFocus);
    maFontSizeBox.SetLoseFocusHdl(aLink);
}

void TextPropertyPanel::EndSpacingPopupMode (void)
{
    maCharSpacePopup.Hide();
}

void TextPropertyPanel::EndUnderlinePopupMode (void)
{
	maUnderlinePopup.Hide();
}


void TextPropertyPanel::InitToolBoxFont()
{
	mpToolBoxFont->SetBackground(Wallpaper());
	mpToolBoxFont->SetPaintTransparent(true);

	Size aTbxSize( mpToolBoxFont->CalcWindowSizePixel() );
	mpToolBoxFont->SetOutputSizePixel( aTbxSize );

	Link aLink  = LINK(this, TextPropertyPanel, ToolboxFontSelectHandler);
	mpToolBoxFont->SetSelectHdl ( aLink );
	aLink = LINK(this, TextPropertyPanel, ToolBoxUnderlineClickHdl);
	mpToolBoxFont->SetDropdownClickHdl(aLink);
}




void TextPropertyPanel::InitToolBoxIncDec()
{
	Size aTbxSize( mpToolBoxIncDec->CalcWindowSizePixel() );
	mpToolBoxIncDec->SetOutputSizePixel( aTbxSize );

	Link aLink = LINK(this, TextPropertyPanel, ToolboxIncDecSelectHdl);
    mpToolBoxIncDec->SetSelectHdl ( aLink );
}




void TextPropertyPanel::InitToolBoxScript()
{
	Size aTbxSize( mpToolBoxScriptSw->CalcWindowSizePixel() );
	mpToolBoxScriptSw->SetOutputSizePixel( aTbxSize );
	
	Link aLink = LINK(this, TextPropertyPanel, ToolBoxSwScriptSelectHdl);
    mpToolBoxScriptSw->SetSelectHdl ( aLink );

	aTbxSize = mpToolBoxScript->CalcWindowSizePixel() ;
	mpToolBoxScript->SetOutputSizePixel( aTbxSize );

	aLink = LINK(this, TextPropertyPanel, ToolBoxScriptSelectHdl);
    mpToolBoxScript->SetSelectHdl ( aLink );
}
void TextPropertyPanel::InitToolBoxSpacing()
{
	Size aTbxSize( mpToolBoxSpacing->CalcWindowSizePixel() );
	mpToolBoxSpacing->SetOutputSizePixel( aTbxSize );
	mpToolBoxSpacing->SetItemBits( TBI_SPACING, mpToolBoxSpacing->GetItemBits( TBI_SPACING ) | TIB_DROPDOWNONLY );

	Link aLink = LINK(this, TextPropertyPanel, SpacingClickHdl);
    mpToolBoxSpacing->SetDropdownClickHdl ( aLink );
	mpToolBoxSpacing->SetSelectHdl( aLink );
}




void TextPropertyPanel::SetupToolboxItems (void)
{
    maSDFontGrow.SetupToolBoxItem(*mpToolBoxIncDec, TBI_INCREASE);
    maSDFontShrink.SetupToolBoxItem(*mpToolBoxIncDec, TBI_DECREASE);

    maWeightControl.SetupToolBoxItem(*mpToolBoxFont, TBI_BOLD);
    maItalicControl.SetupToolBoxItem(*mpToolBoxFont, TBI_ITALIC);
    maUnderlineControl.SetupToolBoxItem(*mpToolBoxFont, TBI_UNDERLINE);
    maStrikeControl.SetupToolBoxItem(*mpToolBoxFont, TBI_STRIKEOUT);
    maShadowControl.SetupToolBoxItem(*mpToolBoxFont, TBI_SHADOWED);

    //for sw
    maSuperScriptControl.SetupToolBoxItem(*mpToolBoxScriptSw, TBI_SUPER_SW);
    maSubScriptControl.SetupToolBoxItem(*mpToolBoxScriptSw, TBI_SUB_SW);
    //for sc and sd
    maSuperScriptControl.SetupToolBoxItem(*mpToolBoxScript, TBI_SUPER);
    maSubScriptControl.SetupToolBoxItem(*mpToolBoxScript, TBI_SUB);
    maSpacingControl.SetupToolBoxItem(*mpToolBoxSpacing, TBI_SPACING);
}




IMPL_LINK( TextPropertyPanel, FontSelHdl, FontNameBox*, pBox )
{
	if ( !pBox->IsTravelSelect() )
	{
		if( SfxViewShell::Current() )
		{
			Window* pShellWnd = SfxViewShell::Current()->GetWindow();

			if ( pShellWnd )
				pShellWnd->GrabFocus();
		}
	}
	return 0;
}

IMPL_LINK( TextPropertyPanel, FontSizeModifyHdl, FontSizeBox*, pSizeBox )
{
	if (pSizeBox == &maFontSizeBox)
	{	
		long nSize = pSizeBox->GetValue();
		mbFocusOnFontSizeCtrl = true;

		float fSize = (float)nSize / 10;
		SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
		SvxFontHeightItem aItem( CalcToUnit( fSize, eUnit ), 100, SID_ATTR_CHAR_FONTHEIGHT ) ;

		mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aItem, 0L );
		mpBindings->Invalidate(SID_ATTR_CHAR_FONTHEIGHT,true,false);
	}
	return 0;
}

IMPL_LINK( TextPropertyPanel, FontSizeSelHdl, FontSizeBox*, pSizeBox )
{
	if ( !pSizeBox->IsTravelSelect() )
	{
		if( SfxViewShell::Current() )
		{
			Window* pShellWnd = SfxViewShell::Current()->GetWindow();

			if ( pShellWnd )
				pShellWnd->GrabFocus();
		}
	}
	
	return 0;
}

IMPL_LINK(TextPropertyPanel, FontSizeLoseFocus, FontSizeBox*, pSizeBox)
{
	if(pSizeBox == &maFontSizeBox)
	{
		mbFocusOnFontSizeCtrl = false;
	}
	return 0;
}

IMPL_LINK(TextPropertyPanel, ToolboxFontSelectHandler, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();

    switch (nId)
    {
        case TBI_BOLD:
        {
            EndTracking();
            if(meWeight != WEIGHT_BOLD)
                meWeight = WEIGHT_BOLD;
            else
                meWeight = WEIGHT_NORMAL;
            SvxWeightItem aWeightItem(meWeight, SID_ATTR_CHAR_WEIGHT);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_WEIGHT, SFX_CALLMODE_RECORD, &aWeightItem, 0L);
            UpdateItem(SID_ATTR_CHAR_WEIGHT);
            break;
        }
        case TBI_ITALIC:
        {
            EndTracking();
            if(meItalic != ITALIC_NORMAL)
                meItalic = ITALIC_NORMAL;
            else
                meItalic = ITALIC_NONE;
            SvxPostureItem aPostureItem(meItalic, SID_ATTR_CHAR_POSTURE);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_POSTURE, SFX_CALLMODE_RECORD, &aPostureItem, 0L);
            UpdateItem(SID_ATTR_CHAR_POSTURE);
            break;
        }   
        case TBI_UNDERLINE:
        {
            EndTracking();
            if(meUnderline == UNDERLINE_NONE)
            {
                meUnderline = UNDERLINE_SINGLE;
                SvxUnderlineItem aLineItem(meUnderline, SID_ATTR_CHAR_UNDERLINE);
                aLineItem.SetColor(meUnderlineColor);
                mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_UNDERLINE, SFX_CALLMODE_RECORD, &aLineItem, 0L);
            }
            else
            {
                meUnderline = UNDERLINE_NONE;
                SvxUnderlineItem aLineItem(meUnderline, SID_ATTR_CHAR_UNDERLINE);
                mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_UNDERLINE, SFX_CALLMODE_RECORD, &aLineItem, 0L);
            }
            UpdateItem(SID_ATTR_CHAR_UNDERLINE);
            break;
        }
        case TBI_STRIKEOUT:
        {
            EndTracking();
            if(meStrike !=  STRIKEOUT_NONE && meStrike != STRIKEOUT_DONTKNOW)
                meStrike = STRIKEOUT_NONE;
            else
                meStrike = STRIKEOUT_SINGLE;
            SvxCrossedOutItem aStrikeItem(meStrike,SID_ATTR_CHAR_STRIKEOUT);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_STRIKEOUT, SFX_CALLMODE_RECORD, &aStrikeItem, 0L);
            UpdateItem(SID_ATTR_CHAR_STRIKEOUT);
            break;
        }
        case TBI_SHADOWED:
        {
            EndTracking();
            mbShadow = !mbShadow;
            SvxShadowedItem aShadowItem(mbShadow, SID_ATTR_CHAR_SHADOWED);
            mpBindings->GetDispatcher()->Execute(SID_ATTR_CHAR_SHADOWED, SFX_CALLMODE_RECORD, &aShadowItem, 0L);
            UpdateItem(SID_ATTR_CHAR_SHADOWED);
            break;
        }
    }
    return 0;
}




IMPL_LINK(TextPropertyPanel, ToolboxIncDecSelectHdl, ToolBox*, pToolBox)
{
    const sal_uInt16 nId = pToolBox->GetCurItemId();

    // font size +/- enhancement in sd
    switch (maContext.GetCombinedContext_DI())
    {
        case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
        case CombinedEnumContext(Application_DrawImpress, Context_Text):
        case CombinedEnumContext(Application_DrawImpress, Context_Table):
        case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
        case CombinedEnumContext(Application_DrawImpress, Context_Draw):
        case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
        case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
            if(nId == TBI_INCREASE)
            {
                EndTracking();
                SfxVoidItem aItem(SID_GROW_FONT_SIZE);
                mpBindings->GetDispatcher()->Execute( SID_GROW_FONT_SIZE, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
            else if(nId == TBI_DECREASE)
            {
                EndTracking();
                SfxVoidItem aItem(SID_SHRINK_FONT_SIZE);
                mpBindings->GetDispatcher()->Execute( SID_SHRINK_FONT_SIZE, SFX_CALLMODE_RECORD, &aItem, 0L );
            }
            break;

        default:
            if(nId == TBI_INCREASE)
            {
                EndTracking();
                mbFocusOnFontSizeCtrl = false;
                sal_Int64 iValue = maFontSizeBox.GetValue();
                int iPos = maFontSizeBox.GetValuePos(iValue, FUNIT_NONE);
                long nSize = iValue;
                if(iPos != LISTBOX_ENTRY_NOTFOUND)
                    nSize = maFontSizeBox.GetValue(iPos+1 , FUNIT_NONE);
                else if(iValue >= 100 && iValue < 105)
                    nSize = 105;
                else if(iValue >= 105 && iValue < 110)
                    nSize = 110;
                else if(iValue < 960)
                {	
                    nSize = (nSize / 10) * 10 + 10;
                    while(maFontSizeBox.GetValuePos(nSize, FUNIT_NONE) == LISTBOX_ENTRY_NOTFOUND)
                        nSize += 10;
                }
                else
                {
                    nSize = iValue;
                }

                float fSize = (float)nSize / 10;

                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                SvxFontHeightItem aItem( CalcToUnit( fSize, eUnit ), 100, SID_ATTR_CHAR_FONTHEIGHT ) ;

                mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aItem, 0L );
                mpBindings->Invalidate(SID_ATTR_CHAR_FONTHEIGHT,true,false);
                maFontSizeBox.SetValue( nSize );
            }
            else if(nId == TBI_DECREASE)
            {
                EndTracking();
                mbFocusOnFontSizeCtrl = false;
                sal_Int64 iValue = maFontSizeBox.GetValue();
                int iPos = maFontSizeBox.GetValuePos(iValue, FUNIT_NONE);
                long nSize = iValue;
                if(iPos != LISTBOX_ENTRY_NOTFOUND)
                    nSize = maFontSizeBox.GetValue(iPos-1 , FUNIT_NONE);
                else if(iValue > 100 && iValue <= 105)
                    nSize = 100;
                else if(iValue > 105 && iValue <= 110)
                    nSize = 105;
                else if(iValue > 960)
                {
                    nSize = 960;
                }
                else if(iValue > 60)
                {	
                    nSize = (nSize / 10) * 10 ;
                    while(maFontSizeBox.GetValuePos(nSize, FUNIT_NONE) == LISTBOX_ENTRY_NOTFOUND)
                        nSize -= 10;
                }
                else
                {
                    nSize = iValue;
                }

                float fSize = (float)nSize / 10;

                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                SvxFontHeightItem aItem( CalcToUnit( fSize, eUnit ), 100, SID_ATTR_CHAR_FONTHEIGHT ) ;

                mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_FONTHEIGHT, SFX_CALLMODE_RECORD, &aItem, 0L );
                mpBindings->Invalidate(SID_ATTR_CHAR_FONTHEIGHT,true,false);
                maFontSizeBox.SetValue( nSize );
            }
	}
    UpdateItem(SID_ATTR_CHAR_FONTHEIGHT);

	return 0;
}



IMPL_LINK(TextPropertyPanel, ToolBoxUnderlineClickHdl, ToolBox*, pToolBox)
{
	const sal_uInt16 nId = pToolBox->GetCurItemId();
	OSL_ASSERT(nId == TBI_UNDERLINE);
	if(nId == TBI_UNDERLINE)
	{
		pToolBox->SetItemDown( nId, true );
		maUnderlinePopup.Rearrange(meUnderline);
		maUnderlinePopup.Show(*pToolBox);

	}
	return 0L;
}




IMPL_LINK(TextPropertyPanel, ToolBoxSwScriptSelectHdl, ToolBox*, pToolBox)
{
	const sal_uInt16 nId = pToolBox->GetCurItemId();
	if( nId == TBI_SUPER_SW )
	{
		if(meEscape != SVX_ESCAPEMENT_SUPERSCRIPT)
		{
			meEscape = SVX_ESCAPEMENT_SUPERSCRIPT;
			SvxEscapementItem aSupItem(DFLT_ESC_SUPER, DFLT_ESC_PROP, SID_ATTR_CHAR_ESCAPEMENT);
			mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aSupItem, 0L );
		}
		else
		{			
			meEscape = SVX_ESCAPEMENT_OFF;
			SvxEscapementItem aNoneItem(0, 100, SID_ATTR_CHAR_ESCAPEMENT);
			mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aNoneItem, 0L );
		}
	}
	else if(TBI_SUB_SW == nId)
	{
		if(meEscape != SVX_ESCAPEMENT_SUBSCRIPT)
		{
			meEscape = (SvxEscapement)SVX_ESCAPEMENT_SUBSCRIPT;
			SvxEscapementItem aSubItem(DFLT_ESC_SUB, DFLT_ESC_PROP, SID_ATTR_CHAR_ESCAPEMENT);
			mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aSubItem, 0L );
		}
		else
		{
			meEscape = SVX_ESCAPEMENT_OFF;
			SvxEscapementItem aNoneItem(0, 100, SID_ATTR_CHAR_ESCAPEMENT);
			mpBindings->GetDispatcher()->Execute( SID_ATTR_CHAR_ESCAPEMENT, SFX_CALLMODE_RECORD, &aNoneItem, 0L );
		}
	}
    UpdateItem(SID_ATTR_CHAR_ESCAPEMENT);

	return 0;
}




IMPL_LINK(TextPropertyPanel, ToolBoxScriptSelectHdl, ToolBox*, pToolBox)
{
	const sal_uInt16 nId = pToolBox->GetCurItemId();
	if( nId == TBI_SUPER )
	{
		mbSuper = !mbSuper;
		SfxBoolItem aSupItem(SID_SET_SUPER_SCRIPT, mbSuper);
		mpBindings->GetDispatcher()->Execute( SID_SET_SUPER_SCRIPT, SFX_CALLMODE_RECORD, &aSupItem, 0L );
        UpdateItem(SID_SET_SUPER_SCRIPT);
	}
	else if(TBI_SUB == nId)
	{

		mbSub = !mbSub;
		SfxBoolItem aSubItem(SID_SET_SUB_SCRIPT, mbSub );
		mpBindings->GetDispatcher()->Execute( SID_SET_SUB_SCRIPT, SFX_CALLMODE_RECORD, &aSubItem, 0L );
        UpdateItem(SID_SET_SUB_SCRIPT);
	}
	return 0;
}




IMPL_LINK(TextPropertyPanel, SpacingClickHdl, ToolBox*, pToolBox)
{
	const sal_uInt16 nId = pToolBox->GetCurItemId();
	OSL_ASSERT(nId == TBI_SPACING);
	if(nId == TBI_SPACING)
	{
		pToolBox->SetItemDown( nId, true );
		maCharSpacePopup.Rearrange(mbKernLBAvailable,mbKernAvailable,mlKerning);
		maCharSpacePopup.Show(*pToolBox);

	}
	return 0L;
}




void TextPropertyPanel::NotifyItemUpdate (
    const sal_uInt16 nSID,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    switch(nSID)
    {
        case SID_ATTR_CHAR_FONT:
        {
            bool bIsControlEnabled (bIsEnabled);
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontItem) )
            {
                const SvxFontItem* pFontItem = (const SvxFontItem*)pState;
                mpFontNameBox->SetText( pFontItem->GetFamilyName() );
            }
            else
            {
                mpFontNameBox->SetText( String() );
                if (SFX_ITEM_DISABLED == eState)
                    bIsControlEnabled = false;
            }
            mpFontNameBox->Enable(bIsControlEnabled);
            break;
        }
        case SID_ATTR_CHAR_FONTHEIGHT:
        {
            bool bIsControlEnabled (bIsEnabled);
            if (  eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxFontHeightItem) )
            {
                mpHeightItem = (SvxFontHeightItem*)pState;//const SvxFontHeightItem* 
                SfxMapUnit eUnit = maFontSizeControl.GetCoreMetric();
                const sal_Int64 nValue (CalcToPoint(mpHeightItem->GetHeight(), eUnit, 10 ));
                mpToolBoxIncDec->Enable();

                mpToolBoxIncDec->SetItemState(TBI_INCREASE, STATE_NOCHECK);
                mpToolBoxIncDec->SetItemState(TBI_DECREASE, STATE_NOCHECK);

                // For Writer we have to update the states of the
                // increase and decrease buttons here, because we have
                // no access to the slots used by Writer.
                switch(maContext.GetCombinedContext_DI())
                {
                    case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
                    case CombinedEnumContext(Application_DrawImpress, Context_Text):
                    case CombinedEnumContext(Application_DrawImpress, Context_Table):
                    case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
                    case CombinedEnumContext(Application_DrawImpress, Context_Draw):
                    case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
                    case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
                        break;

                    default:
                    {
                        mpToolBoxIncDec->EnableItem(TBI_INCREASE, bIsEnabled && nValue<960);
                        mpToolBoxIncDec->EnableItem(TBI_DECREASE, bIsEnabled && nValue>60);
                        break;
                    }
                }
                
                if( mbFocusOnFontSizeCtrl )
                    return;

                maFontSizeBox.SetValue(nValue);	
                maFontSizeBox.LoseFocus();

                UpdateItem(SID_SHRINK_FONT_SIZE);
                UpdateItem(SID_GROW_FONT_SIZE);
            }
            else
            {
                mpHeightItem = NULL;
                maFontSizeBox.SetText( String() );
                //increase decrease diabled when multi-seletion have different font size

                // font size +/- enhancement in sd
                switch(maContext.GetCombinedContext_DI())
                {
                    case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
                    case CombinedEnumContext(Application_DrawImpress, Context_Text):
                    case CombinedEnumContext(Application_DrawImpress, Context_Table):
                    case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
                    case CombinedEnumContext(Application_DrawImpress, Context_Draw):
                    case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
                    case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
                        break;

                    default:
                        mpToolBoxIncDec->Disable();
                }
                if ( eState <= SFX_ITEM_READONLY )
                    bIsControlEnabled = false;
            }
            maFontSizeBox.Enable(bIsControlEnabled);
            break;
        }
            
        case SID_ATTR_CHAR_WEIGHT:
            mbWeightAvailable = (eState >= SFX_ITEM_DONTCARE);
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxWeightItem))
            {
                const SvxWeightItem* pItem = (const SvxWeightItem*)pState;
                meWeight = (FontWeight)pItem->GetValue();
            }
            else
            {
                meWeight = WEIGHT_NORMAL;
            }
            mpToolBoxFont->EnableItem(TBI_BOLD, mbWeightAvailable && bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_BOLD, meWeight==WEIGHT_BOLD ? STATE_CHECK : STATE_NOCHECK);
            break;
            
        case SID_ATTR_CHAR_POSTURE:
            mbPostureAvailable = (eState >= SFX_ITEM_DONTCARE);
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxPostureItem))
            {
                const SvxPostureItem* pItem = (const SvxPostureItem*)pState;
                meItalic = (FontItalic)pItem->GetValue();
            }
            else
            {
                meItalic = ITALIC_NONE;
            }
            mpToolBoxFont->EnableItem(TBI_ITALIC, mbPostureAvailable && bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_ITALIC,	meItalic==ITALIC_NORMAL ? STATE_CHECK : STATE_NOCHECK);
            break;
            
        case SID_ATTR_CHAR_UNDERLINE:
            if( eState >= SFX_ITEM_DEFAULT)
            {
                if(pState->ISA(SvxUnderlineItem))
                {
                    const SvxUnderlineItem* pItem = (const SvxUnderlineItem*)pState;
                    meUnderline = (FontUnderline)pItem->GetValue();
                    meUnderlineColor = pItem->GetColor(); 
                }
            }
            else
            {
                meUnderline = UNDERLINE_NONE;
            }
            mpToolBoxFont->EnableItem(TBI_UNDERLINE, bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_UNDERLINE, meUnderline==UNDERLINE_NONE ? STATE_NOCHECK : STATE_CHECK);
            break;
            
        case SID_ATTR_CHAR_SHADOWED:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxShadowedItem))
            {
                const SvxShadowedItem* pItem = (const SvxShadowedItem*)pState;
                mbShadow = pItem->GetValue();
            }
            else
            {
                mbShadow = false;
            }
            mpToolBoxFont->EnableItem(TBI_SHADOWED, bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_SHADOWED, mbShadow ? STATE_CHECK : STATE_NOCHECK);
            break;
            
        case SID_ATTR_CHAR_STRIKEOUT:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SvxCrossedOutItem))
            {
                const SvxCrossedOutItem* pItem = (const SvxCrossedOutItem*)pState;
                meStrike = (FontStrikeout)pItem->GetValue();
            }
            else
            {
                meStrike = STRIKEOUT_NONE;
            }
            mpToolBoxFont->EnableItem(TBI_STRIKEOUT, bIsEnabled);
            mpToolBoxFont->SetItemState(TBI_STRIKEOUT,
                meStrike!=STRIKEOUT_NONE && meStrike!=STRIKEOUT_DONTKNOW
                    ? STATE_CHECK
                    : STATE_NOCHECK);
            break;

        case SID_ATTR_CHAR_ESCAPEMENT:
        {
            bool bIsItemEnabled (true);
            if (eState == SFX_ITEM_AVAILABLE)
            {
                if (pState->ISA(SvxEscapementItem))
                {
                    const SvxEscapementItem* pItem = (const SvxEscapementItem *)pState;
                    short nEsc = pItem->GetEsc();
                    if(nEsc == 0)
                    {
                        meEscape = SVX_ESCAPEMENT_OFF;
                        mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_NOCHECK);
                        mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_NOCHECK);
                    }
                    else if(nEsc > 0)
                    {
                        meEscape = SVX_ESCAPEMENT_SUPERSCRIPT;
                        mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_CHECK);
                        mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_NOCHECK);
                    }
                    else
                    {
                        meEscape = SVX_ESCAPEMENT_SUBSCRIPT;
                        mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_NOCHECK);
                        mpToolBoxScriptSw->SetItemState(TBI_SUB_SW, STATE_CHECK);
                    }
                }
                else
                {
                    meEscape = SVX_ESCAPEMENT_OFF;
                    mpToolBoxScriptSw->SetItemState(TBI_SUPER_SW, STATE_NOCHECK);
                    mpToolBoxScriptSw->SetItemState(TBI_SUB_SW,	STATE_NOCHECK);
                }
            }
            else if (eState == SFX_ITEM_DISABLED)
            {
                bIsItemEnabled = false;
            }
            else
            {
                meEscape = SVX_ESCAPEMENT_OFF;
            }	
            mpToolBoxScriptSw->EnableItem(TBI_SUPER_SW, bIsItemEnabled && bIsEnabled);
            mpToolBoxScriptSw->EnableItem(TBI_SUB_SW, bIsItemEnabled && bIsEnabled);
            break;
        }
        
        case SID_SET_SUB_SCRIPT:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SfxBoolItem))
            {
                const SfxBoolItem* pItem = (const SfxBoolItem*)pState;
                mbSub = pItem->GetValue();
            }
            else
            {
                mbSub = false;
            }
            mpToolBoxScript->EnableItem(TBI_SUB, bIsEnabled);
            mpToolBoxScript->SetItemState(TBI_SUB, mbSub ? STATE_CHECK : STATE_NOCHECK);
            break;
            
        case SID_SET_SUPER_SCRIPT:
            if( eState >= SFX_ITEM_DEFAULT && pState->ISA(SfxBoolItem))
            {
                const SfxBoolItem* pItem = (const SfxBoolItem*)pState;
                mbSuper = pItem->GetValue();
            }
            else
            {
                mbSuper = false;
            }
            mpToolBoxScript->EnableItem(TBI_SUPER, bIsEnabled);
            mpToolBoxScript->SetItemState(TBI_SUPER, mbSuper ? STATE_CHECK : STATE_NOCHECK);
            break;
            
        case SID_ATTR_CHAR_KERNING:     
            if ( SFX_ITEM_AVAILABLE == eState )
            {
                mbKernLBAvailable = true;
            
                if(pState->ISA(SvxKerningItem))
                {
                    const SvxKerningItem* pKerningItem  = (const SvxKerningItem*)pState;      
                    mlKerning = (long)pKerningItem->GetValue();			
                    mbKernAvailable = true;
                }
                else
                {
                    mlKerning = 0;
                    mbKernAvailable =false;
                }
            }
            else if (SFX_ITEM_DISABLED == eState)
            {
                mbKernLBAvailable = false;
                mbKernAvailable = false;
                mlKerning = 0;
            }
            else
            {
                mbKernLBAvailable = true;
                mbKernAvailable = false;	
                mlKerning = 0;
            }
            mpToolBoxSpacing->EnableItem(TBI_SPACING, bIsEnabled);
            break;

            // font size +/- enhancement in sd
        case SID_SHRINK_FONT_SIZE:
        case SID_GROW_FONT_SIZE:
            switch(maContext.GetCombinedContext_DI())
            {
                case CombinedEnumContext(Application_DrawImpress, Context_DrawText):
                case CombinedEnumContext(Application_DrawImpress, Context_Text):
                case CombinedEnumContext(Application_DrawImpress, Context_Table):
                case CombinedEnumContext(Application_DrawImpress, Context_OutlineText):
                case CombinedEnumContext(Application_DrawImpress, Context_Draw):
                case CombinedEnumContext(Application_DrawImpress, Context_TextObject):
                case CombinedEnumContext(Application_DrawImpress, Context_Graphic):
                {
                    if(eState == SFX_ITEM_DISABLED)
                        mpToolBoxIncDec->Disable();
                    else
                        mpToolBoxIncDec->Enable();
                    const sal_Int64 nSize (maFontSizeBox.GetValue());
                    switch(nSID)
                    {
                        case SID_GROW_FONT_SIZE:
                            mpToolBoxIncDec->EnableItem(TBI_INCREASE, bIsEnabled && nSize<960);
                            break;

                        case SID_SHRINK_FONT_SIZE:
                            mpToolBoxIncDec->EnableItem(TBI_DECREASE, bIsEnabled && nSize>60);
                            break;

                        default:
                            break;
                    }
                }
            }
            break;
    }
}




void TextPropertyPanel::Resize (void)
{
    maLayouter.Layout();
}




void TextPropertyPanel::UpdateItem (const sal_uInt16 nSlotId)
{
    switch (nSlotId)
    {
        case SID_ATTR_CHAR_FONT:
            maFontNameControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_FONTHEIGHT:
            maFontSizeControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_WEIGHT:
            maWeightControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_POSTURE:
            maItalicControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_UNDERLINE:
            maUnderlineControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_STRIKEOUT:
            maStrikeControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_SHADOWED:
            maShadowControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_ESCAPEMENT:
            maScriptControlSw.RequestUpdate();
            break;
        case SID_SET_SUPER_SCRIPT:
            maSuperScriptControl.RequestUpdate();
            break;
        case SID_SET_SUB_SCRIPT:
            maSubScriptControl.RequestUpdate();
            break;
        case SID_ATTR_CHAR_KERNING:
            maSpacingControl.RequestUpdate();
            break;
        case SID_GROW_FONT_SIZE:
            maSDFontGrow.RequestUpdate();
            break;
        case SID_SHRINK_FONT_SIZE:
            maSDFontShrink.RequestUpdate();
            break;
    }
}





Color& TextPropertyPanel::GetUnderlineColor() 
{
	return meUnderlineColor;
}

void TextPropertyPanel::SetUnderline(FontUnderline	eUnderline)
{
	meUnderline = eUnderline;
}



} } // end of namespace svx::sidebar
