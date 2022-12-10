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


#ifndef CHART2_WRAPPEDDEFAULTPROPERTY_HXX
#define CHART2_WRAPPEDDEFAULTPROPERTY_HXX

#include "WrappedProperty.hxx"
#include "charttoolsdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS WrappedDefaultProperty : public WrappedProperty
{
public:
	explicit WrappedDefaultProperty(
        const ::rtl::OUString& rOuterName, const ::rtl::OUString& rInnerName,
        const ::com::sun::star::uno::Any& rNewOuterDefault );
	virtual ~WrappedDefaultProperty();

    virtual void setPropertyToDefault(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any getPropertyDefault(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState getPropertyState(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >& xInnerPropertyState ) const
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::uno::RuntimeException);
private:
    ::com::sun::star::uno::Any m_aOuterDefaultValue;
};

} //  namespace chart

// CHART2_WRAPPEDPROPERTYNEWDEFAULT_HXX
#endif
