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



#ifndef _DXCANVAS_DXIBITMAP_HXX
#define _DXCANVAS_DXIBITMAP_HXX

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <boost/shared_ptr.hpp>
#include <basegfx/vector/b2ivector.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include "dx_graphicsprovider.hxx"

namespace dxcanvas
{
    /// Interface for internal canvas bitmap objects
    struct IBitmap : public GraphicsProvider
    {
        virtual BitmapSharedPtr           getBitmap() const = 0;
        virtual ::basegfx::B2IVector      getSize() const = 0;
        virtual bool                      hasAlpha() const = 0;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > getData(
            ::com::sun::star::rendering::IntegerBitmapLayout&		bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&	rect ) = 0;

        virtual void setData(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&		data,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerRectangle2D&	rect ) = 0;

        virtual void setPixel(
            const ::com::sun::star::uno::Sequence< sal_Int8 >&		color,
            const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&		pos ) = 0;

        virtual ::com::sun::star::uno::Sequence< sal_Int8 > getPixel(
            ::com::sun::star::rendering::IntegerBitmapLayout&		bitmapLayout,
            const ::com::sun::star::geometry::IntegerPoint2D&		pos ) = 0;
    };

    typedef boost::shared_ptr<IBitmap> IBitmapSharedPtr;
}

#endif
