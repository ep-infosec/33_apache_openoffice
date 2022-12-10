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



#ifndef _VCL_GTKFRAME_HXX
#define _VCL_GTKFRAME_HXX

#include <tools/prex.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <tools/postx.h>

#include <salframe.hxx>
#include <vcl/sysdata.hxx>

#include "tools/link.hxx"

#include <list>
#include <vector>

class GtkSalGraphics;
class GtkSalDisplay;

class GtkSalFrame : public SalFrame
{
    static const int nMaxGraphics = 2;

    struct GraphicsHolder
    {
        GtkSalGraphics*     pGraphics;
        bool                bInUse;
        GraphicsHolder()
                : pGraphics( NULL ),
                  bInUse( false )
        {}
        ~GraphicsHolder();
    };
    
    struct IMHandler
    {
        //--------------------------------------------------------
        // Not all GTK Input Methods swallow key release
        // events.  Since they swallow the key press events and we
        // are left with the key release events, we need to
        // manually swallow those.  To do this, we keep a list of
        // the previous 10 key press events in each GtkSalFrame
        // and when we get a key release that matches one of the
        // key press events in our list, we swallow it.
        struct PreviousKeyPress
        {
            GdkWindow *window;
            gint8   send_event;
            guint32 time;
            guint   state;
            guint   keyval;
            guint16 hardware_keycode;
            guint8  group;
    
            PreviousKeyPress (GdkEventKey *event)
            :   window (NULL),
                send_event (0),
                time (0),
                state (0),
                keyval (0),
                hardware_keycode (0),
                group (0)
            {
                if (event)
                {
                    window              = event->window;
                    send_event          = event->send_event;
                    time                = event->time;
                    state               = event->state;
                    keyval              = event->keyval;
                    hardware_keycode    = event->hardware_keycode;
                    group               = event->group;
                }
            }
    
            PreviousKeyPress( const PreviousKeyPress& rPrev )
            :   window( rPrev.window ),
                send_event( rPrev.send_event ),
                time( rPrev.time ),
                state( rPrev.state ),
                keyval( rPrev.keyval ),
                hardware_keycode( rPrev.hardware_keycode ),
                group( rPrev.group )
            {}
    
            bool operator== (GdkEventKey *event) const
            {
                return (event != NULL)
                    && (event->window == window)
                    && (event->send_event == send_event)
                    && (event->state == state)
                    && (event->keyval == keyval)
                    && (event->hardware_keycode == hardware_keycode)
                    && (event->group == group)
                    && (event->time - time < 3)
                    ;
            }
        };
        
        
        GtkSalFrame*                    m_pFrame;
        std::list< PreviousKeyPress >   m_aPrevKeyPresses;
        int                             m_nPrevKeyPresses; // avoid using size()
        GtkIMContext*                   m_pIMContext;
        bool                            m_bFocused;
        bool                            m_bPreeditJustChanged;
        SalExtTextInputEvent            m_aInputEvent;
        std::vector< sal_uInt16 >           m_aInputFlags;
        
        IMHandler( GtkSalFrame* );
        ~IMHandler();

        void            createIMContext();
        void            deleteIMContext();
        void            updateIMSpotLocation();
        void            setInputContext( SalInputContext* pContext );
        void            endExtTextInput( sal_uInt16 nFlags );
        bool            handleKeyEvent( GdkEventKey* pEvent );
        void            focusChanged( bool bFocusIn );
        
        void            doCallEndExtTextInput();
        void            sendEmptyCommit();
        
        
        static void			signalIMCommit( GtkIMContext*, gchar*, gpointer );
        static gboolean		signalIMDeleteSurrounding( GtkIMContext*, gint, gint, gpointer );
        static void			signalIMPreeditChanged( GtkIMContext*, gpointer );
        static void			signalIMPreeditEnd( GtkIMContext*, gpointer );
        static void			signalIMPreeditStart( GtkIMContext*, gpointer );
        static gboolean		signalIMRetrieveSurrounding( GtkIMContext*, gpointer );
    };
    friend struct IMHandler;

