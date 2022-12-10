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



#ifndef SD_FRAMEWORK_TOOL_BAR_MODULE_HXX
#define SD_FRAMEWORK_TOOL_BAR_MODULE_HXX

#include "ToolBarManager.hxx"
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd {
class ViewShellBase;
}



namespace sd { namespace framework {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        ::css::drawing::framework::XConfigurationChangeListener
        > ToolBarModuleInterfaceBase;
}


/** This module is responsible for locking the ToolBarManager during
    configuration updates and for triggering ToolBarManager updates.
*/
class ToolBarModule
    : private ::cppu::BaseMutex,
      public ToolBarModuleInterfaceBase      
{
public:
    /** Create a new module.
        @param rxController
            This is the access point to the drawing framework.
    */
    ToolBarModule (
        const css::uno::Reference<css::frame::XController>& rxController);
    virtual ~ToolBarModule (void);
    
    virtual void SAL_CALL disposing (void);

    
    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<
        css::drawing::framework::XConfigurationController> mxConfigurationController;
    ViewShellBase* mpBase;
    ::boost::scoped_ptr<ToolBarManager::UpdateLock> mpToolBarManagerLock;
    bool mbMainViewSwitchUpdatePending;

    void HandleUpdateStart (void);
    void HandleUpdateEnd (void);
};

} } // end of namespace sd::framework

#endif
