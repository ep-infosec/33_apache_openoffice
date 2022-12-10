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


#ifndef CHART_CHARACTERPROPERTYITEMCONVERTER_HXX
#define CHART_CHARACTERPROPERTYITEMCONVERTER_HXX

#include "ItemConverter.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <memory>

namespace chart
{
namespace wrapper
{

class CharacterPropertyItemConverter :
        public ::comphelper::ItemConverter
{
public:
    CharacterPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool );
    CharacterPropertyItemConverter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rPropertySet,
        SfxItemPool& rItemPool,
        ::std::auto_ptr< ::com::sun::star::awt::Size > pRefSize,
        const ::rtl::OUString & rRefSizePropertyName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & rRefSizePropSet =
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >() );
    virtual ~CharacterPropertyItemConverter();

protected:
    virtual const sal_uInt16 * GetWhichPairs() const;
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

    virtual void FillSpecialItem( sal_uInt16 nWhichId, SfxItemSet & rOutItemSet ) const
        throw( ::com::sun::star::uno::Exception );
    virtual bool ApplySpecialItem( sal_uInt16 nWhichId, const SfxItemSet & rItemSet )
        throw( ::com::sun::star::uno::Exception );

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >  GetRefSizePropertySet() const;

private:
    ::std::auto_ptr< ::com::sun::star::awt::Size > m_pRefSize;
    ::rtl::OUString                                m_aRefSizePropertyName;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >    m_xRefSizePropSet;
};

} //  namespace wrapper
} //  namespace chart

// CHART_CHARACTERPROPERTYITEMCONVERTER_HXX
#endif
