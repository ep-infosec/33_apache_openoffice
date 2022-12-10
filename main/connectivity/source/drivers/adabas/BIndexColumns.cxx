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
#include "precompiled_connectivity.hxx"

#ifndef _CONNECTIVITY_ADABAS_INDEXCOLUMNS_HXX_
#include "adabas/BIndexColumns.hxx"
#endif
#include "connectivity/sdbcx/VIndexColumn.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include "adabas/BTable.hxx"
#include "adabas/BCatalog.hxx"
#include <comphelper/types.hxx>
#include <comphelper/property.hxx>

using namespace connectivity::adabas;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
//	using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -------------------------------------------------------------------------
ObjectType OIndexColumns::createObject(const ::rtl::OUString& _rName)
{

	Reference< XResultSet > xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getIndexInfo(Any(),
					m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getTableName(),sal_False,sal_False);

	sal_Bool bAsc = sal_True;
	if(xResult.is())
	{
                Reference< XRow > xRow(xResult,UNO_QUERY);
		::rtl::OUString aD(::rtl::OUString::createFromAscii("D"));
		while(xResult->next())
		{
			if(xRow->getString(9) == _rName)
				bAsc = xRow->getString(10) != aD;
		}
		::comphelper::disposeComponent(xResult);
	}

	xResult = m_pIndex->getTable()->getConnection()->getMetaData()->getColumns(Any(),
			m_pIndex->getTable()->getSchema(),m_pIndex->getTable()->getTableName(),_rName);

    ObjectType xRet = NULL;
	if(xResult.is())
	{
                Reference< XRow > xRow(xResult,UNO_QUERY);
		while(xResult->next())
		{
			if(xRow->getString(4) == _rName)
			{
				sal_Int32 nType				= xRow->getInt(5);
				::rtl::OUString sTypeName	= xRow->getString(6);
				sal_Int32 nPrec				= xRow->getInt(7);
				OAdabasCatalog::correctColumnProperties(nPrec,nType,sTypeName);

				OIndexColumn* pRet = new OIndexColumn(bAsc,
													_rName,
													sTypeName,
													xRow->getString(13),
													xRow->getInt(11),
													nPrec,
													xRow->getInt(9),
													nType,
													sal_False,sal_False,sal_False,sal_True);
				xRet = pRet;
				break;
			}
		}
		::comphelper::disposeComponent(xResult);
	}

	return xRet;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexColumns::createDescriptor()
{
	return new OIndexColumn(sal_True);
}
// -----------------------------------------------------------------------------
void OIndexColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
	m_pIndex->refreshColumns();
}
// -----------------------------------------------------------------------------
ObjectType OIndexColumns::appendObject( const ::rtl::OUString& /*_rForName*/, const Reference< XPropertySet >& descriptor )
{
    return cloneDescriptor( descriptor );
}
// -----------------------------------------------------------------------------



