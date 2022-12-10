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
#include "precompiled_vcl.hxx"

#include "svsys.h"

#include "comphelper/processfactory.hxx"

#include "osl/module.h"
#include "osl/file.hxx"
#include "osl/thread.h"

#include "rtl/tencinfo.h"
#include "rtl/instance.hxx"

#include "vos/process.hxx"
#include "vos/mutex.hxx"

#include "tools/tools.h"
#include "tools/debug.hxx"
#include "tools/time.hxx"

#include "i18npool/mslangid.hxx"

#include "unotools/syslocaleoptions.hxx"

#include "vcl/settings.hxx"
#include "vcl/keycod.hxx"
#include "vcl/event.hxx"
#include "vcl/vclevent.hxx"
#include "vcl/virdev.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/svapp.hxx"
#include "vcl/cvtgrf.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/timer.hxx"
#include "vcl/unohelp.hxx"
#include "vcl/lazydelete.hxx"

#include "salinst.hxx"
#include "salframe.hxx"
#include "salsys.hxx"
#include "svdata.hxx"
#include "salimestatus.hxx"
#include "xconnection.hxx"
#include "window.h"
#include "accmgr.hxx"
#include "idlemgr.hxx"
#include "svids.hrc"

#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/awt/XToolkit.hpp"
#include "com/sun/star/uno/XNamingService.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"

#include <utility>

#ifdef WNT
#include <tchar.h>
#endif

using namespace ::com::sun::star::uno;

// keycodes handled internally by VCL
class ImplReservedKey
{
public:
    ImplReservedKey( KeyCode aKeyCode, sal_uInt16 nResId ) :
      mKeyCode(aKeyCode), mnResId( nResId)
     {}

    KeyCode mKeyCode;
    sal_uInt16  mnResId;
};

typedef std::pair<ImplReservedKey*, size_t> ReservedKeys;
namespace
{
    struct ImplReservedKeysImpl
    {
        ReservedKeys* operator()()
        {
            static ImplReservedKey ImplReservedKeys[] =
            {
			    ImplReservedKey(KeyCode(KEY_F1,0),                  SV_SHORTCUT_HELP),
			    ImplReservedKey(KeyCode(KEY_F1,KEY_SHIFT),          SV_SHORTCUT_ACTIVEHELP),
    			ImplReservedKey(KeyCode(KEY_F1,KEY_MOD1),           SV_SHORTCUT_CONTEXTHELP),
    			ImplReservedKey(KeyCode(KEY_F2,KEY_SHIFT),          SV_SHORTCUT_CONTEXTHELP),
    			ImplReservedKey(KeyCode(KEY_F4,KEY_MOD1),           SV_SHORTCUT_DOCKUNDOCK),
    			ImplReservedKey(KeyCode(KEY_F4,KEY_MOD2),           SV_SHORTCUT_DOCKUNDOCK),
    			ImplReservedKey(KeyCode(KEY_F4,KEY_MOD1|KEY_MOD2),  SV_SHORTCUT_DOCKUNDOCK),
    			ImplReservedKey(KeyCode(KEY_F6,0),                  SV_SHORTCUT_NEXTSUBWINDOW),
    			ImplReservedKey(KeyCode(KEY_F6,KEY_MOD1),           SV_SHORTCUT_TODOCUMENT),
    			ImplReservedKey(KeyCode(KEY_F6,KEY_SHIFT),          SV_SHORTCUT_PREVSUBWINDOW),
    			ImplReservedKey(KeyCode(KEY_F6,KEY_MOD1|KEY_SHIFT), SV_SHORTCUT_SPLITTER),
    			ImplReservedKey(KeyCode(KEY_F10,0),                 SV_SHORTCUT_MENUBAR)
#ifdef UNX
                ,
                ImplReservedKey(KeyCode(KEY_1,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_2,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_3,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_4,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_5,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_6,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_7,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_8,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_9,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_0,KEY_SHIFT|KEY_MOD1), 0),
                ImplReservedKey(KeyCode(KEY_ADD,KEY_SHIFT|KEY_MOD1), 0)
#endif
            };
            static ReservedKeys aKeys
            (
                &ImplReservedKeys[0],
                sizeof(ImplReservedKeys) / sizeof(ImplReservedKey)
            );
            return &aKeys;
        }
    };

    struct ImplReservedKeys
        : public rtl::StaticAggregate<ReservedKeys, ImplReservedKeysImpl> {};
}


// #include <usr/refl.hxx>
class Reflection;



extern "C" {
    typedef UnoWrapperBase* (SAL_CALL *FN_TkCreateUnoWrapper)();
}

// =======================================================================

// --------------
// - ImplHotKey -
// --------------

struct ImplHotKey
{
    ImplHotKey*             mpNext;
    void*                   mpUserData;
    KeyCode                 maKeyCode;
    Link                    maLink;
};

// =======================================================================

// -----------------
// - ImplEventHook -
// -----------------

struct ImplEventHook
{
    ImplEventHook*          mpNext;
    void*                   mpUserData;
    VCLEventHookProc        mpProc;
};

// =======================================================================

// ---------------------
// - ImplPostEventData -
// ---------------------

struct ImplPostEventData
{
    sal_uLong			mnEvent;
    const Window*	mpWin;
	sal_uLong			mnEventId;
    KeyEvent    	maKeyEvent;
	MouseEvent		maMouseEvent;
	

   	ImplPostEventData( sal_uLong nEvent, const Window* pWin, const KeyEvent& rKeyEvent ) :
    	mnEvent( nEvent ), mpWin( pWin ), mnEventId( 0 ), maKeyEvent( rKeyEvent ) {}
   	ImplPostEventData( sal_uLong nEvent, const Window* pWin, const MouseEvent& rMouseEvent ) :
    	mnEvent( nEvent ), mpWin( pWin ), mnEventId( 0 ), maMouseEvent( rMouseEvent ) {}

	~ImplPostEventData() {}
};

typedef ::std::pair< Window*, ImplPostEventData* > ImplPostEventPair;

static ::std::list< ImplPostEventPair > aPostedEventList;

// =======================================================================

Application* GetpApp()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData )
        return NULL;
    return pSVData->mpApp;
}

// -----------------------------------------------------------------------

Application::Application()
{
    if( ! ImplGetSVData() )
        ImplInitSVData();
    ImplGetSVData()->mpApp = this;
    InitSalData();
}

// -----------------------------------------------------------------------

Application::~Application()
{
    ImplDeInitSVData();
    DeInitSalData();
    ImplGetSVData()->mpApp = NULL;
    ImplDestroySVData();
    GlobalDeInitTools();
}

// -----------------------------------------------------------------------

void Application::InitAppRes( const ResId& )
{
}

// -----------------------------------------------------------------------

sal_Bool Application::QueryExit()
{
    WorkWindow* pAppWin = ImplGetSVData()->maWinData.mpAppWin;

    // Aufruf des Close-Handlers des Applikationsfensters
    if ( pAppWin )
        return pAppWin->Close();
    else
        return sal_True;
}

// -----------------------------------------------------------------------

void Application::UserEvent( sal_uLong, void* )
{
}

// -----------------------------------------------------------------------

void Application::ShowStatusText( const XubString& )
{
}

// -----------------------------------------------------------------------

void Application::ShowHelpStatusText( const XubString& )
{
}

// -----------------------------------------------------------------------

void Application::ActivateExtHelp()
{
}

// -----------------------------------------------------------------------

void Application::DeactivateExtHelp()
{
}

// -----------------------------------------------------------------------

