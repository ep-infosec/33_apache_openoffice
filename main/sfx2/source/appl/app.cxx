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
#include "precompiled_sfx2.hxx"

#if defined UNX
#include <limits.h>
#else // UNX
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif // UNX

#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <vos/process.hxx>
#include <tools/simplerm.hxx>
#include <tools/config.hxx>
#include <basic/basrdll.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbmod.hxx>
#include <svtools/asynclink.hxx>
#include <svl/stritem.hxx>
#include <vcl/sound.hxx>
#include <svl/eitem.hxx>
#include <svl/urlbmk.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <svl/svdde.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/mozilla/XPluginInstance.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <basic/basmgr.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <rtl/logfile.hxx>
#include <sfx2/appuno.hxx>
#include "sfx2/sfxhelp.hxx"
#include <sfx2/request.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"
#include "arrdecl.hxx"
#include <sfx2/progress.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include "fltfnc.hxx"
#include "nfltdlg.hxx"
#include <sfx2/new.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/genlink.hxx>
#include <sfx2/viewfrm.hxx>
#include "appdata.hxx"
#include "openflag.hxx"
#include "app.hrc"
#include "virtmenu.hxx"
#include <sfx2/module.hxx>
#include <sfx2/event.hxx>
#include "imestatuswindow.hxx"
#include "workwin.hxx"
#include <sfx2/module.hxx>
#include <sfx2/tbxctrl.hxx>
#include <sfx2/sfxdlg.hxx>
#include "sfx2/stbitem.hxx"
#include "eventsupplier.hxx"
#include <sfx2/dockwin.hxx>
#include <tools/svlibrary.hxx>

#ifdef DBG_UTIL
#include <sfx2/tbxctrl.hxx>
#include <sfx2/mnuitem.hxx>
#endif

#if defined( WNT ) || defined( OS2 )
#define DDE_AVAILABLE
#endif

#include <unotools/saveopt.hxx>
#include <unotools/undoopt.hxx>
#include <svtools/helpopt.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/historyoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/startoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/localisationoptions.hxx>
#include <unotools/inetoptions.hxx>
#include <unotools/fontoptions.hxx>
#include <unotools/internaloptions.hxx>
#include <unotools/workingsetoptions.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <unotools/syslocale.hxx>
#include <framework/addonsoptions.hxx>
#include <svtools/ttprops.hxx>
#include <unotools/extendedsecurityoptions.hxx>

using namespace ::com::sun::star;

// Static member
SfxApplication* SfxApplication::pApp = NULL;
static BasicDLL*       pBasic   = NULL;

class SfxPropertyHandler : public PropertyHandler
{
    virtual void Property( ApplicationProperty& );
};

static SfxPropertyHandler*	pPropertyHandler = 0;

SfxPropertyHandler* GetOrCreatePropertyHandler()
{
	if ( !pPropertyHandler )
	{
		::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
		if ( !pPropertyHandler )
			pPropertyHandler = new SfxPropertyHandler;
	}

	return pPropertyHandler;
}

