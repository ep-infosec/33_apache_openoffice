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



#ifndef INCLUDED_CANVAS_ICACHEDPRIMITIVE_HXX
#define INCLUDED_CANVAS_ICACHEDPRIMITIVE_HXX

#include <sal/types.h>

#include <boost/shared_ptr.hpp>


namespace basegfx
{
    class B2IRange;
    class B2IPoint;
}

namespace canvas
{
    /** Objects with this interface are returned from every Bitmap
        render operation.

        These objects can be used to implement the
        rendering::XCachedPrimitive interface, which in turn caches
        render state and objects to facilitate quick redraws.
     */
	struct ICachedPrimitive
	{
        virtual ~ICachedPrimitive() {}

        /** Redraw the primitive with the given view state

            Note that the primitive will <em>always</em> be redrawn on
            the bitmap it was created from.
         */
        virtual sal_Int8 redraw( const ::com::sun::star::rendering::ViewState& aState ) const = 0;
	};

    typedef ::boost::shared_ptr< ICachedPrimitive > ICachedPrimitiveSharedPtr;
}

#endif /* INCLUDED_CANVAS_ICACHEDPRIMITIVE_HXX */
