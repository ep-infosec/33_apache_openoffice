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



#include "precompiled_sd.hxx"

#include "PresenterHelper.hxx"
#include "CanvasUpdateRequester.hxx"
#include "PresenterCanvas.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowClass.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <osl/file.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/imagerepository.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sd { namespace presenter {

//===== Service ===============================================================

Reference<XInterface> SAL_CALL PresenterHelperService_createInstance (
    const Reference<XComponentContext>& rxContext)
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterHelper(rxContext)));
}




::rtl::OUString PresenterHelperService_getImplementationName (void)
    throw(RuntimeException)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper");
}




Sequence<rtl::OUString> SAL_CALL PresenterHelperService_getSupportedServiceNames (void)
    throw (RuntimeException)
{
	static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.PresenterHelper"));
	return Sequence<rtl::OUString>(&sServiceName, 1);
}




//===== PresenterHelper =======================================================

PresenterHelper::PresenterHelper (
    const Reference<XComponentContext>& rxContext)
    : PresenterHelperInterfaceBase(m_aMutex),
      mxComponentContext(rxContext)
{
}



PresenterHelper::~PresenterHelper (void)
{
}




//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterHelper::initialize (const Sequence<Any>& rArguments)
    throw(Exception,RuntimeException)
{
    (void)rArguments;
}
    



//----- XPaneHelper ----------------------------------------------------

Reference<awt::XWindow> SAL_CALL PresenterHelper::createWindow (
    const Reference<awt::XWindow>& rxParentWindow,
    sal_Bool bCreateSystemChildWindow,
    sal_Bool bInitiallyVisible,
    sal_Bool bEnableChildTransparentMode,
    sal_Bool bEnableParentClip)
    throw (css::uno::RuntimeException)
{
    ::Window* pParentWindow = VCLUnoHelper::GetWindow(rxParentWindow);

    // Create a new window.
    ::Window* pWindow = NULL;
    if (bCreateSystemChildWindow)
    {
        pWindow = new WorkWindow(pParentWindow, WB_SYSTEMCHILDWINDOW);
    }
    else
    {
        pWindow = new ::Window(pParentWindow);
    }
    Reference<awt::XWindow> xWindow (pWindow->GetComponentInterface(), UNO_QUERY);

    if (bEnableChildTransparentMode)
    {
        // Make the frame window transparent and make the parent able to
        // draw behind it.
        if (pParentWindow != NULL)
            pParentWindow->EnableChildTransparentMode(sal_True);
    }

    if (pWindow != NULL)
    {
        pWindow->Show(bInitiallyVisible);

        pWindow->SetMapMode(MAP_PIXEL);
        pWindow->SetBackground();
        if ( ! bEnableParentClip)
        {
            pWindow->SetParentClipMode(PARENTCLIPMODE_NOCLIP);
            pWindow->SetPaintTransparent(sal_True);
        }
        else
        {
            pWindow->SetParentClipMode(PARENTCLIPMODE_CLIP);
            pWindow->SetPaintTransparent(sal_False);
        }

    }

    return xWindow;
}




Reference<rendering::XCanvas> SAL_CALL PresenterHelper::createSharedCanvas (
    const Reference<rendering::XSpriteCanvas>& rxUpdateCanvas,
    const Reference<awt::XWindow>& rxUpdateWindow,
    const Reference<rendering::XCanvas>& rxSharedCanvas,
    const Reference<awt::XWindow>& rxSharedWindow,
    const Reference<awt::XWindow>& rxWindow)
    throw (css::uno::RuntimeException)
{
    if ( ! rxSharedCanvas.is()
        || ! rxSharedWindow.is()
        || ! rxWindow.is())
    {
        throw RuntimeException(
            OUString::createFromAscii("illegal argument"),
            Reference<XInterface>(static_cast<XWeak*>(this)));
    }
        
    if (rxWindow == rxSharedWindow)
        return rxSharedCanvas;
    else
        return new PresenterCanvas(
            rxUpdateCanvas,
            rxUpdateWindow,
            rxSharedCanvas,
            rxSharedWindow,
            rxWindow);
}




