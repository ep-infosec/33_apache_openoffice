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
#include "precompiled_desktop.hxx"

#include "osl/file.hxx"
#include "osl/mutex.hxx"

#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/logfile.hxx>
#include "cppuhelper/compbase3.hxx"

#include "vcl/wrkwin.hxx"
#include "vcl/timer.hxx"

#include <unotools/configmgr.hxx>
#include "toolkit/helper/vclunohelper.hxx"

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include "com/sun/star/deployment/XPackage.hpp"
#include "com/sun/star/deployment/ExtensionManager.hpp"
#include "com/sun/star/deployment/LicenseException.hpp"
#include "com/sun/star/deployment/ui/LicenseDialog.hpp"
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include <com/sun/star/util/XChangesBatch.hpp>

#include "app.hxx"

#include "../deployment/inc/dp_misc.h"

using rtl::OUString;
using namespace desktop;
using namespace com::sun::star;

#define UNISTRING(s) OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace
{
//For use with XExtensionManager.synchronize
class SilentCommandEnv
    : public ::cppu::WeakImplHelper3< ucb::XCommandEnvironment,
                                      task::XInteractionHandler,
                                      ucb::XProgressHandler >
{
    Desktop    *mpDesktop;
    sal_Int32   mnLevel;
    sal_Int32   mnProgress;

public:
             SilentCommandEnv( Desktop* pDesktop );
    virtual ~SilentCommandEnv();

    // XCommandEnvironment
    virtual uno::Reference<task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (uno::RuntimeException);
    virtual uno::Reference<ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        uno::Reference<task::XInteractionRequest > const & xRequest )
        throw (uno::RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( uno::Any const & Status )
        throw (uno::RuntimeException);
    virtual void SAL_CALL update( uno::Any const & Status )
        throw (uno::RuntimeException);
    virtual void SAL_CALL pop() throw (uno::RuntimeException);
};

//-----------------------------------------------------------------------------
SilentCommandEnv::SilentCommandEnv( Desktop* pDesktop )
{
    mpDesktop = pDesktop;
    mnLevel = 0;
    mnProgress = 25;
}

//-----------------------------------------------------------------------------
SilentCommandEnv::~SilentCommandEnv()
{
    mpDesktop->SetSplashScreenText( OUString() );
}

//-----------------------------------------------------------------------------
Reference<task::XInteractionHandler> SilentCommandEnv::getInteractionHandler()
    throw (uno::RuntimeException)
{
    return this;
}

//-----------------------------------------------------------------------------
Reference<ucb::XProgressHandler> SilentCommandEnv::getProgressHandler()
    throw (uno::RuntimeException)
{
    return this;
}

//-----------------------------------------------------------------------------
// XInteractionHandler
void SilentCommandEnv::handle( Reference< task::XInteractionRequest> const & xRequest )
    throw (uno::RuntimeException)
{
	deployment::LicenseException licExc;

    uno::Any request( xRequest->getRequest() );
    bool bApprove = true;

	if ( request >>= licExc )
    {
        uno::Reference< uno::XComponentContext > xContext = comphelper_getProcessComponentContext();
        uno::Reference< ui::dialogs::XExecutableDialog > xDialog(
            deployment::ui::LicenseDialog::create(
            xContext, VCLUnoHelper::GetInterface( NULL ), 
            licExc.ExtensionName, licExc.Text ) );
        sal_Int16 res = xDialog->execute();
        if ( res == ui::dialogs::ExecutableDialogResults::CANCEL )
            bApprove = false;
        else if ( res == ui::dialogs::ExecutableDialogResults::OK )
            bApprove = true;
        else
        {
            OSL_ASSERT(0);
        }
	}

    // We approve everything here
    uno::Sequence< Reference< task::XInteractionContinuation > > conts( xRequest->getContinuations() );
    Reference< task::XInteractionContinuation > const * pConts = conts.getConstArray();
    sal_Int32 len = conts.getLength();

    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        if ( bApprove )
        {
            uno::Reference< task::XInteractionApprove > xInteractionApprove( pConts[ pos ], uno::UNO_QUERY );
            if ( xInteractionApprove.is() )
                xInteractionApprove->select();
        }
        else
        {
            uno::Reference< task::XInteractionAbort > xInteractionAbort( pConts[ pos ], uno::UNO_QUERY );
            if ( xInteractionAbort.is() )
                xInteractionAbort->select();
        }
    }
}

