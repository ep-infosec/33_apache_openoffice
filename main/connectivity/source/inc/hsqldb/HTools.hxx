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



#ifndef CONNECTIVITY_HTOOLS_HXX
#define CONNECTIVITY_HTOOLS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <rtl/ustrbuf.hxx>

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

	//====================================================================
	//= HTools
	//====================================================================
    class HTools
    {
    public:
        /** appens a proper WHERE clause to the given buffer, which filters
            for a given table name

            @param _bShortForm
                <TRUE/> if the column names of the system table which is being asked
                have the short form (TABLE_CAT instead of TABLE_CATALOG, and so on)
        */
        static void appendTableFilterCrit(
            ::rtl::OUStringBuffer& _inout_rBuffer, const ::rtl::OUString& _rCatalog,
            const ::rtl::OUString _rSchema, const ::rtl::OUString _rName,
            bool _bShortForm
        );
    };

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

#endif // CONNECTIVITY_HTOOLS_HXX
