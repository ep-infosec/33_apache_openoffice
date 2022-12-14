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
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/characterattributeshelper.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/ctrl.hxx>

#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//	----------------------------------------------------
//	class VCLXAccessibleTextComponent
//	----------------------------------------------------

VCLXAccessibleTextComponent::VCLXAccessibleTextComponent( VCLXWindow* pVCLXWindow )
	:VCLXAccessibleComponent( pVCLXWindow )
{
	if ( GetWindow() )
		m_sText = OutputDevice::GetNonMnemonicString( GetWindow()->GetText() );
}

// -----------------------------------------------------------------------------

VCLXAccessibleTextComponent::~VCLXAccessibleTextComponent()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::SetText( const ::rtl::OUString& sText )
{
	Any aOldValue, aNewValue;
	if ( implInitTextChangedEvent( m_sText, sText, aOldValue, aNewValue ) )
	{
		m_sText = sText;
		NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
	}
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
	switch ( rVclWindowEvent.GetId() )
	{
		case VCLEVENT_WINDOW_FRAMETITLECHANGED:
		{
			VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
			SetText( implGetText() );
		}
		break;
		default:
			VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
	}
}

// -----------------------------------------------------------------------------
// OCommonAccessibleText
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::implGetText()
{
	::rtl::OUString aText;
	if ( GetWindow() )
		aText = OutputDevice::GetNonMnemonicString( GetWindow()->GetText() );

	return aText;
}

// -----------------------------------------------------------------------------

lang::Locale VCLXAccessibleTextComponent::implGetLocale()
{
	return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
	nStartIndex = 0;
	nEndIndex = 0;
}

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::disposing()
{
	VCLXAccessibleComponent::disposing();

	m_sText = ::rtl::OUString();
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleTextComponent, VCLXAccessibleComponent, VCLXAccessibleTextComponent_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleTextComponent, VCLXAccessibleComponent, VCLXAccessibleTextComponent_BASE )

// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getCaretPosition() throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return -1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTextComponent::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return setSelection( nIndex, nIndex );
}

// -----------------------------------------------------------------------------

sal_Unicode VCLXAccessibleTextComponent::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getCharacter( nIndex );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleTextComponent::getCharacterAttributes( sal_Int32 nIndex, const Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	Sequence< PropertyValue > aValues;
	::rtl::OUString sText( implGetText() );

	if ( !implIsValidIndex( nIndex, sText.getLength() ) )
		throw IndexOutOfBoundsException();

	if ( GetWindow() )
	{
		Font aFont = GetWindow()->GetControlFont();

		sal_Int32 nBackColor = GetWindow()->GetControlBackground().GetColor();
		sal_Int32 nColor = GetWindow()->GetControlForeground().GetColor();

		// MT: Code with default font was introduced with the IA2 CWS, but I am not convinced that this is the correct font...
		// Decide what to do when we have a concrete issue.
		/*
		Font aDefaultVCLFont;
		OutputDevice* pDev = Application::GetDefaultDevice();
		if ( pDev )
		{
			aDefaultVCLFont = pDev->GetSettings().GetStyleSettings().GetAppFont();
			if ( !aFont.GetName().Len() )
			{
				String aDefaultName = aDefaultVCLFont.GetName();
				aFont.SetName( aDefaultName );
			}
			if ( !aFont.GetHeight() )
			{
				aFont.SetHeight( aDefaultVCLFont.GetHeight() );
			}
			if ( aFont.GetWeight() == WEIGHT_DONTKNOW )
			{
				aFont.SetWeight( aDefaultVCLFont.GetWeight() );
			}

			//if nColor is -1, it may indicate that the default color black is using.
			if ( nColor == -1)
			{
				nColor = aDefaultVCLFont.GetColor().GetColor();
			}
		}
		*/

		// MT: Adjustment stuff was introduced with the IA2 CWS, but adjustment is not a character attribute...
		// In case we reintroduce it, use adjustment as extra parameter for the CharacterAttributesHelper...
		/*
		WinBits aBits = GetWindow()->GetStyle();
		sal_Int16 nAdjust = -1;
		if ( aBits & WB_LEFT )
		{
			nAdjust = style::ParagraphAdjust_LEFT;
		}
		else if ( aBits & WB_RIGHT )
		{
			nAdjust = style::ParagraphAdjust_RIGHT;
		}
		else if ( aBits & WB_CENTER )
		{
			nAdjust = style::ParagraphAdjust_CENTER;
		}
		*/

		::std::auto_ptr< CharacterAttributesHelper > pHelper( new CharacterAttributesHelper( aFont, nBackColor, nColor ) );

		aValues = pHelper->GetCharacterAttributes( aRequestedAttributes );
	}

	return aValues;
}

// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleTextComponent::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
		throw IndexOutOfBoundsException();

	awt::Rectangle aRect;
	Control* pControl = static_cast< Control* >( GetWindow() );
	if ( pControl )
		aRect = AWTRectangle( pControl->GetCharacterBounds( nIndex ) );

	return aRect;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getCharacterCount() throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getCharacterCount();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	sal_Int32 nIndex = -1;
	Control* pControl = static_cast< Control* >( GetWindow() );
	if ( pControl )
		nIndex = pControl->GetIndexForPoint( VCLPoint( aPoint ) );

	return nIndex;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::getSelectedText() throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getSelectedText();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getSelectionStart() throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getSelectionStart();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getSelectionEnd() throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getSelectionEnd();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTextComponent::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
		throw IndexOutOfBoundsException();

	return sal_False;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::getText() throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getText();
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleTextComponent::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleTextComponent::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleTextComponent::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTextComponent::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	sal_Bool bReturn = sal_False;

	if ( GetWindow() )
	{
		Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
		if ( xClipboard.is() )
		{
			::rtl::OUString sText( getTextRange( nStartIndex, nEndIndex ) );

			::vcl::unohelper::TextDataObject* pDataObj = new ::vcl::unohelper::TextDataObject( sText );
			const sal_uInt32 nRef = Application::ReleaseSolarMutex();
			xClipboard->setContents( pDataObj, NULL );

			Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
			if( xFlushableClipboard.is() )
				xFlushableClipboard->flushClipboard();

			Application::AcquireSolarMutex( nRef );

			bReturn = sal_True;
		}
	}

	return bReturn;
}

// -----------------------------------------------------------------------------
