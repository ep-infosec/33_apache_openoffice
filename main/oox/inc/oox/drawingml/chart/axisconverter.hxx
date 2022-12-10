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



#ifndef OOX_DRAWINGML_CHART_AXISCONVERTER_HXX
#define OOX_DRAWINGML_CHART_AXISCONVERTER_HXX

#include "oox/drawingml/chart/converterbase.hxx"

namespace com { namespace sun { namespace star {
    namespace chart2 { class XAxis; }
    namespace chart2 { class XCoordinateSystem; }
} } }

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

struct AxisModel;
class TypeGroupConverter;

class AxisConverter : public ConverterBase< AxisModel >
{
public:
    explicit            AxisConverter(
                            const ConverterRoot& rParent,
                            AxisModel& rModel );
    virtual             ~AxisConverter();

    /** Creates a chart2 axis and inserts it into the passed coordinate system. */
    void                convertFromModel(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >& rxCoordSystem,
                            TypeGroupConverter& rTypeGroup,
                            const AxisModel* pCrossingAxis,
                            sal_Int32 nAxesSetIdx,
                            sal_Int32 nAxisIdx );
};

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

#endif
