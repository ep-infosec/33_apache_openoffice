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

// includes --------------------------------------------------------------
#include <accessibility/standard/vclxaccessiblecheckbox.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

#include <vcl/button.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// VCLXAccessibleCheckBox
// -----------------------------------------------------------------------------

VCLXAccessibleCheckBox::VCLXAccessibleCheckBox( VCLXWindow* pVCLWindow )
	:VCLXAccessibleTextComponent( pVCLWindow )
{
	m_bChecked = IsChecked();
	m_bIndeterminate = IsIndeterminate();
}

// -----------------------------------------------------------------------------

VCLXAccessibleCheckBox::~VCLXAccessibleCheckBox()
{
}

// -----------------------------------------------------------------------------

bool VCLXAccessibleCheckBox::IsChecked()
{
	bool bChecked = false;

	VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
	if ( pVCLXCheckBox && pVCLXCheckBox->getState() == (sal_Int16) 1 )
		bChecked = true;

	return bChecked;
}

// -----------------------------------------------------------------------------

bool VCLXAccessibleCheckBox::IsIndeterminate()
{
	bool bIndeterminate = false;

	VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
	if ( pVCLXCheckBox && pVCLXCheckBox->getState() == (sal_Int16) 2 )
		bIndeterminate = true;

	return bIndeterminate;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleCheckBox::SetChecked( bool bChecked )
{
	if ( m_bChecked != bChecked )
	{
		Any aOldValue, aNewValue;
		if ( m_bChecked )
			aOldValue <<= AccessibleStateType::CHECKED;
		else
			aNewValue <<= AccessibleStateType::CHECKED;
		m_bChecked = bChecked;
		NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
	}
}

// -----------------------------------------------------------------------------

void VCLXAccessibleCheckBox::SetIndeterminate( bool bIndeterminate )
{
	if ( m_bIndeterminate != bIndeterminate )
	{
		Any aOldValue, aNewValue;
		if ( m_bIndeterminate )
			aOldValue <<= AccessibleStateType::INDETERMINATE;
		else
			aNewValue <<= AccessibleStateType::INDETERMINATE;
		m_bIndeterminate = bIndeterminate;
		NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
	}
}

// -----------------------------------------------------------------------------

void VCLXAccessibleCheckBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
	switch ( rVclWindowEvent.GetId() )
	{
		case VCLEVENT_CHECKBOX_TOGGLE:
		{
			SetChecked( IsChecked() );
			SetIndeterminate( IsIndeterminate() );
		}
		break;
		default:
			VCLXAccessibleTextComponent::ProcessWindowEvent( rVclWindowEvent );
	}
}

// -----------------------------------------------------------------------------

void VCLXAccessibleCheckBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
	VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

	rStateSet.AddState( AccessibleStateType::FOCUSABLE );

	if ( IsChecked() )
		rStateSet.AddState( AccessibleStateType::CHECKED );

	if ( IsIndeterminate() )
		rStateSet.AddState( AccessibleStateType::INDETERMINATE );
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleCheckBox, VCLXAccessibleTextComponent, VCLXAccessibleCheckBox_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleCheckBox, VCLXAccessibleTextComponent, VCLXAccessibleCheckBox_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleCheckBox::getImplementationName() throw (RuntimeException)
{
	return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleCheckBox" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleCheckBox::getSupportedServiceNames() throw (RuntimeException)
{
	Sequence< ::rtl::OUString > aNames(1);
	aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleCheckBox" );
	return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleCheckBox::getAccessibleActionCount( ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	return 1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleCheckBox::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
		throw IndexOutOfBoundsException();

	CheckBox* pCheckBox = (CheckBox*) GetWindow();
	VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
	if ( pCheckBox && pVCLXCheckBox )
	{
		sal_Int32 nValueMin = (sal_Int32) 0;
		sal_Int32 nValueMax = (sal_Int32) 1;

		if ( pCheckBox->IsTriStateEnabled() )
			nValueMax = (sal_Int32) 2;

		sal_Int32 nValue = (sal_Int32) pVCLXCheckBox->getState();

		++nValue;

		if ( nValue > nValueMax )
			nValue = nValueMin;

		pVCLXCheckBox->setState( (sal_Int16) nValue );
	}

	return sal_True;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleCheckBox::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
		throw IndexOutOfBoundsException();

	if(IsChecked())
		return ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_UNCHECK ) );
	else
		return ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CHECK ) );
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleCheckBox::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
	OExternalLockGuard aGuard( this );

	if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
		throw IndexOutOfBoundsException();

	OAccessibleKeyBindingHelper* pKeyBindingHelper = new OAccessibleKeyBindingHelper();
	Reference< XAccessibleKeyBinding > xKeyBinding = pKeyBindingHelper;

	Window* pWindow = GetWindow();
	if ( pWindow )
	{
		KeyEvent aKeyEvent = pWindow->GetActivationKey();
		KeyCode aKeyCode = aKeyEvent.GetKeyCode();
		if ( aKeyCode.GetCode() != 0 )
		{
			awt::KeyStroke aKeyStroke;
			aKeyStroke.Modifiers = 0;
			if ( aKeyCode.IsShift() )
				aKeyStroke.Modifiers |= awt::KeyModifier::SHIFT;
			if ( aKeyCode.IsMod1() )
				aKeyStroke.Modifiers |= awt::KeyModifier::MOD1;
			if ( aKeyCode.IsMod2() )
				aKeyStroke.Modifiers |= awt::KeyModifier::MOD2;
			if ( aKeyCode.IsMod3() )
				aKeyStroke.Modifiers |= awt::KeyModifier::MOD3;
			aKeyStroke.KeyCode = aKeyCode.GetCode();
			aKeyStroke.KeyChar = aKeyEvent.GetCharCode();
			aKeyStroke.KeyFunc = static_cast< sal_Int16 >( aKeyCode.GetFunction() );
			pKeyBindingHelper->AddKeyBinding( aKeyStroke );
		}
	}

	return xKeyBinding;
}

// -----------------------------------------------------------------------------
// XAccessibleValue
// -----------------------------------------------------------------------------

Any VCLXAccessibleCheckBox::getCurrentValue(  ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	Any aValue;

	VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
	if ( pVCLXCheckBox )
		aValue <<= (sal_Int32) pVCLXCheckBox->getState();

	return aValue;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleCheckBox::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	sal_Bool bReturn = sal_False;

	VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
	if ( pVCLXCheckBox )
	{
		sal_Int32 nValue = 0, nValueMin = 0, nValueMax = 0;
		OSL_VERIFY( aNumber >>= nValue );
		OSL_VERIFY( getMinimumValue() >>= nValueMin );
		OSL_VERIFY( getMaximumValue() >>= nValueMax );

		if ( nValue < nValueMin )
			nValue = nValueMin;
		else if ( nValue > nValueMax )
			nValue = nValueMax;

		pVCLXCheckBox->setState( (sal_Int16) nValue );
		bReturn = sal_True;
	}

	return bReturn;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleCheckBox::getMaximumValue(  ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	Any aValue;

	CheckBox* pCheckBox = (CheckBox*) GetWindow();
	if ( pCheckBox && pCheckBox->IsTriStateEnabled() )
		aValue <<= (sal_Int32) 2;
	else
		aValue <<= (sal_Int32) 1;

	return aValue;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleCheckBox::getMinimumValue(  ) throw (RuntimeException)
{
	OExternalLockGuard aGuard( this );

	Any aValue;
	aValue <<= (sal_Int32) 0;

	return aValue;
}

// -----------------------------------------------------------------------------
