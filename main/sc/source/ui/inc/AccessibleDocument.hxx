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




#ifndef _SC_ACCESSIBLEDOCUMENT_HXX
#define _SC_ACCESSIBLEDOCUMENT_HXX

#include "AccessibleDocumentBase.hxx"
#include "viewdata.hxx"
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/accessibility/XAccessibleExtendedAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleGetAccFlowTo.hpp>
#include <svx/IAccessibleViewForwarder.hxx>

class ScTabViewShell;
class ScAccessibleSpreadsheet;
class ScChildrenShapes;
class ScAccessibleEditObject;

namespace accessibility
{
	class AccessibleShape;
}
namespace utl
{
    class AccessibleRelationSetHelper;
}

/**	@descr
        This base class provides an implementation of the
        <code>AccessibleContext</code> service.
*/

typedef cppu::ImplHelper3< ::com::sun::star::accessibility::XAccessibleSelection,
							::com::sun::star::accessibility::XAccessibleExtendedAttributes,
							::com::sun::star::view::XSelectionChangeListener >
					ScAccessibleDocumentImpl;

class ScAccessibleDocument
	:	public ScAccessibleDocumentBase,
		public ScAccessibleDocumentImpl,
	       public com::sun::star::accessibility::XAccessibleGetAccFlowTo,
        public accessibility::IAccessibleViewForwarder
{
public:
	//=====  internal  ========================================================
	ScAccessibleDocument(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& rxParent,
		ScTabViewShell* pViewShell,
		ScSplitPos eSplitPos);
	
	virtual void Init();


	DECL_LINK( WindowChildEventListener, VclSimpleEvent* );
protected:
	virtual ~ScAccessibleDocument(void);

    using ScAccessibleDocumentBase::IsDefunc;

public:

 	virtual void SAL_CALL disposing();

   ///=====  SfxListener  =====================================================

	virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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

    virtual void SAL_CALL grabFocus(  )
		throw (::com::sun::star::uno::RuntimeException);

	///=====  XAccessibleContext  ==============================================

    ///	Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
    	getAccessibleChildCount(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///	Return the specified child or NULL if index is invalid.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
    	getAccessibleChild(sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException,
				::com::sun::star::lang::IndexOutOfBoundsException);

    ///	Return the set of current states.
	virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
    	getAccessibleStateSet(void)
        throw (::com::sun::star::uno::RuntimeException);

	virtual ::rtl::OUString SAL_CALL
		getAccessibleName(void)
		throw (::com::sun::star::uno::RuntimeException);

	virtual ::com::sun::star::uno::Any SAL_CALL getExtendedAttributes() 
		throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException) ;
	///=====  XAccessibleSelection  ===========================================

    virtual void SAL_CALL 
		selectAccessibleChild( sal_Int32 nChildIndex ) 
		throw (::com::sun::star::lang::IndexOutOfBoundsException, 
		::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL 
		isAccessibleChildSelected( sal_Int32 nChildIndex ) 
		throw (::com::sun::star::lang::IndexOutOfBoundsException,
		::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL 
		clearAccessibleSelection(  ) 
		throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL 
		selectAllAccessibleChildren(  ) 
		throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL 
		getSelectedAccessibleChildCount(  ) 
		throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessible > SAL_CALL 
		getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) 
		throw (::com::sun::star::lang::IndexOutOfBoundsException,
		::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL 
		deselectAccessibleChild( sal_Int32 nChildIndex ) 
		throw (::com::sun::star::lang::IndexOutOfBoundsException,
		::com::sun::star::uno::RuntimeException);

	///=====  XSelectionListener  =============================================

    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) 
		throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
		throw (::com::sun::star::uno::RuntimeException);

	///=====  XServiceInfo  ===================================================

    /**	Returns an identifier for the implementation of this object.
    */
	virtual ::rtl::OUString SAL_CALL
    	getImplementationName(void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Returns a list of all supported services.
    */
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    	getSupportedServiceNames(void)
        throw (::com::sun::star::uno::RuntimeException);

	///=====  XTypeProvider  ===================================================

	/// returns the possible types
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL 
		getTypes() 
		throw (::com::sun::star::uno::RuntimeException);

	/**	Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

	///=====  IAccessibleViewForwarder  ========================================

    /** This method informs you about the state of the forwarder.  Do not
        use it when the returned value is <false/>.

    	@return
            Return <true/> if the view forwarder is valid and <false/> else.
     */
	virtual sal_Bool IsValid (void) const;

    /** Returns the area of the underlying document that is visible in the
    * corresponding window.

    	@return 
            The rectangle of the visible part of the document. The values
            are, contrary to the base class, in internal coordinates.
     */
    virtual Rectangle GetVisibleArea() const;

    /** Transform the specified point from internal coordinates to an
        absolute screen position.

    	@param rPoint
            Point in internal coordinates.

        @return 
            The same point but in screen coordinates relative to the upper
            left corner of the (current) screen.
     */
    virtual Point LogicToPixel (const Point& rPoint) const;

    /** Transform the specified size from internal coordinates to a screen
    * oriented pixel size.

    	@param rSize
            Size in internal coordinates.

        @return 
            The same size but in screen coordinates.
     */
    virtual Size LogicToPixel (const Size& rSize) const;

    /** Transform the specified point from absolute screen coordinates to
        internal coordinates.

    	@param rPoint
            Point in screen coordinates relative to the upper left corner of
            the (current) screen.

        @return 
            The same point but in internal coordinates.
     */
    virtual Point PixelToLogic (const Point& rPoint) const;

    /** Transform the specified size from screen coordinates to internal
        coordinates.

    	@param rSize
            Size in screen coordinates.

        @return
            The same size but in internal coordinates.
     */
    virtual Size PixelToLogic (const Size& rSize) const;

    ///======== internal =====================================================
    
    utl::AccessibleRelationSetHelper* GetRelationSet(const ScAddress* pAddress) const;

	::com::sun::star::uno::Reference
		< ::com::sun::star::accessibility::XAccessible >
		GetAccessibleSpreadsheet();

protected:
    ///	Return this object's description.
	virtual ::rtl::OUString SAL_CALL
    	createAccessibleDescription(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///	Return the object's current name.
	virtual ::rtl::OUString SAL_CALL
    	createAccessibleName(void)
        throw (::com::sun::star::uno::RuntimeException);

    ///	Return the object's current bounding box relative to the desktop.
	virtual Rectangle GetBoundingBoxOnScreen(void) const
		throw (::com::sun::star::uno::RuntimeException);

	///	Return the object's current bounding box relative to the parent object.
	virtual Rectangle GetBoundingBox(void) const
		throw (::com::sun::star::uno::RuntimeException);

private:
	ScTabViewShell*	mpViewShell;
	ScSplitPos		meSplitPos;
	ScAccessibleSpreadsheet* mpAccessibleSpreadsheet;
    ScChildrenShapes* mpChildrenShapes;
    ScAccessibleEditObject* mpTempAccEdit;
    com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessible> mxTempAcc;
    Rectangle maVisArea;
	sal_Bool mbCompleteSheetSelected;

public:
    SCTAB getVisibleTable() const; // use it in ScChildrenShapes

private:
	void FreeAccessibleSpreadsheet();

	sal_Bool IsTableSelected() const;

    sal_Bool IsDefunc(
		const com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
	sal_Bool IsEditable(
		const com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

    void AddChild(const com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessible>& xAcc, sal_Bool bFireEvent);
    void RemoveChild(const com::sun::star::uno::Reference<com::sun::star::accessibility::XAccessible>& xAcc, sal_Bool bFireEvent);

    rtl::OUString GetCurrentCellName() const;
    rtl::OUString GetCurrentCellDescription() const;

    Rectangle GetVisibleArea_Impl() const;
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > GetScAccFlowToSequence();
public:
	ScDocument *GetDocument() const ;
	ScAddress   GetCurCellAddress() const;
	//=====  XAccessibleGetAccFromXShape  ============================================
	::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
		SAL_CALL get_AccFlowTo(const ::com::sun::star::uno::Any& rAny, sal_Int32 nType)
		throw ( ::com::sun::star::uno::RuntimeException );
	
	 virtual sal_Int32 SAL_CALL getForeground(  ) 
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getBackground(  ) 
        throw (::com::sun::star::uno::RuntimeException);
protected:
	void SwitchViewFireFocus();
};


#endif
