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



#ifndef _XMLOFF_XMLINDEXCHAPTERINFOENTRYCONTEXT_HXX_
#define _XMLOFF_XMLINDEXCHAPTERINFOENTRYCONTEXT_HXX_

#include "XMLIndexSimpleEntryContext.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>


namespace com { namespace sun { namespace star {
	namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl {	class OUString; }
class XMLIndexTemplateContext;

/**
 * Import chapter info index entry templates
 */
class XMLIndexChapterInfoEntryContext : public XMLIndexSimpleEntryContext
{
	// chapter format
	sal_Int16 nChapterInfo;
	sal_Bool bChapterInfoOK;
	sal_Bool bTOC;
    sal_Int16 nOutlineLevel;
    sal_Bool bOutlineLevelOK;

public:

	TYPEINFO();

	XMLIndexChapterInfoEntryContext(
		SvXMLImport& rImport, 
		XMLIndexTemplateContext& rTemplate,
		sal_uInt16 nPrfx,
		const ::rtl::OUString& rLocalName,
		sal_Bool bTOC );

	~XMLIndexChapterInfoEntryContext();

protected:

	/** process parameters */
	virtual void StartElement(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::xml::sax::XAttributeList> & xAttrList);

	/** fill property values for this template entry */
	virtual void FillPropertyValues(
		::com::sun::star::uno::Sequence<
			::com::sun::star::beans::PropertyValue> & rValues);
};

#endif
