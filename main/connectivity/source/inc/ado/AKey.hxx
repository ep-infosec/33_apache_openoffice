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



#ifndef _CONNECTIVITY_ADO_KEY_HXX_
#define _CONNECTIVITY_ADO_KEY_HXX_

#include "connectivity/sdbcx/VKey.hxx"
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include "ado/Awrapadox.hxx"

namespace connectivity
{
	namespace ado
	{
		typedef sdbcx::OKey OKey_ADO;

		class OConnection;
		class OAdoKey : public OKey_ADO
		{
			WpADOKey		m_aKey;
			OConnection*	m_pConnection;
		protected:
            void fillPropertyValues();
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const ::com::sun::star::uno::Any& rValue)throw (::com::sun::star::uno::Exception);
		public:
			virtual void refreshColumns();
		public:
			virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
			OAdoKey(sal_Bool _bCase,OConnection* _pConnection,ADOKey* _pKey);
			OAdoKey(sal_Bool _bCase,OConnection* _pConnection);

			// com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

			WpADOKey		getImpl() const { return m_aKey;}
			// map the update/delete rules
			static RuleEnum Map2Rule(const sal_Int32& _eNum);
			static sal_Int32 MapRule(const RuleEnum& _eNum);

			// map the keytypes
			static sal_Int32 MapKeyRule(const KeyTypeEnum& _eNum);
			static KeyTypeEnum Map2KeyRule(const sal_Int32& _eNum);
		};
	}
}
#endif // _CONNECTIVITY_ADO_KEY_HXX_