void Application::HideStatusText()
{
}

// -----------------------------------------------------------------------

void Application::HideHelpStatusText()
{
}

// -----------------------------------------------------------------------

void Application::FocusChanged()
{
}

// -----------------------------------------------------------------------

void Application::DataChanged( const DataChangedEvent& )
{
}

// -----------------------------------------------------------------------
void Application::Init()
{
}

// -----------------------------------------------------------------------

void Application::DeInit()
{
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetCommandLineParamCount()
{
    vos::OStartupInfo aStartInfo;
    return (sal_uInt16)aStartInfo.getCommandArgCount();
}

// -----------------------------------------------------------------------

XubString Application::GetCommandLineParam( sal_uInt16 nParam )
{
    vos::OStartupInfo   aStartInfo;
    rtl::OUString       aParam;
    aStartInfo.getCommandArg( nParam, aParam );
    return XubString( aParam );
}

// -----------------------------------------------------------------------

const XubString& Application::GetAppFileName()
{
    ImplSVData* pSVData = ImplGetSVData();
    DBG_ASSERT( pSVData->maAppData.mpAppFileName, "AppFileName vor SVMain ?!" );
    if ( pSVData->maAppData.mpAppFileName )
        return *pSVData->maAppData.mpAppFileName;

    /*
     *  #91147# provide a fallback for people without initialized
     *  vcl here (like setup in responsefile mode)
     */
    static String aAppFileName;
    if( !aAppFileName.Len() )
    {
        vos::OStartupInfo   aStartInfo;
        ::rtl::OUString		aExeFileName;

        aStartInfo.getExecutableFile( aExeFileName );

        // convert path to native file format
        rtl::OUString aNativeFileName;
        osl::FileBase::getSystemPathFromFileURL( aExeFileName, aNativeFileName );
        aAppFileName = aNativeFileName;
    }

    return aAppFileName;
}

// -----------------------------------------------------------------------

sal_uInt16 Application::Exception( sal_uInt16 nError )
{
    switch ( nError & EXC_MAJORTYPE )
    {
        // Bei System machen wir nichts und lassen dem System den
        // vortritt
        case EXC_SYSTEM:
            return 0;

        case EXC_DISPLAY:
        case EXC_REMOTE:
            return 0;

#ifdef DBG_UTIL
        case EXC_RSCNOTLOADED:
            Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "Resource not loaded" ) ) );
            break;
        case EXC_SYSOBJNOTCREATED:
            Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "System Object not created" ) ) );
            break;
        default:
            Abort( XubString( RTL_CONSTASCII_USTRINGPARAM( "Unknown Error" ) ) );
            break;
#else
        default:
            Abort( ImplGetSVEmptyStr() );
            break;
#endif
    }

    return 0;
}

// -----------------------------------------------------------------------

void Application::Abort( const XubString& rErrorText )
{
    SalAbort( rErrorText );
}

// -----------------------------------------------------------------------

sal_uLong   Application::GetReservedKeyCodeCount()
{
    return ImplReservedKeys::get()->second;
}

const KeyCode*  Application::GetReservedKeyCode( sal_uLong i )
{
    if( i >= GetReservedKeyCodeCount() )
        return NULL;
    else
        return &ImplReservedKeys::get()->first[i].mKeyCode;
}

String Application::GetReservedKeyCodeDescription( sal_uLong i )
{
    ResMgr* pResMgr = ImplGetResMgr();
    if( ! pResMgr )
        return String();
    ImplReservedKey *pImplReservedKeys = ImplReservedKeys::get()->first;
    if( i >= GetReservedKeyCodeCount() || ! pImplReservedKeys[i].mnResId )
        return String();
    else
        return String( ResId( pImplReservedKeys[i].mnResId, *pResMgr ) );
}

// -----------------------------------------------------------------------

void Application::Execute()
{
    DBG_STARTAPPEXECUTE();

    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mbInAppExecute = sal_True;

    while ( !pSVData->maAppData.mbAppQuit )
        Application::Yield();

    pSVData->maAppData.mbInAppExecute = sal_False;

    DBG_ENDAPPEXECUTE();
}

// -----------------------------------------------------------------------

inline void ImplYield( bool i_bWait, bool i_bAllEvents )
{
    ImplSVData* pSVData = ImplGetSVData();
    
    // run timers that have timed out
    if ( !pSVData->mbNoCallTimer )
        while ( pSVData->mbNotAllTimerCalled )
            Timer::ImplTimerCallbackProc();

    pSVData->maAppData.mnDispatchLevel++;
    // do not wait for events if application was already quit; in that
    // case only dispatch events already available
    // do not wait for events either if the app decided that it is too busy for timers
    // (feature added for the slideshow)
    pSVData->mpDefInst->Yield( i_bWait && !pSVData->maAppData.mbAppQuit && !pSVData->maAppData.mbNoYield, i_bAllEvents );
    pSVData->maAppData.mnDispatchLevel--;

    // flush lazy deleted objects
    if( pSVData->maAppData.mnDispatchLevel == 0 )
        vcl::LazyDelete::flush();
    
    // the system timer events will not necesseraly come in in non waiting mode
    // e.g. on aqua; need to trigger timer checks manually
    if( pSVData->maAppData.mbNoYield && !pSVData->mbNoCallTimer )
    {
        do
        {
            Timer::ImplTimerCallbackProc();
        }
        while( pSVData->mbNotAllTimerCalled );
    }

    // call post yield listeners
    if( pSVData->maAppData.mpPostYieldListeners )
        pSVData->maAppData.mpPostYieldListeners->callListeners( NULL );
}

// -----------------------------------------------------------------------

void Application::Reschedule( bool i_bAllEvents )
{
    ImplYield( false, i_bAllEvents );
}

// -----------------------------------------------------------------------

void Application::Yield( bool i_bAllEvents )
{
    ImplYield( true, i_bAllEvents );
}

// -----------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( ImplSVAppData, ImplQuitMsg, void*, EMPTYARG )
{
    ImplGetSVData()->maAppData.mbAppQuit = sal_True;
    return 0;
}

// -----------------------------------------------------------------------

void Application::Quit()
{
    Application::PostUserEvent( STATIC_LINK( NULL, ImplSVAppData, ImplQuitMsg ) );
}

// -----------------------------------------------------------------------

vos::IMutex& Application::GetSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    return *(pSVData->mpDefInst->GetYieldMutex());
}

// -----------------------------------------------------------------------

vos::OThread::TThreadIdentifier Application::GetMainThreadIdentifier()
{
    return ImplGetSVData()->mnMainThreadId;
}

// -----------------------------------------------------------------------

sal_uLong Application::ReleaseSolarMutex()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->mpDefInst->ReleaseYieldMutex();
}

// -----------------------------------------------------------------------

void Application::AcquireSolarMutex( sal_uLong nCount )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AcquireYieldMutex( nCount );
}

// -----------------------------------------------------------------------

