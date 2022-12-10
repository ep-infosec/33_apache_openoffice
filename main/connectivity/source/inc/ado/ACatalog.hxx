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


#ifndef _CONNECTIVITY_ADO_CATALOG_HXX_
#define _CONNECTIVITY_ADO_CATALOG_HXX_

#include "connectivity/sdbcx/VCatalog.hxx"
#include "ado/Awrapadox.hxx"

namespace connectivity
{
	namespace ado
	{
		class OConnection;

		class OCatalog : public connectivity::sdbcx::OCatalog
		{
			WpADOCatalog	m_aCatalog;
			OConnection*	m_pConnection;

		public:
			virtual void refreshTables();
			virtual void refreshViews()	;
			virtual void refreshGroups();
			virtual void refreshUsers()	;

		public:
			OCatalog(_ADOCatalog* _pCatalog,OConnection* _pCon);
			~OCatalog();

			OConnection*		getConnection()		const { return m_pConnection;	}
			sdbcx::OCollection*	getPrivateTables()	const { return m_pTables;		}
			WpADOCatalog		getCatalog()		const { return m_aCatalog;		}
		};
	}
}
#endif // _CONNECTIVITY_ADO_CATALOG_HXX_

