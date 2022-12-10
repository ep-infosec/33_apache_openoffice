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



#ifndef _SV_SVAPP_HXX
#define _SV_SVAPP_HXX

#ifndef _VOS_THREAD_HXX
#include <vos/thread.hxx>
#endif
#include <tools/string.hxx>
#include <tools/link.hxx>
#include <tools/unqid.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/apptypes.hxx>
#ifndef _VCL_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#include <vcl/vclevent.hxx>
class Link;
class AllSettings;
class DataChangedEvent;
class Accelerator;
class Help;
class OutputDevice;
class Window;
class WorkWindow;
class MenuBar;
class UnoWrapperBase;
class Reflection;
class KeyCode;
class NotifyEvent;
class KeyEvent;
class MouseEvent;

namespace vos { class IMutex; }
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/connection/XConnection.hpp>

namespace com {
namespace sun {
namespace star {
namespace lang {
    class XMultiServiceFactory;
}
namespace awt {
    class XToolkit;
    class XDisplayConnection;
}
} } }

// helper needed by SalLayout implementations as well as svx/source/dialog/svxbmpnumbalueset.cxx
VCL_DLLPUBLIC sal_UCS4 GetMirroredChar( sal_UCS4 );

// --------------------
// - SystemWindowMode -
// --------------------

#define SYSTEMWINDOW_MODE_NOAUTOMODE    ((sal_uInt16)0x0001)
#define SYSTEMWINDOW_MODE_DIALOG        ((sal_uInt16)0x0002)

// -------------
// - EventHook -
// -------------

typedef long (*VCLEventHookProc)( NotifyEvent& rEvt, void* pData );

// --------------------
// - ApplicationEvent -
// --------------------

// Erstmal wieder eingebaut, damit AppEvents auf dem MAC funktionieren
#ifdef UNX
// enum Doppelt in daemon.cxx unter unix Achtung !!!
enum Service { SERVICE_OLE, SERVICE_APPEVENT, SERVICE_IPC };
#endif

class VCL_DLLPUBLIC ApplicationAddress
{
friend class Application;
protected:
    UniString           aHostName;
    UniString           aDisplayName;
    UniString           aDomainName;
    int                 nPID;

public:
                        ApplicationAddress();
                        ApplicationAddress( const UniString& rDomain );
                        ApplicationAddress( const UniString& rHost,
                                            const UniString& rDisp,
                                            const UniString& rDomain );
                        ApplicationAddress( const UniString& rHost, int nPID );

    const UniString&    GetHost() const     { return aHostName; }
    const UniString&    GetDisplay() const  { return aDisplayName; }
    const UniString&    GetDomain() const   { return aDomainName; }
    int                 GetPID() const      { return nPID; }

    sal_Bool                IsConnectToSame( const ApplicationAddress& rAdr ) const;
};

inline ApplicationAddress::ApplicationAddress()
{
    nPID = 0;
}

inline ApplicationAddress::ApplicationAddress( const UniString& rDomain )
{
    aDomainName     = rDomain;
    nPID            = 0;
}

inline ApplicationAddress::ApplicationAddress( const UniString& rHost,
                                               const UniString& rDisp,
                                               const UniString& rDomain )
{
    aHostName       = rHost;
    aDisplayName    = rDisp;
    aDomainName     = rDomain;
    nPID            = 0;
}

inline ApplicationAddress::ApplicationAddress( const UniString& rHost, int nPIDPar )
{
    aHostName       = rHost;
    nPID            = nPIDPar;
}

inline sal_Bool ApplicationAddress::IsConnectToSame( const ApplicationAddress& rAdr ) const
{
    if ( nPID && ((nPID == rAdr.nPID) && (aHostName.Equals( rAdr.aHostName))) )
        return sal_True;
    else
        return sal_False;
}

#define APPEVENT_PARAM_DELIMITER    	'\n'

#define APPEVENT_OPEN_STRING        	"Open"
#define APPEVENT_PRINT_STRING       	"Print"
#define APPEVENT_DISKINSERT_STRING  	"DiskInsert"
#define APPEVENT_SAVEDOCUMENTS_STRING	"SaveDocuments"

class VCL_DLLPUBLIC ApplicationEvent
{
private:
    UniString           aSenderAppName; // Absender Applikationsname
    ByteString          aEvent;         // Event
    UniString           aData;          // Uebertragene Daten
    ApplicationAddress  aAppAddr;       // Absender Addresse

public:
                        ApplicationEvent() {}
                        ApplicationEvent( const UniString& rSenderAppName,
                                          const ApplicationAddress& rAppAddr,
                                          const ByteString& rEvent,
                                          const UniString& rData );

