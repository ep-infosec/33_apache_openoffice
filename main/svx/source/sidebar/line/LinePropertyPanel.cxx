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

#include <sfx2/sidebar/ResourceDefinitions.hrc>
#include <sfx2/sidebar/Theme.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/sidebar/Layouter.hxx>
#include <LinePropertyPanel.hxx>
#include <LinePropertyPanel.hrc>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xtable.hxx>
#include <svx/xdash.hxx>
#include <svx/drawitem.hxx>
#include <svx/svxitems.hrc>
#include <svtools/valueset.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <i18npool/mslangid.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <vcl/svapp.hxx>
#include <svx/xlnwtit.hxx>
#include <vcl/lstbox.hxx>
#include <svx/tbxcolorupdate.hxx>
#include <vcl/toolbox.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlnedit.hxx>
#include <svx/xlncapit.hxx>
#include <svx/xlinjoit.hxx>
#include "svx/sidebar/PopupContainer.hxx"
#include "svx/sidebar/PopupControl.hxx"
#include <svx/sidebar/ColorControl.hxx>
#include "LineWidthControl.hxx"
#include <boost/bind.hpp>

using namespace css;
using namespace cssu;
using ::sfx2::sidebar::Layouter;
using ::sfx2::sidebar::Theme;


#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace {
    short GetItemId_Impl_line( ValueSet& rValueSet, const Color& rCol )
    {
        if(rCol == COL_AUTO)
            return 0;

        bool	bFound = false;
        sal_uInt16 nCount = rValueSet.GetItemCount();
        sal_uInt16	n	   = 1;

        while ( !bFound && n <= nCount )
        {
            Color aValCol = rValueSet.GetItemColor(n);

            bFound = (   aValCol.GetRed()   == rCol.GetRed()
                && aValCol.GetGreen() == rCol.GetGreen()
                && aValCol.GetBlue()  == rCol.GetBlue() );

            if ( !bFound )
                n++;
        }
        return bFound ? n : -1;
    }

    void FillLineEndListBox(ListBox& rListBoxStart, ListBox& rListBoxEnd, const XLineEndListSharedPtr aList)
    {
        const sal_uInt32 nCount(aList.get() ? aList->Count() : 0);
        const String sNone(SVX_RES(RID_SVXSTR_NONE));

        rListBoxStart.SetUpdateMode(false);
        rListBoxEnd.SetUpdateMode(false);

        rListBoxStart.Clear();
        rListBoxEnd.Clear();

        // add 'none' entries
        rListBoxStart.InsertEntry(sNone);
        rListBoxEnd.InsertEntry(sNone);

        for(sal_uInt32 i(0); i < nCount; i++)
        {
            XLineEndEntry* pEntry = aList->GetLineEnd(i);
            const Bitmap aBitmap = aList->GetUiBitmap(i);

            if(!aBitmap.IsEmpty())
            {
                Bitmap aCopyStart(aBitmap);
                Bitmap aCopyEnd(aBitmap);
                // delete pBitmap;
                const Size aBmpSize(aCopyStart.GetSizePixel());
                const Rectangle aCropRectStart(Point(), Size(aBmpSize.Width() / 2, aBmpSize.Height()));
                const Rectangle aCropRectEnd(Point(aBmpSize.Width() / 2, 0), Size(aBmpSize.Width() / 2, aBmpSize.Height()));

                aCopyStart.Crop(aCropRectStart);
                rListBoxStart.InsertEntry(
                    pEntry->GetName(),
                    aCopyStart);

                aCopyEnd.Crop(aCropRectEnd);
                rListBoxEnd.InsertEntry(
                    pEntry->GetName(),
                    aCopyEnd);
            }
            else
            {
                rListBoxStart.InsertEntry(pEntry->GetName());
                rListBoxEnd.InsertEntry(pEntry->GetName());
            }
        }

        rListBoxStart.SetUpdateMode(true);
        rListBoxEnd.SetUpdateMode(true);
    }

    void FillLineStyleListBox(ListBox& rListBox, const XDashListSharedPtr aList)
    {
        const sal_uInt32 nCount(aList.get() ? aList->Count() : 0);
        rListBox.SetUpdateMode(false);

        rListBox.Clear();

        // entry for 'none'
        rListBox.InsertEntry(aList->GetStringForUiNoLine());

        // entry for solid line
        rListBox.InsertEntry(aList->GetStringForUiSolidLine(), aList->GetBitmapForUISolidLine());

        for(sal_uInt32 i(0); i < nCount; i++)
        {
            XDashEntry* pEntry = aList->GetDash(i);
            const Bitmap aBitmap = aList->GetUiBitmap(i);

            if(!aBitmap.IsEmpty())
            {
                rListBox.InsertEntry(
                    pEntry->GetName(),
                    aBitmap);
                // delete pBitmap;
            }
            else
            {
                rListBox.InsertEntry(pEntry->GetName());
            }
        }

        rListBox.SetUpdateMode(true);
    }
} // end of anonymous namespace

// namespace open

