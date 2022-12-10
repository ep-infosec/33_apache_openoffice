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


#ifndef _BASIDESH_HXX
#define _BASIDESH_HXX

#include "sbxitem.hxx"

#include "scriptdocument.hxx"
#include "doceventnotifier.hxx"
#include <sfx2/viewfac.hxx>
#include <sfx2/shell.hxx>
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#include <sfx2/viewsh.hxx>
#include <svx/ifaceids.hxx>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAMPROVIDER_HXX_
#include <com/sun/star/io/XInputStreamProvider.hpp>
#endif

#include <com/sun/star/container/XContainerListener.hpp>

//----------------------------------------------------------------------------

const sal_uLong BASICIDE_UI_FEATURE_SHOW_BROWSER = 0x00000001;

//----------------------------------------------------------------------------

class ModulWindow;
class ModulWindowLayout;
class DialogWindow;
class SdrView;
class ObjectCatalog;
class BasicIDETabBar;
class TabBar;
class IDEBaseWindow;
class SbxObject;
class SbModule;
class StarBASIC;
class LocalizationMgr;
struct BasicIDEShell_Impl;

#if _SOLAR__PRIVATE
DECLARE_TABLE( IDEWindowTable, IDEBaseWindow* )
#else
typedef Table IDEWindowTable;
#endif

namespace BasicIDE
{
    bool RemoveDialog( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );
}