void SfxPropertyHandler::Property( ApplicationProperty& rProp )
{
	TTProperties* pTTProperties = PTR_CAST( TTProperties, &rProp );
	if ( pTTProperties )
	{
		pTTProperties->nPropertyVersion = TT_PROPERTIES_VERSION;
		switch ( pTTProperties->nActualPR )
		{
			case TT_PR_SLOTS:
			{
				pTTProperties->nSidOpenUrl = SID_OPENURL;
				pTTProperties->nSidFileName = SID_FILE_NAME;
				pTTProperties->nSidNewDocDirect = SID_NEWDOCDIRECT;
				pTTProperties->nSidCopy = SID_COPY;
				pTTProperties->nSidPaste = SID_PASTE;
				pTTProperties->nSidSourceView = SID_SOURCEVIEW;
				pTTProperties->nSidSelectAll = SID_SELECTALL;
				pTTProperties->nSidReferer = SID_REFERER;
				pTTProperties->nActualPR = 0;
			}
			break;
			case TT_PR_DISPATCHER:
			{
                // interface for TestTool
                SfxViewFrame* pViewFrame=0;
                SfxDispatcher* pDispatcher=0;
				pViewFrame = SfxViewFrame::Current();
				if ( !pViewFrame )
					pViewFrame = SfxViewFrame::GetFirst();
				if ( pViewFrame )
					pDispatcher = pViewFrame->GetDispatcher();
				else
					pDispatcher = NULL;
				if ( !pDispatcher )
					pTTProperties->nActualPR = TT_PR_ERR_NODISPATCHER;
				else
				{
					pDispatcher->SetExecuteMode(EXECUTEMODE_DIALOGASYNCHRON);
                    if ( pTTProperties->mnSID == SID_NEWDOCDIRECT
                      || pTTProperties->mnSID == SID_OPENDOC )
                    {
                        SfxPoolItem** pArgs = pTTProperties->mppArgs;
                        SfxAllItemSet aSet( SFX_APP()->GetPool() );
                        if ( pArgs && *pArgs )
                        {
                            for ( SfxPoolItem **pArg = pArgs; *pArg; ++pArg )
                                aSet.Put( **pArg );
                        }
                        if ( pTTProperties->mnSID == SID_NEWDOCDIRECT )
                        {
                            String aFactory = String::CreateFromAscii("private:factory/");
                            if ( pArgs && *pArgs )
                            {
                                SFX_ITEMSET_ARG( &aSet, pFactoryName, SfxStringItem, SID_NEWDOCDIRECT, sal_False );
                                if ( pFactoryName )
                                    aFactory += pFactoryName->GetValue();
                                else
                                    aFactory += String::CreateFromAscii("swriter");
                            }
                            else
                                aFactory += String::CreateFromAscii("swriter");

                            aSet.Put( SfxStringItem( SID_FILE_NAME, aFactory ) );
                            aSet.ClearItem( SID_NEWDOCDIRECT );
                            pTTProperties->mnSID = SID_OPENDOC;
                        }

                        aSet.Put( SfxStringItem( SID_TARGETNAME, DEFINE_CONST_UNICODE("_blank") ) );
                        if ( pDispatcher->ExecuteFunction( pTTProperties->mnSID, aSet, pTTProperties->mnMode )
                                    == EXECUTE_NO )
                            pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                        else
                            pTTProperties->nActualPR = 0;
                    }
                    else
                    {
                        if ( pDispatcher->ExecuteFunction(
                                pTTProperties->mnSID, pTTProperties->mppArgs, pTTProperties->mnMode )
                            == EXECUTE_NO )
                            pTTProperties->nActualPR = TT_PR_ERR_NOEXECUTE;
                        else
                            pTTProperties->nActualPR = 0;
                    }
				}
			}
			break;
/*
			case TT_PR_IMG:
			{
				SvDataMemberObjectRef aDataObject = new SvDataMemberObject();
				SvData* pDataBmp = new SvData( FORMAT_BITMAP );
				pDataBmp->SetData( pTTProperties->mpBmp );
				aDataObject->Append( pDataBmp );
				aDataObject->CopyClipboard();
				pTTProperties->nActualPR = 0;
			}
			break;
*/
			default:
			{
				pTTProperties->nPropertyVersion = 0;
			}
		}
		return;
	}
}

#include <framework/imageproducer.hxx>
#include <framework/acceleratorinfo.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include "sfx2/imagemgr.hxx"
#include "fwkhelper.hxx"

::osl::Mutex SfxApplication::gMutex;

SfxApplication* SfxApplication::GetOrCreate()
{
	// SFX on demand
    ::osl::MutexGuard aGuard(SfxApplication::gMutex);
    if ( !pApp )
    {
        SfxApplication *pNew = new SfxApplication;

        //TODO/CLEANUP
        //ist das Mutex-Handling OK?
        static ::osl::Mutex aProtector;
        ::osl::MutexGuard aGuard2( aProtector );

        RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SetApp" );
        pApp = pNew;

        // at the moment a bug may occur when Initialize_Impl returns sal_False, but this is only temporary because all code that may cause such a
        // fault will be moved outside the SFX
        pApp->Initialize_Impl();

		::framework::SetImageProducer( GetImage );
        ::framework::SetRefreshToolbars( RefreshToolbars );
        ::framework::SetToolBoxControllerCreator( SfxToolBoxControllerFactory );
        ::framework::SetStatusBarControllerCreator( SfxStatusBarControllerFactory );
		::framework::SetDockingWindowCreator( SfxDockingWindowFactory );
        ::framework::SetIsDockingWindowVisible( IsDockingWindowVisible );
        ::framework::SetActivateToolPanel( &SfxViewFrame::ActivateToolPanel );

		SfxHelp* pSfxHelp = new SfxHelp;
		Application::SetHelp( pSfxHelp );
		if ( SvtHelpOptions().IsHelpTips() )
			Help::EnableQuickHelp();
		else
			Help::DisableQuickHelp();
		if ( SvtHelpOptions().IsHelpTips() && SvtHelpOptions().IsExtendedHelp() )
			Help::EnableBalloonHelp();
		else
			Help::DisableBalloonHelp();
	}
	return pApp;
}