namespace svx { namespace sidebar {

LinePropertyPanel::LinePropertyPanel(
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
:   Control(
        pParent, 
        SVX_RES(RID_SIDEBAR_LINE_PANEL)),
    mpFTWidth(new FixedText(this, SVX_RES(FT_WIDTH))),
    mpTBWidthBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBWidth(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBWidthBackground.get(), SVX_RES(TB_WIDTH))),
    mpFTColor(new FixedText(this, SVX_RES(FT_COLOR))),
    mpTBColorBackground(sfx2::sidebar::ControlFactory::CreateToolBoxBackground(this)),
    mpTBColor(sfx2::sidebar::ControlFactory::CreateToolBox(mpTBColorBackground.get(), SVX_RES(TB_COLOR))),
    mpFTStyle(new FixedText(this, SVX_RES(FT_STYLE))),
    mpLBStyle(new ListBox(this, SVX_RES(LB_STYLE))),
    mpFTTrancparency(new FixedText(this, SVX_RES(FT_TRANSPARENT))),
    mpMFTransparent(new MetricField(this, SVX_RES(MF_TRANSPARENT))),
    mpFTArrow(new FixedText(this, SVX_RES(FT_ARROW))),
    mpLBStart(new ListBox(this, SVX_RES(LB_START))),
    mpLBEnd(new ListBox(this, SVX_RES(LB_END))),
    mpFTEdgeStyle(new FixedText(this, SVX_RES(FT_EDGESTYLE))),
    mpLBEdgeStyle(new ListBox(this, SVX_RES(LB_EDGESTYLE))),
    mpFTCapStyle(new FixedText(this, SVX_RES(FT_CAPSTYLE))),
    mpLBCapStyle(new ListBox(this, SVX_RES(LB_CAPSTYLE))),
    maStyleControl(SID_ATTR_LINE_STYLE, *pBindings, *this),
    maDashControl (SID_ATTR_LINE_DASH, *pBindings, *this),
    maWidthControl(SID_ATTR_LINE_WIDTH, *pBindings, *this),
    maColorControl(SID_ATTR_LINE_COLOR, *pBindings, *this),
    maStartControl(SID_ATTR_LINE_START, *pBindings, *this),
    maEndControl(SID_ATTR_LINE_END, *pBindings, *this),
    maLineEndListControl(SID_LINEEND_LIST, *pBindings, *this),
    maLineStyleListControl(SID_DASH_LIST, *pBindings, *this),
    maTransControl(SID_ATTR_LINE_TRANSPARENCE, *pBindings, *this),
    maEdgeStyle(SID_ATTR_LINE_JOINT, *pBindings, *this),
    maCapStyle(SID_ATTR_LINE_CAP, *pBindings, *this),
    maColor(COL_BLACK),
    mpColorUpdater(new ::svx::ToolboxButtonColorUpdater(SID_ATTR_LINE_COLOR, TBI_COLOR, mpTBColor.get(), TBX_UPDATER_MODE_CHAR_COLOR_NEW)),
    mpStyleItem(),
    mpDashItem(),
    mnTrans(0),
    meMapUnit(SFX_MAPUNIT_MM),
    mnWidthCoreValue(0),
    maLineEndList(),
    maLineStyleList(),
    mpStartItem(0),
    mpEndItem(0),
    maColorPopup(this, ::boost::bind(&LinePropertyPanel::CreateColorPopupControl, this, _1)),
    maLineWidthPopup(this, ::boost::bind(&LinePropertyPanel::CreateLineWidthPopupControl, this, _1)),
    maIMGColor(SVX_RES(IMG_COLOR)),
    maIMGNone(SVX_RES(IMG_NONE_ICON)),
    mpIMGWidthIcon(),
    mpIMGWidthIconH(),
    mxFrame(rxFrame),
    mpBindings(pBindings),
    mbColorAvailable(true),
    mbWidthValuable(true),
    maLayouter(*this)
{
    Initialize();
    FreeResource();

    // Setup the grid layouter.
    const sal_Int32 nMappedToolBoxWidth (Layouter::MapWidth(*this, TOOLBOX_WIDTH));
    
    maLayouter.GetCell(0,0).SetControl(*mpFTWidth);
    maLayouter.GetCell(1,0).SetControl(*mpTBWidthBackground).SetFixedWidth();

    maLayouter.GetCell(0,2).SetControl(*mpFTColor);
    maLayouter.GetCell(1,2).SetControl(*mpTBColorBackground).SetFixedWidth();

    maLayouter.GetCell(2,0).SetControl(*mpFTStyle);
    maLayouter.GetCell(3,0).SetControl(*mpLBStyle);
    
    maLayouter.GetCell(2,2).SetControl(*mpFTTrancparency);
    maLayouter.GetCell(3,2).SetControl(*mpMFTransparent);
    
    maLayouter.GetCell(4,0).SetControl(*mpFTArrow).SetGridWidth(3);
    maLayouter.GetCell(5,0).SetControl(*mpLBStart);
    maLayouter.GetCell(5,2).SetControl(*mpLBEnd);
        
    maLayouter.GetCell(6,0).SetControl(*mpFTEdgeStyle);
    maLayouter.GetCell(7,0).SetControl(*mpLBEdgeStyle);
    
    maLayouter.GetCell(6,2).SetControl(*mpFTCapStyle);
    maLayouter.GetCell(7,2).SetControl(*mpLBCapStyle);

    maLayouter.GetColumn(0)
        .SetWeight(1)
        .SetLeftPadding(Layouter::MapWidth(*this,SECTIONPAGE_MARGIN_HORIZONTAL))
        .SetMinimumWidth(nMappedToolBoxWidth);
    maLayouter.GetColumn(1)
        .SetWeight(0)
        .SetMinimumWidth(Layouter::MapWidth(*this, CONTROL_SPACING_HORIZONTAL));
    maLayouter.GetColumn(2)
        .SetWeight(1)
        .SetRightPadding(Layouter::MapWidth(*this,SECTIONPAGE_MARGIN_HORIZONTAL))
        .SetMinimumWidth(nMappedToolBoxWidth);

    // Make controls that display text handle short widths more
    // graceful.
    Layouter::PrepareForLayouting(*mpFTWidth);
    Layouter::PrepareForLayouting(*mpFTColor);
    Layouter::PrepareForLayouting(*mpFTStyle);
    Layouter::PrepareForLayouting(*mpFTTrancparency);
    Layouter::PrepareForLayouting(*mpFTArrow);
    Layouter::PrepareForLayouting(*mpFTEdgeStyle);
    Layouter::PrepareForLayouting(*mpFTCapStyle);
}



LinePropertyPanel::~LinePropertyPanel()
{
    // Destroy the toolboxes, then their background windows.
    mpTBWidth.reset();
    mpTBColor.reset();

    mpTBWidthBackground.reset();
    mpTBColorBackground.reset();
}



void LinePropertyPanel::Initialize()
{
    mpFTWidth->SetBackground(Wallpaper());
    mpFTColor->SetBackground(Wallpaper());
    mpFTStyle->SetBackground(Wallpaper());
    mpFTTrancparency->SetBackground(Wallpaper());
    mpFTArrow->SetBackground(Wallpaper());
    mpFTEdgeStyle->SetBackground(Wallpaper());
    mpFTCapStyle->SetBackground(Wallpaper());

    mpIMGWidthIcon.reset(new Image[8]);
    mpIMGWidthIcon[0] = Image(SVX_RES(IMG_WIDTH1_ICON));
    mpIMGWidthIcon[1] = Image(SVX_RES(IMG_WIDTH2_ICON));
    mpIMGWidthIcon[2] = Image(SVX_RES(IMG_WIDTH3_ICON));
    mpIMGWidthIcon[3] = Image(SVX_RES(IMG_WIDTH4_ICON));
    mpIMGWidthIcon[4] = Image(SVX_RES(IMG_WIDTH5_ICON));
    mpIMGWidthIcon[5] = Image(SVX_RES(IMG_WIDTH6_ICON));
    mpIMGWidthIcon[6] = Image(SVX_RES(IMG_WIDTH7_ICON));
    mpIMGWidthIcon[7] = Image(SVX_RES(IMG_WIDTH8_ICON));

    //high contrast
    mpIMGWidthIconH.reset(new Image[8]);
    mpIMGWidthIconH[0] = Image(SVX_RES(IMG_WIDTH1_ICON_H));
    mpIMGWidthIconH[1] = Image(SVX_RES(IMG_WIDTH2_ICON_H));
    mpIMGWidthIconH[2] = Image(SVX_RES(IMG_WIDTH3_ICON_H));
    mpIMGWidthIconH[3] = Image(SVX_RES(IMG_WIDTH4_ICON_H));
    mpIMGWidthIconH[4] = Image(SVX_RES(IMG_WIDTH5_ICON_H));
    mpIMGWidthIconH[5] = Image(SVX_RES(IMG_WIDTH6_ICON_H));
    mpIMGWidthIconH[6] = Image(SVX_RES(IMG_WIDTH7_ICON_H));
    mpIMGWidthIconH[7] = Image(SVX_RES(IMG_WIDTH8_ICON_H));

    meMapUnit = maWidthControl.GetCoreMetric();

    mpTBColor->SetItemImage(TBI_COLOR, maIMGColor);
    Size aTbxSize( mpTBColor->CalcWindowSizePixel() );
    mpTBColor->SetOutputSizePixel( aTbxSize );
    mpTBColor->SetItemBits( TBI_COLOR, mpTBColor->GetItemBits( TBI_COLOR ) | TIB_DROPDOWNONLY );
    mpTBColor->SetQuickHelpText(TBI_COLOR,String(SVX_RES(STR_QH_TB_COLOR))); //Add
    mpTBColor->SetBackground(Wallpaper());
    mpTBColor->SetPaintTransparent(true);
    Link aLink = LINK(this, LinePropertyPanel, ToolboxColorSelectHdl);
    mpTBColor->SetDropdownClickHdl ( aLink );
    mpTBColor->SetSelectHdl ( aLink );

    FillLineStyleList();
    SelectLineStyle();
    aLink = LINK( this, LinePropertyPanel, ChangeLineStyleHdl );
    mpLBStyle->SetSelectHdl( aLink );
    mpLBStyle->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Style")));
    mpLBStyle->AdaptDropDownLineCountToMaximum();

    mpTBWidth->SetItemImage(TBI_WIDTH, mpIMGWidthIcon[0]);
    aTbxSize = mpTBWidth->CalcWindowSizePixel() ;
    mpTBWidth->SetOutputSizePixel( aTbxSize );
    mpTBWidth->SetItemBits( TBI_WIDTH, mpTBWidth->GetItemBits( TBI_WIDTH ) | TIB_DROPDOWNONLY );
    mpTBWidth->SetQuickHelpText(TBI_WIDTH,String(SVX_RES(STR_QH_TB_WIDTH))); //Add
    mpTBWidth->SetBackground(Wallpaper());
    mpTBWidth->SetPaintTransparent(true);
    aLink = LINK(this, LinePropertyPanel, ToolboxWidthSelectHdl);
    mpTBWidth->SetDropdownClickHdl ( aLink );
    mpTBWidth->SetSelectHdl ( aLink );

    FillLineEndList();
    SelectEndStyle(true);
    SelectEndStyle(false);
    aLink = LINK( this, LinePropertyPanel, ChangeStartHdl );
    mpLBStart->SetSelectHdl( aLink );
    mpLBStart->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Beginning Style")));	//wj acc
    mpLBStart->AdaptDropDownLineCountToMaximum();
    aLink = LINK( this, LinePropertyPanel, ChangeEndHdl );
    mpLBEnd->SetSelectHdl( aLink );
    mpLBEnd->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Ending Style")));	//wj acc
    mpLBEnd->AdaptDropDownLineCountToMaximum();

    aLink = LINK(this, LinePropertyPanel, ChangeTransparentHdl);
    mpMFTransparent->SetModifyHdl(aLink);
    mpMFTransparent->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Transparency")));	//wj acc
    
    mpTBWidth->SetAccessibleRelationLabeledBy(mpFTWidth.get());
    mpTBColor->SetAccessibleRelationLabeledBy(mpFTColor.get());
    mpLBStyle->SetAccessibleRelationLabeledBy(mpFTStyle.get());
    mpMFTransparent->SetAccessibleRelationLabeledBy(mpFTTrancparency.get());
    mpLBStart->SetAccessibleRelationLabeledBy(mpFTArrow.get());
    mpLBEnd->SetAccessibleRelationLabeledBy(mpLBEnd.get());

    aLink = LINK( this, LinePropertyPanel, ChangeEdgeStyleHdl );
    mpLBEdgeStyle->SetSelectHdl( aLink );
    mpLBEdgeStyle->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Corner Style")));

    aLink = LINK( this, LinePropertyPanel, ChangeCapStyleHdl );
    mpLBCapStyle->SetSelectHdl( aLink );
    mpLBCapStyle->SetAccessibleName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Cap Style")));
}



void LinePropertyPanel::SetupIcons(void)
{
    if(Theme::GetBoolean(Theme::Bool_UseSymphonyIcons))
    {
        // todo
    }
    else
    {
        // todo
    }
}



LinePropertyPanel* LinePropertyPanel::Create (
    Window* pParent,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw lang::IllegalArgumentException(A2S("no parent Window given to LinePropertyPanel::Create"), NULL, 0);
    if ( ! rxFrame.is())
        throw lang::IllegalArgumentException(A2S("no XFrame given to LinePropertyPanel::Create"), NULL, 1);
    if (pBindings == NULL)
        throw lang::IllegalArgumentException(A2S("no SfxBindings given to LinePropertyPanel::Create"), NULL, 2);
    
    return new LinePropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}




void LinePropertyPanel::DataChanged(
    const DataChangedEvent& rEvent)
{
    (void)rEvent;
    
    SetupIcons();
}




void LinePropertyPanel::NotifyItemUpdate( 
    sal_uInt16 nSID, 
    SfxItemState eState, 
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;
    const bool bDisabled(SFX_ITEM_DISABLED == eState);

    switch(nSID)
    {
        case SID_ATTR_LINE_COLOR:
        {
            if(bDisabled)
            { 
                mpFTColor->Disable();
                mpTBColor->Disable();
            }
            else
            {
                mpFTColor->Enable();
                mpTBColor->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineColorItem* pItem = dynamic_cast< const XLineColorItem* >(pState);
                if(pItem)
                {
                    maColor = pItem->GetColorValue();
                    mbColorAvailable = true;
                    mpColorUpdater->Update(maColor);
                    break;
                }
            }

            mbColorAvailable = false;
            mpColorUpdater->Update(COL_WHITE);
            break;
        }
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_STYLE:
        {
            if(bDisabled)
            {
                mpFTStyle->Disable();
                mpLBStyle->Disable();
            }
            else
            {
                mpFTStyle->Enable();
                mpLBStyle->Enable();
            }

            if(eState  >= SFX_ITEM_DEFAULT)
            {
                if(nSID == SID_ATTR_LINE_STYLE)
                {
                    const XLineStyleItem* pItem = dynamic_cast< const XLineStyleItem* >(pState);

                    if(pItem)
                    {
                        mpStyleItem.reset(pState ? (XLineStyleItem*)pItem->Clone() : 0);
                    }
                }
                else // if(nSID == SID_ATTR_LINE_DASH)
                {
                    const XLineDashItem* pItem = dynamic_cast< const XLineDashItem* >(pState);

                    if(pItem)
                    {
                        mpDashItem.reset(pState ? (XLineDashItem*)pItem->Clone() : 0);
                    }
                }
            }
            else
            {
                if(nSID == SID_ATTR_LINE_STYLE)
                {
                    mpStyleItem.reset(0);
                }
                else 
                {
                    mpDashItem.reset(0);
                }
            }

            SelectLineStyle();
            break;
        }
        case SID_ATTR_LINE_TRANSPARENCE:
        {
            if(bDisabled)
            {
                mpFTTrancparency->Disable();
                mpMFTransparent->Disable();
            }
            else
            {
                mpFTTrancparency->Enable();
                mpMFTransparent->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineTransparenceItem* pItem = dynamic_cast< const XLineTransparenceItem* >(pState);

                if(pItem)
                {
                    mnTrans = pItem->GetValue();
                    mpMFTransparent->SetValue(mnTrans);
                    break;
                }
            }

            mpMFTransparent->SetValue(0);//add 
            mpMFTransparent->SetText(String());
            break;
        }
        case SID_ATTR_LINE_WIDTH:
        {
            if(bDisabled)
            {
                mpTBWidth->Disable();
                mpFTWidth->Disable();
            }
            else
            {
                mpTBWidth->Enable();
                mpFTWidth->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineWidthItem* pItem = dynamic_cast< const XLineWidthItem* >(pState);

                if(pItem)
                {
                    mnWidthCoreValue = pItem->GetValue();
                    mbWidthValuable = true;
                    SetWidthIcon();
                    break;
                }
            }

            mbWidthValuable = false;
            SetWidthIcon();
            break;
        }
        case SID_ATTR_LINE_START:
        {
            if(bDisabled)
            {
                mpFTArrow->Disable();
                mpLBStart->Disable();
            }
            else
            {
                mpFTArrow->Enable();
                mpLBStart->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineStartItem* pItem = dynamic_cast< const XLineStartItem* >(pState);

                if(pItem)
                {
                    mpStartItem.reset(pItem ? (XLineStartItem*)pItem->Clone() : 0);
                    SelectEndStyle(true);
                    break;
                }
            }

            mpStartItem.reset(0);
            SelectEndStyle(true);
            break;
        }
        case SID_ATTR_LINE_END:
        {
            if(bDisabled)
            {
                mpFTArrow->Disable();
                mpLBEnd->Disable();
            }
            else
            {
                mpFTArrow->Enable();
                mpLBEnd->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineEndItem* pItem = dynamic_cast< const XLineEndItem* >(pState);

                if(pItem)
                {
                    mpEndItem.reset(pItem ? (XLineEndItem*)pItem->Clone() : 0);
                    SelectEndStyle(false);
                    break;
                }
            }

            mpEndItem.reset(0);
            SelectEndStyle(false);
            break;
        }
        case SID_LINEEND_LIST:
        {
            FillLineEndList();
            SelectEndStyle(true);
            SelectEndStyle(false);
            break;
        }
        case SID_DASH_LIST:
        {
            FillLineStyleList();
            SelectLineStyle();
            break;
        }
        case SID_ATTR_LINE_JOINT:
        {
            if(bDisabled)
            {
                mpLBEdgeStyle->Disable();
                mpFTEdgeStyle->Disable();
            }
            else
            {
                mpLBEdgeStyle->Enable();
                mpFTEdgeStyle->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineJointItem* pItem = dynamic_cast< const XLineJointItem* >(pState);

                if(pItem)
                {
                    sal_uInt16 nEntryPos(0);

                    switch(pItem->GetValue())
                    {
                        case com::sun::star::drawing::LineJoint_MIDDLE:
                        case com::sun::star::drawing::LineJoint_ROUND:
                        {
                            nEntryPos = 1;
                            break;
                        }
                        case com::sun::star::drawing::LineJoint_NONE:
                        {
                            nEntryPos = 2;
                            break;
                        }
                        case com::sun::star::drawing::LineJoint_MITER:
                        {
                            nEntryPos = 3;
                            break;
                        }
                        case com::sun::star::drawing::LineJoint_BEVEL:
                        {
                            nEntryPos = 4;
                            break;
                        }

                        default:
                            break;
                    }

                    if(nEntryPos)
                    {
                        mpLBEdgeStyle->SelectEntryPos(nEntryPos - 1);
                        break;
                    }
                }
            }

            mpLBEdgeStyle->SetNoSelection();
            break;
        }
        case SID_ATTR_LINE_CAP:
        {
            if(bDisabled)
            {
                mpLBCapStyle->Disable();
                mpFTCapStyle->Disable();
            }
            else
            {
                mpLBCapStyle->Enable();
                mpLBCapStyle->Enable();
            }

            if(eState >= SFX_ITEM_DEFAULT)
            {
                const XLineCapItem* pItem = dynamic_cast< const XLineCapItem* >(pState);

                if(pItem)
                {
                    sal_uInt16 nEntryPos(0);

                    switch(pItem->GetValue())
                    {
                        case com::sun::star::drawing::LineCap_BUTT:
                        {
                            nEntryPos = 1;
                            break;
                        }
                        case com::sun::star::drawing::LineCap_ROUND:
                        {
                            nEntryPos = 2;
                            break;
                        }
                        case com::sun::star::drawing::LineCap_SQUARE:
                        {
                            nEntryPos = 3;
                            break;
                        }

                        default:
                            break;
                    }

                    if(nEntryPos)
                    {
                        mpLBCapStyle->SelectEntryPos(nEntryPos - 1);
                        break;
                    }
                }
            }

            mpLBCapStyle->SetNoSelection();
            break;
        }
    }
}




SfxBindings* LinePropertyPanel::GetBindings() 
{ 
    return mpBindings; 
}



IMPL_LINK( LinePropertyPanel, ImplPopupModeEndHdl, FloatingWindow*, EMPTYARG )
{	
    return 0;
}




IMPL_LINK(LinePropertyPanel, ToolboxColorSelectHdl,ToolBox*, pToolBox)
{
    sal_uInt16 nId = pToolBox->GetCurItemId();
    if(nId == TBI_COLOR)
    {
        maColorPopup.Show(*pToolBox);
        maColorPopup.SetCurrentColor(maColor, mbColorAvailable);
    }
    return 0;
}




IMPL_LINK(LinePropertyPanel, ChangeLineStyleHdl, ToolBox*, /* pToolBox */)
{
    const sal_uInt16 nPos(mpLBStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && nPos != mpLBStyle->GetSavedValue())
    {
        if(0 == nPos)
        {
            // XLINE_NONE
            const XLineStyleItem aItem(XLINE_NONE);

            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SFX_CALLMODE_RECORD, &aItem, 0L);
        }
        else if(1 == nPos)
        {
            // XLINE_SOLID
            const XLineStyleItem aItem(XLINE_SOLID);

            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SFX_CALLMODE_RECORD, &aItem, 0L);
        }
        else if(maLineStyleList.get() && maLineStyleList->Count() > (long)(nPos - 2))
        {
            // XLINE_DASH
            const XLineStyleItem aItemA(XLINE_DASH);
            const XDashEntry* pDashEntry = maLineStyleList->GetDash(nPos - 2);
            OSL_ENSURE(pDashEntry, "OOps, got empty XDash from XDashList (!)");
            const XLineDashItem aItemB(
                pDashEntry ? pDashEntry->GetName() : String(),
                pDashEntry ? pDashEntry->GetDash() : XDash());

            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SFX_CALLMODE_RECORD, &aItemA, 0L);
            GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_DASH, SFX_CALLMODE_RECORD, &aItemB, 0L);
        }
    }

    return 0;
}



