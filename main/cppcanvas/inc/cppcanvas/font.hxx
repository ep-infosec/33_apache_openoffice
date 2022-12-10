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



#ifndef _CPPCANVAS_FONT_HXX
#define _CPPCANVAS_FONT_HXX

#include <com/sun/star/uno/Reference.hxx>

#include <boost/shared_ptr.hpp>

namespace rtl
{
    class OUString;
}

namespace com { namespace sun { namespace star { namespace rendering 
{
    class  XCanvasFont;
} } } }

/* Definition of Font class */

namespace cppcanvas
{

    class Font
    {
    public:
        virtual ~Font() {}

        virtual ::rtl::OUString getName() const = 0;
        virtual double 			getCellSize() const = 0;

        virtual ::com::sun::star::uno::Reference< 
            ::com::sun::star::rendering::XCanvasFont > getUNOFont() const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::Font > FontSharedPtr;
}

#endif /* _CPPCANVAS_FONT_HXX */