SfxApplication::SfxApplication()
    : pAppData_Impl( 0 )
{
	RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mb93783) ::SfxApplication::SfxApplication" );

    SetName( DEFINE_CONST_UNICODE("StarOffice") );
	GetpApp()->SetPropertyHandler( GetOrCreatePropertyHandler() );

    SvtViewOptions::AcquireOptions();

	pAppData_Impl = new SfxAppData_Impl( this );
	pAppData_Impl->UpdateApplicationSettings( SvtMenuOptions().IsEntryHidingEnabled() );
    pAppData_Impl->m_xImeStatusWindow->init();
    pApp->PreInit();

	RTL_LOGFILE_CONTEXT_TRACE( aLog, "{ initialize DDE" );

#ifdef DDE_AVAILABLE
#ifndef DBG_UTIL
    InitializeDde();
#else
    if( !InitializeDde() )
    {
        ByteString aStr( "Kein DDE-Service moeglich. Fehler: " );
        if( GetDdeService() )
            aStr += ByteString::CreateFromInt32(GetDdeService()->GetError());
        else
            aStr += '?';
        DBG_ASSERT( sal_False, aStr.GetBuffer() );
    }
#endif
#endif

    pBasic   = new BasicDLL;
	StarBASIC::SetGlobalErrorHdl( LINK( this, SfxApplication, GlobalBasicErrorHdl_Impl ) );
	RTL_LOGFILE_CONTEXT_TRACE( aLog, "} initialize DDE" );
}

SfxApplication::~SfxApplication()
{
    OSL_ENSURE( GetObjectShells_Impl().Count() == 0, "Memory leak: some object shells were not removed!" );

    Broadcast( SfxSimpleHint(SFX_HINT_DYING) );

    SfxModule::DestroyModules_Impl();

    // delete global options
    SvtViewOptions::ReleaseOptions();
	delete pBasic;

    if ( !pAppData_Impl->bDowning )
        Deinitialize();

    delete pAppData_Impl;
    pApp = 0;
}

//====================================================================

const String& SfxApplication::GetLastDir_Impl() const

/*  [Beschreibung]

    Interne Methode, mit der im SFx das zuletzt mit der Methode
    <SfxApplication::SetLastDir_Impl()> gesetzte Verzeichnis
    zurueckgegeben wird.

    Dieses ist i.d.R. das zuletzt durch den SfxFileDialog
    angesprochene Verzeichnis.

    [Querverweis]
    <SfxApplication::SetLastDir_Impl()>
*/

{
    return pAppData_Impl->aLastDir;
}

const String& SfxApplication::GetLastSaveDirectory() const

/*  [Beschreibung]

    Wie <SfxApplication::GetLastDir_Impl()>, nur extern

    [Querverweis]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    return GetLastDir_Impl();
}

//--------------------------------------------------------------------

void SfxApplication::SetLastDir_Impl
(
    const String&   rNewDir     /*  kompletter Verzeichnis-Pfad als String */
    )

/*  [Beschreibung]

    Interne Methode, mit der ein Verzeichnis-Pfad gesetzt wird, der
    zuletzt (z.B. durch den SfxFileDialog) angesprochen wurde.

    [Querverweis]
    <SfxApplication::GetLastDir_Impl()>
*/

{
    pAppData_Impl->aLastDir = rNewDir;
}

//--------------------------------------------------------------------

void SfxApplication::ResetLastDir()
{
    String aEmpty;
    pAppData_Impl->aLastDir = aEmpty;
}

//--------------------------------------------------------------------

SfxDispatcher* SfxApplication::GetDispatcher_Impl()
{
    return pAppData_Impl->pViewFrame? pAppData_Impl->pViewFrame->GetDispatcher(): pAppData_Impl->pAppDispat;
}