    const UniString&    GetSenderAppName() const { return aSenderAppName; }
    const ByteString&   GetEvent() const { return aEvent; }
    const UniString&    GetData() const { return aData; }
    const ApplicationAddress& GetAppAddress() const { return aAppAddr; }

    sal_Bool                IsOpenEvent() const;
    sal_Bool                IsPrintEvent() const;
    sal_Bool                IsDiskInsertEvent() const;

    sal_uInt16              GetParamCount() const { return aData.GetTokenCount( APPEVENT_PARAM_DELIMITER ); }
    UniString           GetParam( sal_uInt16 nParam ) const { return aData.GetToken( nParam, APPEVENT_PARAM_DELIMITER ); }
};

inline ApplicationEvent::ApplicationEvent( const UniString& rSenderAppName,
                                           const ApplicationAddress& rAppAddr,
                                           const ByteString& rEvent,
                                           const UniString& rData ) :
    aSenderAppName( rSenderAppName ),
    aEvent( rEvent ),
    aData( rData ),
    aAppAddr( rAppAddr )
{
}

inline sal_Bool ApplicationEvent::IsOpenEvent() const
{
    if ( aEvent.Equals( APPEVENT_OPEN_STRING ))
        return sal_True;
    else
        return sal_False;
}

inline sal_Bool ApplicationEvent::IsPrintEvent() const
{
    if ( aEvent.Equals( APPEVENT_PRINT_STRING ))
        return sal_True;
    else
        return sal_False;
}

inline sal_Bool ApplicationEvent::IsDiskInsertEvent() const
{
    if ( aEvent.Equals( APPEVENT_DISKINSERT_STRING ))
        return sal_True;
    else
        return sal_False;
}

class VCL_DLLPUBLIC PropertyHandler
{
public:
    virtual void                Property( ApplicationProperty& ) = 0;
};

// ---------------
// - Application -
// ---------------

class VCL_DLLPUBLIC Application
{
public:
                                Application();
    virtual                     ~Application();

    virtual void                Main() = 0;

    virtual sal_Bool            QueryExit();

    virtual void                UserEvent( sal_uLong nEvent, void* pEventData );

    virtual void                ActivateExtHelp();
    virtual void                DeactivateExtHelp();

    virtual void                ShowStatusText( const XubString& rText );
    virtual void                HideStatusText();

    virtual void                ShowHelpStatusText( const XubString& rText );
    virtual void                HideHelpStatusText();

    virtual void                FocusChanged();
    virtual void                DataChanged( const DataChangedEvent& rDCEvt );

    virtual void                Init();
    virtual void                DeInit();

    static void                 InitAppRes( const ResId& rResId );

    static sal_uInt16           GetCommandLineParamCount();
    static XubString            GetCommandLineParam( sal_uInt16 nParam );
    static const XubString&     GetAppFileName();

    virtual sal_uInt16          Exception( sal_uInt16 nError );
    static void                 Abort( const XubString& rErrorText );

    static void                 Execute();
    static void                 Quit();
    static void                 Reschedule( bool bAllEvents = false );
    static void                 Yield( bool bAllEvents = false );
    static void                 EndYield();
    static vos::IMutex&                     GetSolarMutex();
    static vos::OThread::TThreadIdentifier  GetMainThreadIdentifier();
    static sal_uLong            ReleaseSolarMutex();
    static void                 AcquireSolarMutex( sal_uLong nCount );
    static void                 EnableNoYieldMode( bool i_bNoYield );
    static void                 AddPostYieldListener( const Link& i_rListener );
    static void                 RemovePostYieldListener( const Link& i_rListener );

    static sal_Bool             IsInMain();
    static sal_Bool             IsInExecute();
    static sal_Bool             IsShutDown();
    static sal_Bool             IsInModalMode();
    static sal_uInt16           GetModalModeCount();

    static sal_uInt16           GetDispatchLevel();
    static sal_Bool             AnyInput( sal_uInt16 nType = INPUT_ANY );
    static sal_uLong            GetLastInputInterval();
    static sal_Bool             IsUICaptured();
    static sal_Bool             IsUserActive( sal_uInt16 nTest = USERACTIVE_ALL );

    virtual void                SystemSettingsChanging( AllSettings& rSettings,
                                                        Window* pFrame );
    static void                 MergeSystemSettings( AllSettings& rSettings );
    /** validate that the currently selected system UI font is suitable
        to display the application's UI.

        A localized test string will be checked if it can be displayed
        in the currently selected system UI font. If no glyphs are
        missing it can be assumed that the font is proper for display
        of the application's UI.

        @returns
        <TRUE/> if the system font is suitable for our UI
        <FALSE/> if the test string could not be displayed with the system font
     */
    static bool					ValidateSystemFont();

