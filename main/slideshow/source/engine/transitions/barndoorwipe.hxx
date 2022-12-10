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



#if ! defined INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX
#define INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX

#include <basegfx/polygon/b2dpolygon.hxx>

#include "parametricpolypolygon.hxx"
#include "transitiontools.hxx"


namespace slideshow {
namespace internal {

/// Generate a barn door wipe or double barn door wipe:
class BarnDoorWipe : public ParametricPolyPolygon
{
public:
    BarnDoorWipe( bool doubled = false )
        : m_unitRect( createUnitRect() ), m_doubled(doubled) {}
    virtual ::basegfx::B2DPolyPolygon operator()( double x );
private:
    const ::basegfx::B2DPolygon m_unitRect;
    const bool m_doubled;
};

}
}

#endif /* INCLUDED_SLIDESHOW_BARNDOORWIPE_HXX */