IMPL_LINK(LinePropertyPanel, ChangeStartHdl, void*, EMPTYARG)
{
    sal_uInt16	nPos = mpLBStart->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND && nPos != mpLBStart->GetSavedValue() )
    {
        XLineStartItem* pItem = NULL;
        if( nPos == 0 )
            pItem = new XLineStartItem();
        else if( maLineEndList.get() && maLineEndList->Count() > (long) ( nPos - 1 ) )
            pItem = new XLineStartItem( mpLBStart->GetSelectEntry(),maLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINEEND_STYLE, SFX_CALLMODE_RECORD, pItem,  0L);
        delete pItem;
    }
    return 0;
}




IMPL_LINK(LinePropertyPanel, ChangeEndHdl, void*, EMPTYARG)
{
    sal_uInt16	nPos = mpLBEnd->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND && nPos != mpLBEnd->GetSavedValue() )
    {
        XLineEndItem* pItem = NULL;
        if( nPos == 0 )
            pItem = new XLineEndItem();
        else if( maLineEndList.get() && maLineEndList->Count() > (long) ( nPos - 1 ) )
            pItem = new XLineEndItem( mpLBEnd->GetSelectEntry(), maLineEndList->GetLineEnd( nPos - 1 )->GetLineEnd() );
        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINEEND_STYLE, SFX_CALLMODE_RECORD, pItem,  0L);
        delete pItem;
    }
    return 0;
}




