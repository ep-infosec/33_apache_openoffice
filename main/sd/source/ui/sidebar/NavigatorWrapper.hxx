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

#ifndef SD_SIDEBAR_NAVIGATOR_WRAPPER_HXX
#define SD_SIDEBAR_NAVIGATOR_WRAPPER_HXX

#include <sfx2/sidebar/ILayoutableWindow.hxx>
#include <vcl/ctrl.hxx>
#include "navigatr.hxx"


class SfxBindings;
namespace sd { class ViewShellBase; }

namespace css = ::com::sun::star;

namespace sd { namespace sidebar {

/** Present the navigator as control that can be displayed inside the
    sidebar.
    This wrapper has two main responsibilities:
    - Watch for document changes and update the navigator when one
    happens.
    - Forward size changes from sidebar to navigator.
*/
class NavigatorWrapper
    : public Control,
      public sfx2::sidebar::ILayoutableWindow
{
public:
    NavigatorWrapper (
        ::Window* pParent,
        sd::ViewShellBase& rViewShellBase,
        SfxBindings* pBindings);

    virtual ~NavigatorWrapper (void);

    // Control
    virtual void Resize (void);
    virtual void GetFocus (void);

    // From ILayoutableWindow
    virtual css::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth);

private:
    ViewShellBase& mrViewShellBase;
    SdNavigatorWin maNavigator;

    void UpdateNavigator (void);
};


} } // end of namespace sd::sidebar

#endif
