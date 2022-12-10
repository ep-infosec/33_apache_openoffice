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



#ifndef _SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX
#define _SD_ACCESSIBILITY_ACCESSIBLE_DRAW_DOCUMENT_VIEW_HXX

#include "AccessibleDocumentViewBase.hxx"

#include <com/sun/star/accessibility/XAccessibleGroupPosition.hpp>

namespace accessibility {


/** This class makes draw documents in the general view modes
    accessible.  It passes all shapes on the current draw page to a
    children manager and additionally creates a new shape that
    represents the actual draw page.

    Please see the documentation of the base class for further
    explanations of the individual methods.
*/
class AccessibleDrawDocumentView : 
    public AccessibleDocumentViewBase
    ,public ::com::sun::star::accessibility::XAccessibleGroupPosition
{
public:
    //=====  internal  ========================================================

    AccessibleDrawDocumentView (::sd::Window* pSdWindow,
        ::sd::ViewShell* pViewShell,
	    const ::com::sun::star::uno::Reference<
	    ::com::sun::star::frame::XController>& rxController,
	    const ::com::sun::star::uno::Reference<
	    ::com::sun::star::accessibility::XAccessible>& rxParent);

    virtual ~AccessibleDrawDocumentView	(void);

    /** Complete the initialization begun in the constructor.
    */
    virtual void Init (void);


    //=====  IAccessibleViewForwarderListener  ================================

    virtual void ViewForwarderChanged (ChangeType aChangeType, 
        const IAccessibleViewForwarder* pViewForwarder);

    //=====  XAccessibleContext  ==============================================

    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount (void)
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::IndexOutOfBoundsException);

	virtual ::rtl::OUString SAL_CALL
		getAccessibleName(void)
		throw (::com::sun::star::uno::RuntimeException);

    //=====  lang::XEventListener  ============================================

	virtual void SAL_CALL
		disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XPropertyChangeListener  =========================================

	virtual void SAL_CALL
		propertyChange (const ::com::sun::star::beans::PropertyChangeEvent& rEventObject)
        throw (::com::sun::star::uno::RuntimeException);
    //=====  XInterface  ======================================================
    
    virtual com::sun::star::uno::Any SAL_CALL
		queryInterface (const com::sun::star::uno::Type & rType)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        acquire (void) 
        throw ();
        
    virtual void SAL_CALL
        release (void)
        throw ();
	
    //=====  XAccessibleGroupPosition  =========================================
	virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL
		getGroupPosition( const ::com::sun::star::uno::Any& rAny )
        throw (::com::sun::star::uno::RuntimeException);
	virtual ::rtl::OUString SAL_CALL getObjectLink( const ::com::sun::star::uno::Any& accoject )
        throw (::com::sun::star::uno::RuntimeException);	

protected:

    //=====  XServiceInfo  ====================================================
    
    virtual ::rtl::OUString SAL_CALL
    	getImplementationName (void)
	    throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);


	virtual sal_Bool 
        implIsSelected( sal_Int32 nAccessibleChildIndex ) 
        throw (::com::sun::star::uno::RuntimeException);

    /** Select or deselect the specified child or all children if the given
        index has the special value ACCESSIBLE_SELECTION_CHILD_ALL.
        Selecting or deselecting a child sets or resets the
        <const>SELECTED</const> state and selects or deslects the UNO shape
        being made accessible by the child.
        @param nAccessibleChildIndex
            Index of the child to select or deselect.  If the parameter has
            the value ACCESSIBLE_SELECTION_CHILD_ALL then all children are
            selected or deslected.
        @param bSelect
            Indicates whether to select or deselect the specified child
            reps. children.
    */
    virtual void
        implSelect( sal_Int32 nAccessibleChildIndex, sal_Bool bSelect ) 
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
private:
	::sd::ViewShell* mpSdViewSh;

protected:
    /** This object manages the shapes of the represented draw page.  It is
        responsible to determine the visible shapes and create on demand the
        accessible objects representing them.
    */
    ChildrenManager* mpChildrenManager;

    // This method is called from the component helper base class while
    // disposing.
    virtual void SAL_CALL disposing (void);

    /** Create a shape the represents the page as seen on the screen.
    */
    AccessiblePageShape* CreateDrawPageShape (void);

    ///	Create an accessible name that contains the current view mode.
    virtual ::rtl::OUString
    	CreateAccessibleName ()
        throw (::com::sun::star::uno::RuntimeException);

    /**	Create an accessible description that contains the current
    	view mode.
    */
    virtual ::rtl::OUString
    	CreateAccessibleDescription ()
        throw (::com::sun::star::uno::RuntimeException);

    /** Make sure that the currently focused shape sends a FOCUSED state
        change event indicating that it has (regained) the focus.
    */
    virtual void Activated (void);

    /** Make sure that the currently focused shape sends a FOCUSED state
        change event indicating that it has lost the focus.
    */
    virtual void Deactivated (void);

    virtual void impl_dispose (void);

    //=====  XAccessibleGetAccFromXShape  ============================================
	::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
		SAL_CALL get_AccFlowTo(const ::com::sun::star::uno::Any& rAny, sal_Int32 nType)
		throw ( ::com::sun::star::uno::RuntimeException );
	::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
		GetSelAccContextInTable();
    
private:
    void UpdateAccessibleName (void);
};

} // end of namespace accessibility

#endif
