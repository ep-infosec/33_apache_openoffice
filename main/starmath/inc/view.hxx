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


#ifndef VIEW_HXX
#define VIEW_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/viewsh.hxx>
#include <svtools/scrwin.hxx>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/colorcfg.hxx>
#include "edit.hxx"
#include "node.hxx"
#include "accessibility.hxx"

class Menu;
class DataChangedEvent;
class SmClipboardChangeListener;
class SmDocShell;
class SmViewShell;
class SmPrintUIOptions;

/**************************************************************************/

class SmGraphicWindow : public ScrollableWindow
{
	Point			aFormulaDrawPos;
	Rectangle		aCursorRect;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible >  xAccessible;
    SmGraphicAccessible *                                       pAccessible;

    SmViewShell    *pViewShell;
	sal_uInt16			nZoom;
	short			nModifyCount;
	sal_Bool			bIsCursorVisible;

protected:
	void		SetFormulaDrawPos(const Point &rPos) { aFormulaDrawPos = rPos; }
	void		SetIsCursorVisible(sal_Bool bVis) { bIsCursorVisible = bVis; }
    using   Window::SetCursor;
    void        SetCursor(const SmNode *pNode);
	void 		SetCursor(const Rectangle &rRect);

	virtual void DataChanged( const DataChangedEvent& );
	virtual void Paint(const Rectangle&);
	virtual void KeyInput(const KeyEvent& rKEvt);
	virtual void Command(const CommandEvent& rCEvt);
	virtual void StateChanged( StateChangedType eChanged );
	DECL_LINK(MenuSelectHdl, Menu *);

public:
	SmGraphicWindow(SmViewShell* pShell);
    ~SmGraphicWindow();

    // Window
    virtual void    MouseButtonDown(const MouseEvent &rMEvt);
    virtual void    GetFocus();
    virtual void    LoseFocus();

    SmViewShell *   GetView()   { return pViewShell; }

    using   Window::SetZoom;
	void   SetZoom(sal_uInt16 Factor);
    using   Window::GetZoom;
	sal_uInt16 GetZoom() const { return nZoom; }

    const Point &   GetFormulaDrawPos() const { return aFormulaDrawPos; }

    void ZoomToFitInWindow();
    using   ScrollableWindow::SetTotalSize;
	void SetTotalSize();

	sal_Bool IsCursorVisible() const { return bIsCursorVisible; }
	void ShowCursor(sal_Bool bShow);
	const SmNode * SetCursorPos(sal_uInt16 nRow, sal_uInt16 nCol);

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );

    // for Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

    using   Window::GetAccessible;
    SmGraphicAccessible *   GetAccessible_Impl()  { return pAccessible; }
};

/**************************************************************************/

class SmGraphicController: public SfxControllerItem
{
protected:
	SmGraphicWindow &rGraphic;
public:
	SmGraphicController(SmGraphicWindow &, sal_uInt16, SfxBindings & );
	virtual void StateChanged(sal_uInt16			 nSID,
							  SfxItemState		 eState,
							  const SfxPoolItem* pState);
};

/**************************************************************************/

class SmEditController: public SfxControllerItem
{
protected:
	SmEditWindow &rEdit;

public:
	SmEditController(SmEditWindow &, sal_uInt16, SfxBindings  & );
#if OSL_DEBUG_LEVEL > 1
    virtual ~SmEditController();
#endif

	virtual void StateChanged(sal_uInt16			 nSID,
							  SfxItemState		 eState,
							  const SfxPoolItem* pState);
};

/**************************************************************************/

class SmCmdBoxWindow : public SfxDockingWindow
{
	SmEditWindow		aEdit;
	SmEditController	aController;
    sal_Bool                bExiting;
    
    Timer               aInitialFocusTimer;
    
    DECL_LINK(InitialFocusTimerHdl, Timer *);

protected :

    // Window
    virtual void    GetFocus();
	virtual void Resize();
	virtual void Paint(const Rectangle& rRect);
    virtual void StateChanged( StateChangedType nStateChange );

    virtual Size CalcDockingSize(SfxChildAlignment eAlign);
	virtual SfxChildAlignment CheckAlignment(SfxChildAlignment eActual,
											 SfxChildAlignment eWish);

