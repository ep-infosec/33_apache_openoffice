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



#ifndef _TASKBAR_HXX
#define _TASKBAR_HXX

#include "svtools/svtdllapi.h"
#include <tools/time.hxx>
#ifndef _TOOLS_LIST_HXX
#include <tools/list.hxx>
#endif
#include <vcl/timer.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/status.hxx>

class TaskBar;
class TaskStatusFieldItem;
class ImplTaskItemList;
class ImplTaskSBItemList;
class ImplTaskBarFloat;
struct ImplTaskSBFldItem;

// -----------------
// - Dokumentation -
// -----------------

/*

TaskToolBox
===========

StartUpdateTask()/UpdateTask()/EndUpdateTask()
Diese muessen gerufen werden, wenn die Task upgedatet werden muessen.
Dann muss StartUpdateTask() gerufen werden, dann UpdateTask() fuer alle
Task's und danach EndUpdateTask() wo dann die TaskButtons entsprechend
neu angeordnet werden.

ActivateTask()
Handler der gerufen wird, wenn ein Task aktiviert werden muss. Mit
GetTaskItem() kann abgefragt werden, welcher Task aktiviert werden muss.

ContextMenu()
Dieser Handler wird gerufen, wenn ein ContextMenu angezeigt werden soll.
Mit GetTaskMode() kann abgefragt werden, ob fuer einen Task oder ein
Item.

GetTaskItem()
Diese Methode liefert das Item zurueck, welches bei UpdateTask an der
entsprechenden Position eingefuegt wurde.

GetContextMenuPos()
Liefert die Position zurueck, wo das Contextmenu angezeigt werden soll.


TaskStatusBar
=============

InsertStatusField()/RemoveStatusField()
Fuegt ein Statusfeld ein, wo die aktuelle Uhrzeit angezeigt wird. In
dieses Feld koennen dann mit AddStatusFielItem(), ModifyStatusFielItem()
und RemoveStatusFielItem() Status-Items eingefuegt werden. Bei diesen
muss man ein Image angeben, welches dann angezeigt wird. Ausserdem kann
man bei diesen noch Hilfe-Texte angeben oder sagen, ob sie blinken
sollen und ein Notify-Object, worueber man informiert wird, wenn ein
Kontextmenu angezeigt wird oder das Item angeklickt wird. Am
TaskStatusBar kann auch ein Notify-Object gesetzt werden, wenn man
benachrichtigt werden will, wenn die Uhrzeit oder die TaskStatusBar
angeklickt wird. Wenn der Notify fuer die Uhrzeit kommt, ist die
Id TASKSTATUSBAR_CLOCKID, wenn er fuer die TaskStatusBar kommt, ist
die Id 0. Mit SetFieldFlags() kann am TaskStatusBar auch die Flags
hinterher umgesetzt werden, um zum Beispiel die Uhrzeit ein- und
auszuschalten.


TaskBar
=======

Erlaubte StyleBits
------------------

WB_BORDER       - Border an der oberen Kante
WB_SIZEABLE     - Zwischen TaskToolBox und TaskStatusBar kann der Anwender
                  die Groesse aendern.

Wenn WB_SIZEABLE gesetzt ist, kann die Breite des StatusBars gesetzt und
abgefragt werden. Dazu kann man SetStatusSize()/GetStatusSize() aufrufen.
0 steht dabei fuer optimale Groesse, was auch der Default ist. Bei einem
Doppelklick auf den Trenner kann der Anwender auch wieder die optimale
Groesse einstellen.

Wichtige Methoden
------------------

virtual TaskToolBox* TaskBar::CreateButtonBar();
virtual TaskToolBox* TaskBar::CreateTaskToolBox();
virtual TaskStatusBar* TaskBar::CreateTaskStatusBar();

Diese Methoden muesste man ueberladen, wenn man eine eigene Klasse anlegen
will.

void TaskBar::ShowStatusText( const String& rText );
void TaskBar::HideStatusText();

Blendet den ButtonBar und die TaskBar ein bzw. aus um den Hilfetexte in der
gesammten Zeile anzuzeigen.
*/

// -----------------
// - TaskButtonBar -
// -----------------

class TaskButtonBar : public ToolBox
{
    friend class TaskBar;

private:
    TaskBar*            mpNotifyTaskBar;
    void*               mpDummy1;
    void*               mpDummy2;
    void*               mpDummy3;
    void*               mpDummy4;

public:
                        TaskButtonBar( Window* pParent, WinBits nWinStyle = 0 );
                        ~TaskButtonBar();