sal_Bool Application::IsInMain()
{
    return ImplGetSVData()->maAppData.mbInAppMain;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsInExecute()
{
    return ImplGetSVData()->maAppData.mbInAppExecute;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsShutDown()
{
    return ImplGetSVData()->maAppData.mbAppQuit;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsInModalMode()
{
    return (ImplGetSVData()->maAppData.mnModalMode != 0);
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetModalModeCount()
{
    return ImplGetSVData()->maAppData.mnModalMode;
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetDispatchLevel()
{
    return ImplGetSVData()->maAppData.mnDispatchLevel;
}

// -----------------------------------------------------------------------

sal_Bool Application::AnyInput( sal_uInt16 nType )
{
    return (sal_Bool)ImplGetSVData()->mpDefInst->AnyInput( nType );
}

// -----------------------------------------------------------------------

sal_uLong Application::GetLastInputInterval()
{
    return (Time::GetSystemTicks()-ImplGetSVData()->maAppData.mnLastInputTime);
}

// -----------------------------------------------------------------------

extern int nImplSysDialog;

sal_Bool Application::IsUICaptured()
{
    ImplSVData* pSVData = ImplGetSVData();
    // Wenn Mouse gecaptured, oder im TrackingModus oder im Auswahlmodus
    // eines FloatingWindows (wie Menus, Aufklapp-ToolBoxen) soll kein
    // weiteres Fenster aufgezogen werden
    // D&D aktive !!!
    if ( pSVData->maWinData.mpCaptureWin || pSVData->maWinData.mpTrackWin ||
         pSVData->maWinData.mpFirstFloat || nImplSysDialog )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsUserActive( sal_uInt16 nTest )
{
    if ( nTest & (USERACTIVE_MOUSEDRAG | USERACTIVE_INPUT) )
    {
        if ( IsUICaptured() )
            return sal_True;
    }

    if ( nTest & USERACTIVE_INPUT )
    {
        if ( GetLastInputInterval() < 500 )
            return sal_True;

        if ( AnyInput( INPUT_KEYBOARD ) )
            return sal_True;
    }

    if ( nTest & USERACTIVE_MODALDIALOG )
    {
        if ( ImplGetSVData()->maAppData.mnModalDialog )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void Application::SystemSettingsChanging( AllSettings& /*rSettings*/,
                                          Window* /*pFrame*/ )
{
}

// -----------------------------------------------------------------------

void Application::MergeSystemSettings( AllSettings& rSettings )
{
    Window* pWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    if( ! pWindow )
        pWindow = ImplGetDefaultWindow();
    if( pWindow )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if ( !pSVData->maAppData.mbSettingsInit )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            pWindow->ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = sal_True;
        }
        // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
        pWindow->ImplUpdateGlobalSettings( rSettings, sal_False );
    }
}

// -----------------------------------------------------------------------

bool Application::ValidateSystemFont()
{
    Window* pWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    if( ! pWindow )
        pWindow = ImplGetDefaultWindow();

    if( pWindow )
    {
        AllSettings aSettings;
        pWindow->ImplGetFrame()->UpdateSettings( aSettings );
        return pWindow->ImplCheckUIFont( aSettings.GetStyleSettings().GetAppFont() );
    }
    return false;
}

// -----------------------------------------------------------------------

void Application::SetSettings( const AllSettings& rSettings )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mpSettings )
    {
		GetSettings();
        *pSVData->maAppData.mpSettings = rSettings;
        ResMgr::SetDefaultLocale( rSettings.GetUILocale() ); 
    }
    else
    {
        AllSettings aOldSettings = *pSVData->maAppData.mpSettings;
        if( aOldSettings.GetUILanguage() != rSettings.GetUILanguage() && pSVData->mpResMgr )
        {
            delete pSVData->mpResMgr;
            pSVData->mpResMgr = NULL;
        }
        ResMgr::SetDefaultLocale( rSettings.GetUILocale() ); 
        *pSVData->maAppData.mpSettings = rSettings;
        sal_uLong nChangeFlags = aOldSettings.GetChangeFlags( *pSVData->maAppData.mpSettings );
        if ( nChangeFlags )
        {
            DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
            GetpApp()->DataChanged( aDCEvt );

            // notify data change handler
            ImplCallEventListeners( VCLEVENT_APPLICATION_DATACHANGED, NULL, &aDCEvt);

            // Update all windows
            Window* pFirstFrame = pSVData->maWinData.mpFirstFrame;
            // Daten, die neu berechnet werden muessen, zuruecksetzen
            long nOldDPIX = 0;
            long nOldDPIY = 0;
            if ( pFirstFrame )
            {
                nOldDPIX = pFirstFrame->mnDPIX;
                nOldDPIY = pFirstFrame->mnDPIY;
                pSVData->maGDIData.mnAppFontX = 0;
            }
            Window* pFrame = pFirstFrame;
            while ( pFrame )
            {
                // AppFont-Cache-Daten zuruecksetzen
                pFrame->mpWindowImpl->mpFrameData->meMapUnit = MAP_PIXEL;

                // UpdateSettings am ClientWindow aufrufen, damit
                // die Daten nicht doppelt geupdatet werden
                Window* pClientWin = pFrame;
                while ( pClientWin->ImplGetClientWindow() )
                    pClientWin = pClientWin->ImplGetClientWindow();
                pClientWin->UpdateSettings( rSettings, sal_True );

                Window* pTempWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
                while ( pTempWin )
                {
                    // UpdateSettings am ClientWindow aufrufen, damit
                    // die Daten nicht doppelt geupdatet werden
                    pClientWin = pTempWin;
                    while ( pClientWin->ImplGetClientWindow() )
                        pClientWin = pClientWin->ImplGetClientWindow();
                    pClientWin->UpdateSettings( rSettings, sal_True );
                    pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
                }

                pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
            }

            // Wenn sich die DPI-Aufloesung fuer Screen-Ausgaben
            // geaendert hat, setzen wir auch bei allen
            // Screen-Kompatiblen VirDev's die neue Aufloesung
            pFirstFrame = pSVData->maWinData.mpFirstFrame;
            if ( pFirstFrame )
            {
                if ( (pFirstFrame->mnDPIX != nOldDPIX) ||
                     (pFirstFrame->mnDPIY != nOldDPIY) )
                {
                    VirtualDevice* pVirDev = pSVData->maGDIData.mpFirstVirDev;
                    while ( pVirDev )
                    {
                        if ( pVirDev->mbScreenComp &&
                             (pVirDev->mnDPIX == nOldDPIX) &&
                             (pVirDev->mnDPIY == nOldDPIY) )
                        {
                            pVirDev->mnDPIX = pFirstFrame->mnDPIX;
                            pVirDev->mnDPIY = pFirstFrame->mnDPIY;
                            if ( pVirDev->IsMapMode() )
                            {
                                MapMode aMapMode = pVirDev->GetMapMode();
                                pVirDev->SetMapMode();
                                pVirDev->SetMapMode( aMapMode );
                            }
                        }

                        pVirDev = pVirDev->mpNext;
                    }
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

const AllSettings& Application::GetSettings()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maAppData.mpSettings )
	{
		pSVData->maAppData.mpCfgListener = new LocaleConfigurationListener;
        pSVData->maAppData.mpSettings = new AllSettings();
		pSVData->maAppData.mpSettings->GetSysLocale().GetOptions().AddListener( pSVData->maAppData.mpCfgListener );
	}

    return *(pSVData->maAppData.mpSettings);
}

// -----------------------------------------------------------------------

void Application::NotifyAllWindows( DataChangedEvent& rDCEvt )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFrame = pSVData->maWinData.mpFirstFrame;
    while ( pFrame )
    {
        pFrame->NotifyAllChilds( rDCEvt );

        Window* pSysWin = pFrame->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            pSysWin->NotifyAllChilds( rDCEvt );
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }
}

// -----------------------------------------------------------------------

void Application::ImplCallEventListeners( sal_uLong nEvent, Window *pWin, void* pData )
{
    ImplSVData* pSVData = ImplGetSVData();
    VclWindowEvent aEvent( pWin, nEvent, pData );

    if ( pSVData->maAppData.mpEventListeners )
        if ( !pSVData->maAppData.mpEventListeners->empty() )
            pSVData->maAppData.mpEventListeners->Call( &aEvent );
}

// -----------------------------------------------------------------------

void Application::ImplCallEventListeners( VclSimpleEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpEventListeners )
        if ( !pSVData->maAppData.mpEventListeners->empty() )
            pSVData->maAppData.mpEventListeners->Call( pEvent );
}

// -----------------------------------------------------------------------

void Application::AddEventListener( const Link& rEventListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( !pSVData->maAppData.mpEventListeners )
        pSVData->maAppData.mpEventListeners = new VclEventListeners;
    pSVData->maAppData.mpEventListeners->push_back( rEventListener );
}

// -----------------------------------------------------------------------

void Application::RemoveEventListener( const Link& rEventListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maAppData.mpEventListeners )
        pSVData->maAppData.mpEventListeners->remove( rEventListener );
}

// -----------------------------------------------------------------------
void Application::AddKeyListener( const Link& rKeyListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( !pSVData->maAppData.mpKeyListeners )
        pSVData->maAppData.mpKeyListeners = new VclEventListeners;
    pSVData->maAppData.mpKeyListeners->push_back( rKeyListener );
}

// -----------------------------------------------------------------------

void Application::RemoveKeyListener( const Link& rKeyListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maAppData.mpKeyListeners )
        pSVData->maAppData.mpKeyListeners->remove( rKeyListener );
}

// -----------------------------------------------------------------------

sal_Bool Application::HandleKey( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent )
{
    // let listeners process the key event
    VclWindowEvent aEvent( pWin, nEvent, (void *) pKeyEvent );

    ImplSVData* pSVData = ImplGetSVData();
    sal_Bool bProcessed = sal_False;

    if ( pSVData->maAppData.mpKeyListeners )
        if ( !pSVData->maAppData.mpKeyListeners->empty() )
            bProcessed = pSVData->maAppData.mpKeyListeners->Process( &aEvent );

    return bProcessed;
}

// -----------------------------------------------------------------------------

sal_uLong Application::PostKeyEvent( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent )
{
	const ::vos::OGuard	aGuard( GetSolarMutex() );
	sal_uLong 				nEventId = 0;
	
    if( pWin && pKeyEvent )
    {
		ImplPostEventData* pPostEventData = new ImplPostEventData( nEvent, pWin, *pKeyEvent );
	
        PostUserEvent( nEventId,
					   STATIC_LINK( NULL, Application, PostEventHandler ),
                       pPostEventData );
	
		if( nEventId )
		{
			pPostEventData->mnEventId = nEventId;
			aPostedEventList.push_back( ImplPostEventPair( pWin, pPostEventData ) );
		}			
		else
			delete pPostEventData;
    }

	return nEventId;
}

// -----------------------------------------------------------------------------

sal_uLong Application::PostMouseEvent( sal_uLong nEvent, Window *pWin, MouseEvent* pMouseEvent )
{
	const ::vos::OGuard	aGuard( GetSolarMutex() );
	sal_uLong 				nEventId = 0;
	
    if( pWin && pMouseEvent )
    {
		Point aTransformedPos( pMouseEvent->GetPosPixel() );
		
		aTransformedPos.X() += pWin->mnOutOffX;
		aTransformedPos.Y() += pWin->mnOutOffY;
		
		const MouseEvent aTransformedEvent( aTransformedPos, pMouseEvent->GetClicks(), pMouseEvent->GetMode(),
											pMouseEvent->GetButtons(), pMouseEvent->GetModifier() );

		ImplPostEventData* pPostEventData = new ImplPostEventData( nEvent, pWin, aTransformedEvent );
											
        PostUserEvent( nEventId,
					   STATIC_LINK( NULL, Application, PostEventHandler ),
                       pPostEventData );
	
		if( nEventId )
		{
			pPostEventData->mnEventId = nEventId;
			aPostedEventList.push_back( ImplPostEventPair( pWin, pPostEventData ) );
		}	
		else
			delete pPostEventData;
    }

	return nEventId;
}

// -----------------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( Application, PostEventHandler, void*, pCallData )
{
	const ::vos::OGuard	aGuard( GetSolarMutex() );
    ImplPostEventData*	pData = static_cast< ImplPostEventData * >( pCallData );
	const void*			pEventData;
    sal_uLong               nEvent;
	const sal_uLong			nEventId = pData->mnEventId;
	
    switch( pData->mnEvent )
    {
        case VCLEVENT_WINDOW_MOUSEMOVE: 
			nEvent = SALEVENT_EXTERNALMOUSEMOVE;
			pEventData = &pData->maMouseEvent; 
		break;
        
		case VCLEVENT_WINDOW_MOUSEBUTTONDOWN: 
			nEvent = SALEVENT_EXTERNALMOUSEBUTTONDOWN;
			pEventData = &pData->maMouseEvent;
		break;
        
		case VCLEVENT_WINDOW_MOUSEBUTTONUP: 
			nEvent = SALEVENT_EXTERNALMOUSEBUTTONUP; 
			pEventData = &pData->maMouseEvent;
		break;

        case VCLEVENT_WINDOW_KEYINPUT: 
			nEvent = SALEVENT_EXTERNALKEYINPUT; 
			pEventData = &pData->maKeyEvent;
		break;
        
		case VCLEVENT_WINDOW_KEYUP: 
			nEvent = SALEVENT_EXTERNALKEYUP; 
			pEventData = &pData->maKeyEvent;
		break;

        default:
			nEvent = 0;
			pEventData = NULL;
		break;
    };

    if( pData->mpWin && pData->mpWin->mpWindowImpl->mpFrameWindow && pEventData )
		ImplWindowFrameProc( pData->mpWin->mpWindowImpl->mpFrameWindow, NULL, (sal_uInt16) nEvent, pEventData );
		
	// remove this event from list of posted events, watch for destruction of internal data
	::std::list< ImplPostEventPair >::iterator aIter( aPostedEventList.begin() );
	
	while( aIter != aPostedEventList.end() )
	{
		if( nEventId == (*aIter).second->mnEventId )
		{
			delete (*aIter).second;
			aIter = aPostedEventList.erase( aIter );
		}
		else
			++aIter;
	}
	
    return 0;
}

// -----------------------------------------------------------------------

void Application::RemoveMouseAndKeyEvents( Window* pWin )
{
	const ::vos::OGuard	aGuard( GetSolarMutex() );
	
	// remove all events for specific window, watch for destruction of internal data
	::std::list< ImplPostEventPair >::iterator aIter( aPostedEventList.begin() );
	
	while( aIter != aPostedEventList.end() )
	{
		if( pWin == (*aIter).first )
		{
			if( (*aIter).second->mnEventId )
				RemoveUserEvent( (*aIter).second->mnEventId );
			
			delete (*aIter).second;
			aIter = aPostedEventList.erase( aIter );
		}
		else
			++aIter;
	}
}

// -----------------------------------------------------------------------

sal_Bool Application::IsProcessedMouseOrKeyEvent( sal_uLong nEventId )
{
    const ::vos::OGuard aGuard( GetSolarMutex() );
    
    // find event
    ::std::list< ImplPostEventPair >::iterator aIter( aPostedEventList.begin() );
    
    while( aIter != aPostedEventList.end() )
    {
        if( (*aIter).second->mnEventId == nEventId )
            return sal_False;
            
        else
            ++aIter;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

sal_uLong Application::PostUserEvent( sal_uLong nEvent, void* pEventData )
{
    sal_uLong nEventId;
    PostUserEvent( nEventId, nEvent, pEventData );
    return nEventId;
}

// -----------------------------------------------------------------------

sal_uLong Application::PostUserEvent( const Link& rLink, void* pCaller )
{
    sal_uLong nEventId;
    PostUserEvent( nEventId, rLink, pCaller );
    return nEventId;
}

// -----------------------------------------------------------------------

sal_Bool Application::PostUserEvent( sal_uLong& rEventId, sal_uLong nEvent, void* pEventData )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = nEvent;
    pSVEvent->mpData    = pEventData;
    pSVEvent->mpLink    = NULL;
    pSVEvent->mpWindow  = NULL;
    pSVEvent->mbCall    = sal_True;
    rEventId = (sal_uLong)pSVEvent;
    Window* pDefWindow = ImplGetDefaultWindow();
    if ( pDefWindow && pDefWindow->ImplGetFrame()->PostEvent( pSVEvent ) )
        return sal_True;
    else
    {
        rEventId = 0;
        delete pSVEvent;
        return sal_False;
    }
}

// -----------------------------------------------------------------------

sal_Bool Application::PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mnEvent   = 0;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = NULL;
    pSVEvent->mbCall    = sal_True;
    rEventId = (sal_uLong)pSVEvent;
    Window* pDefWindow = ImplGetDefaultWindow();
    if ( pDefWindow && pDefWindow->ImplGetFrame()->PostEvent( pSVEvent ) )
        return sal_True;
    else
    {
        rEventId = 0;
        delete pSVEvent;
        return sal_False;
    }
}

// -----------------------------------------------------------------------

void Application::RemoveUserEvent( sal_uLong nUserEvent )
{
    if(nUserEvent)
	{
		ImplSVEvent* pSVEvent = (ImplSVEvent*)nUserEvent;

		DBG_ASSERT( !pSVEvent->mpWindow,
					"Application::RemoveUserEvent(): Event is send to a window" );
		DBG_ASSERT( pSVEvent->mbCall,
					"Application::RemoveUserEvent(): Event is already removed" );

		if ( pSVEvent->mpWindow )
		{
            if( ! pSVEvent->maDelData.IsDelete() )
                pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
			pSVEvent->mpWindow = NULL;
		}

		pSVEvent->mbCall = sal_False;
	}
}

// -----------------------------------------------------------------------

sal_Bool Application::InsertIdleHdl( const Link& rLink, sal_uInt16 nPrio )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
    if ( !pSVData->maAppData.mpIdleMgr )
        pSVData->maAppData.mpIdleMgr = new ImplIdleMgr;

    return pSVData->maAppData.mpIdleMgr->InsertIdleHdl( rLink, nPrio );
}

// -----------------------------------------------------------------------

void Application::RemoveIdleHdl( const Link& rLink )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpIdleMgr )
        pSVData->maAppData.mpIdleMgr->RemoveIdleHdl( rLink );
}

