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

#include "DeckTitleBar.hxx"
#include "sfx2/sidebar/Theme.hxx"
#include "sfx2/sfxresid.hxx"
#include "Sidebar.hrc"

#include <vcl/image.hxx>

#ifdef DEBUG
#include "sfx2/sidebar/Tools.hxx"
#endif


namespace sfx2 { namespace sidebar {

static const sal_Int32 gaLeftGripPadding (3);
static const sal_Int32 gaRightGripPadding (3);


DeckTitleBar::DeckTitleBar (
    const ::rtl::OUString& rsTitle,
    Window* pParentWindow,
    const ::boost::function<void(void)>& rCloserAction)
    : TitleBar(rsTitle, pParentWindow, GetBackgroundPaint()),
      mnCloserItemIndex(1),
      maCloserAction(rCloserAction),
      mbIsCloserVisible(false)
{
    OSL_ASSERT(pParentWindow != NULL);
    
    if (maCloserAction)
        SetCloserVisible(true);

#ifdef DEBUG
    SetText(A2S("DeckTitleBar"));
#endif
}




DeckTitleBar::~DeckTitleBar (void)
{
}




void DeckTitleBar::SetCloserVisible (const bool bIsCloserVisible)
{
    if (mbIsCloserVisible != bIsCloserVisible)
    {
        mbIsCloserVisible = bIsCloserVisible;

        if (mbIsCloserVisible)
        {
            maToolBox.InsertItem(
                mnCloserItemIndex,
                Theme::GetImage(Theme::Image_Closer));
            maToolBox.SetQuickHelpText(
                mnCloserItemIndex,
                String(SfxResId(SFX_STR_SIDEBAR_CLOSE_DECK)));
        }
        else
            maToolBox.RemoveItem(
                maToolBox.GetItemPos(mnCloserItemIndex));
    }
}




Rectangle DeckTitleBar::GetTitleArea (const Rectangle& rTitleBarBox)
{
    Image aGripImage (Theme::GetImage(Theme::Image_Grip));
    return Rectangle(
        aGripImage.GetSizePixel().Width() + gaLeftGripPadding + gaRightGripPadding,
        rTitleBarBox.Top(),
        rTitleBarBox.Right(),
        rTitleBarBox.Bottom());
}




void DeckTitleBar::PaintDecoration (const Rectangle& rTitleBarBox)
{
    (void)rTitleBarBox;
}




sidebar::Paint DeckTitleBar::GetBackgroundPaint (void)
{
    return Theme::GetPaint(Theme::Paint_DeckTitleBarBackground);
}




Color DeckTitleBar::GetTextColor (void)
{
    return Theme::GetColor(Theme::Color_DeckTitleFont);
}




void DeckTitleBar::HandleToolBoxItemClick (const sal_uInt16 nItemIndex)
{
    if (nItemIndex == mnCloserItemIndex)
        if (maCloserAction)
            maCloserAction();
}




cssu::Reference<css::accessibility::XAccessible> DeckTitleBar::CreateAccessible (void)
{
    const ::rtl::OUString sAccessibleName(msTitle);
    SetAccessibleName(sAccessibleName);
    SetAccessibleDescription(sAccessibleName);
    return TitleBar::CreateAccessible();
}




void DeckTitleBar::DataChanged (const DataChangedEvent& rEvent)
{
    maToolBox.SetItemImage(
        mnCloserItemIndex,
        Theme::GetImage(Theme::Image_Closer));
    TitleBar::DataChanged(rEvent);
}


} } // end of namespace sfx2::sidebar
