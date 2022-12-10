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
#include "hsqldb/HTables.hxx"
#include "hsqldb/HViews.hxx"
#include "hsqldb/HTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include "hsqldb/HCatalog.hxx"
#include <comphelper/extract.hxx>
#include "connectivity/dbtools.hxx"
#include "connectivity/dbexception.hxx"
#include <cppuhelper/interfacecontainer.h>
#include <comphelper/types.hxx>
#include "TConnection.hxx"

using namespace ::comphelper;
using namespace connectivity;
using namespace ::cppu;
using namespace connectivity::hsqldb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbtools;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OTables::createObject(const ::rtl::OUString& _rName)
{
	::rtl::OUString sCatalog,sSchema,sTable;
	::dbtools::qualifiedNameComponents(m_xMetaData,_rName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

    static const ::rtl::OUString s_sTableTypeView(RTL_CONSTASCII_USTRINGPARAM("VIEW"));
	static const ::rtl::OUString s_sTableTypeTable(RTL_CONSTASCII_USTRINGPARAM("TABLE"));
	static const ::rtl::OUString s_sAll(RTL_CONSTASCII_USTRINGPARAM("%"));

	Sequence< ::rtl::OUString > sTableTypes(3);
	sTableTypes[0] = s_sTableTypeView;
	sTableTypes[1] = s_sTableTypeTable;
	sTableTypes[2] = s_sAll;	// just to be sure to include anything else ....

	Any aCatalog;
	if ( sCatalog.getLength() )
		aCatalog <<= sCatalog;
    Reference< XResultSet > xResult = m_xMetaData->getTables(aCatalog,sSchema,sTable,sTableTypes);

    sdbcx::ObjectType xRet = NULL;
	if ( xResult.is() )
	{
        Reference< XRow > xRow(xResult,UNO_QUERY);
		if ( xResult->next() ) // there can be only one table with this name
		{
            sal_Int32 nPrivileges = ::dbtools::getTablePrivileges( m_xMetaData, sCatalog, sSchema, sTable );
            if ( m_xMetaData->isReadOnly() )
                nPrivileges &= ~( Privilege::INSERT | Privilege::UPDATE | Privilege::DELETE | Privilege::CREATE | Privilege::ALTER | Privilege::DROP );

            // obtain privileges
			OHSQLTable* pRet = new OHSQLTable( this
												,static_cast<OHCatalog&>(m_rParent).getConnection()
												,sTable
												,xRow->getString(4)
												,xRow->getString(5)
												,sSchema
												,sCatalog
												,nPrivileges);
			xRet = pRet;
		}
		::comphelper::disposeComponent(xResult);
	}

	return xRet;
}
// -------------------------------------------------------------------------
void OTables::impl_refresh(  ) throw(RuntimeException)
{
	static_cast<OHCatalog&>(m_rParent).refreshTables();
}
// -------------------------------------------------------------------------
void OTables::disposing(void)
{
m_xMetaData.clear();
	OCollection::disposing();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OTables::createDescriptor()
{
	return new OHSQLTable(this,static_cast<OHCatalog&>(m_rParent).getConnection());
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OTables::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
	createTable(descriptor);
    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OTables::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference< XInterface > xObject( getObject( _nPos ) );
    sal_Bool bIsNew = connectivity::sdbcx::ODescriptor::isNew( xObject );
	if (!bIsNew)
	{
		Reference< XConnection > xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();


		::rtl::OUString sCatalog,sSchema,sTable;
		::dbtools::qualifiedNameComponents(m_xMetaData,_sElementName,sCatalog,sSchema,sTable,::dbtools::eInDataManipulation);

		::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP ");

        Reference<XPropertySet> xProp(xObject,UNO_QUERY);
		sal_Bool bIsView;
		if((bIsView = (xProp.is() && ::comphelper::getString(xProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))) == ::rtl::OUString::createFromAscii("VIEW")))) // here we have a view
			aSql += ::rtl::OUString::createFromAscii("VIEW ");
		else
			aSql += ::rtl::OUString::createFromAscii("TABLE ");

		::rtl::OUString sComposedName(
		    ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInDataManipulation ) );
		aSql += sComposedName;
		Reference< XStatement > xStmt = xConnection->createStatement(  );
		if ( xStmt.is() )
		{
			xStmt->execute(aSql);
			::comphelper::disposeComponent(xStmt);
		}
		// if no exception was thrown we must delete it from the views
		if ( bIsView )
		{
			HViews* pViews = static_cast<HViews*>(static_cast<OHCatalog&>(m_rParent).getPrivateViews());
			if ( pViews && pViews->hasByName(_sElementName) )
				pViews->dropByNameImpl(_sElementName);
		}
    }
}
// -------------------------------------------------------------------------
void OTables::createTable( const Reference< XPropertySet >& descriptor )
{
	Reference< XConnection > xConnection = static_cast<OHCatalog&>(m_rParent).getConnection();
	::rtl::OUString aSql = ::dbtools::createSqlCreateTableStatement(descriptor,xConnection);

    Reference< XStatement > xStmt = xConnection->createStatement(  );
	if ( xStmt.is() )
	{
		xStmt->execute(aSql);
		::comphelper::disposeComponent(xStmt);
	}
}
// -----------------------------------------------------------------------------
void OTables::appendNew(const ::rtl::OUString& _rsNewTable)
{
	insertElement(_rsNewTable,NULL);

	// notify our container listeners
	ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(_rsNewTable), Any(), Any());
	OInterfaceIteratorHelper aListenerLoop(m_aContainerListeners);
	while (aListenerLoop.hasMoreElements())
		static_cast<XContainerListener*>(aListenerLoop.next())->elementInserted(aEvent);
}
// -----------------------------------------------------------------------------
::rtl::OUString OTables::getNameForObject(const sdbcx::ObjectType& _xObject)
{
    OSL_ENSURE(_xObject.is(),"OTables::getNameForObject: Object is NULL!");
	return ::dbtools::composeTableName( m_xMetaData, _xObject, ::dbtools::eInDataManipulation, false, false, false );
}
// -----------------------------------------------------------------------------

