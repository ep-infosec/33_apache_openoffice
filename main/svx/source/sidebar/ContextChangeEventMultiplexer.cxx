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

#include "precompiled_svx.hxx"

#include "svx/sidebar/ContextChangeEventMultiplexer.hxx"

#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <sfx2/viewsh.hxx>
#include <tools/diagnose_ex.h>

using namespace css;
using namespace cssu;


void ContextChangeEventMultiplexer::NotifyContextChange (
    const cssu::Reference<css::frame::XController>& rxController,
    const ::sfx2::sidebar::EnumContext::Context eContext)
{
    if (rxController.is() && rxController->getFrame().is())
    {
        const css::ui::ContextChangeEventObject aEvent(
            rxController,
            GetModuleName(rxController->getFrame()),
            ::sfx2::sidebar::EnumContext::GetContextName(eContext));
    
        cssu::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
            css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        if (xMultiplexer.is())
            xMultiplexer->broadcastContextChangeEvent(aEvent, rxController);
    }
}




void ContextChangeEventMultiplexer::NotifyContextChange (
    SfxViewShell* pViewShell,
    const ::sfx2::sidebar::EnumContext::Context eContext)
{
    if (pViewShell != NULL)
        NotifyContextChange(pViewShell->GetController(), eContext);    
}




::rtl::OUString ContextChangeEventMultiplexer::GetModuleName (
    const cssu::Reference<css::frame::XFrame>& rxFrame)
{
    try
    {
        const ::comphelper::ComponentContext aContext (::comphelper::getProcessServiceFactory());
        const Reference<frame::XModuleManager> xModuleManager (
            aContext.createComponent("com.sun.star.frame.ModuleManager" ),
            UNO_QUERY_THROW );
        return xModuleManager->identify(rxFrame);
    }
    catch (const Exception&)
    {
        // An exception typically means that a context change is notified
        // during initialization or destruction of a view.
        // Ignore it.
    }
    return ::sfx2::sidebar::EnumContext::GetApplicationName(
        ::sfx2::sidebar::EnumContext::Application_None);
}