IMPL_LINK(LinePropertyPanel, ChangeEdgeStyleHdl, void*, EMPTYARG)
{
    const sal_uInt16 nPos(mpLBEdgeStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && nPos != mpLBEdgeStyle->GetSavedValue())
    {
        XLineJointItem* pItem = 0;

        switch(nPos)
        {
            case 0: // rounded
            {
                pItem = new XLineJointItem(com::sun::star::drawing::LineJoint_ROUND);
                break;
            }
            case 1: // none
            {
                pItem = new XLineJointItem(com::sun::star::drawing::LineJoint_NONE);
                break;
            }
            case 2: // mitered
            {
                pItem = new XLineJointItem(com::sun::star::drawing::LineJoint_MITER);
                break;
            }
            case 3: // beveled
            {
                pItem = new XLineJointItem(com::sun::star::drawing::LineJoint_BEVEL);
                break;
            }
        }

        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_JOINT, SFX_CALLMODE_RECORD, pItem,  0L);
        delete pItem;
    }
    return 0;
}




IMPL_LINK(LinePropertyPanel, ChangeCapStyleHdl, void*, EMPTYARG)
{
    const sal_uInt16 nPos(mpLBCapStyle->GetSelectEntryPos());

    if(LISTBOX_ENTRY_NOTFOUND != nPos && nPos != mpLBCapStyle->GetSavedValue())
    {
        XLineCapItem* pItem = 0;

        switch(nPos)
        {
            case 0: // flat
            {
                pItem = new XLineCapItem(com::sun::star::drawing::LineCap_BUTT);
                break;
            }
            case 1: // round
            {
                pItem = new XLineCapItem(com::sun::star::drawing::LineCap_ROUND);
                break;
            }
            case 2: // square
            {
                pItem = new XLineCapItem(com::sun::star::drawing::LineCap_SQUARE);
                break;
            }
        }

        GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_CAP, SFX_CALLMODE_RECORD, pItem,  0L);
        delete pItem;
    }
    return 0;
}




