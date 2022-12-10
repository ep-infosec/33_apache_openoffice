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



#ifndef _DBACORE_RESULTCOLUMN_HXX_
#define _DBACORE_RESULTCOLUMN_HXX_

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HDL_
#include <com/sun/star/sdbc/XResultSetMetaData.hdl>
#endif
#ifndef _DBA_COREAPI_COLUMN_HXX_
#include <column.hxx>
#endif
#include <boost/optional.hpp>
namespace dbaccess
{
	//************************************************************
	//  OResultColumn
	//************************************************************
	class OResultColumn : public OColumn,
						  public ::comphelper::OPropertyArrayUsageHelper < OResultColumn >
	{
	protected:
		::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >	m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >   m_xDBMetaData;
		sal_Int32				    m_nPos;
        ::com::sun::star::uno::Any  m_aIsRowVersion;
        mutable ::boost::optional< sal_Bool > m_isSigned;
        mutable ::boost::optional< sal_Bool > m_isCurrency;
        mutable ::boost::optional< sal_Bool > m_bSearchable;
        mutable ::boost::optional< sal_Bool > m_isCaseSensitive;
        mutable ::boost::optional< sal_Bool > m_isReadOnly;
        mutable ::boost::optional< sal_Bool > m_isWritable;
        mutable ::boost::optional< sal_Bool > m_isDefinitelyWritable;
        mutable ::boost::optional< sal_Bool > m_isAutoIncrement;
        mutable ::boost::optional< sal_Int32 > m_isNullable;
        mutable ::boost::optional< ::rtl::OUString > m_sColumnLabel;
        mutable ::boost::optional< sal_Int32 > m_nColumnDisplaySize;
        mutable ::boost::optional< sal_Int32 > m_nColumnType;
        mutable ::boost::optional< sal_Int32 > m_nPrecision;
        mutable ::boost::optional< sal_Int32 > m_nScale;

		virtual ~OResultColumn();
	public:
		OResultColumn(
            const ::com::sun::star::uno::Reference < ::com::sun::star::sdbc::XResultSetMetaData >& _xMetaData,
			sal_Int32 _nPos,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData >& _rxDBMeta );

	// com::sun::star::lang::XTypeProvider
		virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

	// com::sun::star::lang::XServiceInfo
		virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
		virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

	// cppu::OComponentHelper
		virtual void SAL_CALL disposing(void);

	// comphelper::OPropertyArrayUsageHelper
		virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

	// cppu::OPropertySetHelper
		virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
		virtual void SAL_CALL getFastPropertyValue(
									::com::sun::star::uno::Any& rValue,
									sal_Int32 nHandle
										 ) const;

    private:
        void    impl_determineIsRowVersion_nothrow();

    protected:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
	};
}
#endif // _DBACORE_RESULTCOLUMN_HXX_

