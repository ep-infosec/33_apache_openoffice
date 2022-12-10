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
#include <uifactory/menubarfactory.hxx>

//_________________________________________________________________________________________________________________
//	my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"
#include <uielement/menubarwrapper.hxx>

//_________________________________________________________________________________________________________________
//	interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPLLIER_HPP_
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif

//_________________________________________________________________________________________________________________
//	includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//	Defines
//_________________________________________________________________________________________________________________
// 

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//	XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   MenuBarFactory				                    ,
                                            ::cppu::OWeakObject							    ,
                                            SERVICENAME_MENUBARFACTORY	                    ,
											IMPLEMENTATIONNAME_MENUBARFACTORY
										)

DEFINE_INIT_SERVICE                     (   MenuBarFactory, {} )

MenuBarFactory::MenuBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase()
    , m_xServiceManager( xServiceManager )
    , m_xModuleManager( xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY )
{
}
MenuBarFactory::MenuBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager,bool ) :
    ThreadHelpBase(&Application::GetSolarMutex())
    , m_xServiceManager( xServiceManager )
    , m_xModuleManager( xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY )
{
}

MenuBarFactory::~MenuBarFactory()
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL MenuBarFactory::createUIElement( 
    const ::rtl::OUString& ResourceURL, 
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    MenuBarWrapper* pMenuBarWrapper = new MenuBarWrapper( m_xServiceManager );
    Reference< ::com::sun::star::ui::XUIElement > xMenuBar( (OWeakObject *)pMenuBarWrapper, UNO_QUERY );
    Reference< ::com::sun::star::frame::XModuleManager > xModuleManager = m_xModuleManager;
    aLock.unlock();
    CreateUIElement(ResourceURL,Args,"MenuOnly","private:resource/menubar/",xMenuBar,xModuleManager,m_xServiceManager);
    return xMenuBar;
}
void MenuBarFactory::CreateUIElement(const ::rtl::OUString& ResourceURL
                                     , const Sequence< PropertyValue >& Args
                                     ,const char* _pExtraMode
                                     ,const char* _pAsciiName
                                     ,const Reference< ::com::sun::star::ui::XUIElement >& _xMenuBar
                                     ,const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager >& _xModuleManager
                                     ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceManager)
{
    Reference< XUIConfigurationManager > xCfgMgr;    
    Reference< XUIConfigurationManager > xConfigSource;
    Reference< XFrame >                  xFrame;
    rtl::OUString                        aResourceURL( ResourceURL );
    sal_Bool                             bPersistent( sal_True );
    sal_Bool                             bExtraMode( sal_False );

    for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    {
        if ( Args[n].Name.equalsAscii( "ConfigurationSource" ))
            Args[n].Value >>= xConfigSource;
        else if ( Args[n].Name.equalsAscii( "Frame" ))
            Args[n].Value >>= xFrame;
        else if ( Args[n].Name.equalsAscii( "ResourceURL" ))
            Args[n].Value >>= aResourceURL;
        else if ( Args[n].Name.equalsAscii( "Persistent" ))
            Args[n].Value >>= bPersistent;
        else if ( _pExtraMode && Args[n].Name.equalsAscii( _pExtraMode ))
            Args[n].Value >>= bExtraMode;
    } // for ( sal_Int32 n = 0; n < Args.getLength(); n++ )
    if ( aResourceURL.indexOf( rtl::OUString::createFromAscii(_pAsciiName)) != 0 )
        throw IllegalArgumentException();

    // Identify frame and determine document based ui configuration manager/module ui configuration manager
    if ( xFrame.is() && !xConfigSource.is() )
    {
        bool bHasSettings( false );
        Reference< XModel > xModel;

        Reference< XController > xController = xFrame->getController();
        if ( xController.is() )
            xModel = xController->getModel();

        if ( xModel.is() )
        {
            Reference< XUIConfigurationManagerSupplier > xUIConfigurationManagerSupplier( xModel, UNO_QUERY );
            if ( xUIConfigurationManagerSupplier.is() )
            {
                xCfgMgr = xUIConfigurationManagerSupplier->getUIConfigurationManager();
                bHasSettings = xCfgMgr->hasSettings( aResourceURL );
            }
        }

        if ( !bHasSettings )
        {
            rtl::OUString aModuleIdentifier = _xModuleManager->identify( Reference< XInterface >( xFrame, UNO_QUERY ));
            if ( aModuleIdentifier.getLength() )
            {
                Reference< ::com::sun::star::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(
                    _xServiceManager->createInstance( SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ), UNO_QUERY );
                xCfgMgr = xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier );
                bHasSettings = xCfgMgr->hasSettings( aResourceURL );
            }
        }
    }
    
    PropertyValue aPropValue;
    Sequence< Any > aPropSeq( _pExtraMode ? 5 : 4);
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropValue.Value <<= xFrame;
    aPropSeq[0] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
    aPropValue.Value <<= xCfgMgr;
    aPropSeq[1] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceURL" ));
    aPropValue.Value <<= aResourceURL;
    aPropSeq[2] <<= aPropValue;
    aPropValue.Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
    aPropValue.Value <<= bPersistent;
    aPropSeq[3] <<= aPropValue;
    if ( _pExtraMode )
    {
        aPropValue.Name = rtl::OUString::createFromAscii(_pExtraMode);
        aPropValue.Value <<= bExtraMode;
        aPropSeq[4] <<= aPropValue;
    }
    
    vos::OGuard	aGuard( Application::GetSolarMutex() );
    Reference< XInitialization > xInit( _xMenuBar, UNO_QUERY );
    xInit->initialize( aPropSeq );
}

} // namespace framework
