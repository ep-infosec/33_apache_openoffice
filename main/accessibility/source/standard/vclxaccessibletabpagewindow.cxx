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
#include "precompiled_accessibility.hxx"
#include <accessibility/standard/vclxaccessibletabpagewindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//	----------------------------------------------------
//	class VCLXAccessibleTabPageWindow
//	----------------------------------------------------

VCLXAccessibleTabPageWindow::VCLXAccessibleTabPageWindow( VCLXWindow* pVCLXWindow )
	:VCLXAccessibleComponent( pVCLXWindow )
{
	m_pTabPage = static_cast< TabPage* >( GetWindow() );
	if ( m_pTabPage )
	{
		Window* pParent = m_pTabPage->GetAccessibleParentWindow();
		if ( pParent && pParent->GetType() == WINDOW_TABCONTROL )
		{
			m_pTabControl = static_cast< TabControl* >( pParent );
			if ( m_pTabControl )
			{
				for ( sal_uInt16 i = 0, nCount = m_pTabControl->GetPageCount(); i < nCount; ++i )
				{
					sal_uInt16 nPageId = m_pTabControl->GetPageId( i );
					if ( m_pTabControl->GetTabPage( nPageId ) == m_pTabPage )
						m_nPageId = nPageId;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------

VCLXAccessibleTabPageWindow::~VCLXAccessibleTabPageWindow()
{
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleTabPageWindow::implGetBounds() throw (RuntimeException)
{
	awt::Rectangle aBounds( 0, 0, 0, 0 );

	if ( m_pTabControl )
	{
		Rectangle aPageRect = m_pTabControl->GetTabBounds( m_nPageId );
		if ( m_pTabPage )
		{
			Rectangle aRect = Rectangle( m_pTabPage->GetPosPixel(), m_pTabPage->GetSizePixel() );
			aRect.Move( -aPageRect.Left(), -aPageRect.Top() );
			aBounds = AWTRectangle( aRect );
		}
	}

	return aBounds;
}

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleTabPageWindow::disposing()
{
	VCLXAccessibleComponent::disposing();

	m_pTabControl = NULL;
	m_pTabPage = NULL;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabPageWindow::getAccessibleParent(  ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	Reference< XAccessible > xParent;
	if ( m_pTabControl )
	{
		Reference< XAccessible > xAcc( m_pTabControl->GetAccessible() );
		if ( xAcc.is() )
		{
			Reference< XAccessibleContext > xCont( xAcc->getAccessibleContext() );
			if ( xCont.is() )
				xParent = xCont->getAccessibleChild( m_pTabControl->GetPagePos( m_nPageId ) );
		}
	}

	return xParent;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPageWindow::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return 0;
}

// -----------------------------------------------------------------------------