    static void                 SetSettings( const AllSettings& rSettings );
    static const AllSettings&   GetSettings();
    static void                 NotifyAllWindows( DataChangedEvent& rDCEvt );

    static void                 AddEventListener( const Link& rEventListener );
    static void                 RemoveEventListener( const Link& rEventListener );
    static void                 AddKeyListener( const Link& rKeyListener );
    static void                 RemoveKeyListener( const Link& rKeyListener );
    static void                 ImplCallEventListeners( sal_uLong nEvent, Window* pWin, void* pData );
    static void                 ImplCallEventListeners( VclSimpleEvent* pEvent );
    static sal_Bool             HandleKey( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );
    
	static sal_uLong            PostKeyEvent( sal_uLong nEvent, Window *pWin, KeyEvent* pKeyEvent );
    static sal_uLong            PostMouseEvent( sal_uLong nEvent, Window *pWin, MouseEvent* pMouseEvent );
	static void					RemoveMouseAndKeyEvents( Window *pWin );
    static sal_Bool				IsProcessedMouseOrKeyEvent( sal_uLong nEventId );

    static sal_uLong            PostUserEvent( sal_uLong nEvent, void* pEventData = NULL );
    static sal_uLong            PostUserEvent( const Link& rLink, void* pCaller = NULL );
    static sal_Bool             PostUserEvent( sal_uLong& rEventId, sal_uLong nEvent, void* pEventData = NULL );
    static sal_Bool             PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller = NULL );
    static void                 RemoveUserEvent( sal_uLong nUserEvent );

    static sal_Bool             InsertIdleHdl( const Link& rLink, sal_uInt16 nPriority );
    static void                 RemoveIdleHdl( const Link& rLink );

    virtual void                AppEvent( const ApplicationEvent& rAppEvent );

    virtual void                Property( ApplicationProperty& );
    void                        SetPropertyHandler( PropertyHandler* pHandler );

#ifndef NO_GETAPPWINDOW
    static WorkWindow*          GetAppWindow();