//--------------------------------------------------------------------
void SfxApplication::SetViewFrame_Impl( SfxViewFrame *pFrame )
{
    if ( pFrame != pAppData_Impl->pViewFrame )
    {
		// get the containerframes ( if one of the frames is an InPlaceFrame )
        SfxViewFrame *pOldContainerFrame = pAppData_Impl->pViewFrame;
        while ( pOldContainerFrame && pOldContainerFrame->GetParentViewFrame_Impl() )
            pOldContainerFrame = pOldContainerFrame->GetParentViewFrame_Impl();
        SfxViewFrame *pNewContainerFrame = pFrame;
        while ( pNewContainerFrame && pNewContainerFrame->GetParentViewFrame_Impl() )
            pNewContainerFrame = pNewContainerFrame->GetParentViewFrame_Impl();

		// DocWinActivate : both frames belong to the same TopWindow
		// TopWinActivate : both frames belong to different TopWindows
// not used anymore!
//		sal_Bool bDocWinActivate = pOldContainerFrame && pNewContainerFrame &&
//        			pOldContainerFrame->GetTopViewFrame() == pNewContainerFrame->GetTopViewFrame();
		sal_Bool bTaskActivate = pOldContainerFrame != pNewContainerFrame;

		if ( pOldContainerFrame )
		{
			if ( bTaskActivate )
                NotifyEvent( SfxViewEventHint( SFX_EVENT_DEACTIVATEDOC, GlobalEventConfig::GetEventName(STR_EVENT_DEACTIVATEDOC), pOldContainerFrame->GetObjectShell(), pOldContainerFrame->GetFrame().GetController() ) );
		    pOldContainerFrame->DoDeactivate( bTaskActivate, pFrame );

	        if( pOldContainerFrame->GetProgress() )
	            pOldContainerFrame->GetProgress()->Suspend();
		}

        pAppData_Impl->pViewFrame = pFrame;

        //const SfxObjectShell* pSh = pViewFrame ? pViewFrame->GetObjectShell() : 0;
        //if ( !pSh )
        //{
        //    // otherwise BaseURL is set in activation of document
        //    INetURLObject aObject( SvtPathOptions().GetWorkPath() );
        //    aObject.setFinalSlash();
        //    INetURLObject::SetBaseURL( aObject.GetMainURL( INetURLObject::NO_DECODE ) );
        //}

        if( pNewContainerFrame )
		{
            pNewContainerFrame->DoActivate( bTaskActivate );
	        if ( bTaskActivate && pNewContainerFrame->GetObjectShell() )
            {
                pNewContainerFrame->GetObjectShell()->PostActivateEvent_Impl( pNewContainerFrame );
                NotifyEvent(SfxViewEventHint(SFX_EVENT_ACTIVATEDOC, GlobalEventConfig::GetEventName(STR_EVENT_ACTIVATEDOC), pNewContainerFrame->GetObjectShell(), pNewContainerFrame->GetFrame().GetController() ) );
            }

            SfxProgress *pProgress = pNewContainerFrame->GetProgress();
            if ( pProgress )
            {
                if( pProgress->IsSuspended() )
                    pProgress->Resume();
                else
                    pProgress->SetState( pProgress->GetState() );
            }

            if ( pAppData_Impl->pViewFrame->GetViewShell() )
            {
                SfxDispatcher* pDisp = pAppData_Impl->pViewFrame->GetDispatcher();
                pDisp->Flush();
                pDisp->Update_Impl(sal_True);
			}
		}
    }

    // even if the frame actually didn't change, ensure its document is forwarded
    // to SfxObjectShell::SetCurrentComponent.
    // Otherwise, the CurrentComponent might not be correct, in case it has meanwhile
    // been reset to some other document, by some non-SFX component.
    // #i49133# / 2007-12-19 / frank.schoenheit@sun.com
    if ( pFrame && pFrame->GetViewShell() )
        pFrame->GetViewShell()->SetCurrentDocument();
}

//--------------------------------------------------------------------

short SfxApplication::QuerySave_Impl( SfxObjectShell& rDoc, sal_Bool /*bAutoSave*/ )
{
    if ( !rDoc.IsModified() )
        return RET_NO;

    String aMsg( SfxResId( STR_ISMODIFIED ) );
    aMsg.SearchAndReplaceAscii( "%1", rDoc.GetTitle() );

    SfxFrame& rFrame = SfxViewFrame::GetFirst(&rDoc)->GetFrame();
    rFrame.Appear();

	WinBits nBits = WB_YES_NO_CANCEL | WB_DEF_NO;
    QueryBox aBox( &rFrame.GetWindow(), nBits, aMsg );

    return aBox.Execute();
}

