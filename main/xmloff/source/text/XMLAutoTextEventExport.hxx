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



#ifndef _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX
#define _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX

#include <xmloff/xmlexp.hxx>

#include <set>


namespace rtl {	class OUString; }
namespace com { namespace sun { namespace star {
	namespace container { class XNameAccess; }
	namespace frame { class XModel; }
	namespace lang { class XMultiServiceFactory; }
	namespace uno { template<class X> class Reference; }
	namespace uno { template<class X> class Sequence; }
	namespace uno { class XInterface; }
	namespace uno { class Exception; }
	namespace xml { namespace sax { class XDocumentHandler; } }
} } }


/** 
 * Component for the export of events attached to autotext blocks.
 * Via the XInitialization interface it expects up to two strings, the
 * first giving the file name (URL) of the autotext group, and the second 
 * identifying the autotext. If one of the strings is not given, it 
 * will export the whole group / all groups.
 */
class XMLAutoTextEventExport : public SvXMLExport
{
	::com::sun::star::uno::Reference<
		::com::sun::star::container::XNameAccess> xEvents;

	const ::rtl::OUString sEventType;
	const ::rtl::OUString sNone;


public:

	// #110680#
	//XMLAutoTextEventExport();
	XMLAutoTextEventExport(
		const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory, sal_uInt16 nFlags
		);

	// #110680#
	//XMLAutoTextEventExport(
	//	const ::rtl::OUString& rFileName,
	//	const ::com::sun::star::uno::Reference<
	//		::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
	//	const ::com::sun::star::uno::Reference<
	//		::com::sun::star::frame::XModel > & rModel,
	//	const ::com::sun::star::uno::Reference<
	//		::com::sun::star::container::XNameAccess > & rEvents);
	XMLAutoTextEventExport(
		const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
		const ::rtl::OUString& rFileName,
		const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
		const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & rModel,
		const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & rEvents, sal_uInt16 nFlags);

	~XMLAutoTextEventExport();

	// XInitialization
    virtual void SAL_CALL initialize( 
		const ::com::sun::star::uno::Sequence<
			::com::sun::star::uno::Any> & rArguments ) 
		throw(
			::com::sun::star::uno::Exception, 
			::com::sun::star::uno::RuntimeException);

protected:

	/// export the events off all autotexts
	virtual sal_uInt32 exportDoc( 
        enum ::xmloff::token::XMLTokenEnum eClass = xmloff::token::XML_TOKEN_INVALID );

	/// does the document have any events ?
	sal_Bool hasEvents();

	/// export the events element
	void exportEvents();


	/// add the namespaces used by events 
	/// (to be called for the document element)
	void addNamespaces();


	// methods without content:
	virtual void _ExportMeta();
	virtual void _ExportScripts();
	virtual void _ExportFontDecls();
	virtual void _ExportStyles( sal_Bool bUsed ) ;
	virtual void _ExportAutoStyles();
	virtual void _ExportMasterStyles();
	virtual void _ExportChangeTracking();
	virtual void _ExportContent();
};



// global functions to support the component

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
	XMLAutoTextEventExport_getSupportedServiceNames()
	throw();

::rtl::OUString SAL_CALL XMLAutoTextEventExport_getImplementationName() 
	throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
	XMLAutoTextEventExportOOO_createInstance(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::lang::XMultiServiceFactory > & )
	throw( ::com::sun::star::uno::Exception );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
	XMLAutoTextEventExportOOO_getSupportedServiceNames()
	throw();

::rtl::OUString SAL_CALL XMLAutoTextEventExportOOO_getImplementationName() 
	throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
	XMLAutoTextEventExportOOO_createInstance(
		const ::com::sun::star::uno::Reference< 
			::com::sun::star::lang::XMultiServiceFactory > & )
	throw( ::com::sun::star::uno::Exception );

#endif

