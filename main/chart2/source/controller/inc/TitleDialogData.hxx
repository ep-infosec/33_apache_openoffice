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


#ifndef _CHART2_TITLEDIALOGDATA_HXX
#define _CHART2_TITLEDIALOGDATA_HXX

#include "ReferenceSizeProvider.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

struct TitleDialogData
{
    ::com::sun::star::uno::Sequence< sal_Bool > aPossibilityList;
    ::com::sun::star::uno::Sequence< sal_Bool > aExistenceList;
    ::com::sun::star::uno::Sequence< rtl::OUString > aTextList;
    ::std::auto_ptr< ReferenceSizeProvider >    apReferenceSizeProvider;

    TitleDialogData( ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider =
                     ::std::auto_ptr< ReferenceSizeProvider >( 0 ));

    void readFromModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    /* return true if anything has changed;
    when pOldState is NULL then all data are written to the model
    */
    bool writeDifferenceToModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
                        , TitleDialogData* pOldState=NULL );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
