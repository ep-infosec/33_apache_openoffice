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



#ifndef SD_TOOL_PANEL_VIEW_SHELL_HXX
#define SD_TOOL_PANEL_VIEW_SHELL_HXX

#include "ViewShell.hxx"
#include "glob.hxx"
#include "taskpane/PanelId.hxx"
#include "framework/FrameworkHelper.hxx"
#include <vcl/button.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/viewfac.hxx>
#include <sfx2/dockwin.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

class PopupMenu;

namespace com { namespace sun { namespace star { namespace ui {
	class XUIElement;
} } } }

namespace sd {
class PaneDockingWindow;

namespace toolpanel {
class TaskPaneShellManager;
class TitleToolBox;
class TitleBar;
class TitledControl;
class ToolPanelViewShell_Impl;
/** The tool panel is a view shell for some very specific reasons:
	- It fits better into the concept of panes being docking windows whose
	content, a view shell, can be exchanged on runtime.
	- A control in the tool panel that wants to show a context menu has to
	do that over the dispatcher of a shell. These shells, usually
	implemented by the controls themselves, have to be managed by someone.
	If interpreted as object bars this can be handled by the
	ObjectBarManager of the ViewShell.
*/
class ToolPanelViewShell
	: public ViewShell
{
public:
	TYPEINFO();
	SFX_DECL_INTERFACE(SD_IF_SDTOOLPANELSHELL)

	ToolPanelViewShell (
		SfxViewFrame* pFrame,
		ViewShellBase& rViewShellBase,
		::Window* pParentWindow,
		FrameView* pFrameView);
	virtual ~ToolPanelViewShell (void);

	/** Register the SFX interfaces so that (some of) the controls can be
		pushed as SFX shells on the shell stack and process slot calls and
		so on.
	*/
	static void RegisterControls (void);

	virtual void GetFocus (void);
	virtual void LoseFocus (void);
	virtual void KeyInput (const KeyEvent& rEvent);
	using sd::ViewShell::KeyInput;

	virtual SdPage*	GetActualPage (void);
	virtual SdPage*	getCurrentPage (void) const;

	virtual void ArrangeGUIElements (void);

	TaskPaneShellManager& GetSubShellManager (void) const;

	/** deactivates the given panel, bypassing the configuration controller. Only valid for tool panels which are
		not under the drawing framework's control.
	*/
	void ActivatePanel( const ::rtl::OUString& i_rPanelResourceURL );

	/** deactivates the given panel, bypassing the configuration controller
	*/
	void DeactivatePanel( const ::rtl::OUString& i_rPanelResourceURL );

	/** Return a pointer to the docking window that is the parent or a
		predecessor of the content window.
		@return
			When the view shell is not placed in a docking window, e.g. when
			shown in the center pane, then <NULL?> is returned.
	*/
	DockingWindow* GetDockingWindow (void);

	virtual ::com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessible>
		CreateAccessibleDocumentView (::sd::Window* pWindow);

	virtual css::uno::Reference<css::drawing::XDrawSubController> CreateSubController (void);

	/** Relocate all toplevel controls to the given parent window.
	*/
	virtual bool RelocateToParentWindow (::Window* pParentWindow);

	/// returns <TRUE/> if and only if the given window is the panel anchor window of our ToolPanelDeck
	bool IsPanelAnchorWindow( const ::Window& i_rWindow ) const;

	/** creates an XUIElement for the given standard panel
	*/
	::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement >
			CreatePanelUIElement(
				const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& i_rDocFrame,
				const ::rtl::OUString& i_rPanelResourceURL
			);

private:
	::boost::scoped_ptr< ToolPanelViewShell_Impl > mpImpl;

	::boost::shared_ptr<TaskPaneShellManager> mpSubShellManager;

	/** The id of the menu in the menu bar/tool box of the parent docking
		window.
	*/
	sal_uInt16 mnMenuId;

	/** Create a popup menu. It contains two sections, one for
		docking or un-docking the tool panel, one for toggling the
		visibility state of the tool panel items.
		@param bIsDocking
			According to this flag one of the lock/unlock entries is
			made disabled.
	*/
	::std::auto_ptr<PopupMenu> CreatePopupMenu (bool bIsDocking);


	/** Initialize the task pane view shell if that has not yet been done
		before. If mbIsInitialized is already set to <TRUE/> then this
		method returns immediately.
	*/
	void Initialize (void);
};




} } // end of namespace ::sd::toolpanel

#endif
