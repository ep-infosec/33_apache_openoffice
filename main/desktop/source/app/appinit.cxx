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

#include <algorithm>

#include "app.hxx"
#include "cmdlineargs.hxx"
#include "desktopresid.hxx"
#include "desktop.hrc"
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>


#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/ucb/XContentProviderFactory.hpp>
#include <uno/current_context.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <osl/module.h>
#include <vos/process.hxx>
#include <rtl/uri.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <comphelper/regpathhelper.hxx>
#include <tools/debug.hxx>
#include <tools/tempfile.hxx>
#include <ucbhelper/configurationkeys.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <tools/urlobj.hxx>
#include <tools/rcid.h>

#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/svapp.hxx>
#include <unotools/startoptions.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/internaloptions.hxx>


#define	DEFINE_CONST_OUSTRING(CONSTASCII)		OUString(RTL_CONSTASCII_USTRINGPARAM(CONSTASCII))

#define DESKTOP_TEMPDIRNAME						"soffice.tmp"

using namespace rtl;
using namespace vos;
using namespace desktop;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::ucb;

namespace desktop
{

// -----------------------------------------------------------------------------

static bool configureUcb(bool bServer, rtl::OUString const & rPortalConnect)
{
	RTL_LOGFILE_CONTEXT( aLog, "desktop (sb93797) ::configureUcb" );
	Reference< XMultiServiceFactory >
        xServiceFactory( comphelper::getProcessServiceFactory() );
	if (!xServiceFactory.is())
	{
		DBG_ERROR("configureUcb(): No XMultiServiceFactory");
		return false;
	}

	rtl::OUString aPipe;
	vos::OSecurity().getUserIdent(aPipe);

	rtl::OUStringBuffer aPortal;
	if (rPortalConnect.getLength() != 0)
	{
		aPortal.append(sal_Unicode(','));
		aPortal.append(rPortalConnect);
	}

	Sequence< Any > aArgs(6);
	aArgs[0]
		<<= rtl::OUString::createFromAscii(bServer ?
										       UCB_CONFIGURATION_KEY1_SERVER :
										       UCB_CONFIGURATION_KEY1_LOCAL);
	aArgs[1]
		<<= rtl::OUString::createFromAscii(UCB_CONFIGURATION_KEY2_OFFICE);
	aArgs[2] <<= rtl::OUString::createFromAscii("PIPE");
	aArgs[3] <<= aPipe;
	aArgs[4] <<= rtl::OUString::createFromAscii("PORTAL");
	aArgs[5] <<= aPortal.makeStringAndClear();

    bool ret =
        ::ucbhelper::ContentBroker::initialize( xServiceFactory, aArgs ) != false;

#ifdef GNOME_VFS_ENABLED
    // register GnomeUCP if necessary
    ::ucbhelper::ContentBroker* cb = ::ucbhelper::ContentBroker::get();
    if(cb) {
        try {
            Reference< XCurrentContext > xCurrentContext(
                getCurrentContext());
            if (xCurrentContext.is())
            {
                Any aValue = xCurrentContext->getValueByName(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                       "system.desktop-environment" ) )
                );
                rtl::OUString aDesktopEnvironment;
                if ((aValue >>= aDesktopEnvironment)
                    && aDesktopEnvironment.equalsAscii("GNOME"))
                {
                    Reference<XContentProviderManager> xCPM =
                        cb->getContentProviderManagerInterface();
#if 0
                    try
                    {

                        Reference<XContentProviderFactory> xCPF(
                            xServiceFactory->createInstance(
                                rtl::OUString::createFromAscii(
                                    "com.sun.star.ucb.ContentProviderProxyFactory")),
                            UNO_QUERY);
                        if(xCPF.is())
                            xCPM->registerContentProvider(
                                xCPF->createContentProvider(
                                    rtl::OUString::createFromAscii(
                                        "com.sun.star.ucb.GnomeVFSContentProvider"
                                    )
                                ),
                                rtl::OUString::createFromAscii(".*"),
                                false);
                    } catch (...)
                    {
                    }
#else

		    // Workaround for P1 #124597#.  Instanciate GNOME-VFS-UCP in the thread that initialized
 		    // GNOME in order to avoid a deadlock that may occur in case UCP gets initialized from
		    // a different thread. The latter may happen when calling the Office remotely via UNO.
		    // THIS IS NOT A FIX, JUST A WORKAROUND!

                    try
                    {
                        Reference<XContentProvider> xCP(
                            xServiceFactory->createInstance(
                                rtl::OUString::createFromAscii(
                                    "com.sun.star.ucb.GnomeVFSContentProvider")),
                            UNO_QUERY);
                        if(xCP.is())
                            xCPM->registerContentProvider(
                                xCP,
                                rtl::OUString::createFromAscii(".*"),
                                false);
                    } catch (...)
                    {
                    }
                }
#endif
            }
        } catch (RuntimeException e) {
        }
    }
