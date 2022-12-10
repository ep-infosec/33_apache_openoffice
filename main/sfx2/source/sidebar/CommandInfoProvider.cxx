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

#include "sfx2/sidebar/CommandInfoProvider.hxx"

#include <comphelper/processfactory.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>

using namespace css;
using namespace cssu;
using ::rtl::OUString;


#define A2S(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))


namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::lang::XEventListener
        > FrameListenerInterfaceBase;
    class FrameListener
        : public ::cppu::BaseMutex,
          public FrameListenerInterfaceBase
    {
    public:
        FrameListener (sfx2::sidebar::CommandInfoProvider& rInfoProvider, const Reference<frame::XFrame>& rxFrame)
            : FrameListenerInterfaceBase(m_aMutex),
              mrInfoProvider(rInfoProvider),
              mxFrame(rxFrame)
        {
            if (mxFrame.is())
                mxFrame->addEventListener(this);
        }
        virtual ~FrameListener (void)
        {
        }
        virtual void SAL_CALL disposing (void)
        {
            if (mxFrame.is())
                mxFrame->removeEventListener(this);
        }
        virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
            throw (cssu::RuntimeException)
        {
            (void)rEvent;
            mrInfoProvider.SetFrame(NULL);
            mxFrame = NULL;
        }    

    private:
        sfx2::sidebar::CommandInfoProvider& mrInfoProvider;
        Reference<frame::XFrame> mxFrame;
    };
}



namespace sfx2 { namespace sidebar {

CommandInfoProvider& CommandInfoProvider::Instance (void)
{
    static CommandInfoProvider aProvider;
    return aProvider;
}




CommandInfoProvider::CommandInfoProvider (void)
    : mxServiceFactory(comphelper::getProcessServiceFactory()),
      mxCachedDataFrame(),
      mxCachedDocumentAcceleratorConfiguration(),
      mxCachedModuleAcceleratorConfiguration(),
      mxCachedGlobalAcceleratorConfiguration(),
      msCachedModuleIdentifier(),
      mxFrameListener()    
{
}




CommandInfoProvider::~CommandInfoProvider (void)
{
    if (mxFrameListener.is())
    {
        mxFrameListener->dispose();
        mxFrameListener = NULL;
    }
}




OUString CommandInfoProvider::GetLabelForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);
    
    const OUString sLabel (GetCommandLabel(rsCommandName));
    const OUString sShortCut (GetCommandShortcut(rsCommandName));
    if (sShortCut.getLength() > 0)
        return sLabel + A2S(" (") + sShortCut + A2S(")");
    else
        return sLabel;
}




void CommandInfoProvider::SetFrame (const Reference<frame::XFrame>& rxFrame)
{
    if (rxFrame != mxCachedDataFrame)
    {
        // Detach from the old frame.
        if (mxFrameListener.is())
        {
            mxFrameListener->dispose();
            mxFrameListener = NULL;
        }

        // Release objects that are tied to the old frame.
        mxCachedDocumentAcceleratorConfiguration = NULL;
        mxCachedModuleAcceleratorConfiguration = NULL;
        msCachedModuleIdentifier = OUString();
        mxCachedDataFrame = rxFrame;

        // Connect to the new frame.
        if (rxFrame.is())
            mxFrameListener = new FrameListener(*this, rxFrame);
    }
}




Reference<ui::XAcceleratorConfiguration> CommandInfoProvider::GetDocumentAcceleratorConfiguration (void)
{
    if ( ! mxCachedDocumentAcceleratorConfiguration.is())
    {
        // Get the accelerator configuration for the document.
        if (mxCachedDataFrame.is())
        {
            Reference<frame::XController> xController = mxCachedDataFrame->getController();
            if (xController.is())
            {
                Reference<frame::XModel> xModel (xController->getModel());
                if (xModel.is())
                {
                    Reference<ui::XUIConfigurationManagerSupplier> xSupplier (xModel, UNO_QUERY);
                    if (xSupplier.is())
                    {
                        Reference<ui::XUIConfigurationManager> xConfigurationManager(
                            xSupplier->getUIConfigurationManager(),
                            UNO_QUERY);
                        if (xConfigurationManager.is())
                        {
                            mxCachedDocumentAcceleratorConfiguration = Reference<ui::XAcceleratorConfiguration>(
                                xConfigurationManager->getShortCutManager(),
                                UNO_QUERY);
                        }
                    }
                }
            }
        }
    }
    return mxCachedDocumentAcceleratorConfiguration;
}




