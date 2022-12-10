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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedNumberFormatProperty.hxx"
#include "macros.hxx"

// header for define DBG_ERROR
#include <tools/debug.hxx>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{
//.............................................................................

WrappedNumberFormatProperty::WrappedNumberFormatProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedDirectStateProperty( C2U("NumberFormat"), C2U("NumberFormat") )
        , m_spChart2ModelContact(spChart2ModelContact)
{
    m_aOuterValue = getPropertyDefault( 0 );
}

WrappedNumberFormatProperty::~WrappedNumberFormatProperty()
{
    if( m_pWrappedLinkNumberFormatProperty )
    {
        if( m_pWrappedLinkNumberFormatProperty->m_pWrappedNumberFormatProperty == this )
            m_pWrappedLinkNumberFormatProperty->m_pWrappedNumberFormatProperty = 0;
    }
}

void WrappedNumberFormatProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nFormat = 0;
    if( ! (rOuterValue >>= nFormat) )
        throw lang::IllegalArgumentException( C2U("Property 'NumberFormat' requires value of type sal_Int32"), 0, 0 );

    m_aOuterValue = rOuterValue;
    if(xInnerPropertySet.is())
    {
        bool bUseSourceFormat = !xInnerPropertySet->getPropertyValue( C2U("NumberFormat" )).hasValue();
        if( bUseSourceFormat )
        {
            uno::Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
            if( xChartDoc.is() && xChartDoc->hasInternalDataProvider() )
                bUseSourceFormat = false;
        }
        if( !bUseSourceFormat )
            xInnerPropertySet->setPropertyValue( m_aInnerName, this->convertOuterToInnerValue( rOuterValue ) );
    }
}

Any WrappedNumberFormatProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR("missing xInnerPropertySet in WrappedNumberFormatProperty::getPropertyValue");
        return Any();
    }
    Any aRet( xInnerPropertySet->getPropertyValue( m_aInnerName ));
    if( !aRet.hasValue() )
    {
        sal_Int32 nKey = 0;
        Reference< chart2::XDataSeries > xSeries( xInnerPropertySet, uno::UNO_QUERY );
        if( xSeries.is() )
            nKey = m_spChart2ModelContact->getExplicitNumberFormatKeyForSeries( xSeries );
        else
        {
            Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
            nKey = m_spChart2ModelContact->getExplicitNumberFormatKeyForAxis( xAxis );
        }
        aRet <<= nKey;
    }
    return aRet;
}

Any WrappedNumberFormatProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( sal_Int32( 0 ) );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedLinkNumberFormatProperty::WrappedLinkNumberFormatProperty( WrappedNumberFormatProperty* pWrappedNumberFormatProperty )
        : WrappedProperty( C2U("LinkNumberFormatToSource"), C2U("") )
        , m_pWrappedNumberFormatProperty( pWrappedNumberFormatProperty )
{
    if( m_pWrappedNumberFormatProperty )
    {
        m_pWrappedNumberFormatProperty->m_pWrappedLinkNumberFormatProperty = this;
    }
}

WrappedLinkNumberFormatProperty::~WrappedLinkNumberFormatProperty()
{
    if( m_pWrappedNumberFormatProperty )
    {
        if( m_pWrappedNumberFormatProperty->m_pWrappedLinkNumberFormatProperty == this )
            m_pWrappedNumberFormatProperty->m_pWrappedLinkNumberFormatProperty = 0;
    }
}

void WrappedLinkNumberFormatProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR("missing xInnerPropertySet in WrappedNumberFormatProperty::setPropertyValue");
        return;
    }

    bool bLinkFormat = false;
    if( rOuterValue >>= bLinkFormat )
    {
        Any aValue;
        if( bLinkFormat )
        {
            if( m_pWrappedNumberFormatProperty )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( m_pWrappedNumberFormatProperty->m_spChart2ModelContact->getChart2Document() );
                if( xChartDoc.is() && xChartDoc->hasInternalDataProvider() )
                    return;
            }
        }
        else
        {
            if( m_pWrappedNumberFormatProperty )
            {
                aValue = m_pWrappedNumberFormatProperty->getPropertyValue( xInnerPropertySet );
            }
            else
                aValue <<= sal_Int32( 0 );
        }

        xInnerPropertySet->setPropertyValue( C2U("NumberFormat"), aValue );
    }
}

Any WrappedLinkNumberFormatProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR("missing xInnerPropertySet in WrappedNumberFormatProperty::getPropertyValue");
        return getPropertyDefault(0);
    }
    bool bLink = ! xInnerPropertySet->getPropertyValue( C2U("NumberFormat" )).hasValue();
    return uno::makeAny( bLink );
}

Any WrappedLinkNumberFormatProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bLink = true;
    return uno::makeAny( bLink );
}

//.............................................................................
} //namespace wrapper
} //namespace chart
//.............................................................................
