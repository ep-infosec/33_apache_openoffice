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

#include "svx/sidebar/SelectionChangeHandler.hxx"
#include "svx/sidebar/SelectionAnalyzer.hxx"
#include "svx/sidebar/ContextChangeEventMultiplexer.hxx"
#include "svx/svdmrkv.hxx"

#include <sfx2/sidebar/EnumContext.hxx>
#include <sfx2/shell.hxx>


using namespace css;
using namespace cssu;

using namespace sfx2::sidebar;

namespace svx { namespace sidebar {

SelectionChangeHandler::SelectionChangeHandler (
    const boost::function<rtl::OUString(void)>& rSelectionChangeCallback,
    const Reference<frame::XController>& rxController,
    const EnumContext::Context eDefaultContext)
    : SelectionChangeHandlerInterfaceBase(m_aMutex),
      maSelectionChangeCallback(rSelectionChangeCallback),
      mxController(rxController),
      meDefaultContext(eDefaultContext),
      mbIsConnected(false)
{
}




SelectionChangeHandler::~SelectionChangeHandler (void)
{
}




void SAL_CALL SelectionChangeHandler::selectionChanged (const lang::EventObject&)
    throw (uno::RuntimeException)
{
    if (maSelectionChangeCallback)
    {
        const EnumContext::Context eContext (
            EnumContext::GetContextEnum(maSelectionChangeCallback()));
        ContextChangeEventMultiplexer::NotifyContextChange(
            mxController,
            eContext==EnumContext::Context_Unknown
                ? meDefaultContext
                : eContext);
    }
}
        



void SAL_CALL SelectionChangeHandler::disposing (const lang::EventObject&)
    throw (uno::RuntimeException)
{
}




void SAL_CALL SelectionChangeHandler::disposing (void)
    throw (uno::RuntimeException)
{
    if (mbIsConnected)
        Disconnect();
}




void SelectionChangeHandler::Connect (void)
{
    uno::Reference<view::XSelectionSupplier> xSupplier (mxController, uno::UNO_QUERY);
    if (xSupplier.is())
    {
        mbIsConnected = true;
        xSupplier->addSelectionChangeListener(this);
    }
}




void SelectionChangeHandler::Disconnect (void)
{
    uno::Reference<view::XSelectionSupplier> xSupplier (mxController, uno::UNO_QUERY);
    if (xSupplier.is())
    {
        mbIsConnected = false;
        xSupplier->removeSelectionChangeListener(this);
    }
}


} } // end of namespace svx::sidebar