#endif // GNOME_VFS_ENABLED

    return ret;;
}

Reference< XMultiServiceFactory > Desktop::CreateApplicationServiceManager()
{
	RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createApplicationServiceManager" );

	try
	{
		Reference<XComponentContext> xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
		Reference<XMultiServiceFactory> xMS(xComponentContext->getServiceManager(), UNO_QUERY);

		return xMS;
	}
	catch( ::com::sun::star::uno::Exception& )
	{
	}

	return Reference< XMultiServiceFactory >();
}

void Desktop::DestroyApplicationServiceManager( Reference< XMultiServiceFactory >& xSMgr )
{
    Reference< XPropertySet > xProps( xSMgr, UNO_QUERY );
    if ( xProps.is() )
    {
        try
        {
            Reference< XComponent > xComp;
            if (xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xComp )
            {
                xComp->dispose();
            }
        }
        catch ( UnknownPropertyException& )
        {
        }
    }
}

void Desktop::RegisterServices( Reference< XMultiServiceFactory >& xSMgr )
{
    if( !m_bServicesRegistered )
    {
	    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::registerServices" );

	    // read command line parameters
	    ::rtl::OUString conDcp;
	    ::rtl::OUString aClientDisplay;
	    ::rtl::OUString aTmpString;
	    sal_Bool		bHeadlessMode = sal_False;

	    // interpret command line arguments
	    CommandLineArgs* pCmdLine = GetCommandLineArgs();

	    // read accept string from configuration
	    conDcp = SvtStartOptions().GetConnectionURL();

	    if ( pCmdLine->GetAcceptString( aTmpString ))
		    conDcp = aTmpString;

	    // Headless mode for FAT Office
	    bHeadlessMode	= pCmdLine->IsHeadless();
	    if ( bHeadlessMode )
		    Application::EnableHeadlessMode();

	    // ConversionMode
	    if ( pCmdLine->IsConversionMode() )
		    Application::EnableConversionMode();

        if ( conDcp.getLength() > 0 )
	    {
		    // accept incoming connections (scripting and one rvp)
		    RTL_LOGFILE_CONTEXT( aLog, "desktop (lo119109) desktop::Desktop::createAcceptor()" );
		    createAcceptor(conDcp);
	    }

	    // improves parallel processing on Sun ONE Webtop
	    // servicemanager up -> copy user installation
	    if ( pCmdLine->IsServer() )
	    {
		    // Check some mandatory environment states if "-server" is possible. Otherwise ignore
		    // this parameter.
		    Reference< com::sun::star::container::XContentEnumerationAccess > rContent( xSMgr , UNO_QUERY );
		    if( rContent.is() )
		    {
			    OUString sPortalService = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.portal.InstallUser" ) );
			    Reference < com::sun::star::container::XEnumeration > rEnum = rContent->createContentEnumeration( sPortalService );
			    if ( !rEnum.is() )
			    {
				    // Reset server parameter so it is ignored in the furthermore startup process
				    pCmdLine->SetBoolParam( CommandLineArgs::CMD_BOOLPARAM_SERVER, sal_False );
			    }
		    }
	    }

        ::rtl::OUString	aPortalConnect;
        bool bServer = (bool)pCmdLine->IsServer();

	    pCmdLine->GetPortalConnectString( aPortalConnect );
        if ( !configureUcb( bServer, aPortalConnect ) )
	    {
            DBG_ERROR( "Can't configure UCB" );
		    throw com::sun::star::uno::Exception(rtl::OUString::createFromAscii("RegisterServices, configureUcb"), NULL);
	    }

	    CreateTemporaryDirectory();
	    m_bServicesRegistered = true;
    }
}

namespace
{
    struct acceptorMap : public rtl::Static< AcceptorMap, acceptorMap > {};
    struct CurrentTempURL : public rtl::Static< String, CurrentTempURL > {};
}

static sal_Bool bAccept = sal_False;

void Desktop::createAcceptor(const OUString& aAcceptString)
{
	// check whether the requested acceptor already exists
	AcceptorMap &rMap = acceptorMap::get();
	AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
	if (pIter == rMap.end() ) {

		Sequence< Any > aSeq( 2 );
		aSeq[0] <<= aAcceptString;
		aSeq[1] <<= bAccept;
		Reference<XInitialization> rAcceptor(
			::comphelper::getProcessServiceFactory()->createInstance(
			OUString::createFromAscii( "com.sun.star.office.Acceptor" )), UNO_QUERY );
		if ( rAcceptor.is() ) {
			try{
				rAcceptor->initialize( aSeq );
				rMap.insert(AcceptorMap::value_type(aAcceptString, rAcceptor));
			} catch (com::sun::star::uno::Exception&) {
			// no error handling needed...
			// acceptor just won't come up
			OSL_ENSURE(sal_False, "Acceptor could not be created.");
		}
	} else {
		// there is already an acceptor with this description
		OSL_ENSURE(sal_False, "Acceptor already exists.");
	}

	}
}