//-----------------------------------------------------------------------------
// XProgressHandler
void SilentCommandEnv::push( uno::Any const & rStatus )
    throw (uno::RuntimeException)
{
    OUString sText;
    mnLevel += 1;

    if ( rStatus.hasValue() && ( rStatus >>= sText) )
    {
        if ( mnLevel <= 3 )
            mpDesktop->SetSplashScreenText( sText );
        else
            mpDesktop->SetSplashScreenProgress( ++mnProgress );
    }
}

//-----------------------------------------------------------------------------
void SilentCommandEnv::update( uno::Any const & rStatus )
    throw (uno::RuntimeException)
{
    OUString sText;
    if ( rStatus.hasValue() && ( rStatus >>= sText) )
    {
        mpDesktop->SetSplashScreenText( sText );
    }
}

//-----------------------------------------------------------------------------
void SilentCommandEnv::pop() throw (uno::RuntimeException)
{
    mnLevel -= 1;
}

} // end namespace

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static const OUString sConfigSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) );
static const OUString sAccessSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) );
//------------------------------------------------------------------------------
static sal_Int16 impl_showExtensionDialog( uno::Reference< uno::XComponentContext > &xContext )
{
    rtl::OUString sServiceName = UNISTRING("com.sun.star.deployment.ui.UpdateRequiredDialog");
    uno::Reference< uno::XInterface > xService;
    sal_Int16 nRet = 0;

    uno::Reference< lang::XMultiComponentFactory > xServiceManager( xContext->getServiceManager() );
    if( !xServiceManager.is() )
        throw uno::RuntimeException( 
            UNISTRING( "impl_showExtensionDialog(): unable to obtain service manager from component context" ), uno::Reference< uno::XInterface > () );

    xService = xServiceManager->createInstanceWithContext( sServiceName, xContext );
    uno::Reference< ui::dialogs::XExecutableDialog > xExecuteable( xService, uno::UNO_QUERY );
    if ( xExecuteable.is() )
        nRet = xExecuteable->execute();

    return nRet;
}

