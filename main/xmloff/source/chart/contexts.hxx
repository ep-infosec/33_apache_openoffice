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


#ifndef _SCH_XML_CONTEXTS_HXX_
#define _SCH_XML_CONTEXTS_HXX_

#include "SchXMLImport.hxx"
#include "SchXMLTableContext.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltkmap.hxx>

#include <xmloff/xmlmetai.hxx>

namespace com { namespace sun { namespace star { namespace xml { namespace sax {
		class XAttributeList;
}}}}}

/* ========================================

   These contexts are only nedded by
   SchXMLImport not by the SchXMLImportHelper
   that is also used by other applications

   ======================================== */

class SchXMLDocContext : public virtual SvXMLImportContext
{
protected:
	SchXMLImportHelper& mrImportHelper;

public:
	SchXMLDocContext(
		SchXMLImportHelper& rImpHelper,
		SvXMLImport& rImport,
		sal_uInt16 nPrefix,
		const rtl::OUString& rLName );
	virtual ~SchXMLDocContext();

	TYPEINFO();

	virtual SvXMLImportContext* CreateChildContext(
		sal_uInt16 nPrefix,
		const ::rtl::OUString& rLocalName,
		const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ========================================

// context for flat file xml format
class SchXMLFlatDocContext_Impl
    : public SchXMLDocContext, public SvXMLMetaDocumentContext
{
public:
    SchXMLFlatDocContext_Impl(
		SchXMLImportHelper& i_rImpHelper,
        SchXMLImport& i_rImport,
        sal_uInt16 i_nPrefix, const ::rtl::OUString & i_rLName,
        const com::sun::star::uno::Reference<com::sun::star::document::XDocumentProperties>& i_xDocProps,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XDocumentHandler>& i_xDocBuilder);

    virtual ~SchXMLFlatDocContext_Impl();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 i_nPrefix, const ::rtl::OUString& i_rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& i_xAttrList);
};

// ========================================

class SchXMLBodyContext : public SvXMLImportContext
{
private:
	SchXMLImportHelper& mrImportHelper;

public:
	SchXMLBodyContext(
		SchXMLImportHelper& rImpHelper,
		SvXMLImport& rImport,
		sal_uInt16 nPrefix,
		const rtl::OUString& rLName );
	virtual ~SchXMLBodyContext();

	virtual void EndElement();
	virtual SvXMLImportContext* CreateChildContext(
		sal_uInt16 nPrefix,
		const ::rtl::OUString& rLocalName,
		const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

// ========================================

#endif	// _SCH_XML_CONTEXTS_HXX_
