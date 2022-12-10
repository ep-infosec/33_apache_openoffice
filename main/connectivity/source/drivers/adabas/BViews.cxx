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
#include "adabas/BViews.hxx"
#include "adabas/BTables.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/CheckOption.hpp>
#include "adabas/BCatalog.hxx"
#include "adabas/BConnection.hxx"
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include "connectivity/sdbcx/VView.hxx"
#include <comphelper/types.hxx>

using namespace ::comphelper;

using namespace ::cppu;
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OViews::createObject(const ::rtl::OUString& _rName)
{
	::rtl::OUString aName,aSchema;
	sal_Int32 nLen = _rName.indexOf('.');
	aSchema = _rName.copy(0,nLen);
	aName	= _rName.copy(nLen+1);

	::rtl::OUString sStmt = ::rtl::OUString::createFromAscii("SELECT DISTINCT * FROM DOMAIN.SHOW_VIEW WHERE ");
	if(aSchema.getLength())
	{
		sStmt += ::rtl::OUString::createFromAscii("OWNER = '");
		sStmt += aSchema;
		sStmt += ::rtl::OUString::createFromAscii("' AND ");
	}
	sStmt += ::rtl::OUString::createFromAscii("VIEWNAME = '");
	sStmt += aName;
	sStmt += ::rtl::OUString::createFromAscii("'");
	Reference<XConnection> xConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
    Reference< XStatement > xStmt = xConnection->createStatement(  );
	Reference< XResultSet > xResult = xStmt->executeQuery(sStmt);

    sdbcx::ObjectType xRet = NULL;
	if(xResult.is())
	{
        Reference< XRow > xRow(xResult,UNO_QUERY);
		if(xResult->next()) // there can be only one table with this name
		{
			connectivity::sdbcx::OView* pRet = new connectivity::sdbcx::OView(sal_True,
																				aName,
																				xConnection->getMetaData(),
																				CheckOption::NONE,
																				xRow->getString(3),
																				aSchema);
			xRet = pRet;
		}
		::comphelper::disposeComponent(xResult);
	}
	::comphelper::disposeComponent(xStmt);

	return xRet;
}
// -------------------------------------------------------------------------
void OViews::impl_refresh(  ) throw(RuntimeException)
{
	static_cast<OAdabasCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OViews::disposing(void)
{
m_xMetaData.clear();
	OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OViews::createDescriptor()
{
	Reference<XConnection> xConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
	return new connectivity::sdbcx::OView(sal_True,xConnection->getMetaData());
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OViews::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
	createView(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OViews::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
	if(m_bInDrop)
		return;

    Reference< XInterface > xObject( getObject( _nPos ) );
    sal_Bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
	if (!bIsNew)
	{
		OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );

		::rtl::OUString aName,aSchema;
		sal_Int32 nLen = _sElementName.indexOf('.');
		aSchema = _sElementName.copy(0,nLen);
		aName	= _sElementName.copy(nLen+1);
		::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP VIEW");
		const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

		aSql = aSql + m_xMetaData->getIdentifierQuoteString(  ) + aSchema + m_xMetaData->getIdentifierQuoteString(  );
		aSql = aSql + sDot;
		aSql = aSql + m_xMetaData->getIdentifierQuoteString(  ) + aName + m_xMetaData->getIdentifierQuoteString(  );
		xStmt->execute(aSql);
		::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------
void OViews::dropByNameImpl(const ::rtl::OUString& elementName)
{
	m_bInDrop = sal_True;
	OCollection_TYPE::dropByName(elementName);
	m_bInDrop = sal_False;
}
// -----------------------------------------------------------------------------
void OViews::createView( const Reference< XPropertySet >& descriptor )
{
	::rtl::OUString aSql	= ::rtl::OUString::createFromAscii("CREATE VIEW ");
	::rtl::OUString aQuote	= static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getIdentifierQuoteString(  );
	const ::rtl::OUString& sDot = OAdabasCatalog::getDot();
	::rtl::OUString sSchema,sCommand;

	descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= sSchema;
	if(sSchema.getLength())
		aSql += ::dbtools::quoteName(aQuote, sSchema) + sDot;
	else
		descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),makeAny(sSchema = static_cast<OAdabasCatalog&>(m_rParent).getConnection()->getMetaData()->getUserName()));

	aSql += ::dbtools::quoteName(aQuote, getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))))
				+ ::rtl::OUString::createFromAscii(" AS ");
	descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND)) >>= sCommand;
	aSql += sCommand;

	OAdabasConnection* pConnection = static_cast<OAdabasCatalog&>(m_rParent).getConnection();
        Reference< XStatement > xStmt = pConnection->createStatement(  );
	xStmt->execute(aSql);
	::comphelper::disposeComponent(xStmt);

	// insert the new view also in the tables collection
	OTables* pTables = static_cast<OTables*>(static_cast<OAdabasCatalog&>(m_rParent).getPrivateTables());
	if(pTables)
	{
		::rtl::OUString sName = sSchema;
		sName += sDot;
		sName += getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)));
		pTables->appendNew(sName);
	}
}
