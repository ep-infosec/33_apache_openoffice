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
#include "precompiled_sdext.hxx"

#include "PresenterScreen.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterController.hxx"
#include "PresenterFrameworkObserver.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/framework/Configuration.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
#include <com/sun/star/presentation/XSlideShow.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <boost/bind.hpp>
#include <tools/debug.hxx>

#include <com/sun/star/view/XSelectionSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#define A2S(s) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s)))

namespace sdext { namespace presenter {

namespace {

    static sal_Bool lcl_IsPresenterEnabled(
        const css::uno::Reference< css::uno::XComponentContext > &rxContext )
    {
        sal_Bool bEnabled( sal_True );
        PresenterConfigurationAccess aConfig(
            rxContext,
            A2S( "/org.openoffice.Office.Impress" ),
            PresenterConfigurationAccess::READ_ONLY );
        if ( aConfig.IsValid() )
        {
            sal_Bool bVal( sal_False );
            if ( ( aConfig.GetConfigurationNode(
                A2S( "Misc/Start/PresenterScreen" )) >>= bVal ) )
                bEnabled = bVal;
        }
        return bEnabled;
    }

    typedef ::cppu::WeakComponentImplHelper1 <
		css::document::XEventListener
        > PresenterScreenListenerInterfaceBase;

    /** One instance of a PresenterScreenListener is registered per Impress
        document and waits for the full screen slide show to start and to
        end.
    */
    class PresenterScreenListener
        : private ::boost::noncopyable,
          private ::cppu::BaseMutex,
          public PresenterScreenListenerInterfaceBase
    {
    public:
        PresenterScreenListener (
            const css::uno::Reference<css::uno::XComponentContext>& rxContext,
            const css::uno::Reference<css::frame::XModel2>& rxModel);
        virtual ~PresenterScreenListener (void);

        void Initialize (void);
        virtual void SAL_CALL disposing (void);

        // document::XEventListener
        
        virtual void SAL_CALL notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException);

        // XEventListener
    
        virtual void SAL_CALL disposing ( const css::lang::EventObject& rEvent) throw (css::uno::RuntimeException);

    private:
        css::uno::Reference<css::frame::XModel2 > mxModel;
        css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
        rtl::Reference<PresenterScreen> mpPresenterScreen;

