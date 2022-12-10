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



#include "bin.hxx"

#include <sal/macros.h>

namespace layoutimpl
{

using namespace css;

/* Bin */

Bin::Bin() : Container()
{
}

void SAL_CALL
Bin::addChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException, awt::MaxChildrenException)
{
    if ( mxChild.is() )
        throw awt::MaxChildrenException();
    if ( xChild.is() )
    {
        mxChild = xChild;
        setChildParent( xChild );
        queueResize();
    }
}

void SAL_CALL
Bin::removeChild( const uno::Reference< awt::XLayoutConstrains >& xChild )
    throw (uno::RuntimeException)
{
    if ( xChild == mxChild )
    {
        mxChild = uno::Reference< awt::XLayoutConstrains >();
        unsetChildParent( xChild );
        queueResize();
    }
}

uno::Sequence< uno::Reference< awt::XLayoutConstrains > > SAL_CALL
Bin::getChildren()
    throw (uno::RuntimeException)
{
    return getSingleChild (mxChild);
}

void SAL_CALL
Bin::allocateArea( const awt::Rectangle &rArea )
    throw (uno::RuntimeException)
{
    maAllocation = rArea;
    if ( mxChild.is() )
        allocateChildAt( mxChild, rArea );
}

awt::Size SAL_CALL
Bin::getMinimumSize()
    throw(uno::RuntimeException)
{
    if ( mxChild.is() )
        return maRequisition = maChildRequisition = mxChild->getMinimumSize();
    return maRequisition = awt::Size( 0, 0 );
}

uno::Reference< beans::XPropertySet > SAL_CALL
Bin::getChildProperties( const uno::Reference< awt::XLayoutConstrains >& )
    throw (uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySet >();
}

sal_Bool SAL_CALL
Bin::hasHeightForWidth()
    throw(uno::RuntimeException)
{
    uno::Reference< awt::XLayoutContainer > xChildCont( mxChild, uno::UNO_QUERY );
    if ( xChildCont.is() )
        return xChildCont->hasHeightForWidth();
    return false;
}

sal_Int32 SAL_CALL
Bin::getHeightForWidth( sal_Int32 nWidth )
    throw(uno::RuntimeException)
{
    uno::Reference< awt::XLayoutContainer > xChildCont( mxChild, uno::UNO_QUERY );
    if ( xChildCont.is() )
        return xChildCont->getHeightForWidth( nWidth );
    return maRequisition.Height;
}

/* Align */

Align::Align() : Bin()
{
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Halign" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fHorAlign );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Valign" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fVerAlign );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Hfill" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fHorFill );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "Vfill" ),
             ::getCppuType( static_cast< const float* >( NULL ) ),
             &fVerFill );

    fHorAlign = fVerAlign = 0.5;
    fHorFill = fVerFill = 0;
}

void SAL_CALL
Align::allocateArea( const awt::Rectangle &rArea )
    throw (uno::RuntimeException)
{
    maAllocation = rArea;
    if ( !mxChild.is() )
        return;

    awt::Rectangle aChildArea;
    aChildArea.Width = SAL_MIN( rArea.Width, maChildRequisition.Width );
    aChildArea.Width += (sal_Int32) SAL_MAX(
        0, (rArea.Width - maChildRequisition.Width) * fHorFill );
    aChildArea.Height = SAL_MIN( rArea.Height, maChildRequisition.Height );
    aChildArea.Height += (sal_Int32) SAL_MAX(
        0, (rArea.Height - maChildRequisition.Height) * fVerFill );

    aChildArea.X = rArea.X + (sal_Int32)( (rArea.Width - aChildArea.Width) * fHorAlign );
    aChildArea.Y = rArea.Y + (sal_Int32)( (rArea.Height - aChildArea.Height) * fVerAlign );

    allocateChildAt( mxChild, aChildArea );
}

bool
Align::emptyVisible ()
{
    return true;
}

/* MinSize */

MinSize::MinSize() : Bin()
{
    mnMinWidth = mnMinHeight = 0;
    addProp( RTL_CONSTASCII_USTRINGPARAM( "MinWidth" ),
             ::getCppuType( static_cast< const long* >( NULL ) ),
             &mnMinWidth );
    addProp( RTL_CONSTASCII_USTRINGPARAM( "MinHeight" ),
             ::getCppuType( static_cast< const long* >( NULL ) ),
             &mnMinHeight );
}

bool
MinSize::emptyVisible ()
{
    return true;
}

awt::Size SAL_CALL MinSize::getMinimumSize()
    throw(uno::RuntimeException)
{
    Bin::getMinimumSize();
    maRequisition.Width = SAL_MAX( maRequisition.Width, mnMinWidth );
    maRequisition.Height = SAL_MAX( maRequisition.Height, mnMinHeight );
    return maRequisition;
}

} // namespace layoutimpl
