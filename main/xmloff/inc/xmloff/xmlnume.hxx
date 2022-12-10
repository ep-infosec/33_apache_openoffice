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



#ifndef _XMLOFF_XMLNUME_HXX
#define _XMLOFF_XMLNUME_HXX

#include <rtl/ustring.hxx>
#include <tools/mapunit.hxx>
#include <xmloff/XMLStringVector.hxx>
#include <xmloff/attrlist.hxx>

namespace com { namespace sun { namespace star {
	namespace frame { class XModel; }
	namespace style { class XStyle; }
	namespace container { class XIndexReplace; }
	namespace beans { struct PropertyValue; }
} } }

class SvXMLNamespaceMap;
class SvXMLUnitConverter;
class SvXMLExportItemMapper;
class SvXMLExport;
class XMLTextListAutoStylePool;

class SvxXMLNumRuleExport
{
	SvXMLExport& rExport;
	const ::rtl::OUString sCDATA;
	const ::rtl::OUString sWS;
	const ::rtl::OUString sNumberingRules;
	const ::rtl::OUString sIsPhysical;
	const ::rtl::OUString sIsContinuousNumbering;
    // --> OD 2008-06-06 #i89178#
    // Boolean indicating, if properties for position-and-space-mode LABEL_ALIGNEMNT
    // are exported or not.
    // These properties have been introduced in ODF 1.2. Thus, its export have
    // to be suppressed on writing ODF 1.0 respectively ODF 1.1
    bool mbExportPositionAndSpaceModeLabelAlignment;
    // <--

	void exportLevelStyles(
			const ::com::sun::star::uno::Reference<
				::com::sun::star::container::XIndexReplace > & xNumRule,
			sal_Bool bOutline=sal_False );

	void exportLevelStyle(
			sal_Int32 nLevel,
			const ::com::sun::star::uno::Sequence<
				::com::sun::star::beans::PropertyValue>& rProps,
			sal_Bool bOutline=sal_False );

protected:

	// This method may be overloaded to add attributes to the <list-style>
	// element.
	virtual void AddListStyleAttributes();

	sal_Bool exportStyle( const ::com::sun::star::uno::Reference<
							::com::sun::star::style::XStyle >& rStyle );
	void exportOutline();

	SvXMLExport& GetExport() { return rExport; }

 	virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >  GetUNONumRule() const;

public:

	SvxXMLNumRuleExport( SvXMLExport& rExport );
	virtual ~SvxXMLNumRuleExport();

	void exportStyles( sal_Bool bUsed,
					   XMLTextListAutoStylePool *pPool,
					   sal_Bool bExportChapterNumbering = sal_True );
	void exportNumberingRule(
			const ::rtl::OUString& rName,
			const ::com::sun::star::uno::Reference<
				::com::sun::star::container::XIndexReplace > & xNumRule );

    // --> OD 2008-06-17 #i90780#
    // refactoring: removing unused methods
//    void Export( const ::rtl::OUString& rOutline, sal_Bool bContNumbering );
//    void ExportOutline();
    // <--

	static sal_Bool GetOutlineStyles( XMLStringVector& rStyleNames,
			   const ::com::sun::star::uno::Reference<
							::com::sun::star::frame::XModel > & rModel );

};

#endif	//  _XMLOFF_XMLNUME_HXX

