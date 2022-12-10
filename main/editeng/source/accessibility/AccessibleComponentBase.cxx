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
#include "precompiled_editeng.hxx"


#include <editeng/AccessibleComponentBase.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <tools/color.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace	::com::sun::star::accessibility;

namespace accessibility {

//=====  internal  ============================================================

AccessibleComponentBase::AccessibleComponentBase (void)
{
}




AccessibleComponentBase::~AccessibleComponentBase (void)
{
}




//=====  XAccessibleComponent  ================================================

sal_Bool SAL_CALL AccessibleComponentBase::containsPoint (
        const ::com::sun::star::awt::Point& aPoint) 
        throw (::com::sun::star::uno::RuntimeException)
{
    awt::Size aSize (getSize());
    return (aPoint.X >= 0)
        && (aPoint.X < aSize.Width)
        && (aPoint.Y >= 0)
        && (aPoint.Y < aSize.Height);
}




uno::Reference<XAccessible > SAL_CALL 
    AccessibleComponentBase::getAccessibleAtPoint (
        const awt::Point& /*aPoint*/) 
    throw (uno::RuntimeException)
{
    return uno::Reference<XAccessible>();
}




awt::Rectangle SAL_CALL AccessibleComponentBase::getBounds (void)
    throw (uno::RuntimeException)
{
    return awt::Rectangle();
}




awt::Point SAL_CALL AccessibleComponentBase::getLocation (void) 
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Point (aBBox.X, aBBox.Y);
}




awt::Point SAL_CALL AccessibleComponentBase::getLocationOnScreen (void) 
    throw (::com::sun::star::uno::RuntimeException)
{
    return awt::Point();
}




::com::sun::star::awt::Size SAL_CALL AccessibleComponentBase::getSize (void) 
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox (getBounds());
    return awt::Size (aBBox.Width, aBBox.Height);
}




void SAL_CALL AccessibleComponentBase::addFocusListener (
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& /*xListener*/) 
    throw (::com::sun::star::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::removeFocusListener (const ::com::sun::star::uno::Reference<
    ::com::sun::star::awt::XFocusListener >& /*xListener*/ ) 
    throw (::com::sun::star::uno::RuntimeException)
{
    // Ignored
}




void SAL_CALL AccessibleComponentBase::grabFocus (void) 
    throw (::com::sun::star::uno::RuntimeException)
{
    uno::Reference<XAccessibleContext> xContext (this, uno::UNO_QUERY);
    uno::Reference<XAccessibleSelection> xSelection (
        xContext->getAccessibleParent(), uno::UNO_QUERY);
    if (xSelection.is())
    {
        // Do a single selection on this object.
        xSelection->clearAccessibleSelection();
        xSelection->selectAccessibleChild (xContext->getAccessibleIndexInParent());
    }
}




sal_Int32 SAL_CALL AccessibleComponentBase::getForeground (void) 
        throw (::com::sun::star::uno::RuntimeException)
{
    return Color(COL_BLACK).GetColor();
}




sal_Int32 SAL_CALL AccessibleComponentBase::getBackground (void) 
    throw (::com::sun::star::uno::RuntimeException)
{
    return Color(COL_WHITE).GetColor();
}




//=====  XAccessibleExtendedComponent  ========================================

::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL 
        AccessibleComponentBase::getFont (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return uno::Reference<awt::XFont>();
}




::rtl::OUString SAL_CALL AccessibleComponentBase::getTitledBorderText (void) 
        throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("");
}




::rtl::OUString SAL_CALL AccessibleComponentBase::getToolTipText (void) 
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii ("");
}




//=====  XTypeProvider  ===================================================
    
uno::Sequence<uno::Type> SAL_CALL
    AccessibleComponentBase::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation...
	uno::Sequence<uno::Type> aTypeList (2);
    // ...and add the additional type for the component.
    const uno::Type aComponentType = 
     	::getCppuType((const uno::Reference<XAccessibleComponent>*)0);
    const uno::Type aExtendedComponentType = 
    	::getCppuType((const uno::Reference<XAccessibleExtendedComponent>*)0);
    aTypeList[0] = aComponentType;
    aTypeList[1] = aExtendedComponentType;

	return aTypeList;
}


} // end of namespace accessibility