#endif

	static Window*              GetFocusWindow();
    static OutputDevice*        GetDefaultDevice();

    static Window*              GetFirstTopLevelWindow();
    static Window*              GetNextTopLevelWindow( Window* pWindow );

    static long                 GetTopWindowCount();
    static Window*              GetTopWindow( long nIndex );
    static Window*              GetActiveTopWindow();

    static void                 SetAppName( const String& rUniqueName );
    static String               GetAppName();

    static void                 SetDisplayName( const UniString& rDisplayName );
    static UniString            GetDisplayName();
    
    static unsigned int         GetScreenCount();
    // IsMultiDisplay returns:
    //        true:  different screens are separate and windows cannot be moved
    //               between them (e.g. Xserver with multiple screens)
    //        false: screens form up one large display area
    //               windows can be moved between single screens
    //               (e.g. Xserver with Xinerama, Windows)
    static bool                 IsMultiDisplay();
    static Rectangle            GetScreenPosSizePixel( unsigned int nScreen );
    static Rectangle            GetWorkAreaPosSizePixel( unsigned int nScreen );
	static rtl::OUString		GetScreenName( unsigned int nScreen );
    static unsigned int         GetDefaultDisplayNumber();
    // if IsMultiDisplay() == false the return value will be
    // nearest screen of the target rectangle
    // in case of IsMultiDisplay() == true the return value
    // will always be GetDefaultDisplayNumber()
    static unsigned int         GetBestScreen( const Rectangle& );

	static const LocaleDataWrapper& GetAppLocaleDataWrapper();

    static sal_Bool             InsertAccel( Accelerator* pAccel );
    static void                 RemoveAccel( Accelerator* pAccel );
    static void                 FlushAccel();
    static sal_Bool             CallAccel( const KeyCode& rKeyCode, sal_uInt16 nRepeat = 0 );

    static sal_uLong            AddHotKey( const KeyCode& rKeyCode, const Link& rLink, void* pData = NULL );
    static void                 RemoveHotKey( sal_uLong nId );
    static sal_uLong            AddEventHook( VCLEventHookProc pProc, void* pData = NULL );
    static void                 RemoveEventHook( sal_uLong nId );
    static long                 CallEventHooks( NotifyEvent& rEvt );
    static long                 CallPreNotify( NotifyEvent& rEvt );
    static long                 CallEvent( NotifyEvent& rEvt );

    static void                 SetHelp( Help* pHelp = NULL );
    static Help*                GetHelp();

    static void                 EnableAutoHelpId( sal_Bool bEnabled = sal_True );
    static sal_Bool             IsAutoHelpIdEnabled();

    static void                 EnableAutoMnemonic( sal_Bool bEnabled = sal_True );
    static sal_Bool             IsAutoMnemonicEnabled();

    static sal_uLong            GetReservedKeyCodeCount();
    static const KeyCode*       GetReservedKeyCode( sal_uLong i );
    static String               GetReservedKeyCodeDescription( sal_uLong i );

    static void                 SetDefDialogParent( Window* pWindow );
    static Window*              GetDefDialogParent();

    static void                 EnableDialogCancel( sal_Bool bDialogCancel = sal_True );
    static sal_Bool             IsDialogCancelEnabled();

    static void                 SetSystemWindowMode( sal_uInt16 nMode );
    static sal_uInt16           GetSystemWindowMode();

    static void                 SetDialogScaleX( short nScale );
    static short                GetDialogScaleX();

    static void                 SetFontPath( const String& rPath );
    static const String&        GetFontPath();

    static UniqueItemId         CreateUniqueId();

    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDisplayConnection > GetDisplayConnection();

	// The global service manager has to be created before!
    static ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > GetVCLToolkit();
    static UnoWrapperBase*      GetUnoWrapper( sal_Bool bCreateIfNotExists = sal_True );
    static void                 SetUnoWrapper( UnoWrapperBase* pWrapper );

    static void                 SetFilterHdl( const Link& rLink );
    static const Link&          GetFilterHdl();
    
    static sal_Bool             IsAccessibilityEnabled();

    static void                 EnableHeadlessMode( sal_Bool bEnable = sal_True );
    static sal_Bool             IsHeadlessModeEnabled();

    static void                 EnableConversionMode( bool bEnableConv = true );
    static bool                 IsConversionModeEnabled();

    
    static void                 ShowNativeErrorBox(const String& sTitle  ,
                                                   const String& sMessage);
	static bool					EnableAccessInterface(bool bEnable);
	static bool					IsEnableAccessInterface();

    // IME Status Window Control:

    /** Return true if any IME status window can be toggled on and off
        externally.

        Must only be called with the Solar mutex locked.
     */
    static bool CanToggleImeStatusWindow();

    /** Toggle any IME status window on and off.

        This only works if CanToggleImeStatusWinodw returns true (otherwise,
        any calls of this method are ignored).

        Must only be called with the Solar mutex locked.
     */
    static void ShowImeStatusWindow(bool bShow);

    /** Return true if any IME status window should be turned on by default
        (this decision can be locale dependent, for example).

        Can be called without the Solar mutex locked.
     */
    static bool GetShowImeStatusWindowDefault();
    
    /** Returns a string representing the desktop environment 
        the process is currently running in.
     */
    static const ::rtl::OUString& GetDesktopEnvironment();

    /** Add a file to the system shells recent document list if there is any.
          This function may have no effect under Unix because there is no 
          standard API among the different desktop managers.
          
          @param rFileUrl
                    The file url of the document.
                    
          @param rMimeType
          The mime content type of the document specified by aFileUrl.
          If an empty string will be provided "application/octet-stream"
          will be used.
    */
    static void AddToRecentDocumentList(const rtl::OUString& rFileUrl, const rtl::OUString& rMimeType);

private:

    DECL_STATIC_LINK( Application, PostEventHandler, void* );
};

VCL_DLLPUBLIC Application* GetpApp();

VCL_DLLPUBLIC sal_Bool InitVCL( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );
VCL_DLLPUBLIC void DeInitVCL();

VCL_DLLPUBLIC sal_Bool InitAccessBridge( sal_Bool bAllowCancel, sal_Bool &rCancelled );

// only allowed to call, if no thread is running. You must call JoinMainLoopThread to free all memory.
VCL_DLLPUBLIC void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData );
VCL_DLLPUBLIC void JoinMainLoopThread();

inline void Application::EndYield()
{
    PostUserEvent( Link() );
}

#ifdef WNT
VCL_DLLPUBLIC sal_Bool HasAtHook();
VCL_DLLPUBLIC bool IsWNTInitAccessBridge();
bool WNTEnableAccessInterface(bool bEnable);

class VCL_DLLPUBLIC CEnableAccessInterface
{
public:
	CEnableAccessInterface(bool bEnable = false)
	{
		m_bIsEnableAccessInterface = Application::IsEnableAccessInterface();
		Application::EnableAccessInterface(bEnable);
	}
	~CEnableAccessInterface()
	{
		Application::EnableAccessInterface(m_bIsEnableAccessInterface);
	}
private:
	bool m_bIsEnableAccessInterface;
};
#endif
#endif // _APP_HXX
