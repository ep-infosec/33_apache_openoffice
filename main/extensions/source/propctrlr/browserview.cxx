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
#include "precompiled_extensions.hxx"
#include "browserview.hxx"
#include "propertyeditor.hxx"
#include "propctrlr.hrc"
#include <tools/debug.hxx>
#include <memory>

//............................................................................
namespace pcr
{
//............................................................................

	using namespace ::com::sun::star::uno;
	using namespace ::com::sun::star::lang;


	//========================================================================
	//= class OPropertyBrowserView
	//========================================================================
	DBG_NAME(OPropertyBrowserView)
	//------------------------------------------------------------------------
	OPropertyBrowserView::OPropertyBrowserView( const Reference< XMultiServiceFactory >& _rxORB,
								 Window* _pParent, WinBits nBits)
				  :Window(_pParent, nBits | WB_3DLOOK)
				  ,m_xORB(_rxORB)
				  ,m_nActivePage(0)
	{
		DBG_CTOR(OPropertyBrowserView,NULL);

		m_pPropBox = new OPropertyEditor( this );
		m_pPropBox->SetHelpId(HID_FM_PROPDLG_TABCTR);
		m_pPropBox->setPageActivationHandler(LINK(this, OPropertyBrowserView, OnPageActivation));

		m_pPropBox->Show();
	}

	//------------------------------------------------------------------------
	IMPL_LINK(OPropertyBrowserView, OnPageActivation, void*, EMPTYARG)
	{
		m_nActivePage = m_pPropBox->GetCurPage();
		if (m_aPageActivationHandler.IsSet())
			m_aPageActivationHandler.Call(NULL);
		return 0L;
	}

	//------------------------------------------------------------------------
	OPropertyBrowserView::~OPropertyBrowserView()
	{
		if(m_pPropBox)
		{
			sal_uInt16 nTmpPage = m_pPropBox->GetCurPage();
			if (nTmpPage)
				m_nActivePage = nTmpPage;
			::std::auto_ptr<Window> aTemp(m_pPropBox);
			m_pPropBox = NULL;
		}
		m_xORB = NULL;

		DBG_DTOR(OPropertyBrowserView, NULL);
	}

	//------------------------------------------------------------------------
	void OPropertyBrowserView::activatePage(sal_uInt16 _nPage)
	{
		m_nActivePage = _nPage;
		getPropertyBox().SetPage(m_nActivePage);
	}

	//------------------------------------------------------------------------
	void OPropertyBrowserView::GetFocus()
	{
		if (m_pPropBox)
			m_pPropBox->GrabFocus();
		else
			Window::GetFocus();
	}

	//------------------------------------------------------------------------
	long OPropertyBrowserView::Notify( NotifyEvent& _rNEvt )
    {
		if ( EVENT_KEYINPUT == _rNEvt.GetType() )
		{
			sal_uInt16 nKey = _rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

			if ( ( KEY_DELETE == nKey ) || ( KEY_BACKSPACE == nKey ) )
                // silence this, we don't want to propagate this outside the property
                // browser, as it will probably do harm there
                // #i63285# / 2006-12-06 / frank.schoenheit@sun.com
				return 1;
		}
        return Window::Notify( _rNEvt );
    }

	//------------------------------------------------------------------------
	void OPropertyBrowserView::Resize()
	{
		Size aSize = GetOutputSizePixel();
		m_pPropBox->SetSizePixel(aSize);
	}

	// #95343# ---------------------------------------------------------------
	::com::sun::star::awt::Size OPropertyBrowserView::getMinimumSize()
	{
		Size aSize = GetOutputSizePixel();
		if( m_pPropBox )
        {
		    aSize.setHeight( m_pPropBox->getMinimumHeight() );
			aSize.setWidth( m_pPropBox->getMinimumWidth() );
        }
		return ::com::sun::star::awt::Size( aSize.Width(), aSize.Height() );
	}
	
//............................................................................
} // namespace pcr
//............................................................................