    int                             m_nScreen;
    GtkWidget*                      m_pWindow;
    GdkWindow*                      m_pForeignParent;
    GdkNativeWindow                 m_aForeignParentWindow;
    GdkWindow*                      m_pForeignTopLevel;
    GdkNativeWindow                 m_aForeignTopLevelWindow;
    Pixmap                          m_hBackgroundPixmap;
    sal_uLong                     m_nStyle;
    SalExtStyle                     m_nExtStyle;
    GtkFixed*                       m_pFixedContainer;
    GtkSalFrame*                    m_pParent;
    std::list< GtkSalFrame* >       m_aChildren;
    GdkWindowState                  m_nState;
    SystemEnvData                   m_aSystemData;
    GraphicsHolder                  m_aGraphics[ nMaxGraphics ];
    sal_uInt16                          m_nKeyModifiers;
    GdkCursor                      *m_pCurrentCursor;
    GdkVisibilityState              m_nVisibility;
    PointerStyle                    m_ePointerStyle;
    int                             m_nSavedScreenSaverTimeout;
    guint                           m_nGSMCookie;
    int                             m_nWorkArea;
    bool                            m_bFullscreen;
    bool                            m_bSingleAltPress;
    bool                            m_bDefaultPos;
    bool                            m_bDefaultSize;
    bool                            m_bSendModChangeOnRelease;
    bool                            m_bWindowIsGtkPlug;
    bool                            m_bSetFocusOnMap;
    String                          m_aTitle;
    
    IMHandler*                      m_pIMHandler;
    
    Size                            m_aMaxSize;
    Size                            m_aMinSize;
    Rectangle                       m_aRestorePosSize;    
    
    GdkRegion*			            m_pRegion;

    void Init( SalFrame* pParent, sal_uLong nStyle );
    void Init( SystemParentData* pSysData );
    void InitCommon();

    // signals
    static gboolean		signalButton( GtkWidget*, GdkEventButton*, gpointer );
    static void			signalStyleSet( GtkWidget*, GtkStyle* pPrevious, gpointer );
    static gboolean		signalExpose( GtkWidget*, GdkEventExpose*, gpointer );
    static gboolean		signalFocus( GtkWidget*, GdkEventFocus*, gpointer );
    static gboolean		signalMap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean		signalUnmap( GtkWidget*, GdkEvent*, gpointer );
    static gboolean		signalConfigure( GtkWidget*, GdkEventConfigure*, gpointer );
    static gboolean		signalMotion( GtkWidget*, GdkEventMotion*, gpointer );
    static gboolean		signalKey( GtkWidget*, GdkEventKey*, gpointer );
    static gboolean		signalDelete( GtkWidget*, GdkEvent*, gpointer );
    static gboolean		signalState( GtkWidget*, GdkEvent*, gpointer );
    static gboolean		signalScroll( GtkWidget*, GdkEvent*, gpointer );
    static gboolean		signalCrossing( GtkWidget*, GdkEventCrossing*, gpointer );
    static gboolean		signalVisibility( GtkWidget*, GdkEventVisibility*, gpointer );
    static void			signalDestroy( GtkObject*, gpointer );

    void			Center();
    void			SetDefaultSize();
	void			setAutoLock( bool bLock );
	void			setScreenSaverTimeout( int nTimeout );

    void            doKeyCallback( guint state,
                                   guint keyval,
                                   guint16 hardware_keycode,
                                   guint8 group,
                                   guint32 time,
                                   sal_Unicode aOrigCode,
                                   bool bDown,
                                   bool bSendRelease
                                   );


    GdkNativeWindow findTopLevelSystemWindow( GdkNativeWindow aWindow );

    static int m_nFloats;

    bool isFloatGrabWindow() const
    {
        return
            (m_nStyle & SAL_FRAME_STYLE_FLOAT) &&                // only a float can be floatgrab
            !(m_nStyle & SAL_FRAME_STYLE_TOOLTIP) &&             // tool tips are not
            !(m_nStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) && // toolbars are also not
            !(m_nStyle & SAL_FRAME_STYLE_FLOAT_FOCUSABLE);       // focusable floats are not
    }
    
    bool isChild( bool bPlug = true, bool bSysChild = true )
    {
        sal_uLong nMask = 0;
        if( bPlug )
            nMask |= SAL_FRAME_STYLE_PLUG;
        if( bSysChild )
            nMask |= SAL_FRAME_STYLE_SYSTEMCHILD;
        return (m_nStyle & nMask) != 0;
    }
    
