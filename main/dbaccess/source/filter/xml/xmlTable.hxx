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


#ifndef DBA_XMLTABLE_HXX
#define DBA_XMLTABLE_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

namespace dbaxml
{
	class ODBFilter;
	class OXMLTable : public SvXMLImportContext
	{
	protected:
		::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >	m_xParentContainer;
		::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >		m_xTable;
		::rtl::OUString m_sFilterStatement;
		::rtl::OUString m_sOrderStatement;
		::rtl::OUString m_sName;
		::rtl::OUString m_sSchema;
		::rtl::OUString m_sCatalog;
		::rtl::OUString m_sStyleName;
		::rtl::OUString m_sServiceName;
		sal_Bool		m_bApplyFilter;
		sal_Bool		m_bApplyOrder;

	
		ODBFilter& GetOwnImport();

		void fillAttributes(	sal_uInt16 nPrfx
								,const ::rtl::OUString& _sLocalName
								,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
								, ::rtl::OUString& _rsCommand
								,::rtl::OUString& _rsTableName
								,::rtl::OUString& _rsTableSchema
								,::rtl::OUString& _rsTableCatalog
							);

		virtual void setProperties(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xProp);
	public:

		OXMLTable( ODBFilter& rImport
					, sal_uInt16 nPrfx
					,const ::rtl::OUString& rLName
					,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
					,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
					,const ::rtl::OUString& _sServiceName
					);
		virtual ~OXMLTable();

		virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
					const ::rtl::OUString& rLocalName,
					const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
		virtual void EndElement();
	};
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLTABLE_HXX
