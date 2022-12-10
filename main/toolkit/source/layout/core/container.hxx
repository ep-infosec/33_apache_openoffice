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



#ifndef LAYOUT_CORE_CONTAINER_HXX
#define LAYOUT_CORE_CONTAINER_HXX

#include <layout/core/helper.hxx>

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/awt/MaxChildrenException.hpp>

namespace layoutimpl
{
namespace css = ::com::sun::star;

typedef ::cppu::WeakImplHelper2< css::awt::XLayoutContainer,
                                 css::awt::XLayoutConstrains > Container_Base;

class TOOLKIT_DLLPUBLIC Container : public Container_Base, public PropHelper, public PropHelper::Listener
{
    friend class ChildProps;
protected:
    // Widget properties
    css::uno::Reference< css::awt::XLayoutContainer > mxParent;
    css::uno::Reference< css::awt::XLayoutUnit > mxLayoutUnit;
    css::awt::Size maRequisition;
    css::awt::Rectangle maAllocation;

    // Container properties
    sal_Int32 mnBorderWidth;

    // Utilities
    void allocateChildAt( const css::uno::Reference< css::awt::XLayoutConstrains > &xChild,
                          const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException);
    static css::uno::Sequence< css::uno::Reference< css::awt::XLayoutConstrains > >
    getSingleChild (const css::uno::Reference< css::awt::XLayoutConstrains > &xChildOrNil);
    void setChildParent( const css::uno::Reference< css::awt::XLayoutConstrains >& xChild );
    void unsetChildParent( const css::uno::Reference< css::awt::XLayoutConstrains >& xChild );

    void queueResize();
    void forceRecalc() { allocateArea( maAllocation ); }

public:
    Container();
    virtual ~Container() {}

    virtual bool emptyVisible ();

    // XInterface
    virtual void SAL_CALL acquire() throw() { PropHelper::acquire(); }
    virtual void SAL_CALL release() throw() { PropHelper::release(); }
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // css::awt::XLayoutContainer
    virtual void SAL_CALL addChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException, css::awt::MaxChildrenException) = 0;
    virtual void SAL_CALL removeChild( const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException) = 0;

    virtual css::uno::Sequence< css::uno::Reference
                                < css::awt::XLayoutConstrains > > SAL_CALL getChildren()
        throw (css::uno::RuntimeException) = 0;

    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL getChildProperties(
        const css::uno::Reference< css::awt::XLayoutConstrains >& Child )
        throw (css::uno::RuntimeException) = 0;

    virtual void SAL_CALL allocateArea( const css::awt::Rectangle &rArea )
        throw (css::uno::RuntimeException) = 0;

    void SAL_CALL setLayoutUnit( const css::uno::Reference< css::awt::XLayoutUnit > &xUnit )
        throw(css::uno::RuntimeException)
    { mxLayoutUnit = xUnit; }
    css::uno::Reference< css::awt::XLayoutUnit > SAL_CALL getLayoutUnit()
        throw(css::uno::RuntimeException)
    { return mxLayoutUnit; }

    css::awt::Size SAL_CALL getRequestedSize() throw(css::uno::RuntimeException)
    { return maRequisition; }
    com::sun::star::awt::Rectangle SAL_CALL getAllocatedArea() throw(css::uno::RuntimeException)
    { return maAllocation; }

    virtual sal_Bool SAL_CALL hasHeightForWidth()
        throw(css::uno::RuntimeException) = 0;
    virtual sal_Int32 SAL_CALL getHeightForWidth( sal_Int32 nWidth )
        throw(css::uno::RuntimeException) = 0;

    // css::awt::XLayoutContainer: css::container::XChild
    css::uno::Reference< css::uno::XInterface > SAL_CALL getParent()
        throw (css::uno::RuntimeException)
    { return mxParent; }
    void SAL_CALL setParent( const css::uno::Reference< css::uno::XInterface > &xParent )
        throw (css::uno::RuntimeException)
    { mxParent = css::uno::Reference< css::awt::XLayoutContainer >( xParent, css::uno::UNO_QUERY ); }

    // css::awt::XLayoutConstrains
    virtual css::awt::Size SAL_CALL getMinimumSize()
        throw(css::uno::RuntimeException) = 0;
    // (not properly implemented in toolkit, ignore it.)
    css::awt::Size SAL_CALL getPreferredSize()
        throw(css::uno::RuntimeException) { return getMinimumSize(); } // TODO: use this for flow?
    css::awt::Size SAL_CALL calcAdjustedSize( const css::awt::Size& rNewSize )
        throw(css::uno::RuntimeException) { return rNewSize; }

protected:
    void propertiesChanged();
};

} //  namespace layoutimpl

#endif /* LAYOUT_CORE_CONTAINER_HXX */
