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



#ifndef _HIERARCHYDATASUPPLIER_HXX
#define _HIERARCHYDATASUPPLIER_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <ucbhelper/resultset.hxx>

namespace hierarchy_ucp {

class HierarchyEntryData;
struct DataSupplier_Impl;
class HierarchyContent;

class HierarchyResultSetDataSupplier : 
        public ::ucbhelper::ResultSetDataSupplier
{
	DataSupplier_Impl*	m_pImpl;

private:
	sal_Bool checkResult( const HierarchyEntryData& rResult );

public:
	HierarchyResultSetDataSupplier(
					const com::sun::star::uno::Reference<
						com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
                    const rtl::Reference< HierarchyContent >& rContent,
					sal_Int32 nOpenMode = com::sun::star::ucb::OpenMode::ALL );
	virtual ~HierarchyResultSetDataSupplier();

	virtual rtl::OUString queryContentIdentifierString( sal_uInt32 nIndex );
	virtual com::sun::star::uno::Reference<
				com::sun::star::ucb::XContentIdentifier >
	queryContentIdentifier( sal_uInt32 nIndex );
	virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent >
	queryContent( sal_uInt32 nIndex );

	virtual sal_Bool getResult( sal_uInt32 nIndex );

	virtual sal_uInt32 totalCount();
	virtual sal_uInt32 currentCount();
	virtual sal_Bool isCountFinal();

	virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XRow >
	queryPropertyValues( sal_uInt32 nIndex  );
	virtual void releasePropertyValues( sal_uInt32 nIndex );

	virtual void close();

	virtual void validate()
		throw( com::sun::star::ucb::ResultSetException );
};

} // namespace hierarchy_ucp

#endif /* !_HIERARCHYDATASUPPLIER_HXX */
