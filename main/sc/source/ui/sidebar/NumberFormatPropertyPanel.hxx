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

#ifndef SC_PROPERTYPANEL_NUMFORMAT_HXX
#define SC_PROPERTYPANEL_NUMFORMAT_HXX

#include <sfx2/sidebar/ControllerItem.hxx>
#include <sfx2/sidebar/IContextChangeReceiver.hxx>
#include <boost/scoped_ptr.hpp>

class FixedText;
class ListBox;
class NumericField;

namespace sc { namespace sidebar {

class NumberFormatPropertyPanel
:   public Control,
    public ::sfx2::sidebar::IContextChangeReceiver,
    public ::sfx2::sidebar::ControllerItem::ItemUpdateReceiverInterface
{
public:
public:
    static NumberFormatPropertyPanel* Create(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);

    virtual void DataChanged(
        const DataChangedEvent& rEvent);

    virtual void HandleContextChange(
        const ::sfx2::sidebar::EnumContext aContext);

    virtual void NotifyItemUpdate(
        const sal_uInt16 nSId,
        const SfxItemState eState,
        const SfxPoolItem* pState,
        const bool bIsEnabled);

    SfxBindings* GetBindings();

    virtual void Resize (void);

private:
    //ui controls
    ::boost::scoped_ptr< FixedText >        mpFtCategory;
    ::boost::scoped_ptr< ListBox >          mpLbCategory;
    ::boost::scoped_ptr< Window >           mpTBCategoryBackground;
    ::boost::scoped_ptr< ToolBox >          mpTBCategory;
    ::boost::scoped_ptr< FixedText >        mpFtDecimals;
    ::boost::scoped_ptr< NumericField >     mpEdDecimals;
    ::boost::scoped_ptr< FixedText >        mpFtLeadZeroes;
    ::boost::scoped_ptr< NumericField >     mpEdLeadZeroes;
    ::boost::scoped_ptr< CheckBox >         mpBtnNegRed;
    ::boost::scoped_ptr< CheckBox >         mpBtnThousand;

    ::sfx2::sidebar::ControllerItem         maNumFormatControl;
    ::sfx2::sidebar::ControllerItem         maFormatControl;

    sal_uInt16                              mnCategorySelected;

    cssu::Reference<css::frame::XFrame>     mxFrame;
    ::sfx2::sidebar::EnumContext            maContext;
    SfxBindings*                            mpBindings;

    DECL_LINK(NumFormatHdl, ToolBox*);
    DECL_LINK(NumFormatSelectHdl, ListBox*);
    DECL_LINK(NumFormatValueHdl, void*);

    // constructor/destuctor
    NumberFormatPropertyPanel(
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        SfxBindings* pBindings);
    virtual ~NumberFormatPropertyPanel();

    void Initialize();
};

} } // end of namespace ::sc::sidebar

#endif

// eof
