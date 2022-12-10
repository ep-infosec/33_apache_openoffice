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

#ifndef SFX_SIDEBAR_PAINT_HXX
#define SFX_SIDEBAR_PAINT_HXX

#include <tools/color.hxx>
#include <vcl/gradient.hxx>
#include <vcl/wall.hxx>
#include <com/sun/star/awt/Gradient.hpp>

#include <boost/variant.hpp>

namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** Abstraction of different ways to fill outlines.
    Can be
     - none (empty: outline is not filled)
     - singular color
     - gradient
*/
class Paint
{
public:
    enum Type
    {
        NoPaint,
        ColorPaint,
        GradientPaint
    };

    // Create a Paint object for an Any that may contain a color, a
    // awt::Gradient, or nothing.
    static Paint Create (const cssu::Any& rValue);
    
    // Create paint with type NoPaint.
    explicit Paint (void);

    // Create a Paint object for the given color.
    explicit Paint (const Color& rColor);

    // Create a Paint object for the given gradient.
    explicit Paint (const Gradient& rGradient);

    void Set (const ::sfx2::sidebar::Paint& rOther);
    
    Type GetType (void) const;
    const Color& GetColor (void) const;
    const Gradient& GetGradient (void) const;

    Wallpaper GetWallpaper (void) const;

private:
    Type meType;
    ::boost::variant<
        Color,
        Gradient
    > maValue;
};


} } // end of namespace sfx2::sidebar

#endif
