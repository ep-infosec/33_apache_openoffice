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

#include "MenuButton.hxx"

#include "DrawHelper.hxx"
#include "Paint.hxx"
#include "sfx2/sidebar/Tools.hxx"
#include "sfx2/sidebar/Theme.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


namespace sfx2 { namespace sidebar {


MenuButton::MenuButton (Window* pParentWindow)
    : CheckBox(pParentWindow),
      mbIsLeftButtonDown(false),
      mePaintType(PT_Theme)
{
#ifdef DEBUG
    SetText(A2S("MenuButton"));
#endif
}




MenuButton::~MenuButton (void)
{
}




void MenuButton::Paint (const Rectangle& rUpdateArea)
{
    switch(mePaintType)
    {
        case PT_Theme:
        default:
        {
            const bool bIsSelected (IsChecked());
            const bool bIsHighlighted (IsMouseOver() || HasFocus());
            DrawHelper::DrawRoundedRectangle(
                *this,
                Rectangle(Point(0,0), GetSizePixel()),
                3,
                bIsHighlighted||bIsSelected
                    ? Theme::GetColor(Theme::Color_TabItemBorder)
                    : Color(0xffffffff),
                bIsHighlighted
                    ? Theme::GetPaint(Theme::Paint_TabItemBackgroundHighlight)
                    : Theme::GetPaint(Theme::Paint_TabItemBackgroundNormal));
        
            const Image aIcon (Button::GetModeImage(Theme::IsHighContrastMode()
                    ? BMP_COLOR_HIGHCONTRAST
                    : BMP_COLOR_NORMAL));
            const Size aIconSize (aIcon.GetSizePixel());
            const Point aIconLocation(
                (GetSizePixel().Width() - aIconSize.Width())/2,
                (GetSizePixel().Height() - aIconSize.Height())/2);
            DrawImage(
                aIconLocation,
                aIcon);
            break;
        }
        case PT_Native:
            Button::Paint(rUpdateArea);
            //            DrawImage(maIconPosition, maIcon);
            break;
    }
}




void MenuButton::MouseMove (const MouseEvent& rEvent)
{
    if (rEvent.IsEnterWindow() || rEvent.IsLeaveWindow())
        Invalidate();
    CheckBox::MouseMove(rEvent);
}




void MenuButton::MouseButtonDown (const MouseEvent& rMouseEvent)
{
    if (rMouseEvent.IsLeft())
    {
        mbIsLeftButtonDown = true;
        CaptureMouse();
        Invalidate();
    }
}




void MenuButton::MouseButtonUp (const MouseEvent& rMouseEvent)
{
    if (IsMouseCaptured())
        ReleaseMouse();
    
    if (rMouseEvent.IsLeft())
    {
        if (mbIsLeftButtonDown)
        {
            Check();
            Click();
            GetParent()->Invalidate();
        }
    }
    if (mbIsLeftButtonDown)
    {
        mbIsLeftButtonDown = false;
        Invalidate();
    }
}


} } // end of namespace sfx2::sidebar