    void resizeWindow( long nWidth, long nHeight );
    void moveWindow( long nX, long nY );

    Size calcDefaultSize();

    void setMinMaxSize();
    void createNewWindow( XLIB_Window aParent, bool bXEmbed, int nScreen );
    void askForXEmbedFocus( sal_Int32 nTimecode );
    
    DECL_LINK( ImplDelayedFullScreenHdl, void* );
public:
    GtkSalFrame( SalFrame* pParent, sal_uLong nStyle );
    GtkSalFrame( SystemParentData* pSysData );

    // dispatches an event, returns true if dispatched
    // and false else; if true was returned the event should
    // be swallowed
    bool Dispatch( const XEvent* pEvent );
    void grabPointer( sal_Bool bGrab, sal_Bool bOwnerEvents = sal_False );

	GtkSalDisplay*	getDisplay();
	GdkDisplay*		getGdkDisplay();
    GtkWidget*	getWindow() const { return m_pWindow; }
    GtkFixed*	getFixedContainer() const { return m_pFixedContainer; }
    GdkWindow*	getForeignParent() const { return m_pForeignParent; }
    GdkNativeWindow	getForeignParentWindow() const { return m_aForeignParentWindow; }
    GdkWindow*	getForeignTopLevel() const { return m_pForeignTopLevel; }
    GdkNativeWindow	getForeignTopLevelWindow() const { return m_aForeignTopLevelWindow; }
    GdkVisibilityState getVisibilityState() const
    { return m_nVisibility; }
    Pixmap getBackgroundPixmap() const { return m_hBackgroundPixmap; }
    int getScreenNumber() const { return m_nScreen; }
    void updateScreenNumber();
    
    void moveToScreen( int nScreen );

    virtual ~GtkSalFrame();

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual sal_Bool                PostEvent( void* pData );

    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( sal_uInt16 nIcon );
    virtual void                SetMenu( SalMenu *pSalMenu );
    virtual void                DrawMenuBar();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = sal_False );
    virtual void                Enable( sal_Bool bEnable );
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void                SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual sal_Bool                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay );
    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void                StartPresentation( sal_Bool bStart );
    // Show Window over all other Windows
    virtual void                SetAlwaysOnTop( sal_Bool bOnTop );

    // Window to top and grab focus
    virtual void                ToTop( sal_uInt16 nFlags );

    // this function can call with the same
    // pointer style
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( sal_Bool bMouse );
    virtual void                SetPointerPos( long nX, long nY );

    // flush output buffer
    using SalFrame::Flush;
    virtual void                Flush();
    // flush output buffer, wait till outstanding operations are done
    virtual void                Sync();

    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( sal_uInt16 nFlags );

    virtual String              GetKeyName( sal_uInt16 nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, sal_uInt16 nKeyCode );
    virtual sal_Bool            MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType        GetInputLanguage();

    virtual SalBitmap*          SnapShot();

    virtual void                UpdateSettings( AllSettings& rSettings );

    virtual void                Beep( SoundType eSoundType );

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*    GetSystemData() const;


    // get current modifier and button mask
    virtual SalPointerState     GetPointerState();

    // set new parent window
    virtual void                SetParent( SalFrame* pNewParent );
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window internally
    // return false to indicate failure
    virtual bool                SetPluginParent( SystemParentData* pNewParent );

    virtual void                SetBackgroundBitmap( SalBitmap* );

    virtual void                SetScreenNumber( unsigned int );

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
	virtual void					ResetClipRegion();
    // start setting the clipregion consisting of nRects rectangles
	virtual void					BeginSetClipRegion( sal_uLong nRects );
    // add a rectangle to the clip region
	virtual void					UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // done setting up the clipregion
	virtual void					EndSetClipRegion();

	static GtkSalFrame         *getFromWindow( GtkWindow *pWindow );
};


#define OOO_TYPE_FIXED ooo_fixed_get_type()

extern "C" {

GType ooo_fixed_get_type( void );

} // extern "C"

#endif //_VCL_GTKFRAME_HXX