IMPL_LINK(LinePropertyPanel, ToolboxWidthSelectHdl,ToolBox*, pToolBox)
{
    if (pToolBox->GetCurItemId() == TBI_WIDTH)
    {
        maLineWidthPopup.SetWidthSelect(mnWidthCoreValue, mbWidthValuable, meMapUnit);
        maLineWidthPopup.Show(*pToolBox);
    }
    return 0;
}




IMPL_LINK( LinePropertyPanel, ChangeTransparentHdl, void *, EMPTYARG )
{
    sal_uInt16 nVal = (sal_uInt16)mpMFTransparent->GetValue();
    XLineTransparenceItem aItem( nVal );

    GetBindings()->GetDispatcher()->Execute(SID_ATTR_LINE_STYLE, SFX_CALLMODE_RECORD, &aItem, 0L);
    return( 0L );
}




namespace 
{
    Color GetTransparentColor (void)
    {
        return COL_TRANSPARENT;
    }
} // end of anonymous namespace

PopupControl* LinePropertyPanel::CreateColorPopupControl (PopupContainer* pParent)
{
    return new ColorControl(
        pParent,
        mpBindings,
        SVX_RES(RID_POPUPPANEL_LINEPAGE_COLOR),
        SVX_RES(VS_COLOR),
        ::boost::bind(GetTransparentColor),
        ::boost::bind(&LinePropertyPanel::SetColor, this, _1, _2),
        pParent,
        0);
}




