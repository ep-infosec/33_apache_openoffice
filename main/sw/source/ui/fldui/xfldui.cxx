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
#include "precompiled_sw.hxx"


#include <tools/debug.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <fldmgr.hxx>
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#include <wrtsh.hxx>		// Actives Fenster
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <swmodule.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;


// ---------------------------------------------------------------------------
// This file contains all routines of the fldui directory, which must compile
// with exceptions. So we can reduce the code of the other files, which don't
// need any exception handling.
// ---------------------------------------------------------------------------

/*--------------------------------------------------------------------
	 Beschreibung: Ist das Datenbankfeld numerisch?
	 Anm: Im Fehlerfall wird sal_True returnt.
 --------------------------------------------------------------------*/

sal_Bool SwFldMgr::IsDBNumeric( const String& rDBName, const String& rTblQryName,
							sal_Bool bIsTable, const String& rFldName)
{
	sal_Bool bNumeric = sal_True;

    SwNewDBMgr* pDBMgr = pWrtShell ? pWrtShell->GetNewDBMgr() :
                            ::GetActiveView()->GetWrtShell().GetNewDBMgr();

    ::rtl::OUString sSource(rDBName);
    Reference< XConnection> xConnection =
                    pDBMgr->RegisterConnection(sSource);

    if( !xConnection.is() )
		return bNumeric;

	Reference<XColumnsSupplier> xColsSupplier;
	if(bIsTable)
	{
		Reference<XTablesSupplier> xTSupplier = Reference<XTablesSupplier>(xConnection, UNO_QUERY);
		if(xTSupplier.is())
		{
			Reference<XNameAccess> xTbls = xTSupplier->getTables();
			DBG_ASSERT(xTbls->hasByName(rTblQryName), "table not available anymore?");
			try
			{
				Any aTable = xTbls->getByName(rTblQryName);
                Reference<XPropertySet> xPropSet;
                aTable >>= xPropSet;
				xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
			}
            catch(Exception&){}
		}
	}
	else
	{
		Reference<XQueriesSupplier> xQSupplier = Reference<XQueriesSupplier>(xConnection, UNO_QUERY);
		if(xQSupplier.is())
		{
			Reference<XNameAccess> xQueries = xQSupplier->getQueries();
			DBG_ASSERT(xQueries->hasByName(rTblQryName), "table not available anymore?");
			try
			{
				Any aQuery = xQueries->getByName(rTblQryName);
                Reference<XPropertySet> xPropSet;
                aQuery >>= xPropSet;
				xColsSupplier = Reference<XColumnsSupplier>(xPropSet, UNO_QUERY);
			}
            catch(Exception&){}
		}
	}

	if(xColsSupplier.is())
	{
		Reference <XNameAccess> xCols;
        try
        {
            xCols = xColsSupplier->getColumns();
        }
        catch(Exception& )
        {
            DBG_ERROR("Exception in getColumns()");
        }
		if(xCols.is() && xCols->hasByName(rFldName))
		{
			Any aCol = xCols->getByName(rFldName);
            Reference <XPropertySet> xCol;
            aCol >>= xCol;
			Any aType = xCol->getPropertyValue( UniString::CreateFromAscii("Type"));
			sal_Int32 eDataType = 0;
			aType >>= eDataType;
			switch(eDataType)
			{
				case DataType::BIT:
				case DataType::BOOLEAN:
				case DataType::TINYINT:
				case DataType::SMALLINT:
				case DataType::INTEGER:
				case DataType::BIGINT:
				case DataType::FLOAT:
				case DataType::REAL:
				case DataType::DOUBLE:
				case DataType::NUMERIC:
				case DataType::DECIMAL:
				case DataType::DATE:
				case DataType::TIME:
				case DataType::TIMESTAMP:
					break;

				case DataType::BINARY:
				case DataType::VARBINARY:
				case DataType::LONGVARBINARY:
				case DataType::SQLNULL:
				case DataType::OTHER:
				case DataType::OBJECT:
				case DataType::DISTINCT:
				case DataType::STRUCT:
				case DataType::ARRAY:
				case DataType::BLOB:
				case DataType::CLOB:
				case DataType::REF:
				case DataType::CHAR:
				case DataType::VARCHAR:
				case DataType::LONGVARCHAR:
				default:
					bNumeric = sal_False;
			}
		}
	}
	return bNumeric;
}