// -----------------------------------------------------------------------

void Application::EnableNoYieldMode( bool i_bNoYield )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mbNoYield = i_bNoYield;
}

// -----------------------------------------------------------------------

void Application::AddPostYieldListener( const Link& i_rListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->maAppData.mpPostYieldListeners )
        pSVData->maAppData.mpPostYieldListeners = new VclEventListeners2();
    pSVData->maAppData.mpPostYieldListeners->addListener( i_rListener );
}

// -----------------------------------------------------------------------

void Application::RemovePostYieldListener( const Link& i_rListener )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maAppData.mpPostYieldListeners )
        pSVData->maAppData.mpPostYieldListeners->removeListener( i_rListener );
}

// -----------------------------------------------------------------------

WorkWindow* Application::GetAppWindow()
{
    return ImplGetSVData()->maWinData.mpAppWin;
}

// -----------------------------------------------------------------------

Window* Application::GetFocusWindow()
{
    return ImplGetSVData()->maWinData.mpFocusWin;
}

// -----------------------------------------------------------------------

OutputDevice* Application::GetDefaultDevice()
{
    return ImplGetDefaultWindow();
}

// -----------------------------------------------------------------------

Window* Application::GetFirstTopLevelWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    return pSVData->maWinData.mpFirstFrame;
}