class enable
{
	private:
	Sequence<Any> m_aSeq;
	public:
	enable() : m_aSeq(1) {
		m_aSeq[0] <<= sal_True;
	}
	void operator() (const AcceptorMap::value_type& val) {
		if (val.second.is()) {
			val.second->initialize(m_aSeq);
		}
	}
};

void Desktop::enableAcceptors()
{
	RTL_LOGFILE_CONTEXT(aLog, "desktop (lo119109) Desktop::enableAcceptors");
	if (!bAccept)
	{
		// from now on, all new acceptors are enabled
		bAccept = sal_True;
		// enable existing acceptors by calling initialize(true)
		// on all existing acceptors
	    AcceptorMap &rMap = acceptorMap::get();
		std::for_each(rMap.begin(), rMap.end(), enable());
	}
}

void Desktop::destroyAcceptor(const OUString& aAcceptString)
{
	// special case stop all acceptors
	AcceptorMap &rMap = acceptorMap::get();
	if (aAcceptString.compareToAscii("all") == 0) {
		rMap.clear();

	} else {
		// try to remove acceptor from map
		AcceptorMap::const_iterator pIter = rMap.find(aAcceptString);
		if (pIter != rMap.end() ) {
			// remove reference from map
			// this is the last reference and the acceptor will be destructed
			rMap.erase(aAcceptString);
		} else {
			OSL_ENSURE(sal_False, "Found no acceptor to remove");
		}
	}
}


void Desktop::DeregisterServices()
{
	// stop all acceptors by clearing the map
	acceptorMap::get().clear();
}

void Desktop::CreateTemporaryDirectory()
{
	RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::createTemporaryDirectory" );

    ::rtl::OUString aTempBaseURL;
    try
    {
        SvtPathOptions aOpt;
	    aTempBaseURL = aOpt.GetTempPath();
    }
    catch ( RuntimeException& e )
    {
        // Catch runtime exception here: We have to add language dependent info
        // to the exception message. Fallback solution uses hard coded string.
        OUString aMsg;
        DesktopResId aResId( STR_BOOTSTRAP_ERR_NO_PATHSET_SERVICE );
        aResId.SetRT( RSC_STRING );
        if ( aResId.GetResMgr()->IsAvailable( aResId ))
            aMsg = String( aResId );
        else
            aMsg = OUString( RTL_CONSTASCII_USTRINGPARAM( "The path manager is not available.\n" ));
        e.Message = aMsg + e.Message;
        throw e;
    }

    // remove possible old directory and base directory
	SvtInternalOptions	aInternalOpt;

	// set temp base directory
	sal_Int32 nLength = aTempBaseURL.getLength();
	if ( aTempBaseURL.matchAsciiL( "/", 1, nLength-1 ) )
		aTempBaseURL = aTempBaseURL.copy( 0, nLength - 1 );

	String aOldTempURL = aInternalOpt.GetCurrentTempURL();
	if ( aOldTempURL.Len() > 0 )
	{
		// remove old temporary directory
	    ::utl::UCBContentHelper::Kill( aOldTempURL );
	}

    String aRet;
	::rtl::OUString aTempPath( aTempBaseURL );

	// create new current temporary directory
	::utl::LocalFileHelper::ConvertURLToPhysicalName( aTempBaseURL, aRet );
	::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
	aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    if ( !aTempPath.getLength() )
    {
		::osl::File::getTempDirURL( aTempBaseURL );

        nLength = aTempBaseURL.getLength();
		if ( aTempBaseURL.matchAsciiL( "/", 1, nLength-1 ) )
			aTempBaseURL = aTempBaseURL.copy( 0, nLength - 1 );

        aTempPath = aTempBaseURL;
        ::osl::FileBase::getFileURLFromSystemPath( aRet, aTempPath );
        aTempPath = ::utl::TempFile::SetTempNameBaseDirectory( aTempPath );
    }

	// set new current temporary directory
    ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aTempPath, aRet );
	aInternalOpt.SetCurrentTempURL( aRet );
	CurrentTempURL::get() = aRet;
}

void Desktop::RemoveTemporaryDirectory()
{
	RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::removeTemporaryDirectory" );

	// remove current temporary directory
	String &rCurrentTempURL = CurrentTempURL::get();
	if ( rCurrentTempURL.Len() > 0 )
	{
		if ( ::utl::UCBContentHelper::Kill( rCurrentTempURL ) )
			SvtInternalOptions().SetCurrentTempURL( String() );
	}
}

} // namespace desktop