//---------------------------------------------------------------------

ResMgr* SfxApplication::CreateResManager( const char *pPrefix )
{
    return ResMgr::CreateResMgr( pPrefix);
}

//---------------------------------------------------------------------

SimpleResMgr* SfxApplication::CreateSimpleResManager()
{
	SimpleResMgr	*pRet;
	const AllSettings& rAllSettings	= Application::GetSettings();
    ::com::sun::star::lang::Locale aLocale = rAllSettings.GetUILocale();
	pRet = new SimpleResMgr( CREATEVERSIONRESMGR_NAME(sfx), aLocale );

	return pRet;
}

//--------------------------------------------------------------------

ResMgr* SfxApplication::GetSfxResManager()
{
	return SfxResId::GetResMgr();
}

//--------------------------------------------------------------------

SimpleResMgr* SfxApplication::GetSimpleResManager()
{
    if ( !pAppData_Impl->pSimpleResManager )
        pAppData_Impl->pSimpleResManager = CreateSimpleResManager();
    return pAppData_Impl->pSimpleResManager;
}

//------------------------------------------------------------------------

void SfxApplication::SetProgress_Impl
(
    SfxProgress *pProgress

)
{
    DBG_ASSERT( ( !pAppData_Impl->pProgress && pProgress ) ||
                ( pAppData_Impl->pProgress && !pProgress ),
                "Progress acitivation/deacitivation mismatch" );

    if ( pAppData_Impl->pProgress && pProgress )
    {
        pAppData_Impl->pProgress->Suspend();
        pAppData_Impl->pProgress->UnLock();
        delete pAppData_Impl->pProgress;
    }

    pAppData_Impl->pProgress = pProgress;
}

//------------------------------------------------------------------------

sal_uInt16 SfxApplication::GetFreeIndex()
{
    return pAppData_Impl->aIndexBitSet.GetFreeIndex()+1;
}

//------------------------------------------------------------------------

void SfxApplication::ReleaseIndex(sal_uInt16 i)
{
    pAppData_Impl->aIndexBitSet.ReleaseIndex(i-1);
}

//--------------------------------------------------------------------

void SfxApplication::EnterAsynchronCall_Impl()
{
    ++pAppData_Impl->nAsynchronCalls;
}

//--------------------------------------------------------------------

void SfxApplication::LeaveAsynchronCall_Impl()
{
    --pAppData_Impl->nAsynchronCalls;
}

//--------------------------------------------------------------------

bool SfxApplication::IsInAsynchronCall_Impl() const
{
    return pAppData_Impl->nAsynchronCalls > 0;
}

//--------------------------------------------------------------------

Window* SfxApplication::GetTopWindow() const
{
    SfxWorkWindow* pWork = GetWorkWindow_Impl( SfxViewFrame::Current() );
    return pWork ? pWork->GetWindow() : NULL;
}

//--------------------------------------------------------------------

uno::Reference< task::XStatusIndicator > SfxApplication::GetStatusIndicator() const
{
    if ( !pAppData_Impl->pViewFrame )
        return uno::Reference< task::XStatusIndicator >();

    SfxViewFrame *pTop = pAppData_Impl->pViewFrame;
    while ( pTop->GetParentViewFrame_Impl() )
        pTop = pTop->GetParentViewFrame_Impl();

    return pTop->GetFrame().GetWorkWindow_Impl()->GetStatusIndicator();
}

SfxTbxCtrlFactArr_Impl&     SfxApplication::GetTbxCtrlFactories_Impl() const
{
    return *pAppData_Impl->pTbxCtrlFac;
}

SfxStbCtrlFactArr_Impl&     SfxApplication::GetStbCtrlFactories_Impl() const
{
    return *pAppData_Impl->pStbCtrlFac;
}

SfxMenuCtrlFactArr_Impl&    SfxApplication::GetMenuCtrlFactories_Impl() const
{
    return *pAppData_Impl->pMenuCtrlFac;
}

SfxViewFrameArr_Impl&       SfxApplication::GetViewFrames_Impl() const
{
    return *pAppData_Impl->pViewFrames;
}

