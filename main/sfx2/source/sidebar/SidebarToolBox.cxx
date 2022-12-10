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

#include "precompiled_sfx2.hxx"

#include "sfx2/sidebar/SidebarToolBox.hxx"
#include "ToolBoxBackground.hxx"
#include "sfx2/sidebar/ControllerFactory.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sidebar/Tools.hxx"

#include <vcl/gradient.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/miscopt.hxx>
#include <framework/imageproducer.hxx>
#include <com/sun/star/frame/XSubToolbarController.hpp>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;


namespace sfx2 { namespace sidebar {


SidebarToolBox::SidebarToolBox (
    Window* pParentWindow,
    const ResId& rResId,
    const cssu::Reference<css::frame::XFrame>& rxFrame)
    : ToolBox(pParentWindow, rResId),
      mbParentIsBorder(false),
      maItemSeparator(Theme::GetImage(Theme::Image_ToolBoxItemSeparator)),
      maControllers(),
      mbAreHandlersRegistered(false)
{
    SetBackground(Wallpaper());
    SetPaintTransparent(true);

    if (rxFrame.is())
    {
        const sal_uInt16 nItemCount (GetItemCount());
        if (nItemCount == 1)
        {
            // When there is only one item then make that as wide as
            // the tool box.
            CreateController(GetItemId(0), rxFrame, GetSizePixel().Width());
        }
        else
            for (sal_uInt16 nItemIndex=0; nItemIndex<nItemCount; ++nItemIndex)
                CreateController(GetItemId(nItemIndex), rxFrame, 0);
        UpdateIcons(rxFrame);

        SetSizePixel(CalcWindowSizePixel());

        RegisterHandlers();
    }

#ifdef DEBUG
    SetText(A2S("SidebarToolBox"));
#endif
}




SidebarToolBox::SidebarToolBox (Window* pParentWindow)
    : ToolBox(pParentWindow, 0),
      mbParentIsBorder(false),
      maItemSeparator(Theme::GetImage(Theme::Image_ToolBoxItemSeparator)),
      maControllers(),
      mbAreHandlersRegistered(false)
{
    SetBackground(Wallpaper());
    SetPaintTransparent(true);

#ifdef DEBUG
    SetText(A2S("SidebarToolBox"));
#endif
}




SidebarToolBox::~SidebarToolBox (void)
{
    ControllerContainer aControllers;
    aControllers.swap(maControllers);
    for (ControllerContainer::iterator iController(aControllers.begin()), iEnd(aControllers.end());
         iController!=iEnd;
         ++iController)
    {
        Reference<lang::XComponent> xComponent (iController->second.mxController, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mbAreHandlersRegistered)
    {
        SetDropdownClickHdl(Link());
        SetClickHdl(Link());
        SetDoubleClickHdl(Link());
        SetSelectHdl(Link());
        SetActivateHdl(Link());
        SetDeactivateHdl(Link());
    }
}




void SidebarToolBox::SetBorderWindow (const Window* pBorderWindow)
{
    if (pBorderWindow != GetParent())
    {
        OSL_ASSERT("SetBorderWindow can only handle parent as border window");
        return;
    }

    if ( ! mbParentIsBorder)
    {
        mbParentIsBorder = true;

        SetPosSizePixel (
            GetPosPixel().X(),
            GetPosPixel().Y(),
            GetSizePixel().Width(),
            GetSizePixel().Height(),
            WINDOW_POSSIZE_ALL);
    }
}




void SidebarToolBox::Paint (const Rectangle& rRect)
{
    ToolBox::Paint(rRect);
    
    if (Theme::GetBoolean(Theme::Bool_UseToolBoxItemSeparator))
    {
        const sal_Int32 nSeparatorY ((GetSizePixel().Height() - maItemSeparator.GetSizePixel().Height())/2);
        const sal_uInt16 nItemCount (GetItemCount());
        int nLastRight (-1);
        for (sal_uInt16 nIndex=0; nIndex<nItemCount; ++nIndex)
        {
            const Rectangle aItemBoundingBox (GetItemPosRect(nIndex));
            if (nLastRight >= 0)
            {
                const int nSeparatorX ((nLastRight + aItemBoundingBox.Left() - 1) / 2);
                DrawImage(Point(nSeparatorX,nSeparatorY), maItemSeparator);
            }
        
            nLastRight = aItemBoundingBox.Right();
        }
    }
}




Point SidebarToolBox::GetPosPixel (void) const
{
    if (mbParentIsBorder)
    {
        const Point aParentPoint (GetParent()->GetPosPixel());
        const Point aChildPoint (ToolBox::GetPosPixel());
        return Point(
            aParentPoint.X() + aChildPoint.X(),
            aParentPoint.Y() + aChildPoint.Y());
    }
    else
        return ToolBox::GetPosPixel();
}




void SidebarToolBox::SetPosSizePixel (
    long nX,
    long nY,
    long nWidth,
    long nHeight,
    sal_uInt16 nFlags)
{
    if (mbParentIsBorder)
    {
        const Point aRelativePosition (static_cast<ToolBoxBackground*>(GetParent())->SetToolBoxChild(
                this,
                nX,
                nY,
                nWidth,
                nHeight,
                nFlags));
        ToolBox::SetPosSizePixel(
            aRelativePosition.X(),
            aRelativePosition.Y(),
            nWidth,
            nHeight,
            nFlags);
    }
    else
        ToolBox::SetPosSizePixel(nX, nY, nWidth, nHeight, nFlags);
}




long SidebarToolBox::Notify (NotifyEvent& rEvent)
{
    if (rEvent.GetType() == EVENT_KEYINPUT)
    {
        if (rEvent.GetKeyEvent()->GetKeyCode().GetCode() == KEY_TAB)
        {
            // Special handling for transferring handling of KEY_TAB
            // that becomes necessary because of our parent that is
            // not the dialog but a background control.
            return DockingWindow::Notify(rEvent);
        }
    }
    return ToolBox::Notify(rEvent);
}




void SidebarToolBox::CreateController (
    const sal_uInt16 nItemId,
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    const sal_Int32 nItemWidth)
{
    ItemDescriptor aDescriptor;

    const OUString sCommandName (GetItemCommand(nItemId));

    aDescriptor.mxController = sfx2::sidebar::ControllerFactory::CreateToolBoxController(
        this,
        nItemId,
        sCommandName,
        rxFrame,
        VCLUnoHelper::GetInterface(this),
        nItemWidth);
    if (aDescriptor.mxController.is())
    {
        aDescriptor.maURL = sfx2::sidebar::Tools::GetURL(sCommandName);
        aDescriptor.msCurrentCommand = sCommandName;

        maControllers.insert(::std::make_pair(nItemId, aDescriptor));
    }
}




Reference<frame::XToolbarController> SidebarToolBox::GetControllerForItemId (const sal_uInt16 nItemId) const
{
    ControllerContainer::const_iterator iController (maControllers.find(nItemId));
    if (iController != maControllers.end())
        return iController->second.mxController;
    else
        return NULL;
}




void SidebarToolBox::SetController(
    const sal_uInt16 nItemId,
    const cssu::Reference<css::frame::XToolbarController>& rxController,
    const ::rtl::OUString& rsCommandName)
{
    ItemDescriptor aDescriptor;
    aDescriptor.mxController = rxController;
    aDescriptor.maURL = sfx2::sidebar::Tools::GetURL(rsCommandName);
    aDescriptor.msCurrentCommand = rsCommandName;

    ControllerContainer::iterator iController (maControllers.find(nItemId));
    if (iController != maControllers.end())
    {
        Reference<lang::XComponent> xComponent (iController->second.mxController, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();

        iController->second = aDescriptor;
    }
    else
    {
        maControllers[nItemId] = aDescriptor;
    }

    if (rxController.is())
        RegisterHandlers();
}




void SidebarToolBox::UpdateIcons (const Reference<frame::XFrame>& rxFrame)
{
    const sal_Bool bBigImages (SvtMiscOptions().AreCurrentSymbolsLarge());
    const bool bIsHighContrastActive (sfx2::sidebar::Theme::IsHighContrastMode());
    
    for (ControllerContainer::iterator iController(maControllers.begin()), iEnd(maControllers.end());
         iController!=iEnd;
         ++iController)
    {
        const ::rtl::OUString sCommandURL (iController->second.msCurrentCommand);
        Image aImage (framework::GetImageFromURL(rxFrame, sCommandURL, bBigImages, bIsHighContrastActive));
        SetItemImage(iController->first, aImage);
    }
}




sal_uInt16 SidebarToolBox::GetItemIdForSubToolbarName (const OUString& rsSubToolbarName) const
{
    for (ControllerContainer::const_iterator iController(maControllers.begin()), iEnd(maControllers.end());
         iController!=iEnd;
         ++iController)
    {
        Reference<frame::XToolbarController> xController (iController->second.mxController);
        Reference<frame::XSubToolbarController> xSubToolbarController (xController, UNO_QUERY);
        if (xSubToolbarController.is())
        {
            const OUString sName (xSubToolbarController->getSubToolbarName());
            if (sName.equals(rsSubToolbarName))
                return iController->first;
        }
    }
    return 0;
}




void SidebarToolBox::RegisterHandlers (void)
{
    if ( ! mbAreHandlersRegistered)
    {
        mbAreHandlersRegistered = true;
        SetDropdownClickHdl(LINK(this, SidebarToolBox, DropDownClickHandler));
        SetClickHdl(LINK(this, SidebarToolBox, ClickHandler));
        SetDoubleClickHdl(LINK(this, SidebarToolBox, DoubleClickHandler));
        SetSelectHdl(LINK(this, SidebarToolBox, SelectHandler));
        SetActivateHdl(LINK(this, SidebarToolBox, Activate));
        SetDeactivateHdl(LINK(this, SidebarToolBox, Deactivate));
    }
}




IMPL_LINK(SidebarToolBox, DropDownClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox != NULL)
    {
        Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
        if (xController.is())
        {
            Reference<awt::XWindow> xWindow = xController->createPopupWindow();
            if (xWindow.is() )
                xWindow->setFocus();
        }
    }
    return 1;
}




IMPL_LINK(SidebarToolBox, ClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;
    
    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->click();

    return 1;
}




IMPL_LINK(SidebarToolBox, DoubleClickHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;
    
    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->doubleClick();

    return 1;
}




IMPL_LINK(SidebarToolBox, SelectHandler, ToolBox*, pToolBox)
{
    if (pToolBox == NULL)
        return 0;

    Reference<frame::XToolbarController> xController (GetControllerForItemId(pToolBox->GetCurItemId()));
    if (xController.is())
        xController->execute((sal_Int16)pToolBox->GetModifier());

    return 1;
}




IMPL_LINK(SidebarToolBox, Activate, ToolBox*, EMPTYARG)
{
    return 1;
}




IMPL_LINK(SidebarToolBox, Deactivate, ToolBox*, EMPTYARG)
{
    return 1;
}



} } // end of namespace sfx2::sidebar