PopupControl* LinePropertyPanel::CreateLineWidthPopupControl (PopupContainer* pParent)
{
    return new LineWidthControl(pParent, *this);
}




void LinePropertyPanel::EndLineWidthPopupMode (void)
{
    maLineWidthPopup.Hide();
}




void LinePropertyPanel::Resize (void)
{
    maLayouter.Layout();
}




void LinePropertyPanel::SetWidthIcon(int n)
{
    if(n==0)
        mpTBWidth->SetItemImage( TBI_WIDTH, maIMGNone);
    else
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[n-1] : mpIMGWidthIcon[n-1]);
}



void LinePropertyPanel::SetWidthIcon()
{
    if(!mbWidthValuable)
    {
        mpTBWidth->SetItemImage( TBI_WIDTH, maIMGNone);
        return;
    }

    long nVal = LogicToLogic(mnWidthCoreValue * 10,(MapUnit)meMapUnit , MAP_POINT);

    if(nVal <= 6)	
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[0] : mpIMGWidthIcon[0]); 
    else if(nVal > 6 && nVal <= 9)	
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[1] : mpIMGWidthIcon[1]);  
    else if(nVal > 9 && nVal <= 12)	
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[2] : mpIMGWidthIcon[2]); 
    else if(nVal > 12 && nVal <= 19)	
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[3] : mpIMGWidthIcon[3]); 
    else if(nVal > 19 && nVal <= 26)		
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[4] : mpIMGWidthIcon[4]); 
    else if(nVal > 26 && nVal <= 37)	
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[5] : mpIMGWidthIcon[5]); 
    else if(nVal > 37 && nVal <=52)		
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[6] : mpIMGWidthIcon[6]); 
    else if(nVal > 52)		
        mpTBWidth->SetItemImage( TBI_WIDTH, GetDisplayBackground().GetColor().IsDark() ? mpIMGWidthIconH[7] : mpIMGWidthIcon[7]); 

}