// -----------------------------------------------------------------------

Window* Application::GetNextTopLevelWindow( Window* pWindow )
{
    return pWindow->mpWindowImpl->mpFrameData->mpNextFrame;
}

// -----------------------------------------------------------------------

long    Application::GetTopWindowCount()
{
    long nRet = 0;
    ImplSVData* pSVData = ImplGetSVData();
    Window *pWin = pSVData ? pSVData->maWinData.mpFirstFrame : NULL;
    while( pWin )
    {
        if( pWin->ImplGetWindow()->IsTopWindow() )
            nRet++;
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return nRet;
}

// -----------------------------------------------------------------------

Window* Application::GetTopWindow( long nIndex )
{
    long nIdx = 0;
    ImplSVData* pSVData = ImplGetSVData();
    Window *pWin = pSVData ? pSVData->maWinData.mpFirstFrame : NULL;
    while( pWin )
    {
        if( pWin->ImplGetWindow()->IsTopWindow() )
        {
            if( nIdx == nIndex )
                return pWin->ImplGetWindow();
            else
                nIdx++;
        }
        pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return NULL;
}

// -----------------------------------------------------------------------

Window* Application::GetActiveTopWindow()
{
    Window *pWin = ImplGetSVData()->maWinData.mpFocusWin;
    while( pWin )
    {
        if( pWin->IsTopWindow() )
            return pWin;
        pWin = pWin->mpWindowImpl->mpParent;
    }
    return NULL;
}

// -----------------------------------------------------------------------

void Application::SetAppName( const XubString& rUniqueName )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
    if ( !pSVData->maAppData.mpAppName )
        pSVData->maAppData.mpAppName = new XubString( rUniqueName );
    else
        *(pSVData->maAppData.mpAppName) = rUniqueName;
}

// -----------------------------------------------------------------------

XubString Application::GetAppName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpAppName )
        return *(pSVData->maAppData.mpAppName);
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void Application::SetDisplayName( const UniString& rName )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Falls er noch nicht existiert, dann anlegen
    if ( !pSVData->maAppData.mpDisplayName )
        pSVData->maAppData.mpDisplayName = new UniString( rName );
    else
        *(pSVData->maAppData.mpDisplayName) = rName;
}

// -----------------------------------------------------------------------

UniString Application::GetDisplayName()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maAppData.mpDisplayName )
        return *(pSVData->maAppData.mpDisplayName);
    else if ( pSVData->maWinData.mpAppWin )
        return pSVData->maWinData.mpAppWin->GetText();
    else
        return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

unsigned int Application::GetScreenCount()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayScreenCount() : 0;
}

rtl::OUString Application::GetScreenName( unsigned int nScreen )
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetScreenName( nScreen ) : rtl::OUString();
}

bool Application::IsMultiDisplay()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->IsMultiDisplay() : false;
}

unsigned int Application::GetDefaultDisplayNumber()
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDefaultDisplayNumber() : 0;
}

Rectangle Application::GetScreenPosSizePixel( unsigned int nScreen )
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayScreenPosSizePixel( nScreen ) : Rectangle();
}

Rectangle Application::GetWorkAreaPosSizePixel( unsigned int nScreen )
{
    SalSystem* pSys = ImplGetSalSystem();
    return pSys ? pSys->GetDisplayWorkAreaPosSizePixel( nScreen ) : Rectangle();
}

