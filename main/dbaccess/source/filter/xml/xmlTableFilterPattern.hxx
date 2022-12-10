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


#ifndef DBA_XMLTABLEFILTERPATTERN_HXX
#define DBA_XMLTABLEFILTERPATTERN_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

namespace dbaxml
{
	class OXMLTableFilterList;
	class OXMLTableFilterPattern : public SvXMLImportContext
	{
		OXMLTableFilterList&	m_rParent;
		sal_Bool				m_bNameFilter;
	public:
		OXMLTableFilterPattern(	SvXMLImport& rImport
								,sal_uInt16 nPrfx
								,const ::rtl::OUString& rLName
								,sal_Bool _bNameFilter
								,OXMLTableFilterList& _rParent);

		virtual ~OXMLTableFilterPattern();

		virtual void Characters( const ::rtl::OUString& rChars );
	};
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLEFILTERPATTERN_HXX
