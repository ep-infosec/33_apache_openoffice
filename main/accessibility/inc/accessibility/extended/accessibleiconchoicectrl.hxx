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



#ifndef ACCESSIBILITY_EXT_ACCESSIBLEICONCHOICECTRL_HXX_
#define ACCESSIBILITY_EXT_ACCESSIBLEICONCHOICECTRL_HXX_

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/implbase2.hxx>
#include <vcl/vclevent.hxx>
#include <toolkit/awt/vclxaccessiblecomponent.hxx>

// class AccessibleListBox -----------------------------------------------

class SvtIconChoiceCtrl;

//........................................................................
namespace accessibility
{
//........................................................................

	typedef ::cppu::ImplHelper2<  ::com::sun::star::accessibility::XAccessible
								, ::com::sun::star::accessibility::XAccessibleSelection> AccessibleIconChoiceCtrl_BASE;

	/** the class OAccessibleListBoxEntry represents the base class for an accessible object of a listbox entry
	*/
	class AccessibleIconChoiceCtrl	:public AccessibleIconChoiceCtrl_BASE
									,public VCLXAccessibleComponent
	{
	protected:
		::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > m_xParent;

	protected:
		virtual ~AccessibleIconChoiceCtrl();

		/** this function is called upon disposing the component */
		virtual void SAL_CALL	disposing();

		virtual void	ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
		virtual void	FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

		SvtIconChoiceCtrl* getCtrl();
	public:
		/** OAccessibleBase needs a valid view
			@param	_rIconCtrl
				is the box for which we implement an accessible object
			@param	_xParent
				is our parent accessible object
		*/
		AccessibleIconChoiceCtrl( SvtIconChoiceCtrl& _rIconCtrl,
								  const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& _xParent );

		// XTypeProvider
		DECLARE_XTYPEPROVIDER()

		// XInterface
		DECLARE_XINTERFACE()

		// XServiceInfo
		virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
		virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
		virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

		// XServiceInfo - static methods
		static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw(com::sun::star::uno::RuntimeException);
		static ::rtl::OUString getImplementationName_Static(void) throw(com::sun::star::uno::RuntimeException);

		// XAccessible
		virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (::com::sun::star::uno::RuntimeException);

		// XAccessibleContext
		virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
		virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
		virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
		virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
		virtual ::rtl::OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
		virtual ::rtl::OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);

		// XAccessibleSelection
		void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
		sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
		void SAL_CALL clearAccessibleSelection(  ) throw (::com::sun::star::uno::RuntimeException);
		void SAL_CALL selectAllAccessibleChildren(  ) throw (::com::sun::star::uno::RuntimeException);
		sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
		::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
		void SAL_CALL deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
	};

//........................................................................
}// namespace accessibility
//........................................................................

#endif // ACCESSIBILITY_EXT_ACCESSIBLEICONCHOICECTRL_HXX_