class BasicIDEShell :public SfxViewShell
                    ,public ::basctl::DocumentEventListener
{
friend class JavaDebuggingListenerImpl;
friend class LocalizationMgr;
friend sal_Bool implImportDialog( Window* pWin, const String& rCurPath, const ScriptDocument& rDocument, const String& aLibName );
friend bool BasicIDE::RemoveDialog( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );

	ObjectCatalog* 		pObjectCatalog;

	IDEWindowTable		aIDEWindowTable;
	sal_uInt16				nCurKey;
	IDEBaseWindow* 		pCurWin;
    ScriptDocument      m_aCurDocument;
    String              m_aCurLibName;
	LocalizationMgr*	m_pCurLocalizationMgr;

	ScrollBar			aHScrollBar;
	ScrollBar			aVScrollBar;
	ScrollBarBox		aScrollBarBox;
	BasicIDETabBar*		pTabBar;
	sal_Bool				bTabBarSplitted;
	sal_Bool				bCreatingWindow;
	ModulWindowLayout*	pModulLayout;
    sal_Bool                m_bAppBasicModified;
    ::basctl::DocumentEventNotifier
                        m_aNotifier;
friend class ContainerListenerImpl;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener > m_xLibListener;

#if _SOLAR__PRIVATE
	void				Init();
	void				InitTabBar();
	void				InitScrollBars();
	void				CheckWindows();
	void				RemoveWindows( const ScriptDocument& rDocument, const String& rLibName, sal_Bool bDestroy );
	void				UpdateWindows();
	void				ShowObjectDialog( sal_Bool bShow, sal_Bool bCreateOrDestroy );
	void				InvalidateBasicIDESlots();
	void				StoreAllWindowData( sal_Bool bPersistent = sal_True );
	void				SetMDITitle();
	void				EnableScrollbars( sal_Bool bEnable );
    void                SetCurLib( const ScriptDocument& rDocument, String aLibName, bool bUpdateWindows = true , bool bCheck = true );
	void				SetCurLibForLocalization( const ScriptDocument& rDocument, String aLibName );

	void				ImplStartListening( StarBASIC* pBasic );

	DECL_LINK( TabBarHdl, TabBar* );
	DECL_LINK( AccelSelectHdl, Accelerator* );
	DECL_LINK( ObjectDialogCancelHdl, ObjectCatalog * );
	DECL_LINK( TabBarSplitHdl, TabBar * );
#endif

protected:
	virtual void    	AdjustPosSizePixel( const Point &rPos, const Size &rSize );
	virtual void		OuterResizePixel( const Point &rPos, const Size &rSize );
	virtual Size    	GetOptimalSizePixel() const;
	sal_uInt16				InsertWindowInTable( IDEBaseWindow* pNewWin );
	virtual sal_uInt16 		PrepareClose( sal_Bool bUI, sal_Bool bForBrowsing );

	void				SetCurWindow( IDEBaseWindow* pNewWin, sal_Bool bUpdateTabBar = sal_False, sal_Bool bRememberAsCurrent = sal_True );
	void				ManageToolbars();
	void 				RemoveWindow( IDEBaseWindow* pWindow, sal_Bool bDestroy, sal_Bool bAllowChangeCurWindow = sal_True );
	void				ArrangeTabBar();

	ModulWindow*		CreateBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName );
    DialogWindow*       CreateDlgWin( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName );

	ModulWindow*		FindBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName, sal_Bool bCreateIfNotExist, sal_Bool bFindSuspended = sal_False );
	ModulWindow* 		ShowActiveModuleWindow( StarBASIC* pBasic );

	virtual void 		SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
								const SfxHint& rHint, const TypeId& rHintType );

	virtual void		Activate(sal_Bool bMDI);
	virtual void		Deactivate(sal_Bool bMDI);

	virtual void		Move();
	virtual	void		ShowCursor( FASTBOOL bOn = sal_True );

	void				CreateModulWindowLayout();
	void				DestroyModulWindowLayout();
	void				UpdateModulWindowLayout( bool bBasicStopped );

    // DocumentEventListener
    virtual void onDocumentCreated( const ScriptDocument& _rDocument );
    virtual void onDocumentOpened( const ScriptDocument& _rDocument );
    virtual void onDocumentSave( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveDone( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAs( const ScriptDocument& _rDocument );
    virtual void onDocumentSaveAsDone( const ScriptDocument& _rDocument );
    virtual void onDocumentClosed( const ScriptDocument& _rDocument );
    virtual void onDocumentTitleChanged( const ScriptDocument& _rDocument );
    virtual void onDocumentModeChanged( const ScriptDocument& _rDocument );

public:
						TYPEINFO();
						SFX_DECL_INTERFACE( SVX_INTERFACE_BASIDE_VIEWSH )
						SFX_DECL_VIEWFACTORY(BasicIDEShell);

						BasicIDEShell( SfxViewFrame *pFrame, SfxViewShell *pOldSh );
						~BasicIDEShell();

	IDEBaseWindow*		GetCurWindow() const 	{ return pCurWin; }
    const ScriptDocument&
                        GetCurDocument() const { return m_aCurDocument; }
    const String&       GetCurLibName() const { return m_aCurLibName; }
	ObjectCatalog* 		GetObjectCatalog() const	{ return pObjectCatalog; }
	LocalizationMgr*	GetCurLocalizationMgr() const { return m_pCurLocalizationMgr; }

	ScrollBar&			GetHScrollBar() 		{ return aHScrollBar; }
	ScrollBar&			GetVScrollBar() 		{ return aVScrollBar; }
	ScrollBarBox&		GetScrollBarBox() 		{ return aScrollBarBox; }
	TabBar*				GetTabBar()				{ return (TabBar*)pTabBar; }
	IDEWindowTable&		GetIDEWindowTable() 	{ return aIDEWindowTable; }

	SdrView*			GetCurDlgView() const;

    ::svl::IUndoManager*
                        GetUndoManager();

    virtual com::sun::star::uno::Reference< com::sun::star::view::XRenderable > GetRenderable();
    
	// virtual sal_uInt16			Print( SfxProgress &rProgress, sal_Bool bIsAPI, PrintDialog *pPrintDialog = 0 );
	virtual SfxPrinter*		GetPrinter( sal_Bool bCreate );
	virtual sal_uInt16			SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
	virtual String			GetSelectionText( sal_Bool bCompleteWords );
	virtual sal_Bool			HasSelection( sal_Bool bText ) const;

	void				GetState( SfxItemSet& );
	void				ExecuteGlobal( SfxRequest& rReq );
	void				ExecuteCurrent( SfxRequest& rReq );
	void				ExecuteBasic( SfxRequest& rReq );
	void				ExecuteDialog( SfxRequest& rReq );

	virtual	sal_Bool	HasUIFeature( sal_uInt32 nFeature );

	long				CallBasicErrorHdl( StarBASIC* pBasic );
	long				CallBasicBreakHdl( StarBASIC* pBasic );

	ModulWindowLayout*	GetLayoutWindow() const { return pModulLayout; }

	IDEBaseWindow*		FindWindow( const ScriptDocument& rDocument, const String& rLibName = String(), const String& rName = String(), sal_uInt16 nType = BASICIDE_TYPE_UNKNOWN, sal_Bool bFindSuspended = sal_False );
    DialogWindow*       FindDlgWin( const ScriptDocument& rDocument, const String& rLibName, const String& rDlgName, sal_Bool bCreateIfNotExist, sal_Bool bFindSuspended = sal_False );
	IDEBaseWindow*		FindApplicationWindow();
    sal_Bool				NextPage( sal_Bool bPrev = sal_False );
    
	sal_Bool	            IsAppBasicModified() const { return m_bAppBasicModified; }
	void			    SetAppBasicModified( sal_Bool bModified = sal_True ) { m_bAppBasicModified = bModified; }

	// For Dialog Drag&Drop in Dialog Organizer
	static void CopyDialogResources(
		::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStreamProvider >& io_xISP,
		const ScriptDocument& rSourceDoc, const String& rSourceLibName, const ScriptDocument& rDestDoc,
		const String& rDestLibName,	const String& rDlgName );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        GetCurrentDocument() const;

};

#endif // _BASIDESH_HXX