    virtual void        RequestHelp( const HelpEvent& rHEvt );

    void                InsertButton( sal_uInt16 nItemId,
                                      const Image& rImage, const String& rText,
                                      sal_uInt16 nPos = TOOLBOX_APPEND )
                            { InsertItem( nItemId, rImage, rText, TIB_LEFT | TIB_AUTOSIZE, nPos ); }
    void                RemoveButton( sal_uInt16 nItemId )
                            { RemoveItem( nItemId ); }
};

// ---------------------
// - TaskToolBox-Types -
// ---------------------

#define TASKTOOLBOX_TASK_NOTFOUND       ((sal_uInt16)0xFFFF)

// ---------------
// - TaskToolBox -
// ---------------

class SVT_DLLPUBLIC TaskToolBox : public ToolBox
{
    friend class TaskBar;

private:
    ImplTaskItemList*   mpItemList;
    TaskBar*            mpNotifyTaskBar;
    Point               maContextMenuPos;
    sal_uLong               mnOldItemCount;
    long                mnMaxTextWidth;
    long                mnDummy1;
    sal_uInt16              mnUpdatePos;
    sal_uInt16              mnUpdateNewPos;
    sal_uInt16              mnActiveItemId;
    sal_uInt16              mnNewActivePos;
    sal_uInt16              mnTaskItem;
    sal_uInt16              mnSmallItem;
    sal_uInt16              mnDummy2;
    sal_Bool                mbMinActivate;
    sal_Bool                mbDummy1;
    Link                maActivateTaskHdl;
    Link                maContextMenuHdl;

#ifdef _TASKBAR_CXX
    SVT_DLLPRIVATE void                ImplFormatTaskToolBox();
#endif

	// Forbidden and not implemented.
	TaskToolBox (const TaskToolBox &);
	TaskToolBox & operator= (const TaskToolBox &);

public:
                        TaskToolBox( Window* pParent, WinBits nWinStyle = 0 );
                        ~TaskToolBox();

    void                ActivateTaskItem( sal_uInt16 nItemId,
                                          sal_Bool bMinActivate = sal_False );
    sal_uInt16              GetTaskItem( const Point& rPos ) const;

    virtual void        ActivateTask();
    virtual void        ContextMenu();

    virtual void        Select();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        Resize();
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        RequestHelp( const HelpEvent& rHEvt );

    void                StartUpdateTask();
    void                UpdateTask( const Image& rImage, const String& rText,
                                    sal_Bool bActive = sal_False );
    void                EndUpdateTask();

    const Point&        GetContextMenuPos() const { return maContextMenuPos; }
    sal_uInt16              GetTaskItem() const { return mnTaskItem; }
    sal_Bool                IsMinActivate() const { return mbMinActivate; }

    void                SetActivateTaskHdl( const Link& rLink ) { maActivateTaskHdl = rLink; }
    const Link&         GetActivateTaskHdl() const { return maActivateTaskHdl; }
    void                SetContextMenuHdl( const Link& rLink ) { maContextMenuHdl = rLink; }
    const Link&         GetContextMenuHdl() const { return maContextMenuHdl; }
};

inline sal_uInt16 TaskToolBox::GetTaskItem( const Point& rPos ) const
{
    sal_uInt16 nId = GetItemId( rPos );
    if ( nId )
        return nId-1;
    else
        return TASKTOOLBOX_TASK_NOTFOUND;
}

// ---------------------
// - ITaskStatusNotify -
// ---------------------

class ITaskStatusNotify
{
public:
    virtual sal_Bool        MouseButtonDown( sal_uInt16 nItemd, const MouseEvent& rMEvt );
    virtual sal_Bool        MouseButtonUp( sal_uInt16 nItemd, const MouseEvent& rMEvt );
    virtual sal_Bool        MouseMove( sal_uInt16 nItemd, const MouseEvent& rMEvt );
    virtual sal_Bool        Command( sal_uInt16 nItemd, const CommandEvent& rCEvt );
    virtual sal_Bool        UpdateHelp( sal_uInt16 nItemd );
};

// -----------------------
// - TaskStatusFieldItem -
// -----------------------

#define TASKSTATUSFIELDITEM_FLASH           ((sal_uInt16)0x0001)

