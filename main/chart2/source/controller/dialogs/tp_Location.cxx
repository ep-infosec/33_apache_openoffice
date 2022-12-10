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

#include "tp_Location.hxx"
#include "tp_Location.hrc"
#include "ResId.hxx"
#include "Strings.hrc"
#include "NoWarningThisInCTOR.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

#ifdef LOCATION_PAGE

LocationTabPage::LocationTabPage( svt::OWizardMachine* pParent
        , const uno::Reference< XChartDocument >& xChartModel
        , const uno::Reference< uno::XComponentContext >& xContext )
        : OWizardPage( pParent, SchResId(TP_LOCATION) )
        , m_aFL_Table( this, SchResId( FL_TABLE ) )
        , m_aFT_Table( this, SchResId( FT_TABLE ) )
        , m_aLB_Table( this, SchResId( LB_TABLE ) )
        , m_xChartModel( xChartModel )
        , m_xCC( xContext )
{
    FreeResource();
    this->SetText( String( SchResId( STR_PAGE_CHART_LOCATION ) ) );
}

LocationTabPage::~LocationTabPage()
{
}

bool LocationTabPage::canAdvance() const
{
	return false;
}
#endif
//.............................................................................
} //namespace chart
//.............................................................................
