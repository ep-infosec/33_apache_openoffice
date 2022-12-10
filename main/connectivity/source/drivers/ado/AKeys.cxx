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
#include "ado/AKeys.hxx"
#ifndef _CONNECTIVITY_ADO_INDEX_HXX_
#include "ado/AKey.hxx"
#endif
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include "ado/AConnection.hxx"
#include <comphelper/types.hxx>
#include "ado/Awrapado.hxx"
#include <comphelper/property.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/ado_res.hrc"

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

sdbcx::ObjectType OKeys::createObject(const ::rtl::OUString& _rName)
{
	return new OAdoKey(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}
// -------------------------------------------------------------------------
void OKeys::impl_refresh() throw(RuntimeException)
{
	m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeys::createDescriptor()
{
	return new OAdoKey(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OKeys::appendObject( const ::rtl::OUString&, const Reference< XPropertySet >& descriptor )
{
	OAdoKey* pKey = NULL;
	if ( !getImplementation( pKey, descriptor ) || pKey == NULL)
        m_pConnection->throwGenericSQLException( STR_INVALID_KEY_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

	// To pass as column parameter to Key's Apppend method
	OLEVariant vOptional;
	vOptional.setNoArg();
	
#if OSL_DEBUG_LEVEL > 0
    KeyTypeEnum eKey =
#endif
	    OAdoKey::Map2KeyRule(getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
#if OSL_DEBUG_LEVEL > 0
    (void)eKey;
#endif

	WpADOKey aKey = pKey->getImpl();
	::rtl::OUString sName = aKey.get_Name();
	if(!sName.getLength())
		aKey.put_Name(::rtl::OUString::createFromAscii("PrimaryKey") );

	ADOKeys* pKeys = m_aCollection;
	if ( FAILED(pKeys->Append(OLEVariant((ADOKey*)aKey),
							adKeyPrimary, // must be every time adKeyPrimary
							vOptional)) )
	{
		ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
		// just make sure that an SQLExceptionis thrown here
        m_pConnection->throwGenericSQLException( STR_INVALID_KEY_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );
	}

	return new OAdoKey(isCaseSensitive(),m_pConnection,pKey->getImpl());
}
// -------------------------------------------------------------------------
// XDrop
void OKeys::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
	if(!m_aCollection.Delete(OLEVariant(_sElementName)))
		ADOS::ThrowException(*m_pConnection->getConnection(),static_cast<XTypeProvider*>(this));
}
// -----------------------------------------------------------------------------