void LinePropertyPanel::SetColor (
    const String& rsColorName,
    const Color aColor)
{
    XLineColorItem aColorItem(rsColorName, aColor);
    mpBindings->GetDispatcher()->Execute(SID_ATTR_LINE_COLOR, SFX_CALLMODE_RECORD, &aColorItem, 0L);
    maColor = aColor;
}



void LinePropertyPanel::SetWidth(long nWidth)
{
    mnWidthCoreValue = nWidth;
    mbWidthValuable = true;
}



void  LinePropertyPanel::FillLineEndList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_LINEEND_LIST ) )
    {
        mpLBStart->Enable();
        SvxLineEndListItem aItem( *(const SvxLineEndListItem*)(pSh->GetItem( SID_LINEEND_LIST ) ) );		
        maLineEndList = aItem.GetLineEndList();

        if(maLineEndList.get())
        {
            FillLineEndListBox(*mpLBStart, *mpLBEnd, maLineEndList);
        }

        mpLBStart->SelectEntryPos(0);
        mpLBEnd->SelectEntryPos(0);
    }
    else
    {
        mpLBStart->Disable();
        mpLBEnd->Disable();
    }
}



void  LinePropertyPanel::FillLineStyleList()
{
    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh && pSh->GetItem( SID_DASH_LIST ) )
    {
        mpLBStyle->Enable();
        SvxDashListItem aItem( *(const SvxDashListItem*)(pSh->GetItem( SID_DASH_LIST ) ) );		
        maLineStyleList = aItem.GetDashList();

        if(maLineStyleList.get())
        {
            FillLineStyleListBox(*mpLBStyle, maLineStyleList);
        }

        mpLBStyle->SelectEntryPos(0);
    }
    else
    {
        mpLBStyle->Disable();
    }
}