Reference<rendering::XCanvas> SAL_CALL PresenterHelper::createCanvas (
    const Reference<awt::XWindow>& rxWindow,
    sal_Int16 nRequestedCanvasFeatures,
    const OUString& rsOptionalCanvasServiceName)
    throw (css::uno::RuntimeException)
{
    (void)nRequestedCanvasFeatures;

    // No shared window is given or an explicit canvas service name is
    // specified.  Create a new canvas.
    ::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != NULL)
    {
        Sequence<Any> aArg (5);
        
        // common: first any is VCL pointer to window (for VCL canvas)
        aArg[0] = makeAny(reinterpret_cast<sal_Int64>(pWindow));
        aArg[1] = Any();
        aArg[2] = makeAny(::com::sun::star::awt::Rectangle());
        aArg[3] = makeAny(sal_False);
        aArg[4] = makeAny(rxWindow);
        
        Reference<lang::XMultiServiceFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        return Reference<rendering::XCanvas>(
            xFactory->createInstanceWithArguments(
                rsOptionalCanvasServiceName.getLength()>0
                    ? rsOptionalCanvasServiceName
                    : OUString::createFromAscii("com.sun.star.rendering.VCLCanvas"),
                aArg),
            UNO_QUERY);
    }
    else
        throw RuntimeException();
}




void SAL_CALL PresenterHelper::toTop (
    const Reference<awt::XWindow>& rxWindow)
    throw (css::uno::RuntimeException)
{
    ::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != NULL)
    {
        pWindow->ToTop();
        pWindow->SetZOrder(NULL, WINDOW_ZORDER_LAST);
    }
}




Reference<rendering::XBitmap> SAL_CALL PresenterHelper::loadBitmap (
    const OUString& rsURL,
    const Reference<rendering::XCanvas>& rxCanvas)
    throw (RuntimeException)
{
    if ( ! rxCanvas.is())
        return NULL;
    
	::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    if (mpGraphicFilter.get() == NULL)
        mpGraphicFilter.reset(new GraphicFilter(sal_False));

    const cppcanvas::CanvasSharedPtr pCanvas (
        cppcanvas::VCLFactory::getInstance().createCanvas(
            Reference<css::rendering::XBitmapCanvas>(rxCanvas,UNO_QUERY)));

    if (pCanvas.get()!=NULL && rsURL.getLength()>0 && mpGraphicFilter.get()!=NULL)
    {
        sal_Int32 nIndex = 0;
        if( rsURL.getToken( 0, '/', nIndex ).equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM( "private:graphicrepository" ) ) )
        {
            OUString sPathName( rsURL.copy( nIndex ) );
            BitmapEx aBitmap;
            if ( ::vcl::ImageRepository::loadImage( sPathName, aBitmap, false )
                && !aBitmap.IsEmpty() )
                return cppcanvas::VCLFactory::getInstance().createBitmap(
                    pCanvas,
                    aBitmap)->getUNOBitmap();
        }
    }

    return NULL;
}





void SAL_CALL PresenterHelper::captureMouse (
    const Reference<awt::XWindow>& rxWindow)
    throw (RuntimeException)
{
	::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Capture the mouse (if not already done.)
    ::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != NULL && ! pWindow->IsMouseCaptured())
    {
        pWindow->CaptureMouse();
    }
}




void SAL_CALL PresenterHelper::releaseMouse (const Reference<awt::XWindow>& rxWindow)
    throw (RuntimeException)
{
	::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    // Release the mouse (if not already done.)
    ::Window* pWindow = VCLUnoHelper::GetWindow(rxWindow);
    if (pWindow != NULL && pWindow->IsMouseCaptured())
    {
        pWindow->ReleaseMouse();
    }
}




awt::Rectangle PresenterHelper::getWindowExtentsRelative (
    const Reference<awt::XWindow>& rxChildWindow,
    const Reference<awt::XWindow>& rxParentWindow)
    throw (RuntimeException)
{
    ::Window* pChildWindow = VCLUnoHelper::GetWindow(rxChildWindow);
    ::Window* pParentWindow = VCLUnoHelper::GetWindow(rxParentWindow);
    if (pChildWindow!=NULL && pParentWindow!=NULL)
    {
        Rectangle aBox (pChildWindow->GetWindowExtentsRelative(pParentWindow));
        return awt::Rectangle(aBox.Left(),aBox.Top(),aBox.GetWidth(),aBox.GetHeight());
    }
    else
        return awt::Rectangle();
}



} } // end of namespace ::sd::presenter