class TaskStatusFieldItem
{
private:
    ITaskStatusNotify*  mpNotify;
    Image               maImage;
    XubString           maQuickHelpText;
    XubString           maHelpText;
    rtl::OString        maHelpId;
    sal_uInt16          mnFlags;

public:
                        TaskStatusFieldItem();
                        TaskStatusFieldItem( const TaskStatusFieldItem& rItem );
                        TaskStatusFieldItem( ITaskStatusNotify* pNotify,
                                             const Image& rImage,
                                             const XubString& rQuickHelpText,
                                             const XubString& rHelpText,
                                             sal_uInt16 nFlags );
                        ~TaskStatusFieldItem();

    void                SetNotifyObject( ITaskStatusNotify* pNotify ) { mpNotify = pNotify; }
    ITaskStatusNotify*  GetNotifyObject() const { return mpNotify; }
    void                SetImage( const Image& rImage ) { maImage = rImage; }
    const Image&        GetImage() const { return maImage; }
    void                SetQuickHelpText( const XubString& rStr ) { maQuickHelpText = rStr; }
    const XubString&    GetQuickHelpText() const { return maQuickHelpText; }
    void                SetHelpText( const XubString& rStr ) { maHelpText = rStr; }
    const XubString&    GetHelpText() const { return maHelpText; }
    void                SetHelpId( const rtl::OString& rHelpId ) { maHelpId = rHelpId; }
    const rtl::OString& GetHelpId() const { return maHelpId; }
    void                SetFlags( sal_uInt16 nFlags ) { mnFlags = nFlags; }
    sal_uInt16          GetFlags() const { return mnFlags; }

    const TaskStatusFieldItem& operator=( const TaskStatusFieldItem& rItem );
};

// -----------------
// - TaskStatusBar -
// -----------------

#define TASKSTATUSBAR_STATUSFIELDID         ((sal_uInt16)61000)

#define TASKSTATUSBAR_CLOCKID               ((sal_uInt16)61000)
#define TASKSTATUSFIELD_CLOCK               ((sal_uInt16)0x0001)

class SVT_DLLPUBLIC TaskStatusBar : public StatusBar
{
    friend class TaskBar;

private:
    ImplTaskSBItemList* mpFieldItemList;
    TaskBar*            mpNotifyTaskBar;
    ITaskStatusNotify*  mpNotify;
    Time                maTime;
    XubString           maTimeText;
    AutoTimer           maTimer;
    long                mnClockWidth;
    long                mnItemWidth;
    long                mnFieldWidth;
    sal_uInt16              mnFieldFlags;
    sal_uInt16              mnDummy1;
    sal_Bool                mbFlashItems;
    sal_Bool                mbOutInterval;
    sal_Bool                mbDummy1;
    sal_Bool                mbDummy2;

#ifdef _TASKBAR_CXX
    SVT_DLLPRIVATE ImplTaskSBFldItem*  ImplGetFieldItem( sal_uInt16 nItemId ) const;
    SVT_DLLPRIVATE ImplTaskSBFldItem*  ImplGetFieldItem( const Point& rPos, sal_Bool& rFieldRect ) const;
    SVT_DLLPRIVATE sal_Bool                ImplUpdateClock();
    SVT_DLLPRIVATE sal_Bool                ImplUpdateFlashItems();
    SVT_DLLPRIVATE void                ImplUpdateField( sal_Bool bItems );
                        DECL_DLLPRIVATE_LINK( ImplTimerHdl, Timer* );
#endif

public:
                        TaskStatusBar( Window* pParent, WinBits nWinStyle = WB_LEFT );
                        ~TaskStatusBar();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    void                InsertStatusField( long nOffset = STATUSBAR_OFFSET,
                                           sal_uInt16 nPos = STATUSBAR_APPEND,
                                           sal_uInt16 nFlags = TASKSTATUSFIELD_CLOCK );
    void                RemoveStatusField()
                            { maTimer.Stop(); RemoveItem( TASKSTATUSBAR_STATUSFIELDID ); }
    void                SetFieldFlags( sal_uInt16 nFlags );
    sal_uInt16              GetFieldFlags() const { return mnFieldFlags; }
    void                SetNotifyObject( ITaskStatusNotify* pNotify ) { mpNotify = pNotify; }
    ITaskStatusNotify*  GetNotifyObject() const { return mpNotify; }

    void                AddStatusFieldItem( sal_uInt16 nItemId, const TaskStatusFieldItem& rItem,
                                            sal_uInt16 nPos = 0xFFFF );
    void                ModifyStatusFieldItem( sal_uInt16 nItemId, const TaskStatusFieldItem& rItem );
    void                RemoveStatusFieldItem( sal_uInt16 nItemId );
    sal_Bool                GetStatusFieldItem( sal_uInt16 nItemId, TaskStatusFieldItem& rItem ) const;
};

