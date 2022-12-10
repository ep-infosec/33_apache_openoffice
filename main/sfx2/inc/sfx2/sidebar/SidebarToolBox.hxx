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

#ifndef SFX_SIDEBAR_TOOLBOX_HXX
#define SFX_SIDEBAR_TOOLBOX_HXX

#include "sfx2/dllapi.h"
#include "vcl/toolbox.hxx"
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/util/URL.hpp>
#include <map>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** The sidebar tool box has two responsibilities:
    1. Coordinated location, size, and other states with its parent
       background window.
    2. Create and handle tool bar controller for its items.
*/
class SFX2_DLLPUBLIC SidebarToolBox
    : public ToolBox
{
public:
    /** Create a new tool box.
        When a valid XFrame is given then the tool box will handle its
        buttons and drop-downs.  Otherwise the caller has to do that.
    */
    SidebarToolBox (
        Window* pParentWindow,
        const ResId& rResId,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    SidebarToolBox (
        Window* pParentWindow);
    virtual ~SidebarToolBox (void);

    void SetBorderWindow (const Window* pBorderWindow);
    virtual void Paint (const Rectangle& rRect);

    virtual Point GetPosPixel (void) const;
    virtual void SetPosSizePixel (
        long nX,
        long nY,
        long nWidth,
        long nHeight,
        sal_uInt16 nFlags);
    virtual long Notify (NotifyEvent& rEvent);

    cssu::Reference<css::frame::XToolbarController> GetControllerForItemId (
        const sal_uInt16 nItemId) const;
    sal_uInt16 GetItemIdForSubToolbarName (
        const ::rtl::OUString& rsCOmmandName) const;

    void SetController (
        const sal_uInt16 nItemId,
        const cssu::Reference<css::frame::XToolbarController>& rxController,
        const ::rtl::OUString& rsCommandName);

private:
    bool mbParentIsBorder;
    Image maItemSeparator;
    class ItemDescriptor
    {
    public:
        cssu::Reference<css::frame::XToolbarController> mxController;
        css::util::URL maURL;
        rtl::OUString msCurrentCommand;
    };
    typedef ::std::map<sal_uInt16, ItemDescriptor> ControllerContainer;
    ControllerContainer maControllers;
    bool mbAreHandlersRegistered;

    DECL_LINK(DropDownClickHandler, ToolBox*);
    DECL_LINK(ClickHandler, ToolBox*);
    DECL_LINK(DoubleClickHandler, ToolBox*);
    DECL_LINK(SelectHandler, ToolBox*);
    DECL_LINK(Activate, ToolBox*);
    DECL_LINK(Deactivate, ToolBox*);

    using ToolBox::Activate;
    using ToolBox::Deactivate;
    using DockingWindow::SetPosSizePixel;
    
    void CreateController (
        const sal_uInt16 nItemId,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        const sal_Int32 nItemWidth = 0);
    void UpdateIcons (
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    void RegisterHandlers (void);
};


} } // end of namespace sfx2::sidebar

#endif
