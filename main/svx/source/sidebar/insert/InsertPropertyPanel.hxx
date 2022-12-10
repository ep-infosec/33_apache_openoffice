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

#ifndef SVX_SIDEBAR_INSERT_PROPERTY_PAGE_HXX
#define SVX_SIDEBAR_INSERT_PROPERTY_PAGE_HXX

#include <boost/scoped_ptr.hpp>

#include <vcl/ctrl.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>

#include <map>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

class ToolBox;

namespace svx { namespace sidebar {

/** This panel provides buttons for inserting shapes into a document.
*/
class InsertPropertyPanel
    : public Control
{
public:
    InsertPropertyPanel (
        Window* pParent,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    virtual ~InsertPropertyPanel (void);

private:
    ::boost::scoped_ptr<Window> mpStandardShapesBackground;
    ::boost::scoped_ptr<ToolBox> mpStandardShapesToolBox;
    ::boost::scoped_ptr<Window> mpCustomShapesBackground;
    ::boost::scoped_ptr<ToolBox> mpCustomShapesToolBox;
    const cssu::Reference<css::frame::XFrame> mxFrame;
    
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};


} } // end of namespace ::svx::sidebar

#endif