//------------------------------------------------------------------------------
// Check dependencies of all packages
//------------------------------------------------------------------------------
static bool impl_checkDependencies( const uno::Reference< uno::XComponentContext > &xContext )
{
    uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;
    uno::Reference< deployment::XExtensionManager > xExtensionManager = deployment::ExtensionManager::get( xContext );

    if ( !xExtensionManager.is() )
    {
        OSL_ENSURE( 0, "Could not get the Extension Manager!" );
        return true;
    }

    try {
        xAllPackages = xExtensionManager->getAllExtensions( uno::Reference< task::XAbortChannel >(),
                                                            uno::Reference< ucb::XCommandEnvironment >() );
    }
    catch ( deployment::DeploymentException & ) { return true; }
    catch ( ucb::CommandFailedException & ) { return true; }
    catch ( ucb::CommandAbortedException & ) { return true; }
    catch ( lang::IllegalArgumentException & e ) {
        throw uno::RuntimeException( e.Message, e.Context );
    }

    sal_Int32 nMax = 2;
#ifdef DEBUG
    nMax = 3;
#endif

    for ( sal_Int32 i = 0; i < xAllPackages.getLength(); ++i )
    {
        uno::Sequence< uno::Reference< deployment::XPackage > > xPackageList = xAllPackages[i];

        for ( sal_Int32 j = 0; (j<nMax) && (j < xPackageList.getLength()); ++j )
        {
            uno::Reference< deployment::XPackage > xPackage = xPackageList[j];
            if ( xPackage.is() )
            {
                bool bRegistered = false;
                try {
                    beans::Optional< beans::Ambiguous< sal_Bool > > option( xPackage->isRegistered( uno::Reference< task::XAbortChannel >(),
                                                                                                    uno::Reference< ucb::XCommandEnvironment >() ) );
                    if ( option.IsPresent )
                    {
                        ::beans::Ambiguous< sal_Bool > const & reg = option.Value;
                        if ( reg.IsAmbiguous )
                            bRegistered = false;
                        else
                            bRegistered = reg.Value ? true : false;
                    }
                    else
                        bRegistered = false;
                }
                catch ( uno::RuntimeException & ) { throw; }
                catch ( uno::Exception & exc) {
                    (void) exc;
                    OSL_ENSURE( 0, ::rtl::OUStringToOString( exc.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
                }

                if ( bRegistered )
                {
                    bool bDependenciesValid = false;
                    try {
                        bDependenciesValid = xPackage->checkDependencies( uno::Reference< ucb::XCommandEnvironment >() );
                    }
                    catch ( deployment::DeploymentException & ) {}
                    if ( ! bDependenciesValid )
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

//------------------------------------------------------------------------------
// resets the 'check needed' flag (needed, if aborted)
//------------------------------------------------------------------------------
static void impl_setNeedsCompatCheck()
{
    try {
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
                xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);

        Sequence< Any > theArgs(1);
        beans::NamedValue v( OUString::createFromAscii("NodePath"), 
                      makeAny( OUString::createFromAscii("org.openoffice.Setup/Office") ) );
        theArgs[0] <<= v;
        Reference< beans::XPropertySet > pset = Reference< beans::XPropertySet >(
            theConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY_THROW );

        Any value = makeAny( OUString::createFromAscii("never") );

        pset->setPropertyValue( OUString::createFromAscii("LastCompatibilityCheckID"), value );
        Reference< util::XChangesBatch >( pset, UNO_QUERY_THROW )->commitChanges();
    }
    catch (const Exception&) {}
}

//------------------------------------------------------------------------------
static bool impl_check()
{
    uno::Reference< uno::XComponentContext > xContext = comphelper_getProcessComponentContext();

    bool bDependenciesValid = impl_checkDependencies( xContext );

    short nRet = 0;

    if ( !bDependenciesValid )
        nRet = impl_showExtensionDialog( xContext );

    if ( nRet == -1 )
    {
        impl_setNeedsCompatCheck();
        return true;
    }
    else
        return false;
}

//------------------------------------------------------------------------------
// to check if we need checking the dependencies of the extensions again, we compare
// the build id of the office with the one of the last check
//------------------------------------------------------------------------------
static bool impl_needsCompatCheck()
{
    bool bNeedsCheck = false;
    rtl::OUString aLastCheckBuildID;
    rtl::OUString aCurrentBuildID( UNISTRING( "${$OOO_BASE_DIR/program/" SAL_CONFIGFILE("version") ":buildid}" ) );
	rtl::Bootstrap::expandMacros( aCurrentBuildID );

    try {
        Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();
        // get configuration provider
        Reference< XMultiServiceFactory > theConfigProvider = Reference< XMultiServiceFactory >(
                xFactory->createInstance(sConfigSrvc), UNO_QUERY_THROW);

        Sequence< Any > theArgs(1);
        beans::NamedValue v( OUString::createFromAscii("NodePath"), 
                      makeAny( OUString::createFromAscii("org.openoffice.Setup/Office") ) );
        theArgs[0] <<= v;
        Reference< beans::XPropertySet > pset = Reference< beans::XPropertySet >(
            theConfigProvider->createInstanceWithArguments( sAccessSrvc, theArgs ), UNO_QUERY_THROW );

        Any result = pset->getPropertyValue( OUString::createFromAscii("LastCompatibilityCheckID") );

        result >>= aLastCheckBuildID;
        if ( aLastCheckBuildID != aCurrentBuildID )
        {
            bNeedsCheck = true;
            result <<= aCurrentBuildID;
            pset->setPropertyValue( OUString::createFromAscii("LastCompatibilityCheckID"), result );
            Reference< util::XChangesBatch >( pset, UNO_QUERY_THROW )->commitChanges();
        }
#ifdef DEBUG
        bNeedsCheck = true;
#endif
    }
    catch (const Exception&) {}

    return bNeedsCheck;
}

//------------------------------------------------------------------------------
// Do we need to check the dependencies of the extensions?
// When there are unresolved issues, we can't continue with startup
sal_Bool Desktop::CheckExtensionDependencies()
{
    sal_Bool bAbort = false;

    if ( impl_needsCompatCheck() )
        bAbort = impl_check();

    return bAbort;
}

void Desktop::SynchronizeExtensionRepositories()
{
    RTL_LOGFILE_CONTEXT(aLog,"desktop (jl) ::Desktop::SynchronizeExtensionRepositories");
    dp_misc::syncRepositories( new SilentCommandEnv( this ) );
}
