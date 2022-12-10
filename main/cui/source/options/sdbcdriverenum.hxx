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



#ifndef _OFFMGR_SDBCDRIVERENUMERATION_HXX_
#define _OFFMGR_SDBCDRIVERENUMERATION_HXX_


#include <sal/types.h>

#include <vector>

namespace rtl
{
	class OUString;
}

//........................................................................
namespace offapp
{
//........................................................................

	//====================================================================
	//= ODriverEnumeration
	//====================================================================
	class ODriverEnumerationImpl;
	/** simple class for accessing SDBC drivers registered within the office
		<p>Rather small, introduced to not contaminate other instances with the
		exception handling (code-size-bloating) implementations here.
		</p>
	*/
	class ODriverEnumeration
	{
	private:
		ODriverEnumerationImpl*	m_pImpl;

	public:
		ODriverEnumeration() throw();
		~ODriverEnumeration() throw();
		typedef ::std::vector< ::rtl::OUString >::const_iterator const_iterator;

		const_iterator	begin() const throw();
		const_iterator	end() const throw();
	};

//........................................................................
}	// namespace offapp
//........................................................................

#endif // _OFFMGR_SDBCDRIVERENUMERATION_HXX_