namespace {
unsigned long calcDistSquare( const Point& i_rPoint, const Rectangle& i_rRect )
{
    const Point aRectCenter( (i_rRect.Left() + i_rRect.Right())/2,
                       (i_rRect.Top() + i_rRect.Bottom())/ 2 );
    const long nDX = aRectCenter.X() - i_rPoint.X();
    const long nDY = aRectCenter.Y() - i_rPoint.Y();
    return nDX*nDX + nDY*nDY;
}
}

unsigned int Application::GetBestScreen( const Rectangle& i_rRect )
{
    if( IsMultiDisplay() )
        return GetDefaultDisplayNumber();
    
    const unsigned int nScreens = GetScreenCount();    
    unsigned int nBestMatchScreen = 0;
    unsigned long nOverlap = 0;
    for( unsigned int i = 0; i < nScreens; i++ )
    {
        const Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        // if a screen contains the rectangle completely it is obviously the best screen
        if( aCurScreenRect.IsInside( i_rRect ) )
            return i;
        // next the screen which contains most of the area of the rect is the best
        Rectangle aIntersection( aCurScreenRect.GetIntersection( i_rRect ) );
        if( ! aIntersection.IsEmpty() )
        {
            const unsigned long nCurOverlap( aIntersection.GetWidth() * aIntersection.GetHeight() );
            if( nCurOverlap > nOverlap )
            {
                nOverlap = nCurOverlap;
                nBestMatchScreen = i;
            }
        }
    }
    if( nOverlap > 0 )
        return nBestMatchScreen;
    
    // finally the screen which center is nearest to the rect is the best
    const Point aCenter( (i_rRect.Left() + i_rRect.Right())/2,
                         (i_rRect.Top() + i_rRect.Bottom())/2 );
    unsigned long nDist = ULONG_MAX;
    for( unsigned int i = 0; i < nScreens; i++ )
    {
        const Rectangle aCurScreenRect( GetScreenPosSizePixel( i ) );
        const unsigned long nCurDist( calcDistSquare( aCenter, aCurScreenRect ) );
        if( nCurDist < nDist )
        {
            nBestMatchScreen = i;
            nDist = nCurDist;
        }
    }
    return nBestMatchScreen;
}

// -----------------------------------------------------------------------

sal_Bool Application::InsertAccel( Accelerator* pAccel )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr = new ImplAccelManager();
    return pSVData->maAppData.mpAccelMgr->InsertAccel( pAccel );
}

// -----------------------------------------------------------------------

void Application::RemoveAccel( Accelerator* pAccel )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr->RemoveAccel( pAccel );
}

// -----------------------------------------------------------------------

void Application::FlushAccel()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpAccelMgr )
        pSVData->maAppData.mpAccelMgr->FlushAccel();
}

// -----------------------------------------------------------------------

sal_Bool Application::CallAccel( const KeyCode& rKeyCode, sal_uInt16 nRepeat )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maAppData.mpAccelMgr )
    {
        if ( pSVData->maAppData.mpAccelMgr->IsAccelKey( rKeyCode, nRepeat ) )
            return sal_True;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void Application::SetHelp( Help* pHelp )
{
    ImplGetSVData()->maAppData.mpHelp = pHelp;
}

// -----------------------------------------------------------------------

Help* Application::GetHelp()
{
    return ImplGetSVData()->maAppData.mpHelp;
}

// -----------------------------------------------------------------------

void Application::EnableAutoHelpId( sal_Bool bEnabled )
{
    ImplGetSVData()->maHelpData.mbAutoHelpId = bEnabled;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsAutoHelpIdEnabled()
{
    return ImplGetSVData()->maHelpData.mbAutoHelpId;
}

// -----------------------------------------------------------------------

void Application::EnableAutoMnemonic( sal_Bool bEnabled )
{
    AllSettings aSettings = GetSettings();
    StyleSettings aStyle = aSettings.GetStyleSettings();
    aStyle.SetAutoMnemonic( bEnabled );
    aSettings.SetStyleSettings( aStyle );
    SetSettings( aSettings );
}

// -----------------------------------------------------------------------

sal_Bool Application::IsAutoMnemonicEnabled()
{
    return GetSettings().GetStyleSettings().GetAutoMnemonic();
}

// -----------------------------------------------------------------------

void Application::SetDialogScaleX( short nScale )
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->maAppData.mnDialogScaleX = nScale;
    pSVData->maGDIData.mnAppFontX = pSVData->maGDIData.mnRealAppFontX;
    if ( nScale )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*nScale)/100;
}

// -----------------------------------------------------------------------

short Application::GetDialogScaleX()
{
    return ImplGetSVData()->maAppData.mnDialogScaleX;
}

// -----------------------------------------------------------------------

void Application::SetDefDialogParent( Window* pWindow )
{
    ImplGetSVData()->maWinData.mpDefDialogParent = pWindow;
}

// -----------------------------------------------------------------------

Window* Application::GetDefDialogParent()
{
    ImplSVData* pSVData = ImplGetSVData();
    // #103442# find some useful dialog parent if there
    // was no default set
    // NOTE: currently even the default is not used
    if( sal_False && pSVData->maWinData.mpDefDialogParent != NULL )
        return pSVData->maWinData.mpDefDialogParent;
    else
    {
        // always use the topmost parent of the candidate
        // window to avoid using dialogs or floaters
        // as DefDialogParent

        // current focus frame
        Window *pWin = NULL;
        if( (pWin = pSVData->maWinData.mpFocusWin) != NULL )
        {
            while( pWin->mpWindowImpl && pWin->mpWindowImpl->mpParent )
                pWin = pWin->mpWindowImpl->mpParent;

            if( (pWin->mpWindowImpl->mnStyle & WB_INTROWIN) == 0 )
            {
                // check for corrupted window hierarchy, #122232#, may be we now crash somewhere else
                if( !pWin->mpWindowImpl )
                {
                    DBG_ERROR( "Window hierarchy corrupted!" );
                    pSVData->maWinData.mpFocusWin = NULL;   // avoid further access
                    return NULL;       
                }
    
                // MAV: before the implementation has used only decorated windows,
                //      but it is not true in case of ActiveX or plugin scenario,
                //      so this check is commented out 
                // if( pWin->mpWindowImpl->mpFrameWindow->GetStyle() & (WB_MOVEABLE | WB_SIZEABLE) )
                    return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
                // else
                //    return NULL;
            }
        }
        // last active application frame
        if( NULL != (pWin = pSVData->maWinData.mpActiveApplicationFrame) )
        {
            return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
        }
        else
        {
            // first visible top window (may be totally wrong....)
            pWin = pSVData->maWinData.mpFirstFrame;
            while( pWin )
            {
                if( pWin->ImplGetWindow()->IsTopWindow() &&
                    pWin->mpWindowImpl->mbReallyVisible &&
                    (pWin->mpWindowImpl->mnStyle & WB_INTROWIN) == 0
                )
                {
                    while( pWin->mpWindowImpl->mpParent )
                        pWin = pWin->mpWindowImpl->mpParent;
                    return pWin->mpWindowImpl->mpFrameWindow->ImplGetWindow();
                }
                pWin = pWin->mpWindowImpl->mpFrameData->mpNextFrame;
            }
            // use the desktop
            return NULL;
        }
    }
}

// -----------------------------------------------------------------------

void Application::EnableDialogCancel( sal_Bool bDialogCancel )
{
    ImplGetSVData()->maAppData.mbDialogCancel = bDialogCancel;
}