SfxViewShellArr_Impl&       SfxApplication::GetViewShells_Impl() const
{
    return *pAppData_Impl->pViewShells;
}

SfxObjectShellArr_Impl&     SfxApplication::GetObjectShells_Impl() const
{
    return *pAppData_Impl->pObjShells;
}

void SfxApplication::Invalidate( sal_uInt16 nId )
{
    for( SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame; pFrame = SfxViewFrame::GetNext( *pFrame ) )
        Invalidate_Impl( pFrame->GetBindings(), nId );
}

#define DOSTRING( x )			   			#x
#define STRING( x )				   			DOSTRING( x )

typedef	long (SAL_CALL *basicide_handle_basic_error)(void*);
typedef	rtl_uString* (SAL_CALL *basicide_choose_macro)(void*, sal_Bool, rtl_uString*);
typedef	void* (SAL_CALL *basicide_macro_organizer)(sal_Int16);

extern "C" { static void SAL_CALL thisModule() {} }

IMPL_LINK( SfxApplication, GlobalBasicErrorHdl_Impl, StarBASIC*, pStarBasic )
{
    // get basctl dllname
    static ::rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "basctl" ) ) );

    // load module
	oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_handle_basic_error" ) );
    basicide_handle_basic_error pSymbol = (basicide_handle_basic_error) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_handle_basic_error in basctl
    long nRet = pSymbol( pStarBasic );

	return nRet;
}

sal_Bool SfxApplication::IsXScriptURL( const String& rScriptURL )
{
	sal_Bool result = sal_False;

    ::com::sun::star::uno::Reference
		< ::com::sun::star::lang::XMultiServiceFactory > xSMgr =
        	::comphelper::getProcessServiceFactory();

    ::com::sun::star::uno::Reference
		< ::com::sun::star::uri::XUriReferenceFactory >
        	xFactory( xSMgr->createInstance(
            	::rtl::OUString::createFromAscii(
                	"com.sun.star.uri.UriReferenceFactory" ) ),
				::com::sun::star::uno::UNO_QUERY );

    if ( xFactory.is() )
    {
		try
		{
        	::com::sun::star::uno::Reference
				< ::com::sun::star::uri::XVndSunStarScriptUrl >
            		xUrl( xFactory->parse( rScriptURL ),
						::com::sun::star::uno::UNO_QUERY );

        	if ( xUrl.is() )
        	{
            	result = sal_True;
        	}
		}
		catch ( ::com::sun::star::uno::RuntimeException& )
		{
			// ignore, will just return FALSE
		}
	}
	return result;
}

::rtl::OUString
SfxApplication::ChooseScript()
{
	::rtl::OUString aScriptURL;

	SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
	if ( pFact )
	{
		OSL_TRACE("create selector dialog");

        const SfxViewFrame* pViewFrame = SfxViewFrame::Current();
        const SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : NULL;
        uno::Reference< frame::XFrame > xFrame( pFrame ? pFrame->GetFrameInterface() : uno::Reference< frame::XFrame >() );

	  	AbstractScriptSelectorDialog* pDlg =
			pFact->CreateScriptSelectorDialog( NULL, sal_False, xFrame );

		OSL_TRACE("done, now exec it");

	  	sal_uInt16 nRet = pDlg->Execute();

		OSL_TRACE("has returned");

		if ( nRet == RET_OK )
		{
			aScriptURL = pDlg->GetScriptURL();
		}

	  	delete pDlg;
	}
    return aScriptURL;
}

void SfxApplication::MacroOrganizer( sal_Int16 nTabId )
{
    // get basctl dllname
    static ::rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( SVLIBRARY( "basctl" ) ) );

    // load module
	oslModule handleMod = osl_loadModuleRelative(
        &thisModule, aLibName.pData, 0 );

    // get symbol
    ::rtl::OUString aSymbol( RTL_CONSTASCII_USTRINGPARAM( "basicide_macro_organizer" ) );
    basicide_macro_organizer pSymbol = (basicide_macro_organizer) osl_getFunctionSymbol( handleMod, aSymbol.pData );

    // call basicide_choose_macro in basctl
    pSymbol( nTabId );
}

ErrCode SfxApplication::CallBasic( const String& rCode, BasicManager* pMgr, SbxArray* pArgs, SbxValue* pRet )
{
    return pMgr->ExecuteMacro( rCode, pArgs, pRet);
}
