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
#include "MultipleChartConverters.hxx"

#include "chartview/ChartSfxItemIds.hxx"
#include "SchWhichPairs.hxx"
#include "AxisItemConverter.hxx"
#include "StatisticsItemConverter.hxx"
#include "GraphicPropertyItemConverter.hxx"
#include "DataPointItemConverter.hxx"
#include "ChartModelHelper.hxx"
#include "TitleHelper.hxx"
#include "TitleItemConverter.hxx"
#include "AxisHelper.hxx"
#include "chartview/ExplicitValueProvider.hxx"
#include "DiagramHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{
namespace wrapper
{

//-----------------------------------------------------------------------------

AllAxisItemConverter::AllAxisItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & /*xNamedPropertyContainerFactory*/,
    ::std::auto_ptr< awt::Size > pRefSize )
        : MultipleItemConverter( rItemPool )
{
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    Sequence< Reference< XAxis > > aElementList( AxisHelper::getAllAxesOfDiagram( xDiagram ) );
    for( sal_Int32 nA = 0; nA < aElementList.getLength(); nA++ )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties(aElementList[nA], uno::UNO_QUERY);
        if( pRefSize.get())
            m_aConverters.push_back( new ::chart::wrapper::AxisItemConverter(
                                         xObjectProperties, rItemPool, rDrawModel,
                                         uno::Reference< chart2::XChartDocument >( xChartModel, uno::UNO_QUERY ), 0, 0,
                                         ::std::auto_ptr< awt::Size >( new awt::Size( *pRefSize )) ));
        else
            m_aConverters.push_back( new ::chart::wrapper::AxisItemConverter(
                                         xObjectProperties, rItemPool, rDrawModel,
                                         uno::Reference< chart2::XChartDocument >( xChartModel, uno::UNO_QUERY ), 0, 0 ) );
    }
}

AllAxisItemConverter::~AllAxisItemConverter()
{
}

const sal_uInt16 * AllAxisItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nAllAxisWhichPairs;
}

//-----------------------------------------------------------------------------

AllGridItemConverter::AllGridItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory )
        : MultipleItemConverter( rItemPool )
{
    Reference< XDiagram > xDiagram( ChartModelHelper::findDiagram( xChartModel ) );
    Sequence< Reference< beans::XPropertySet > > aElementList( AxisHelper::getAllGrids( xDiagram ) );
    for( sal_Int32 nA = 0; nA < aElementList.getLength(); nA++ )
    {
        Reference< beans::XPropertySet > xObjectProperties(aElementList[nA]);
        m_aConverters.push_back( new ::chart::wrapper::GraphicPropertyItemConverter(
                                        xObjectProperties, rItemPool, rDrawModel, xNamedPropertyContainerFactory,
                                        ::chart::wrapper::GraphicPropertyItemConverter::LINE_PROPERTIES ) );
    }
}

AllGridItemConverter::~AllGridItemConverter()
{
}

const sal_uInt16 * AllGridItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nGridWhichPairs;
}

//-----------------------------------------------------------------------------

AllDataLabelItemConverter::AllDataLabelItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    ::std::auto_ptr< awt::Size > pRefSize )
        : MultipleItemConverter( rItemPool )
{
    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::ChartModelHelper::getDataSeries( xChartModel ));

    ::std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        uno::Reference< uno::XComponentContext>	xContext(0);//do not need Context for label properties

        sal_Int32 nNumberFormat=ExplicitValueProvider::getExplicitNumberFormatKeyForDataLabel( xObjectProperties, *aIt, -1/*nPointIndex*/, ChartModelHelper::findDiagram( xChartModel ) );
        sal_Int32 nPercentNumberFormat=ExplicitValueProvider::getExplicitPercentageNumberFormatKeyForDataLabel(
                xObjectProperties,uno::Reference< util::XNumberFormatsSupplier >(xChartModel, uno::UNO_QUERY));

        m_aConverters.push_back( new ::chart::wrapper::DataPointItemConverter(
                                         xChartModel, xContext,
                                         xObjectProperties, *aIt, rItemPool, rDrawModel, NULL,
                                         xNamedPropertyContainerFactory,
                                         GraphicPropertyItemConverter::FILLED_DATA_POINT,
                                         ::std::auto_ptr< awt::Size >( pRefSize.get() ? new awt::Size( *pRefSize ) : 0),
                                         true, /*bDataSeries*/
                                         false, /*bUseSpecialFillColor*/
                                         0, /*nSpecialFillColor*/
                                         true /*bOverwriteLabelsForAttributedDataPointsAlso*/,
                                         nNumberFormat, nPercentNumberFormat
                                         ));
    }
}

AllDataLabelItemConverter::~AllDataLabelItemConverter()
{
}

const sal_uInt16 * AllDataLabelItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nDataLabelWhichPairs;
}

//-----------------------------------------------------------------------------

AllTitleItemConverter::AllTitleItemConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool,
    SdrModel& rDrawModel,
    const uno::Reference< lang::XMultiServiceFactory > & xNamedPropertyContainerFactory,
    ::std::auto_ptr< awt::Size > pRefSize )
        : MultipleItemConverter( rItemPool )
{
    for(sal_Int32 nTitle = TitleHelper::TITLE_BEGIN; nTitle < TitleHelper::NORMAL_TITLE_END; nTitle++ )
    {
        uno::Reference< chart2::XTitle > xTitle( TitleHelper::getTitle( TitleHelper::eTitleType(nTitle), xChartModel ) );
        if(!xTitle.is())
            continue;
        uno::Reference< beans::XPropertySet > xObjectProperties( xTitle, uno::UNO_QUERY);
        ::std::auto_ptr< awt::Size > pSingleRefSize(0);
        if( pRefSize.get())
            pSingleRefSize = ::std::auto_ptr< awt::Size >( new awt::Size( *pRefSize ));
        m_aConverters.push_back( new ::chart::wrapper::TitleItemConverter(
                                     xObjectProperties, rItemPool, rDrawModel, xNamedPropertyContainerFactory, pSingleRefSize ));
    }
}

AllTitleItemConverter::~AllTitleItemConverter()
{
}

const sal_uInt16 * AllTitleItemConverter::GetWhichPairs() const
{
    // must span all used items!
    return nTitleWhichPairs;
}

//-----------------------------------------------------------------------------

AllSeriesStatisticsConverter::AllSeriesStatisticsConverter(
    const uno::Reference< frame::XModel > & xChartModel,
    SfxItemPool& rItemPool )
        : MultipleItemConverter( rItemPool )
{
    ::std::vector< uno::Reference< chart2::XDataSeries > > aSeriesList(
        ::chart::ChartModelHelper::getDataSeries( xChartModel ));

    ::std::vector< uno::Reference< chart2::XDataSeries > >::const_iterator aIt;
    for( aIt = aSeriesList.begin(); aIt != aSeriesList.end(); ++aIt )
    {
        uno::Reference< beans::XPropertySet > xObjectProperties( *aIt, uno::UNO_QUERY);
        m_aConverters.push_back( new ::chart::wrapper::StatisticsItemConverter(
                                     xChartModel, xObjectProperties, rItemPool ));
    }
}

AllSeriesStatisticsConverter::~AllSeriesStatisticsConverter()
{}

const sal_uInt16 * AllSeriesStatisticsConverter::GetWhichPairs() const
{
    // must span all used items!
    return nStatWhichPairs;
}

} //  namespace wrapper
} //  namespace chart