// -----------------------------------------------------------------------

sal_Bool Application::IsDialogCancelEnabled()
{
    return ImplGetSVData()->maAppData.mbDialogCancel;
}

// -----------------------------------------------------------------------

void Application::SetSystemWindowMode( sal_uInt16 nMode )
{
    ImplGetSVData()->maAppData.mnSysWinMode = nMode;
}

// -----------------------------------------------------------------------

sal_uInt16 Application::GetSystemWindowMode()
{
    return ImplGetSVData()->maAppData.mnSysWinMode;
}

// -----------------------------------------------------------------------

const String& Application::GetFontPath()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( !pSVData->maAppData.mpFontPath )
    {
        if( const char* pFontPath = ::getenv( "SAL_FONTPATH_PRIVATE" ) )
            pSVData->maAppData.mpFontPath = new String( String::CreateFromAscii( pFontPath ) );
    }

    if( pSVData->maAppData.mpFontPath )
        return *(pSVData->maAppData.mpFontPath);
    return ImplGetSVEmptyStr();
}

// -----------------------------------------------------------------------

void Application::SetFontPath( const String& rPath )
{
    ImplSVData* pSVData = ImplGetSVData();

    // if it doesn't exist create a new one
    if( !pSVData->maAppData.mpFontPath )
        pSVData->maAppData.mpFontPath = new String( rPath );
    else
        *(pSVData->maAppData.mpFontPath) = rPath;
}

// -----------------------------------------------------------------------

UniqueItemId Application::CreateUniqueId()
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( !pSVData->maAppData.mpUniqueIdCont )
        pSVData->maAppData.mpUniqueIdCont = new UniqueIdContainer( UNIQUEID_SV_BEGIN );
    return pSVData->maAppData.mpUniqueIdCont->CreateId();
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > Application::GetVCLToolkit()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > xT;
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( sal_True );
    if ( pWrapper )
        xT = pWrapper->GetVCLToolkit();
    return xT;
}

// -----------------------------------------------------------------------

extern "C" { static void SAL_CALL thisModule() {} }

UnoWrapperBase* Application::GetUnoWrapper( sal_Bool bCreateIfNotExist )
{
    ImplSVData* pSVData = ImplGetSVData();
    static sal_Bool bAlreadyTriedToCreate = sal_False;
    if ( !pSVData->mpUnoWrapper && bCreateIfNotExist && !bAlreadyTriedToCreate )
    {
        ::rtl::OUString aLibName = ::vcl::unohelper::CreateLibraryName( "ootk", sal_True );
        oslModule hTkLib = osl_loadModuleRelative(
            &thisModule, aLibName.pData, SAL_LOADMODULE_DEFAULT );
        if ( hTkLib )
        {
            ::rtl::OUString aFunctionName( RTL_CONSTASCII_USTRINGPARAM( "CreateUnoWrapper" ) );
            FN_TkCreateUnoWrapper fnCreateWrapper = (FN_TkCreateUnoWrapper)osl_getFunctionSymbol( hTkLib, aFunctionName.pData );
            if ( fnCreateWrapper )
            {
                pSVData->mpUnoWrapper = fnCreateWrapper();
            }
        }
        DBG_ASSERT( pSVData->mpUnoWrapper, "UnoWrapper could not be created!" );
        bAlreadyTriedToCreate = sal_True;
    }
    return pSVData->mpUnoWrapper;
}

// -----------------------------------------------------------------------

void Application::SetUnoWrapper( UnoWrapperBase* pWrapper )
{
    ImplSVData* pSVData = ImplGetSVData();
    DBG_ASSERT( !pSVData->mpUnoWrapper, "SetUnoWrapper: Wrapper already exists" );
    pSVData->mpUnoWrapper = pWrapper;
}

// -----------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayConnection > Application::GetDisplayConnection()
{
    ImplSVData* pSVData = ImplGetSVData();

    if( !pSVData->mxDisplayConnection.is() )
    {
        pSVData->mxDisplayConnection.set( new ::vcl::DisplayConnection );
        pSVData->mxDisplayConnection->start();
    }

    return pSVData->mxDisplayConnection.get();
}

// -----------------------------------------------------------------------

void Application::SetFilterHdl( const Link& rLink )
{
    ImplGetSVData()->maGDIData.mpGrfConverter->SetFilterHdl( rLink );
}

// -----------------------------------------------------------------------

const Link& Application::GetFilterHdl()
{
    return ImplGetSVData()->maGDIData.mpGrfConverter->GetFilterHdl();
}

// -----------------------------------------------------------------------

sal_Bool ImplCallHotKey( const KeyCode& rKeyCode )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        if ( pHotKeyData->maKeyCode.IsDefinedKeyCodeEqual( rKeyCode ) )
        {
            pHotKeyData->maLink.Call( pHotKeyData->mpUserData );
            return sal_True;
        }

        pHotKeyData = pHotKeyData->mpNext;
    }

    return sal_False;
}

// -----------------------------------------------------------------------

void ImplFreeHotKeyData()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pTempHotKeyData;
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        pTempHotKeyData = pHotKeyData->mpNext;
        delete pHotKeyData;
        pHotKeyData = pTempHotKeyData;
    }

    pSVData->maAppData.mpFirstHotKey = NULL;
}

// -----------------------------------------------------------------------

sal_uIntPtr Application::AddHotKey( const KeyCode& rKeyCode, const Link& rLink, void* pData )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pHotKeyData = new ImplHotKey;
    pHotKeyData->mpUserData = pData;
    pHotKeyData->maKeyCode  = rKeyCode;
    pHotKeyData->maLink     = rLink;
    pHotKeyData->mpNext     = pSVData->maAppData.mpFirstHotKey;
    pSVData->maAppData.mpFirstHotKey = pHotKeyData;
    return (sal_uIntPtr)pHotKeyData;
}

// -----------------------------------------------------------------------

void Application::RemoveHotKey( sal_uIntPtr nId )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplHotKey*     pFindHotKeyData = (ImplHotKey*)nId;
    ImplHotKey*     pPrevHotKeyData = NULL;
    ImplHotKey*     pHotKeyData = pSVData->maAppData.mpFirstHotKey;
    while ( pHotKeyData )
    {
        if ( pHotKeyData == pFindHotKeyData )
        {
            if ( pPrevHotKeyData )
                pPrevHotKeyData->mpNext = pFindHotKeyData->mpNext;
            else
                pSVData->maAppData.mpFirstHotKey = pFindHotKeyData->mpNext;
            delete pFindHotKeyData;
            break;
        }

        pPrevHotKeyData = pHotKeyData;
        pHotKeyData = pHotKeyData->mpNext;
    }

    DBG_ASSERT( pHotKeyData, "Application::RemoveHotKey() - HotKey is not added" );
}

// -----------------------------------------------------------------------

void ImplFreeEventHookData()
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplEventHook*  pTempEventHookData;
    ImplEventHook*  pEventHookData = pSVData->maAppData.mpFirstEventHook;
    while ( pEventHookData )
    {
        pTempEventHookData = pEventHookData->mpNext;
        delete pEventHookData;
        pEventHookData = pTempEventHookData;
    }

    pSVData->maAppData.mpFirstEventHook = NULL;
}

// -----------------------------------------------------------------------

