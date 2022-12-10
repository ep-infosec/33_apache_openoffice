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



#ifndef _CONNECTIVITY_DBASE_DSTATEMENT_HXX_
#define _CONNECTIVITY_DBASE_DSTATEMENT_HXX_

#include "file/FStatement.hxx"

namespace connectivity
{
	namespace dbase
	{
		class OConnection;
		class ODbaseStatement : public file::OStatement
		{
		protected:
			virtual file::OResultSet* createResultSet();
		public:
			ODbaseStatement( file::OConnection* _pConnection) : file::OStatement( _pConnection){}
			DECLARE_SERVICE_INFO();
		};
	}
}

#endif //_CONNECTIVITY_DBASE_DSTATEMENT_HXX_