	virtual void	ToggleFloatingMode();

public:
	SmCmdBoxWindow(SfxBindings	  *pBindings,
				   SfxChildWindow *pChildWindow,
				   Window		  *pParent);

	virtual ~SmCmdBoxWindow ();

	void AdjustPosition();

	SmEditWindow *GetEditWindow() { return (&aEdit); }
    SmViewShell  *GetView();
};

/**************************************************************************/

class SmCmdBoxWrapper : public SfxChildWindow
{
	SFX_DECL_CHILDWINDOW(SmCmdBoxWrapper);

protected:
	SmCmdBoxWrapper(Window			*pParentWindow,
					sal_uInt16			 nId,
					SfxBindings 	*pBindings,
					SfxChildWinInfo *pInfo);

#if OSL_DEBUG_LEVEL > 1
    virtual ~SmCmdBoxWrapper();
#endif

public:

	SmEditWindow *GetEditWindow()
	{
		return (((SmCmdBoxWindow *)pWindow)->GetEditWindow());
	}

};

/**************************************************************************/

namespace sfx2 { class FileDialogHelper; }
struct SmViewShell_Impl;

class SmViewShell: public SfxViewShell
{
	// for handling the PasteClipboardState
	friend class SmClipboardChangeListener;

	SmGraphicWindow 	aGraphic;
	SmGraphicController aGraphicController;
	String				StatusText;

	::com::sun::star::uno:: Reference <
			::com::sun::star::lang:: XEventListener > xClipEvtLstnr;
	SmClipboardChangeListener*	pClipEvtLstnr;
    SmViewShell_Impl*   pImpl;
	sal_Bool				bPasteState;

    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );
	virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:

	Size GetTextLineSize(OutputDevice& rDevice,
						 const String& rLine);
	Size GetTextSize(OutputDevice& rDevice,
					 const String& rText,
					 long		   MaxWidth);
	void DrawTextLine(OutputDevice& rDevice,
					  const Point&	rPosition,
					  const String& rLine);
	void DrawText(OutputDevice& rDevice,
				  const Point&	rPosition,
				  const String& rText,
				  sal_uInt16		MaxWidth);

    virtual sal_uInt16 Print(SfxProgress &rProgress, sal_Bool bIsAPI);
	virtual SfxPrinter *GetPrinter(sal_Bool bCreate = sal_False);
	virtual sal_uInt16 SetPrinter(SfxPrinter *pNewPrinter,
                              sal_uInt16     nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false);

    sal_Bool        Insert( SfxMedium& rMedium );
    sal_Bool        InsertFrom(SfxMedium &rMedium);

	virtual SfxTabPage *CreatePrintOptionsPage(Window			*pParent,
											   const SfxItemSet &rOptions);
	virtual void Deactivate(sal_Bool IsMDIActivate);
	virtual void Activate(sal_Bool IsMDIActivate);
	virtual Size GetOptimalSizePixel() const;
	virtual void AdjustPosSizePixel(const Point &rPos, const Size &rSize);
	virtual void InnerResizePixel(const Point &rOfs, const Size  &rSize);
	virtual void OuterResizePixel(const Point &rOfs, const Size  &rSize);
	virtual void QueryObjAreaPixel( Rectangle& rRect ) const;
	virtual void SetZoomFactor( const Fraction &rX, const Fraction &rY );

public:
	TYPEINFO();

    SmViewShell(SfxViewFrame *pFrame, SfxViewShell *pOldSh);
	~SmViewShell();

	SmDocShell * GetDoc()
	{
		return (SmDocShell *) GetViewFrame()->GetObjectShell();
	}

	SmEditWindow * GetEditWindow();
		  SmGraphicWindow & GetGraphicWindow() 		 { return aGraphic; }
	const SmGraphicWindow & GetGraphicWindow() const { return aGraphic; }

	void		SetStatusText(const String& Text);

	void		ShowError( const SmErrorDesc *pErrorDesc );
	void		NextError();
	void		PrevError();

    SFX_DECL_INTERFACE(SFX_INTERFACE_SMA_START+2)
    SFX_DECL_VIEWFACTORY(SmViewShell);

	virtual void Execute( SfxRequest& rReq );
	virtual void GetState(SfxItemSet &);

    void Impl_Print( OutputDevice &rOutDev, const SmPrintUIOptions &rPrintUIOptions,
            Rectangle aOutRect, Point aZeroPoint );
};

#endif

