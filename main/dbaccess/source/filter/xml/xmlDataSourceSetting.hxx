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


#ifndef DBA_XMLDATASOURCESETTING_HXX
#define DBA_XMLDATASOURCESETTING_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace dbaxml
{
	class ODBFilter;
	class OXMLDataSource;
	class OXMLDataSourceSetting : public SvXMLImportContext
	{
		::com::sun::star::beans::PropertyValue m_aSetting;
		::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> m_aInfoSequence;
		OXMLDataSourceSetting* m_pContainer;
		::com::sun::star::uno::Type	m_aPropType;			// the type of the property the instance imports currently
		sal_Bool m_bIsList;

		ODBFilter& GetOwnImport();
		::com::sun::star::uno::Any convertString(const ::com::sun::star::uno::Type& _rExpectedType, const ::rtl::OUString& _rReadCharacters);
	public:

		OXMLDataSourceSetting( ODBFilter& rImport, sal_uInt16 nPrfx,
					const ::rtl::OUString& rLName,
					const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
					,OXMLDataSourceSetting* _pContainer = NULL);
		virtual ~OXMLDataSourceSetting();

		virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
					const ::rtl::OUString& rLocalName,
					const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

		virtual void EndElement();

		virtual void Characters( const ::rtl::OUString& rChars );


		/** adds value to property
			@param	_sValue
				The value to add.
		*/
		void addValue(const ::rtl::OUString& _sValue);
	};
// -----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------

#endif // DBA_XMLDATASOURCESETTING_HXX
