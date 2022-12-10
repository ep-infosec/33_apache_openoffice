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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________
#include <helper/tagwindowasmodified.hxx>
#include <pattern/window.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XWindow.hpp>

#ifndef _COM_SUN_STAR_LANG_XSERVICXEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/FrameAction.hpp>

//_________________________________________________________________________________________________________________
//	other includes
//_________________________________________________________________________________________________________________

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/wintypes.hxx>

//_________________________________________________________________________________________________________________
//	namespace

namespace framework{

//_________________________________________________________________________________________________________________
//	definitions

//*****************************************************************************************************************
//  XInterface, XTypeProvider

DEFINE_XINTERFACE_4(TagWindowAsModified                                                     ,
                    OWeakObject                                                             ,
                    DIRECT_INTERFACE (css::lang::XTypeProvider                             ),
                    DIRECT_INTERFACE (css::lang::XInitialization                           ),
                    DIRECT_INTERFACE (css::util::XModifyListener                           ),
                    DERIVED_INTERFACE(css::lang::XEventListener, css::util::XModifyListener))

DEFINE_XTYPEPROVIDER_4(TagWindowAsModified        ,
                       css::lang::XTypeProvider   ,
                       css::lang::XInitialization ,
                       css::util::XModifyListener ,
                       css::lang::XEventListener  )

//*****************************************************************************************************************
TagWindowAsModified::TagWindowAsModified(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase          (&Application::GetSolarMutex())
    , m_xSMGR                 (xSMGR                        )
{
}

//*****************************************************************************************************************
TagWindowAsModified::~TagWindowAsModified()
{
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XFrame > xFrame;

    if (lArguments.getLength() > 0)
        lArguments[0] >>= xFrame;
    
    if ( ! xFrame.is ())
        return;
                
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_xFrame = xFrame ;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
    
    xFrame->addFrameActionListener(this);
    impl_update (xFrame);
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::modified(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::util::XModifiable > xModel (m_xModel.get (), css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >      xWindow(m_xWindow.get(), css::uno::UNO_QUERY);
    if (
        ( ! xModel.is  ()       ) ||
        ( ! xWindow.is ()       ) ||
        (aEvent.Source != xModel)
       )
        return;
        
    aReadLock.unlock();
    // <- SAFE ----------------------------------
    
    ::sal_Bool bModified = xModel->isModified ();
    
    // SYNCHRONIZED ->
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
    
    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( ! pWindow)
        return;
    
    sal_Bool bSystemWindow = pWindow->IsSystemWindow(); 
    sal_Bool bWorkWindow   = (pWindow->GetType() == WINDOW_WORKWINDOW); 
    if (!bSystemWindow && !bWorkWindow)
        return;

    if (bModified)
        pWindow->SetExtendedStyle(WB_EXT_DOCMODIFIED);
    else
        pWindow->SetExtendedStyle( ! WB_EXT_DOCMODIFIED);
    
    aSolarGuard.clear();
    // <- SYNCHRONIZED
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    if (
        (aEvent.Action != css::frame::FrameAction_COMPONENT_REATTACHED) &&
        (aEvent.Action != css::frame::FrameAction_COMPONENT_ATTACHED  )
       )
        return;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::frame::XFrame > xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    if (
        ( ! xFrame.is ()        ) ||
        (aEvent.Source != xFrame)
       )
        return;
        
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
    
    impl_update (xFrame);
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::frame::XFrame > xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    if (
        (xFrame.is ()           ) &&
        (aEvent.Source == xFrame)
       )
    {
        m_xFrame = css::uno::Reference< css::frame::XFrame >();
        return;
    }
        
    css::uno::Reference< css::frame::XModel > xModel(m_xModel.get(), css::uno::UNO_QUERY);
    if (
        (xModel.is ()           ) &&
        (aEvent.Source == xModel)
       )
    {
        m_xModel = css::uno::Reference< css::frame::XModel >();
        return;
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//*****************************************************************************************************************
void TagWindowAsModified::impl_update (const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    if ( ! xFrame.is ())
        return;

    css::uno::Reference< css::awt::XWindow >       xWindow     = xFrame->getContainerWindow ();
    css::uno::Reference< css::frame::XController > xController = xFrame->getController ();
    css::uno::Reference< css::frame::XModel >      xModel ;
    if (xController.is ())
        xModel = xController->getModel ();
    
    if (
        ( ! xWindow.is ()) ||
        ( ! xModel.is  ())
       )
        return;
    
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    // Note: frame was set as member outside ! we have to refresh connections
    // regarding window and model only here.
    m_xWindow = xWindow;
    m_xModel  = xModel ;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------
    
    css::uno::Reference< css::util::XModifyBroadcaster > xModifiable(xModel, css::uno::UNO_QUERY);
    if (xModifiable.is ())
        xModifiable->addModifyListener (this);
}

} // namespace framework
