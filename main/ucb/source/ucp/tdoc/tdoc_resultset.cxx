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
#include "precompiled_tdoc.hxx"

/**************************************************************************
								TODO
 **************************************************************************

 - This implementation is not a dynamic result set!!! It only implements
   the necessary interfaces, but never recognizes/notifies changes!!!

 *************************************************************************/

#include "ucbhelper/resultset.hxx"

#include "tdoc_datasupplier.hxx"
#include "tdoc_resultset.hxx"
#include "tdoc_content.hxx"

using namespace com::sun::star;
using namespace tdoc_ucp;

//=========================================================================
//=========================================================================
//
// DynamicResultSet Implementation.
//
//=========================================================================
//=========================================================================

DynamicResultSet::DynamicResultSet(
            const uno::Reference< lang::XMultiServiceFactory >& rxSMgr,
            const rtl::Reference< Content >& rxContent,
            const ucb::OpenCommandArgument2& rCommand )
: ResultSetImplHelper( rxSMgr, rCommand ),
  m_xContent( rxContent )
{
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void DynamicResultSet::initStatic()
{
	m_xResultSet1
		= new ::ucbhelper::ResultSet(
            m_xSMgr,
            m_aCommand.Properties,
            new ResultSetDataSupplier( m_xSMgr,
                                       m_xContent,
                                       m_aCommand.Mode ) );
}

//=========================================================================
void DynamicResultSet::initDynamic()
{
	m_xResultSet1
		= new ::ucbhelper::ResultSet(
            m_xSMgr,
            m_aCommand.Properties,
            new ResultSetDataSupplier( m_xSMgr,
                                       m_xContent,
                                       m_aCommand.Mode ) );
	m_xResultSet2 = m_xResultSet1;
}