        void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
    };
}


//----- Service ---------------------------------------------------------------

OUString PresenterScreenJob::getImplementationName_static (void)
{
    return A2S("com.sun.star.comp.presentation.PresenterScreenJob");
}




Sequence<OUString> PresenterScreenJob::getSupportedServiceNames_static (void)
{
	static const ::rtl::OUString sServiceName(
        A2S("com.sun.star.presentation.PresenterScreenJob"));
	return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterScreenJob::Create (const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((css::uno::Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterScreenJob(rxContext)));
}




//===== PresenterScreenJob ====================================================

PresenterScreenJob::PresenterScreenJob (const Reference<XComponentContext>& rxContext)
    : PresenterScreenJobInterfaceBase(m_aMutex),
      mxComponentContext(rxContext)
{
}




PresenterScreenJob::~PresenterScreenJob (void)
{
}




void SAL_CALL PresenterScreenJob::disposing (void)
{
    mxComponentContext = NULL;
}




//----- XJob -----------------------------------------------------------

Any SAL_CALL PresenterScreenJob::execute(
    const Sequence< beans::NamedValue >& Arguments )
    throw (lang::IllegalArgumentException, Exception, RuntimeException)
{
	Sequence< beans::NamedValue > lEnv;

    sal_Int32               i = 0;
    sal_Int32               c = Arguments.getLength();
    const beans::NamedValue* p = Arguments.getConstArray();
    for (i=0; i<c; ++i)
	{
        if (p[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Environment")))
        {
            p[i].Value >>= lEnv;
            break;
        }
	}

    Reference<frame::XModel2> xModel;
    c = lEnv.getLength();
    p = lEnv.getConstArray();
    for (i=0; i<c; ++i)
    {
        if (p[i].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Model")))
        {
            p[i].Value >>= xModel;
            break;
        }
    }

	Reference< XServiceInfo > xInfo( xModel, UNO_QUERY );
	if( !xInfo.is() || !xInfo->supportsService(
        A2S( "com.sun.star.presentation.PresentationDocument" ) ) )
        return Any();

    // Create a new listener that waits for the full screen presentation
    // to start and to end.  It takes care of its own lifetime.
    ::rtl::Reference<PresenterScreenListener> pListener (
        new PresenterScreenListener(mxComponentContext, xModel));
    pListener->Initialize();

    return Any();
}




//===== PresenterScreenListener ===============================================

namespace {

PresenterScreenListener::PresenterScreenListener (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const css::uno::Reference<css::frame::XModel2>& rxModel)
    : PresenterScreenListenerInterfaceBase(m_aMutex),
      mxModel(rxModel),
      mxComponentContext(rxContext),
      mpPresenterScreen()
{
}




void PresenterScreenListener::Initialize (void)
{
    Reference< document::XEventListener > xDocListener(
        static_cast< document::XEventListener* >(this), UNO_QUERY);
    Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
    if( xDocBroadcaster.is() )
        xDocBroadcaster->addEventListener(xDocListener);
}




PresenterScreenListener::~PresenterScreenListener (void)
{
}




void SAL_CALL PresenterScreenListener::disposing (void)
{
    Reference< document::XEventBroadcaster > xDocBroadcaster( mxModel, UNO_QUERY );
    if( xDocBroadcaster.is() )
        xDocBroadcaster->removeEventListener(
            Reference<document::XEventListener>(
                static_cast<document::XEventListener*>(this), UNO_QUERY));
    
    if (mpPresenterScreen.is())
    {
        mpPresenterScreen->RequestShutdownPresenterScreen();
        mpPresenterScreen = NULL;
    }
}




// document::XEventListener
        
void SAL_CALL PresenterScreenListener::notifyEvent( const css::document::EventObject& Event ) throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();

	if( Event.EventName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OnStartPresentation" ) ) )
	{
		mpPresenterScreen = new PresenterScreen(mxComponentContext, mxModel);
        mpPresenterScreen->InitializePresenterScreen();
	}
	else if( Event.EventName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "OnEndPresentation" ) ) )
	{
        if (mpPresenterScreen.is())
        {
            mpPresenterScreen->RequestShutdownPresenterScreen();
            mpPresenterScreen = NULL;
        }
	}
}




// XEventListener
    
void SAL_CALL PresenterScreenListener::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    
    if (mpPresenterScreen.is())
    {
        mpPresenterScreen->RequestShutdownPresenterScreen();
        mpPresenterScreen = NULL;
    }
}




void PresenterScreenListener::ThrowIfDisposed (void) const throw (
    ::com::sun::star::lang::DisposedException)
{
	if (rBHelper.bDisposed || rBHelper.bInDispose)
	{
        throw lang::DisposedException (
            A2S("PresenterScreenListener object has already been disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} // end of anonymous namespace




//===== PresenterScreen =======================================================

PresenterScreen::PresenterScreen (
    const Reference<XComponentContext>& rxContext,
    const css::uno::Reference<css::frame::XModel2>& rxModel)
    : PresenterScreenInterfaceBase(m_aMutex),
	  mxModel(rxModel),
      mxController(),
      mxConfigurationControllerWeak(),
      mxContextWeak(rxContext),
      mxSlideShowControllerWeak(),
      mpPresenterController(),
      mxSlideShowViewId(),
      mxSavedConfiguration(),
      mpPaneContainer(),
      mnComponentIndex(0),
      mxPaneFactory(),
      mxViewFactory(),
      maViewDescriptors()
{
}




PresenterScreen::~PresenterScreen (void)
{
}




void SAL_CALL PresenterScreen::disposing (void)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is() && mxSavedConfiguration.is())
    {
        xCC->restoreConfiguration(mxSavedConfiguration);
    }
    mxConfigurationControllerWeak = Reference<XConfigurationController>(NULL);

    Reference<lang::XComponent> xViewFactoryComponent (mxViewFactory, UNO_QUERY);
    if (xViewFactoryComponent.is())
       xViewFactoryComponent->dispose();
    Reference<lang::XComponent> xPaneFactoryComponent (mxPaneFactory, UNO_QUERY);
    if (xPaneFactoryComponent.is())
        xPaneFactoryComponent->dispose();

    mxModel = NULL;
}




//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterScreen::disposing (const lang::EventObject& /*rEvent*/)
    throw (RuntimeException)
{
	mxSlideShowControllerWeak = WeakReference<presentation::XSlideShowController>();
	RequestShutdownPresenterScreen();
}




//-----------------------------------------------------------------------------

void PresenterScreen::InitializePresenterScreen (void)
{
	try
	{
		Reference<XComponentContext> xContext (mxContextWeak);

        // Check if disabled by configuration
        if (!lcl_IsPresenterEnabled(xContext) )
            return;

        mpPaneContainer =
            new PresenterPaneContainer(Reference<XComponentContext>(xContext));

        Reference<XPresentationSupplier> xPS ( mxModel, UNO_QUERY_THROW);
		Reference<XPresentation2> xPresentation(xPS->getPresentation(), UNO_QUERY_THROW);
		Reference<presentation::XSlideShowController> xSlideShowController( xPresentation->getController() );
        mxSlideShowControllerWeak = xSlideShowController;

		if( !xSlideShowController.is() || !xSlideShowController->isFullScreen() )
			return;

		// find first controller that is not the current controller (the one with the slideshow
		mxController = mxModel->getCurrentController();
		Reference< container::XEnumeration > xEnum( mxModel->getControllers() );
		if( xEnum.is() )
		{
			while( xEnum->hasMoreElements() )
			{
				Reference< frame::XController > xC( xEnum->nextElement(), UNO_QUERY );
				if( xC.is() && (xC != mxController) )
				{
					mxController = xC;
					break;
				}
			}
		}
		// Get the XController from the first argument.
		Reference<XControllerManager> xCM(mxController, UNO_QUERY_THROW);

		Reference<XConfigurationController> xCC( xCM->getConfigurationController());
		mxConfigurationControllerWeak = xCC;

        Reference<drawing::framework::XResourceId> xMainPaneId(
            GetMainPaneId(xPresentation));
        // An empty reference means that the presenter screen can
        // not or must not be displayed.
        if ( ! xMainPaneId.is())
            return;

		if (xCC.is() && xContext.is())
		{
			// Store the current configuration so that we can restore it when
			// the presenter view is deactivated.
			mxSavedConfiguration = xCC->getRequestedConfiguration();
			xCC->lock();

			try
			{
                // At the moment the presenter controller is displayed in its
                // own full screen window that is controlled by the same
                // configuration controller as the Impress document from
                // which the presentation was started.  Therefore the main
                // pane is activated additionally to the already existing
                // panes and does not replace them.
				xCC->requestResourceActivation(
					xMainPaneId,
					ResourceActivationMode_ADD);
				SetupConfiguration(xContext, xMainPaneId);

				mpPresenterController = new PresenterController(
					xContext,
					mxController,
					xSlideShowController,
					mpPaneContainer,
					xMainPaneId);

				// Create pane and view factories and integrate them into the
				// drawing framework.
				SetupPaneFactory(xContext);
				SetupViewFactory(xContext);

                mpPresenterController->GetWindowManager()->RestoreViewMode();
			}
			catch (RuntimeException&)
			{
				xCC->restoreConfiguration(mxSavedConfiguration);
			}
			xCC->unlock();
		}
	}
	catch (Exception&)
	{
	}
}




sal_Int32 PresenterScreen::GetScreenNumber (
    const Reference<presentation::XPresentation2>& rxPresentation) const
{
    // Determine the screen on which the full screen presentation is being
    // displayed.
    sal_Int32 nScreenNumber (0);
    sal_Int32 nScreenCount (1);
    try
    {
        Reference<beans::XPropertySet> xProperties (rxPresentation, UNO_QUERY);
        if ( ! xProperties.is())
            return -1;

        sal_Int32 nDisplayNumber (-1);
        if ( ! (xProperties->getPropertyValue(A2S("Display")) >>= nDisplayNumber))
            return -1;
        if (nDisplayNumber == -1)
        {
            // The special value -1 indicates that the slide show
            // spans all available displays.  That leaves no room for
            // the presenter screen.
            return -1;
        }

        Reference<XComponentContext> xContext (mxContextWeak);
        if ( ! xContext.is())
            return -1;
        Reference<lang::XMultiComponentFactory> xFactory (
            xContext->getServiceManager(), UNO_QUERY);
        if ( ! xFactory.is())
            return -1;
        Reference<beans::XPropertySet> xDisplayProperties (
            xFactory->createInstanceWithContext(A2S("com.sun.star.awt.DisplayAccess"),xContext),
            UNO_QUERY);
        if  ( ! xDisplayProperties.is())
            return -1;
        
        if (nDisplayNumber > 0)
        {
            nScreenNumber = nDisplayNumber - 1;
        }
        else if (nDisplayNumber == 0)
        {
            // A display number value of 0 indicates the primary screen.
            // Instantiate the DisplayAccess service to find out which
            // screen number that is.
            if (nDisplayNumber <= 0 && xDisplayProperties.is())
                xDisplayProperties->getPropertyValue(A2S("DefaultDisplay")) >>= nScreenNumber;
        }
        
        // We still have to determine the number of screens to decide
        // whether the presenter screen may be shown at all.
        Reference<container::XIndexAccess> xIndexAccess (xDisplayProperties, UNO_QUERY);
        if ( ! xIndexAccess.is())
            return -1;
        nScreenCount = xIndexAccess->getCount();

        if (nScreenCount < 2 || nDisplayNumber > nScreenCount)
        {
            // There is either only one screen or the full screen
            // presentation spans all available screens.  The presenter
            // screen is shown only when a special flag in the configuration
            // is set.
            PresenterConfigurationAccess aConfiguration (
                xContext,
                A2S("/org.openoffice.Office.PresenterScreen/"),
                PresenterConfigurationAccess::READ_ONLY);
            bool bStartAlways (false);
            if (aConfiguration.GetConfigurationNode(
                A2S("Presenter/StartAlways")) >>= bStartAlways)
            {
                if (bStartAlways)
                    return nScreenNumber;
            }
            return -1;
        }
    }
    catch (beans::UnknownPropertyException&)
    {
        OSL_ASSERT(false);
        // For some reason we can not access the screen number.  Use
        // the default instead.
    }

    return nScreenNumber;
}




Reference<drawing::framework::XResourceId> PresenterScreen::GetMainPaneId (
    const Reference<presentation::XPresentation2>& rxPresentation) const
{
    // A negative value means that the presentation spans all available
    // displays.  That leaves no room for the presenter.
    const sal_Int32 nScreenNumber(GetScreenNumber(rxPresentation));
    if (nScreenNumber < 0)
        return NULL;
    
    // Setup the resource id of the full screen background pane so that
    // it is displayed on another screen than the presentation.
    sal_Int32 nPresenterScreenNumber (1);
    switch (nScreenNumber)
    {
        case 0:
            nPresenterScreenNumber = 1;
            break;

        case 1:
            nPresenterScreenNumber = 0;
            break;
            
        default:
            // When the full screen presentation is displayed on a screen
            // other than 0 or 1 then place the presenter on the first
            // available screen.
            nPresenterScreenNumber = 0;
            break;
    }
    
    return ResourceId::create(
        Reference<XComponentContext>(mxContextWeak),
        PresenterHelper::msFullScreenPaneURL
            +A2S("?FullScreen=true&ScreenNumber=")
                + OUString::valueOf(nPresenterScreenNumber));
}




void PresenterScreen::RequestShutdownPresenterScreen (void)
{
    // Restore the configuration that was active before the presenter screen
    // has been activated.  Now, that the presenter screen is displayed in
    // its own top level window this probably not necessary, but one never knows.
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is() && mxSavedConfiguration.is())
    {
        xCC->restoreConfiguration(mxSavedConfiguration);
        mxSavedConfiguration = NULL;
    }

    if (xCC.is())
    {
        // The actual restoration of the configuration takes place
        // asynchronously.  The view and pane factories can only by disposed
        // after that.  Therefore, set up a listener and wait for the
        // restoration.
        rtl::Reference<PresenterScreen> pSelf (this);
        PresenterFrameworkObserver::RunOnUpdateEnd(
            xCC,
            ::boost::bind(&PresenterScreen::ShutdownPresenterScreen, pSelf));
        xCC->update();
    }
}




void PresenterScreen::ShutdownPresenterScreen (void)
{
    Reference<lang::XComponent> xViewFactoryComponent (mxViewFactory, UNO_QUERY);
    if (xViewFactoryComponent.is())
        xViewFactoryComponent->dispose();
    mxViewFactory = NULL;
    
    Reference<lang::XComponent> xPaneFactoryComponent (mxPaneFactory, UNO_QUERY);
    if (xPaneFactoryComponent.is())
        xPaneFactoryComponent->dispose();
    mxPaneFactory = NULL;

    if (mpPresenterController.get() != NULL)
    {
        mpPresenterController->dispose();
        mpPresenterController = rtl::Reference<PresenterController>();
    }
    mpPaneContainer = new PresenterPaneContainer(Reference<XComponentContext>(mxContextWeak));
}




void PresenterScreen::SetupPaneFactory (const Reference<XComponentContext>& rxContext)
{
    try
    {
        if ( ! mxPaneFactory.is())
            mxPaneFactory = PresenterPaneFactory::Create(
                rxContext,
                mxController,
                mpPresenterController);
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}




void PresenterScreen::SetupViewFactory (const Reference<XComponentContext>& rxContext)
{
    try
    {
        if ( ! mxViewFactory.is())
            mxViewFactory = PresenterViewFactory::Create(
                rxContext,
                mxController,
                mpPresenterController);
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }
}




void PresenterScreen::SetupConfiguration (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    try
    {
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            A2S("org.openoffice.Office.PresenterScreen"),
            PresenterConfigurationAccess::READ_ONLY);
        maViewDescriptors.clear();
        ProcessViewDescriptions(aConfiguration);
        OUString sLayoutName (RTL_CONSTASCII_USTRINGPARAM("DefaultLayout"));
        aConfiguration.GetConfigurationNode(
            A2S("Presenter/CurrentLayout")) >>= sLayoutName;
        ProcessLayout(aConfiguration, sLayoutName, rxContext, rxAnchorId);
    }
    catch (RuntimeException&)
    {
    }    
}




void PresenterScreen::ProcessLayout (
    PresenterConfigurationAccess& rConfiguration,
    const OUString& rsLayoutName,
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    try
    {
        Reference<container::XHierarchicalNameAccess> xLayoutNode (
            rConfiguration.GetConfigurationNode(
                A2S("Presenter/Layouts/") + rsLayoutName),
            UNO_QUERY_THROW);

        // Read the parent layout first, if one is referenced.
        OUString sParentLayout;
        rConfiguration.GetConfigurationNode(
            xLayoutNode,
            A2S("ParentLayout")) >>= sParentLayout;
        if (sParentLayout.getLength() > 0)
        {
            // Prevent infinite recursion.
            if (rsLayoutName != sParentLayout)
                ProcessLayout(rConfiguration, sParentLayout, rxContext, rxAnchorId);
        }

        // Process the actual layout list.
        Reference<container::XNameAccess> xList (
            rConfiguration.GetConfigurationNode(
                xLayoutNode,
                A2S("Layout")),
            UNO_QUERY_THROW);

        ::std::vector<rtl::OUString> aProperties (6);
        aProperties[0] = A2S("PaneURL");
        aProperties[1] = A2S("ViewURL");
        aProperties[2] = A2S("RelativeX");
        aProperties[3] = A2S("RelativeY");
        aProperties[4] = A2S("RelativeWidth");
        aProperties[5] = A2S("RelativeHeight");
        mnComponentIndex = 1;
        PresenterConfigurationAccess::ForAll(
            xList,
            aProperties,
            ::boost::bind(&PresenterScreen::ProcessComponent, this,
                _1,
                _2,
                rxContext,
                rxAnchorId));
    }
    catch (RuntimeException&)
    {
    }    
}




void PresenterScreen::ProcessViewDescriptions (
    PresenterConfigurationAccess& rConfiguration)
{
    try
    {
        Reference<container::XNameAccess> xViewDescriptionsNode (
            rConfiguration.GetConfigurationNode(A2S("Presenter/Views")),
            UNO_QUERY_THROW);

        ::std::vector<rtl::OUString> aProperties (4);
        aProperties[0] = A2S("ViewURL");
        aProperties[1] = A2S("Title");
        aProperties[2] = A2S("AccessibleTitle");
        aProperties[3] = A2S("IsOpaque");
        mnComponentIndex = 1;
        PresenterConfigurationAccess::ForAll(
            xViewDescriptionsNode,
            aProperties,
            ::boost::bind(&PresenterScreen::ProcessViewDescription, this, _1, _2));
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }    
}




void PresenterScreen::ProcessComponent (
    const OUString& rsKey,
    const ::std::vector<Any>& rValues,
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId)
{
    (void)rsKey;

    if (rValues.size() != 6)
        return;

    try
    {
        OUString sPaneURL;
        OUString sViewURL;
        double nX = 0;
        double nY = 0;
        double nWidth = 0;
        double nHeight = 0;
        rValues[0] >>= sPaneURL;
        rValues[1] >>= sViewURL;
        rValues[2] >>= nX;
        rValues[3] >>= nY;
        rValues[4] >>= nWidth;
        rValues[5] >>= nHeight;

        if (nX>=0 && nY>=0 && nWidth>0 && nHeight>0)
        {
            SetupView(
                rxContext,
                rxAnchorId,
                sPaneURL,
                sViewURL,
                PresenterPaneContainer::ViewInitializationFunction(),
                nX,
                nY,
                nX+nWidth,
                nY+nHeight);
        }
   	}
	catch (Exception& e)
	{
        (void)e;
        OSL_ASSERT(false);
	}
}




void PresenterScreen::ProcessViewDescription (
    const OUString& rsKey,
    const ::std::vector<Any>& rValues)
{
    (void)rsKey;

    if (rValues.size() != 4)
        return;

    try
    {
        ViewDescriptor aViewDescriptor;
        OUString sViewURL;
        rValues[0] >>= sViewURL;
        rValues[1] >>= aViewDescriptor.msTitle;
        rValues[2] >>= aViewDescriptor.msAccessibleTitle;
        rValues[3] >>= aViewDescriptor.mbIsOpaque;
        if (aViewDescriptor.msAccessibleTitle.getLength()==0)
            aViewDescriptor.msAccessibleTitle = aViewDescriptor.msTitle;
        maViewDescriptors[sViewURL] = aViewDescriptor;
   	}
	catch (Exception&)
	{
        OSL_ASSERT(false);
	}
}




void PresenterScreen::SetupView(
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxAnchorId,
    const OUString& rsPaneURL,
    const OUString& rsViewURL,
    const PresenterPaneContainer::ViewInitializationFunction& rViewInitialization,
    const double nLeft,
    const double nTop,
    const double nRight,
    const double nBottom)
{
    Reference<XConfigurationController> xCC (mxConfigurationControllerWeak);
    if (xCC.is())
    {
        Reference<XResourceId> xPaneId (ResourceId::createWithAnchor(rxContext,rsPaneURL,rxAnchorId));
        // Look up the view descriptor.
        ViewDescriptor aViewDescriptor;
        ViewDescriptorContainer::const_iterator iDescriptor (maViewDescriptors.find(rsViewURL));
        if (iDescriptor != maViewDescriptors.end())
            aViewDescriptor = iDescriptor->second;

        // Prepare the pane.
        OSL_ASSERT(mpPaneContainer.get() != NULL);
        mpPaneContainer->PreparePane(
            xPaneId,
            rsViewURL,
            aViewDescriptor.msTitle,
            aViewDescriptor.msAccessibleTitle,
            aViewDescriptor.mbIsOpaque,
            rViewInitialization,
            nLeft,
            nTop,
            nRight,
            nBottom);
    }
}




} } // end of namespace ::sdext::presenter
