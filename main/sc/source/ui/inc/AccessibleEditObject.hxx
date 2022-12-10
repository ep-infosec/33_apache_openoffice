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




#ifndef _SC_ACCESSIBLEEDITOBJECT_HXX
#define _SC_ACCESSIBLEEDITOBJECT_HXX

#include "AccessibleContextBase.hxx"

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include "global.hxx"
namespace accessibility
{
    class AccessibleTextHelper;
}
class EditView;
class Window;

enum EditObjectType
{
    CellInEditMode,
    EditLine,
    EditControl
};

/**	@descr
        This base class provides an implementation of the
        <code>AccessibleCell</code> service.
*/
class ScAccessibleEditObject
	:	public	ScAccessibleContextBase,
        public ::com::sun::star::accessibility::XAccessibleSelection 
{
public:
	//=====  internal  ========================================================
	ScAccessibleEditObject(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
        EditView* pEditView, Window* pWin, const rtl::OUString& rName, 
        const rtl::OUString& rDescription, EditObjectType eObjectType);

protected:
	virtual ~ScAccessibleEditObject();

    using ScAccessibleContextBase::IsDefunc;

public:
    using ScAccessibleContextBase::addEventListener;
    using ScAccessibleContextBase::removeEventListener;
    using ScAccessibleContextBase::disposing;

    virtual void SAL_CALL disposing();

    virtual void LostFocus();

    virtual void GotFocus();
///=====  XInterface  =====================================================

	virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
		::com::sun::star::uno::Type const & rType )
		throw (::com::sun::star::uno::RuntimeException);

	virtual void SAL_CALL acquire() throw ();

	virtual void SAL_CALL release() throw ();
	///=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
		SAL_CALL getAccessibleAtPoint(
		const ::com::sun::star::awt::Point& rPoint )
		throw (::com::sun::star::uno::RuntimeException);

protected:
    ///	Return the object's current bounding box relative to the desktop.
	virtual Rectangle GetBoundingBoxOnScreen(void) const
		throw (::com::sun::star::uno::RuntimeException);

	///	Return the object's current bounding box relative to the parent object.
	virtual Rectangle GetBoundingBox(void) const
		throw (::com::sun::star::uno::RuntimeException);

public:
	///=====  XAccessibleContext  ==============================================

    ///	Return the number of currently visible children.
    // is overloaded to calculate this on demand
    virtual sal_Int32 SAL_CALL
    	getAccessibleChildCount(void)
    				throw (::com::sun::star::uno::RuntimeException);

    ///	Return the specified child or NULL if index is invalid.
    // is overloaded to calculate this on demand
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
    	getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
				::com::sun::star::lang::IndexOutOfBoundsException);

	///	Return the set of current states.
	virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
    	getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

	//=====  XAccessibleSelection  ============================================
		
		virtual void SAL_CALL selectAccessibleChild( 
			sal_Int32 nChildIndex ) 
			throw ( ::com::sun::star::lang::IndexOutOfBoundsException, 
			::com::sun::star::uno::RuntimeException );
		virtual sal_Bool SAL_CALL isAccessibleChildSelected( 
			sal_Int32 nChildIndex ) 
			throw ( ::com::sun::star::lang::IndexOutOfBoundsException, 
			::com::sun::star::uno::RuntimeException );
		virtual void SAL_CALL clearAccessibleSelection(  ) 
			throw ( ::com::sun::star::uno::RuntimeException );
		virtual void SAL_CALL selectAllAccessibleChildren(  ) 
			throw ( ::com::sun::star::uno::RuntimeException );
		virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) 
			throw ( ::com::sun::star::uno::RuntimeException );
		virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( 
			sal_Int32 nSelectedChildIndex ) 
			throw ( ::com::sun::star::lang::IndexOutOfBoundsException, 
			::com::sun::star::uno::RuntimeException);
		virtual void SAL_CALL deselectAccessibleChild( 
			sal_Int32 nSelectedChildIndex ) 
			throw ( ::com::sun::star::lang::IndexOutOfBoundsException, 
			::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    ///	Return this object's description.
	virtual ::rtl::OUString SAL_CALL
    	createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///	Return the object's current name.
	virtual ::rtl::OUString SAL_CALL
    	createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

public:
	///=====  XAccessibleEventBroadcaster  =====================================

    /**	Add listener that is informed of future changes of name,
      	description and so on events.
    */
	virtual void SAL_CALL
    	addEventListener(
        	const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    //	Remove an existing event listener.
	virtual void SAL_CALL
    	removeEventListener(
			const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& xListener)
        throw (com::sun::star::uno::RuntimeException);

    ///=====  XServiceInfo  ====================================================

    /**	Returns an identifier for the implementation of this object.
    */
	virtual ::rtl::OUString SAL_CALL
    	getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

	///=====  XTypeProvider  ===================================================

    /**	Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

private:
	accessibility::AccessibleTextHelper* mpTextHelper;
    EditView* mpEditView;
    Window* mpWindow;
    EditObjectType meObjectType;
    sal_Bool mbHasFocus;

	sal_Bool IsDefunc(
		const com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

	void CreateTextHelper();
	ScDocument *m_pScDoc;
	ScAddress m_curCellAddress;


	///=====  XAccessibleComponent  ============================================
    virtual sal_Int32 SAL_CALL getForeground(  ) 
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  ) 
        throw (::com::sun::star::uno::RuntimeException);

	sal_Int32 GetFgBgColor(  const rtl::OUString &strPropColor) ;
};


#endif