Reference<ui::XAcceleratorConfiguration> CommandInfoProvider::GetModuleAcceleratorConfiguration (void)
{
    if ( ! mxCachedModuleAcceleratorConfiguration.is())
    {
        try
        {
            Reference<ui::XModuleUIConfigurationManagerSupplier> xSupplier (
                mxServiceFactory->createInstance(A2S("com.sun.star.ui.ModuleUIConfigurationManagerSupplier")),
                UNO_QUERY);
            Reference<ui::XUIConfigurationManager> xManager (
                xSupplier->getUIConfigurationManager(GetModuleIdentifier()));
            if (xManager.is())
            {
                mxCachedModuleAcceleratorConfiguration = Reference<ui::XAcceleratorConfiguration>(
                    xManager->getShortCutManager(),
                    UNO_QUERY);
            }
        }
        catch (Exception&)
        {
        }
    }
    return mxCachedModuleAcceleratorConfiguration;
}




Reference<ui::XAcceleratorConfiguration> CommandInfoProvider::GetGlobalAcceleratorConfiguration (void)
{
    // Get the global accelerator configuration.
    if ( ! mxCachedGlobalAcceleratorConfiguration.is())
    {
        mxCachedGlobalAcceleratorConfiguration = Reference<ui::XAcceleratorConfiguration>(
            mxServiceFactory->createInstance(A2S("com.sun.star.ui.GlobalAcceleratorConfiguration")),
            UNO_QUERY);
    }

    return mxCachedGlobalAcceleratorConfiguration;
}




OUString CommandInfoProvider::GetModuleIdentifier (void)
{
    if (msCachedModuleIdentifier.getLength() == 0)
    {
        Reference<frame::XModuleManager> xModuleManager (
            mxServiceFactory->createInstance(A2S("com.sun.star.frame.ModuleManager")),
            UNO_QUERY);
        if (xModuleManager.is())
            msCachedModuleIdentifier = xModuleManager->identify(mxCachedDataFrame);
    }
    return msCachedModuleIdentifier;
}




OUString CommandInfoProvider::GetCommandShortcut (const OUString& rsCommandName)
{
    OUString sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetDocumentAcceleratorConfiguration(), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetModuleAcceleratorConfiguration(), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;
    
    sShortcut = RetrieveShortcutsFromConfiguration(GetGlobalAcceleratorConfiguration(), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;

    return OUString();
}




OUString CommandInfoProvider::RetrieveShortcutsFromConfiguration(
    const Reference<ui::XAcceleratorConfiguration>& rxConfiguration,
    const OUString& rsCommandName)
{
    if (rxConfiguration.is())
    {
        try
        {
            Sequence<OUString> aCommands(1);
            aCommands[0] = rsCommandName;

            Sequence<Any> aKeyCodes (rxConfiguration->getPreferredKeyEventsForCommandList(aCommands));
            if (aCommands.getLength() == 1)
            {
                css::awt::KeyEvent aKeyEvent;
                if (aKeyCodes[0] >>= aKeyEvent)
                {
                    return svt::AcceleratorExecute::st_AWTKey2VCLKey(aKeyEvent).GetName();
                }
            }
        }
        catch (lang::IllegalArgumentException&)
        {
        }
    }
    return OUString();
}




Sequence<beans::PropertyValue> CommandInfoProvider::GetCommandProperties (const OUString& rsCommandName)
{
    Sequence<beans::PropertyValue> aProperties;
    
    try
    {
        const OUString sModuleIdentifier (GetModuleIdentifier());
        if (sModuleIdentifier.getLength() > 0)
        {
            Reference<container::XNameAccess> xNameAccess (
                mxServiceFactory->createInstance(
                    OUString::createFromAscii("com.sun.star.frame.UICommandDescription")),
                UNO_QUERY);
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess.is())
                if (xNameAccess->getByName(sModuleIdentifier) >>= xUICommandLabels)
                    xUICommandLabels->getByName(rsCommandName) >>= aProperties;
        }
    }
    catch (Exception&)
    {
    }

    return aProperties;
}




OUString CommandInfoProvider::GetCommandLabel (const OUString& rsCommandName)
{
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName));
    for (sal_Int32 nIndex=0; nIndex<aProperties.getLength(); ++nIndex)
    {
        if (aProperties[nIndex].Name.equalsAscii("Name"))
        {
            OUString sLabel;
            aProperties[nIndex].Value >>= sLabel;
            return sLabel;
        }
    }
    return OUString();
}


} } // end of namespace sfx2/framework