void LinePropertyPanel::SelectLineStyle()
{
    if( !mpStyleItem.get() || !mpDashItem.get() )
    {
        mpLBStyle->SetNoSelection();
        mpLBStyle->Disable();
        return;
    }

    const XLineStyle eXLS(mpStyleItem ? (XLineStyle)mpStyleItem->GetValue() : XLINE_NONE);
    bool bSelected(false);

    switch(eXLS)
    {
        case XLINE_NONE:
            break;
        case XLINE_SOLID:
            mpLBStyle->SelectEntryPos(1);
            bSelected = true;
            break;
        default:
            if(mpDashItem && maLineStyleList.get())
            {
                const XDash& rDash = mpDashItem->GetDashValue();
                for(sal_Int32 a(0);!bSelected &&  a < maLineStyleList->Count(); a++)
                {
                    XDashEntry* pEntry = maLineStyleList->GetDash(a);
                    const XDash& rEntry = pEntry->GetDash();
                    if(rDash == rEntry)
                    {
                        mpLBStyle->SelectEntryPos((sal_uInt16)a + 2);
                        bSelected = true;
                    }
                }
            }
            break;
    }
    
    if(!bSelected)
        mpLBStyle->SelectEntryPos( 0 );
}

void LinePropertyPanel::SelectEndStyle(bool bStart)
{
    sal_Bool bSelected(false);

    if(bStart)
    {
        if( !mpStartItem.get() )
        {
            mpLBStart->SetNoSelection();
            mpLBStart->Disable();
            return;
        }

        if(mpStartItem && maLineEndList.get())
        {
            const basegfx::B2DPolyPolygon& rItemPolygon = mpStartItem->GetLineStartValue();
            for(sal_Int32 a(0);!bSelected &&  a < maLineEndList->Count(); a++)
            {
                XLineEndEntry* pEntry = maLineEndList->GetLineEnd(a);
                const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();
                if(rItemPolygon == rEntryPolygon)
                {
                    mpLBStart->SelectEntryPos((sal_uInt16)a + 1);
                    bSelected = true;
                }
            }
        }

        if(!bSelected)
        {
            mpLBStart->SelectEntryPos( 0 );
        }
    }
    else
    {
        if( !mpEndItem.get() )
        {
            mpLBEnd->SetNoSelection();
            mpLBEnd->Disable();
            return;
        }

        if(mpEndItem && maLineEndList.get())
        {
            const basegfx::B2DPolyPolygon& rItemPolygon = mpEndItem->GetLineEndValue();
            for(sal_Int32 a(0);!bSelected &&  a < maLineEndList->Count(); a++)
            {
                XLineEndEntry* pEntry = maLineEndList->GetLineEnd(a);
                const basegfx::B2DPolyPolygon& rEntryPolygon = pEntry->GetLineEnd();
                if(rItemPolygon == rEntryPolygon)
                {
                    mpLBEnd->SelectEntryPos((sal_uInt16)a + 1);
                    bSelected = true;
                }
            }
        }

        if(!bSelected)
        {
            mpLBEnd->SelectEntryPos( 0 );
        }
    }
}


} } // end of namespace svx::sidebar

// eof
