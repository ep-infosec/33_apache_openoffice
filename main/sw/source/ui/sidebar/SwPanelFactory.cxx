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

#include "precompiled_sw.hxx"

#include "SwPanelFactory.hxx"

#include <PagePropertyPanel.hxx>
#include <WrapPropertyPanel.hxx>
#include <navipi.hxx>

#include <sfx2/sidebar/SidebarPanelBase.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <rtl/ref.hxx>
#include <comphelper/namedvaluecollection.hxx>

#include <boost/bind.hpp>


using namespace css;
using namespace cssu;
using ::rtl::OUString;


namespace sw { namespace sidebar {

#define A2S(s) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))
#define IMPLEMENTATION_NAME "org.apache.openoffice.comp.sw.sidebar.SwPanelFactory"
#define SERVICE_NAME "com.sun.star.ui.UIElementFactory"


::rtl::OUString SAL_CALL SwPanelFactory::getImplementationName (void)
{
    return A2S(IMPLEMENTATION_NAME);
}


cssu::Reference<cssu::XInterface> SAL_CALL SwPanelFactory::createInstance(
    const uno::Reference<lang::XMultiServiceFactory>& )
{
    ::rtl::Reference<SwPanelFactory> pPanelFactory (new SwPanelFactory());
    cssu::Reference<cssu::XInterface> xService (static_cast<XWeak*>(pPanelFactory.get()), cssu::UNO_QUERY);
    return xService;
}


cssu::Sequence<OUString> SAL_CALL SwPanelFactory::getSupportedServiceNames (void)
{
    cssu::Sequence<OUString> aServiceNames (1);
    aServiceNames[0] = A2S(SERVICE_NAME);
    return aServiceNames;

}


SwPanelFactory::SwPanelFactory (void)
    : PanelFactoryInterfaceBase(m_aMutex)
{
}


SwPanelFactory::~SwPanelFactory (void)
{
}


Reference<ui::XUIElement> SAL_CALL SwPanelFactory::createUIElement (
    const ::rtl::OUString& rsResourceURL,
    const ::cssu::Sequence<css::beans::PropertyValue>& rArguments)
    throw(
        container::NoSuchElementException,
        lang::IllegalArgumentException,
        RuntimeException)
{
    Reference<ui::XUIElement> xElement;

    const ::comphelper::NamedValueCollection aArguments (rArguments);
    Reference<frame::XFrame> xFrame (aArguments.getOrDefault("Frame", Reference<frame::XFrame>()));
    Reference<awt::XWindow> xParentWindow (aArguments.getOrDefault("ParentWindow", Reference<awt::XWindow>()));
    const sal_uInt64 nBindingsValue (aArguments.getOrDefault("SfxBindings", sal_uInt64(0)));
    SfxBindings* pBindings = reinterpret_cast<SfxBindings*>(nBindingsValue);

    ::Window* pParentWindow = VCLUnoHelper::GetWindow(xParentWindow);
    if ( ! xParentWindow.is() || pParentWindow==NULL)
        throw RuntimeException(
            A2S("PanelFactory::createUIElement called without ParentWindow"),
            NULL);
    if ( ! xFrame.is())
        throw RuntimeException(
            A2S("PanelFactory::createUIElement called without Frame"),
            NULL);
    if (pBindings == NULL)
        throw RuntimeException(
            A2S("PanelFactory::createUIElement called without SfxBindings"),
            NULL);

#define DoesResourceEndWith(s) rsResourceURL.endsWithAsciiL(s,strlen(s))
    if (DoesResourceEndWith("/PagePropertyPanel"))
    {
        PagePropertyPanel* pPanel = PagePropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (DoesResourceEndWith("/WrapPropertyPanel"))
    {
        WrapPropertyPanel* pPanel = WrapPropertyPanel::Create( pParentWindow, xFrame, pBindings );
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(-1,-1,-1));
    }
    else if (DoesResourceEndWith("/NavigatorPanel"))
    {
        Window* pPanel = new SwNavigationPI(pBindings, NULL, pParentWindow);
        xElement = sfx2::sidebar::SidebarPanelBase::Create(
            rsResourceURL,
            xFrame,
            pPanel,
            ui::LayoutSize(0,-1,-1));
    }
#undef DoesResourceEndWith

    return xElement;
}

} } // end of namespace sw::sidebar

// eof