// -----------
// - TaskBar -
// -----------

class SVT_DLLPUBLIC TaskBar : public Window
{
private:
    ImplTaskBarFloat*       mpAutoHideBar;
    TaskButtonBar*          mpButtonBar;
    TaskToolBox*            mpTaskToolBox;
    TaskStatusBar*          mpStatusBar;
    void*                   mpDummy1;
    void*                   mpDummy2;
    void*                   mpDummy3;
    void*                   mpDummy4;
    String                  maOldText;
    long                    mnStatusWidth;
    long                    mnMouseOff;
    long                    mnOldStatusWidth;
    long                    mnDummy1;
    long                    mnDummy2;
    long                    mnDummy3;
    long                    mnDummy4;
    WinBits                 mnWinBits;
    sal_uInt16                  mnLines;
    sal_Bool                    mbStatusText;
    sal_Bool                    mbShowItems;
    sal_Bool                    mbAutoHide;
    sal_Bool                    mbAlignDummy1;
    sal_Bool                    mbDummy1;
    sal_Bool                    mbDummy2;
    sal_Bool                    mbDummy3;
    sal_Bool                    mbDummy4;
    Link                    maTaskResizeHdl;

#ifdef _TASKBAR_CXX
    SVT_DLLPRIVATE void                    ImplInitSettings();
    SVT_DLLPRIVATE void                    ImplNewHeight( long nNewHeight );
#endif

public:
                            TaskBar( Window* pParent, WinBits nWinStyle = WB_BORDER | WB_SIZEABLE );
                            ~TaskBar();

    virtual void            TaskResize();

    virtual TaskButtonBar*  CreateButtonBar();
    virtual TaskToolBox*    CreateTaskToolBox();
    virtual TaskStatusBar*  CreateTaskStatusBar();

    virtual void            MouseMove( const MouseEvent& rMEvt );
    virtual void            MouseButtonDown( const MouseEvent& rMEvt );
    virtual void            Tracking( const TrackingEvent& rMEvt );
    virtual void            Paint( const Rectangle& rRect );
    virtual void            Resize();
    virtual void            StateChanged( StateChangedType nType );
    virtual void            DataChanged( const DataChangedEvent& rDCEvt );

    void                    Format();

    void                    SetLines( sal_uInt16 nLines );
    sal_uInt16                  GetLines() const { return mnLines; }
    void                    EnableAutoHide( sal_Bool bAutoHide = sal_True );
    sal_Bool                    IsAutoHideEnabled() const { return mbAutoHide; }

    void                    ShowStatusText( const String& rText );
    void                    HideStatusText();

    void                    SetStatusSize( long nNewSize )
                                { mnStatusWidth=nNewSize; Resize(); }
    long                    GetStatusSize() const { return mnStatusWidth; }

    Size                    CalcWindowSizePixel() const;

    TaskButtonBar*          GetButtonBar() const;
    TaskToolBox*            GetTaskToolBox() const;
    TaskStatusBar*          GetStatusBar() const;

    void                    SetTaskResizeHdl( const Link& rLink ) { maTaskResizeHdl = rLink; }
    const Link&             GetTaskResizeHdl() const { return maTaskResizeHdl; }
};

// -----------------------
// - WindowArrange-Types -
// -----------------------

#define WINDOWARRANGE_TILE      1
#define WINDOWARRANGE_HORZ      2
#define WINDOWARRANGE_VERT      3
#define WINDOWARRANGE_CASCADE   4

class ImplWindowArrangeList;

// -----------------------
// - class WindowArrange -
// -----------------------

class SVT_DLLPUBLIC WindowArrange
{
private:
    List*                   mpWinList;
    void*                   mpDummy;
    sal_uLong                   mnDummy;

#ifdef _TASKBAR_CXX
    SVT_DLLPRIVATE void                    ImplTile( const Rectangle& rRect );
    SVT_DLLPRIVATE void                    ImplHorz( const Rectangle& rRect );
    SVT_DLLPRIVATE void                    ImplVert( const Rectangle& rRect );
    SVT_DLLPRIVATE void                    ImplCascade( const Rectangle& rRect );
#endif

public:
                            WindowArrange();
                            ~WindowArrange();

    void                    AddWindow( Window* pWindow, sal_uLong nPos = LIST_APPEND )
                                { mpWinList->Insert( (void*)pWindow, nPos ); }
    void                    RemoveAllWindows()
                                { mpWinList->Clear(); }

    void                    Arrange( sal_uInt16 nType, const Rectangle& rRect );
};

#endif  // _TASKBAR_HXX