sal_uIntPtr Application::AddEventHook( VCLEventHookProc pProc, void* pData )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplEventHook*  pEventHookData = new ImplEventHook;
    pEventHookData->mpUserData = pData;
    pEventHookData->mpProc     = pProc;
    pEventHookData->mpNext     = pSVData->maAppData.mpFirstEventHook;
    pSVData->maAppData.mpFirstEventHook = pEventHookData;
    return (sal_uIntPtr)pEventHookData;
}

// -----------------------------------------------------------------------

void Application::RemoveEventHook( sal_uIntPtr nId )
{
    ImplSVData*     pSVData = ImplGetSVData();
    ImplEventHook*  pFindEventHookData = (ImplEventHook*)nId;
    ImplEventHook*  pPrevEventHookData = NULL;
    ImplEventHook*  pEventHookData = pSVData->maAppData.mpFirstEventHook;
    while ( pEventHookData )
    {
        if ( pEventHookData == pFindEventHookData )
        {
            if ( pPrevEventHookData )
                pPrevEventHookData->mpNext = pFindEventHookData->mpNext;
            else
                pSVData->maAppData.mpFirstEventHook = pFindEventHookData->mpNext;
            delete pFindEventHookData;
            break;
        }

        pPrevEventHookData = pEventHookData;
        pEventHookData = pEventHookData->mpNext;
    }

    DBG_ASSERT( pEventHookData, "Application::RemoveEventHook() - EventHook is not added" );
}

// -----------------------------------------------------------------------

long Application::CallEventHooks( NotifyEvent& rEvt )
{
    ImplSVData*     pSVData = ImplGetSVData();
    long            nRet = 0;
    ImplEventHook*  pTempEventHookData;
    ImplEventHook*  pEventHookData = pSVData->maAppData.mpFirstEventHook;
    while ( pEventHookData )
    {
        pTempEventHookData = pEventHookData->mpNext;
        nRet = pEventHookData->mpProc( rEvt, pEventHookData->mpUserData );
        if ( nRet )
            break;
        pEventHookData = pTempEventHookData;
    }

    return nRet;
}

// -----------------------------------------------------------------------

long Application::CallPreNotify( NotifyEvent& rEvt )
{
    return ImplCallPreNotify( rEvt );
}

// -----------------------------------------------------------------------

long Application::CallEvent( NotifyEvent& rEvt )
{
    return ImplCallEvent( rEvt );
}

// -----------------------------------------------------------------------

const LocaleDataWrapper& Application::GetAppLocaleDataWrapper()
{
    return GetSettings().GetLocaleDataWrapper();
}

// -----------------------------------------------------------------------

void Application::EnableHeadlessMode( sal_Bool bEnable )
{
    EnableDialogCancel( bEnable );
}

// -----------------------------------------------------------------------

sal_Bool Application::IsHeadlessModeEnabled()
{
    return IsDialogCancelEnabled();
}


void Application::EnableConversionMode( bool bEnableConv )
{
    ImplGetSVData()->maAppData.mbConversionMode = bEnableConv;
}

// -----------------------------------------------------------------------

bool Application::IsConversionModeEnabled()
{
    return ImplGetSVData()->maAppData.mbConversionMode;
}

// -----------------------------------------------------------------------

void Application::ShowNativeErrorBox(const String& sTitle  ,
                                     const String& sMessage)
{
    int btn = ImplGetSalSystem()->ShowNativeMessageBox (
            sTitle,
            sMessage,
            SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK,
            SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK);
    if (btn != SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK) {
        OSL_TRACE("ShowNativeMessageBox returned %d\n", btn);
    }
}

// -----------------------------------------------------------------------

bool Application::CanToggleImeStatusWindow()
{
    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpImeStatus )
        pSVData->mpImeStatus  = pSVData->mpDefInst->CreateI18NImeStatus();
    return pSVData->mpImeStatus->canToggle();
}

void Application::ShowImeStatusWindow(bool bShow)
{
    ImplGetSVData()->maAppData.meShowImeStatusWindow = bShow
        ? ImplSVAppData::ImeStatusWindowMode_SHOW
        : ImplSVAppData::ImeStatusWindowMode_HIDE;

    ImplSVData* pSVData = ImplGetSVData();
    if( ! pSVData->mpImeStatus )
        pSVData->mpImeStatus  = pSVData->mpDefInst->CreateI18NImeStatus();
    pSVData->mpImeStatus->toggle();
}

bool Application::GetShowImeStatusWindowDefault()
{
    rtl_TextEncodingInfo aInfo;
    aInfo.StructSize = sizeof aInfo;
    return rtl_getTextEncodingInfo(osl_getThreadTextEncoding(), &aInfo)
        && aInfo.MaximumCharSize > 1;
}

const ::rtl::OUString& Application::GetDesktopEnvironment()
{
    return SalGetDesktopEnvironment();
}

void Application::AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType)
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mpDefInst->AddToRecentDocumentList(rFileUrl, rMimeType);
}

sal_Bool Application::IsAccessibilityEnabled()
{
#ifdef WNT
	return IsWNTInitAccessBridge();
#else 
    return sal_False;
#endif 
}

sal_Bool InitAccessBridge( sal_Bool bShowCancel, sal_Bool &rCancelled )
{
    sal_Bool bRet = sal_True;

	rCancelled = sal_False;

// Disable Java bridge on UNIX and OS/2
#ifndef WNT
    (void) bShowCancel; // unsued
#else
	// Checking HasAtHook() was introduced with IBM's IA2 CWS.
	if( HasAtHook() )
	{
	    bRet = ImplInitAccessBridge( bShowCancel, rCancelled );
	    
	    if( !bRet && bShowCancel && !rCancelled )
	    {
	        // disable accessibility if the user chooses to continue
	        AllSettings aSettings = Application::GetSettings();
	        MiscSettings aMisc = aSettings.GetMiscSettings();
	        aMisc.SetEnableATToolSupport( sal_False );
	        aSettings.SetMiscSettings( aMisc );
	        Application::SetSettings( aSettings );
	    }
	}
	else
	{
		bRet = false;
	}
#endif // !UNX

    return bRet;
}

#ifdef WNT
sal_Bool HasAtHook()
{
	sal_Int32 bIsRuning=0;
	// BOOL WINAPI SystemParametersInfo(
	//	  __in     UINT uiAction,
	//	  __in     UINT uiParam,
	//	  __inout  PVOID pvParam,
	//	  __in     UINT fWinIni
	//	);
	// pvParam must be BOOL (defined in MFC as int)
	// End
	if(SystemParametersInfo(SPI_GETSCREENREADER,0,&bIsRuning,0))
	{
		if( bIsRuning )
			return sal_True;
	}
	return sal_False;
}
#endif
// MT: AppProperty, AppEvent was in oldsv.cxx, but is still needed...
// ------------------------------------------------------------------------

TYPEINIT0(ApplicationProperty)

// ------------------------------------------------------------------------

static PropertyHandler* pHandler=NULL;

void Application::Property( ApplicationProperty& rProp )
{
    if ( pHandler )
        pHandler->Property( rProp );
}

void Application::SetPropertyHandler( PropertyHandler* p )
{
    if ( pHandler )
        delete pHandler;
    pHandler = p;
}
bool Application::EnableAccessInterface(bool bEnable)
{
#ifdef WNT
	return WNTEnableAccessInterface(bEnable);
#else 
    bEnable = true; // avoid compiler warning
    return true;
#endif 
}
bool Application::IsEnableAccessInterface()
{
	return ImplGetSVData()->maAppData.m_bEnableAccessInterface;
}



void Application::AppEvent( const ApplicationEvent& /*rAppEvent*/ )
{
}
