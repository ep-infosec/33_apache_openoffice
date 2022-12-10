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
#include "odbc/ODriver.hxx"
#include "odbc/OConnection.hxx"
#include "odbc/OFunctions.hxx"
#include "odbc/OTools.hxx"
#include "connectivity/dbexception.hxx"
#include "resource/common_res.hrc"
#include "resource/sharedresources.hxx"

using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
ODBCDriver::ODBCDriver(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory)
	:ODriver_BASE(m_aMutex)
	,m_xORB(_rxFactory)
    ,m_pDriverHandle(SQL_NULL_HANDLE)
{
}
// --------------------------------------------------------------------------------
void ODBCDriver::disposing()
{
	::osl::MutexGuard aGuard(m_aMutex);


	for (OWeakRefArray::iterator i = m_xConnections.begin(); m_xConnections.end() != i; ++i)
	{
		Reference< XComponent > xComp(i->get(), UNO_QUERY);
		if (xComp.is())
			xComp->dispose();
	}
	m_xConnections.clear();

	ODriver_BASE::disposing();
}

// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString ODBCDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
	return rtl::OUString::createFromAscii("com.sun.star.comp.sdbc.ODBCDriver");
		// this name is referenced in the configuration and in the odbc.xml
		// Please take care when changing it.
}

typedef Sequence< ::rtl::OUString > SSEQ;
//------------------------------------------------------------------------------
SSEQ ODBCDriver::getSupportedServiceNames_Static(  ) throw (RuntimeException)
{
	SSEQ aSNS( 1 );
	aSNS[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.Driver");
	return aSNS;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL ODBCDriver::getImplementationName(  ) throw(RuntimeException)
{
	return getImplementationName_Static();
}

//------------------------------------------------------------------
sal_Bool SAL_CALL ODBCDriver::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
	SSEQ aSupported(getSupportedServiceNames());
	const ::rtl::OUString* pSupported = aSupported.getConstArray();
	const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
	for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
		;

	return pSupported != pEnd;
}

//------------------------------------------------------------------
SSEQ SAL_CALL ODBCDriver::getSupportedServiceNames(  ) throw(RuntimeException)
{
	return getSupportedServiceNames_Static();
}

// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL ODBCDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
	if ( ! acceptsURL(url) )
		return NULL;

	if(!m_pDriverHandle)
	{
		::rtl::OUString aPath;
		if(!EnvironmentHandle(aPath))
			throw SQLException(aPath,*this,::rtl::OUString(),1000,Any());
	}
	OConnection* pCon = new OConnection(m_pDriverHandle,this);
	Reference< XConnection > xCon = pCon;
	pCon->Construct(url,info);
	m_xConnections.push_back(WeakReferenceHelper(*pCon));

	return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL ODBCDriver::acceptsURL( const ::rtl::OUString& url )
		throw(SQLException, RuntimeException)
{
	return (!url.compareTo(::rtl::OUString::createFromAscii("sdbc:odbc:"),10));
}
// --------------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL ODBCDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& /*info*/ ) throw(SQLException, RuntimeException)
{
	if ( acceptsURL(url) )
	{
		::std::vector< DriverPropertyInfo > aDriverInfo;

		Sequence< ::rtl::OUString > aBooleanValues(2);
        aBooleanValues[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) );
		aBooleanValues[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet of the database."))
				,sal_False
				,::rtl::OUString()
				,Sequence< ::rtl::OUString >())
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalog"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Use catalog for file-based databases."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SystemDriverSettings"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Driver settings."))
				,sal_False
				,::rtl::OUString()
				,Sequence< ::rtl::OUString >())
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParameterNameSubstitution"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Change named parameters with '?'."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IgnoreDriverPrivileges"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Ignore the privileges from the database driver."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsAutoRetrievingEnabled"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Retrieve generated values."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "false" ) )
				,aBooleanValues)
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoRetrievingStatement"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Auto-increment statement."))
				,sal_False
				,::rtl::OUString()
				,Sequence< ::rtl::OUString >())
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GenerateASBeforeCorrelationName"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Generate AS before table correlation names."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) )
				,aBooleanValues)
				);
		aDriverInfo.push_back(DriverPropertyInfo(
				::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EscapeDateTime"))
				,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Escape date time format."))
				,sal_False
				,::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "true" ) )
				,aBooleanValues)
				);

		return Sequence< DriverPropertyInfo >(&aDriverInfo[0],aDriverInfo.size());
	}
	::connectivity::SharedResources aResources;
    const ::rtl::OUString sMessage = aResources.getResourceString(STR_URI_SYNTAX_ERROR);
    ::dbtools::throwGenericSQLException(sMessage ,*this);
	return Sequence< DriverPropertyInfo >();
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODBCDriver::getMajorVersion(  ) throw(RuntimeException)
{
	return 1;
}
// --------------------------------------------------------------------------------
sal_Int32 SAL_CALL ODBCDriver::getMinorVersion(  ) throw(RuntimeException)
{
	return 0;
}
// --------------------------------------------------------------------------------
//-----------------------------------------------------------------------------


