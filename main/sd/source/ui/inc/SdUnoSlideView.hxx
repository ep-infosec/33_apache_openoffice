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



#ifndef SD_UNO_SLIDE_VIEW_HXX
#define SD_UNO_SLIDE_VIEW_HXX

#include "DrawSubController.hxx"
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/drawing/XDrawPage.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace controller {
class PageSelector;
} } }

namespace sd {

class DrawController;
class SlideViewShell;
class View;


/** This class implements the SlideSorter specific part of the
    controller.
 */
class SdUnoSlideView 
    : private cppu::BaseMutex,
      public DrawSubControllerInterfaceBase
{
public:
	SdUnoSlideView (
        DrawController& rController,
        slidesorter::SlideSorter& rSlideSorter,
        View& rView) throw();
	virtual ~SdUnoSlideView (void) throw();

    // XSelectionSupplier

    virtual sal_Bool SAL_CALL select (const ::com::sun::star::uno::Any& aSelection)
        throw(::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException);
    
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection (void)
        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException);
    
    virtual void SAL_CALL removeSelectionChangeListener (
        const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
        throw(css::uno::RuntimeException);

    
	// XDrawView
    
    virtual void SAL_CALL setCurrentPage (
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xPage)
        throw(::com::sun::star::uno::RuntimeException);
    
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
        getCurrentPage (void)
        throw(::com::sun::star::uno::RuntimeException);


    // XFastPropertySet

    virtual void SAL_CALL setFastPropertyValue (
        sal_Int32 nHandle,
        const css::uno::Any& rValue)
        throw(css::beans::UnknownPropertyException,
            css::beans::PropertyVetoException,
            css::lang::IllegalArgumentException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);
    
    virtual css::uno::Any SAL_CALL getFastPropertyValue (
        sal_Int32 nHandle)
        throw(css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException);

	// XServiceInfo
	virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

private:
    DrawController& mrController;
    slidesorter::SlideSorter& mrSlideSorter;
    sd::View& mrView;
    /*
    virtual void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);
	virtual sal_Bool SAL_CALL convertFastPropertyValue(
		::com::sun::star::uno::Any & rConvertedValue,
		::com::sun::star::uno::Any & rOldValue,
		sal_Int32 nHandle,
		const ::com::sun::star::uno::Any& rValue )
		throw (::com::sun::star::lang::IllegalArgumentException);
	virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
		sal_Int32 nHandle,
		const ::com::sun::star::uno::Any& rValue )
		throw (::com::sun::star::uno::Exception);
	virtual void SAL_CALL getFastPropertyValue(
		::com::sun::star::uno::Any& rValue,
		sal_Int32 nHandle ) const;
    */
};

} // end of namespace sd

#endif
