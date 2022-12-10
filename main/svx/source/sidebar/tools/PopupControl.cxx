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

#include "svx/sidebar/PopupControl.hxx"
#include <sfx2/sidebar/Theme.hxx>
#include <vcl/gradient.hxx>

using ::sfx2::sidebar::Theme;

namespace svx { namespace sidebar {

PopupControl::PopupControl (
    Window* pParent,
    const ResId& rResId)
    : Control( pParent,rResId)
{
	EnableChildTransparentMode(true);
	SetControlBackground();

    SetBackground(Theme::GetWallpaper(Theme::Paint_DropDownBackground));
}




PopupControl::~PopupControl (void)
{
}




void PopupControl::Paint (const Rectangle& rBox)
{
    Control::Paint(rBox);

    // The background is taken care of by setting the background color
    // in the constructor.  Here we just paint the border.
    SetFillColor();
    SetLineColor(Theme::GetColor(Theme::Color_DropDownBorder));
    DrawRect(Rectangle(Point(0,0), GetOutputSizePixel()));
}

} } // end of namespace svx::sidebar
