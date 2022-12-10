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



#ifndef _SD_TABLEDESIGNPANE_HXX
#define _SD_TABLEDESIGNPANE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/LayoutSize.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/sidebar/ILayoutableWindow.hxx>

#include <boost/scoped_ptr.hpp>

#include "TableDesignPane.hrc"

namespace sd
{

namespace tools {
class EventMultiplexerEvent;
}

class ViewShellBase;

// --------------------------------------------------------------------

class TableDesignPane : public Control, public sfx2::sidebar::ILayoutableWindow
{
public:
	TableDesignPane( ::Window* pParent, ViewShellBase& rBase, bool bModal );
	virtual ~TableDesignPane();

	// callbacks
	void onSelectionChanged();

	// Control
	virtual void Resize();

    // ILayoutableWindow
    virtual ::com::sun::star::ui::LayoutSize GetHeightForWidth (const sal_Int32 nWidth);

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

	void ApplyOptions();
	void ApplyStyle();

	bool isStyleChanged() const { return mbStyleSelected; }
	bool isOptionsChanged() const { return mbOptionsChanged; }

private:
	void addListener();
	void removeListener();
	void updateLayout();
	void updateControls();

	void FillDesignPreviewControl();

    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
	DECL_LINK(implValueSetHdl, Control* );
	DECL_LINK(implCheckBoxHdl, Control* );

private:
	ViewShellBase& mrBase;
	const rtl::OUString msTableTemplate;

	boost::scoped_ptr< Control > mxControls[DESIGNPANE_CONTROL_COUNT];
	int mnOrgOffsetY[DESIGNPANE_CONTROL_COUNT];

	::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > mxSelectedTable;
	::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView > mxView;
	::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > mxTableFamily;
	::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > mxToolbar;

	bool mbModal;
	bool mbStyleSelected;
	bool mbOptionsChanged;
};

// --------------------------------------------------------------------

class TableDesignDialog : public ModalDialog
{
public:
	TableDesignDialog( ::Window* pParent, ViewShellBase& rBase );

	virtual short Execute();
private:
	ViewShellBase& mrBase;
	boost::scoped_ptr< TableDesignPane > mpDesignPane;

	boost::scoped_ptr< FixedLine >		 mxFlSep1;
	boost::scoped_ptr< FixedLine >		 mxFlSep2;
	boost::scoped_ptr< HelpButton >		 mxHelpButton;
	boost::scoped_ptr< OKButton >		 mxOkButton;
	boost::scoped_ptr< CancelButton >	 mxCancelButton;
};

}

#endif // _SD_TABLEFORMATPANE_HXX
