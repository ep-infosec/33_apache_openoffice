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


#ifndef CHART_COLUMNCHARTTYPE_HXX
#define CHART_COLUMNCHARTTYPE_HXX

#include "ChartType.hxx"
#include "ServiceMacros.hxx"

namespace chart
{

class ColumnChartType : public ChartType
{
public:
	ColumnChartType(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > const & xContext );
	virtual ~ColumnChartType();

    APPHELPER_XSERVICEINFO_DECL()

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ColumnChartType )

protected:
    explicit ColumnChartType( const ColumnChartType & rOther );

    // ____ XChartType ____
    virtual ::rtl::OUString SAL_CALL getChartType()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);
};

} //  namespace chart

// CHART_COLUMNCHARTTYPE_HXX
#endif
