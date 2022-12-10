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



#ifndef _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_
#define _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_

#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <cppuhelper/implbase1.hxx>
#include "odbc/OFunctions.hxx"
#include "odbc/odbcbasedllapi.hxx"
#ifndef _VECTOR_
#include <vector>
#endif
#include "odbc/OConnection.hxx"

namespace connectivity
{
	namespace odbc
	{
		//**************************************************************
		//************ Class: ResultSetMetaData
		//**************************************************************
        typedef ::cppu::WeakImplHelper1<        ::com::sun::star::sdbc::XResultSetMetaData>   OResultSetMetaData_BASE;

		class OOO_DLLPUBLIC_ODBCBASE OResultSetMetaData :
            public 	OResultSetMetaData_BASE
		{
		protected:
			::std::vector<sal_Int32> m_vMapping; // when not every column is needed
            ::std::map<sal_Int32,sal_Int32> m_aColumnTypes;

			SQLHANDLE		m_aStatementHandle;
			OConnection*	m_pConnection;
			sal_Int32		m_nColCount;
			sal_Bool		m_bUseODBC2Types;

            ::rtl::OUString getCharColAttrib(sal_Int32 column,sal_Int32 ident) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            sal_Int32 getNumColAttrib(sal_Int32 column,sal_Int32 ident) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
		public:
			// ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
			OResultSetMetaData(OConnection*	_pConnection, SQLHANDLE _pStmt )
                :m_aStatementHandle( _pStmt )
				,m_pConnection(_pConnection)
				,m_nColCount(-1)
				,m_bUseODBC2Types(sal_False)
			{}
			OResultSetMetaData(OConnection*	_pConnection, SQLHANDLE _pStmt ,const ::std::vector<sal_Int32> & _vMapping)
                    :m_vMapping(_vMapping)
                    ,m_aStatementHandle( _pStmt )
                    ,m_pConnection(_pConnection)
					,m_nColCount(_vMapping.size()-1)
					,m_bUseODBC2Types(sal_False)
			{}
			virtual ~OResultSetMetaData();


            static SQLLEN getNumColAttrib(OConnection* _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface
                                              ,sal_Int32 _column
                                              ,sal_Int32 ident) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            static SQLSMALLINT getColumnODBCType(OConnection* _pConnection
                                              ,SQLHANDLE _aStatementHandle
                                              ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _xInterface
                                              ,sal_Int32 column)
                                               throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

			inline oslGenericFunction getOdbcFunction(sal_Int32 _nIndex)  const
			{
				return m_pConnection->getOdbcFunction(_nIndex);
			}
			/// Avoid ambigous cast error from the compiler.
            inline operator ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > () throw()
			{ return this; }

            virtual sal_Int32 SAL_CALL getColumnCount(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isAutoIncrement( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCaseSensitive( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isSearchable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isCurrency( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL isNullable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isSigned( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnDisplaySize( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnLabel( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getSchemaName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getPrecision( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
			virtual sal_Int32 SAL_CALL getScale( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getTableName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getCatalogName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getColumnType( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnTypeName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isReadOnly( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Bool SAL_CALL isDefinitelyWritable( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual ::rtl::OUString SAL_CALL getColumnServiceName( sal_Int32 column ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
		};
	}
}
#endif // _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_

