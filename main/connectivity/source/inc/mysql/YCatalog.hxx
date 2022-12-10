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


#ifndef CONNECTIVITY_MYSQL_CATALOG_HXX
#define CONNECTIVITY_MYSQL_CATALOG_HXX

#include "connectivity/sdbcx/VCatalog.hxx"
#include "connectivity/StdTypeDefs.hxx"

namespace connectivity
{
	namespace mysql
	{
		// please don't name the class the same name as in an other namespaces
		// some compilers have problems with this task as I noticed on windows
        class OAdabasConnection;
		class OMySQLCatalog : public connectivity::sdbcx::OCatalog
		{
			::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

			/** calls XDatabaseMetaData::getTables.
				@param	_sKindOfObject
					The type of tables to be fetched.
				@param	_rNames
					The container for the names to be filled. <OUT/>
			*/
			void refreshObjects(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _sKindOfObject,TStringVector& _rNames);

		public:
			// implementation of the pure virtual methods
			virtual void refreshTables();
			virtual void refreshViews()	;
			virtual void refreshGroups();
			virtual void refreshUsers()	;

		public:
			OMySQLCatalog(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection);

			inline sdbcx::OCollection*		getPrivateTables()	const { return m_pTables;}
			inline sdbcx::OCollection*		getPrivateViews()	const { return m_pViews; }
			inline ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() const { return m_xConnection; }

			virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
			// ::cppu::OComponentHelper
			virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
		};
	}
}
#endif // CONNECTIVITY_MYSQL_CATALOG_HXX

