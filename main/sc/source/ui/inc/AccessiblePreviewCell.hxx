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




#ifndef _SC_ACCESSIBLEPREVIEWCELL_HXX
#define _SC_ACCESSIBLEPREVIEWCELL_HXX

#include "AccessibleCellBase.hxx"

class ScPreviewShell;

namespace accessibility
{
    class AccessibleTextHelper;
}

class ScAccessiblePreviewCell : public ScAccessibleCellBase
{
public:
	//=====  internal  ========================================================
	ScAccessiblePreviewCell(
        const ::com::sun::star::uno::Reference<
	        ::com::sun::star::accessibility::XAccessible>& rxParent,
		ScPreviewShell* pViewShell, /* const */ ScAddress& rCellAddress, sal_Int32 nIndex );

protected:
	virtual ~ScAccessiblePreviewCell();

    using ScAccessibleCellBase::IsDefunc;

public:
    using ScAccessibleCellBase::disposing;
    virtual void SAL_CALL disposing();

	///=====  SfxListener  =====================================================

	virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    //=====  XAccessibleComponent  ============================================

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
    						getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint )
    							throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	grabFocus() throw (::com::sun::star::uno::RuntimeException);

	//=====  XAccessibleContext  ==============================================

    // overloaded to calculate this on demand
    virtual sal_Int32 SAL_CALL getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
    						getAccessibleChild( sal_Int32 i )
    							throw (::com::sun::star::lang::IndexOutOfBoundsException,
    								::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
    						getAccessibleStateSet() throw (::com::sun::star::uno::RuntimeException);

	//=====  XServiceInfo  ====================================================

	virtual ::rtl::OUString SAL_CALL getImplementationName()
								throw(::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
								throw(::com::sun::star::uno::RuntimeException);

	///=====  XTypeProvider  ===================================================

    /**	Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId(void)
        throw (::com::sun::star::uno::RuntimeException);

protected:
	virtual Rectangle GetBoundingBoxOnScreen(void) const throw(::com::sun::star::uno::RuntimeException);
	virtual Rectangle GetBoundingBox(void) const throw (::com::sun::star::uno::RuntimeException);

private:
	ScPreviewShell* mpViewShell;

	accessibility::AccessibleTextHelper* mpTextHelper;

	sal_Bool IsDefunc(
		const com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
	virtual sal_Bool IsEditable(
		const com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);
	sal_Bool IsOpaque(
		const com::sun::star::uno::Reference<
		::com::sun::star::accessibility::XAccessibleStateSet>& rxParentStates);

	void CreateTextHelper();

};

#endif

