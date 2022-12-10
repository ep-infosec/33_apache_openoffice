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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include <tools/debug.hxx>

#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace com::sun::star;
using namespace ::xmloff::token;


//===========================================================================

/// builds a DOM tree from SAX events, by forwarding to SAXDocumentBuilder
class XMLDocumentBuilderContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler> mxDocBuilder;

public:
    XMLDocumentBuilderContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler>& rDocBuilder);

    virtual ~XMLDocumentBuilderContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();
};

XMLDocumentBuilderContext::XMLDocumentBuilderContext(SvXMLImport& rImport,
        sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const uno::Reference<xml::sax::XAttributeList>&,
        const uno::Reference<xml::sax::XDocumentHandler>& rDocBuilder) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mxDocBuilder(rDocBuilder)
{
}

XMLDocumentBuilderContext::~XMLDocumentBuilderContext()
{
}

SvXMLImportContext *
XMLDocumentBuilderContext::CreateChildContext( sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& rAttrs)
{
    return new XMLDocumentBuilderContext(
                GetImport(), nPrefix, rLocalName, rAttrs, mxDocBuilder);
}

void XMLDocumentBuilderContext::StartElement(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    mxDocBuilder->startElement(
      GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(), GetLocalName()),
      xAttrList);
}

void XMLDocumentBuilderContext::Characters( const ::rtl::OUString& rChars )
{
    mxDocBuilder->characters(rChars);
}

void XMLDocumentBuilderContext::EndElement()
{
    mxDocBuilder->endElement(
      GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(), GetLocalName()));
}


//===========================================================================

static void
lcl_initDocumentProperties(SvXMLImport & rImport,
        uno::Reference<xml::sax::XDocumentHandler> const& xDocBuilder,
        uno::Reference<document::XDocumentProperties> const& xDocProps)
{
    uno::Sequence< uno::Any > aSeq(1);
    uno::Reference< xml::dom::XSAXDocumentBuilder > const xDB(xDocBuilder,
        uno::UNO_QUERY_THROW);
    aSeq[0] <<= xDB->getDocument();
    uno::Reference< lang::XInitialization > const xInit(xDocProps,
        uno::UNO_QUERY_THROW);
    try {
        xInit->initialize(aSeq);
        rImport.SetStatistics(xDocProps->getDocumentStatistics());
        // convert all URLs from relative to absolute
        xDocProps->setTemplateURL(rImport.GetAbsoluteReference(
            xDocProps->getTemplateURL()));
        xDocProps->setAutoloadURL(rImport.GetAbsoluteReference(
            xDocProps->getAutoloadURL()));
        SvXMLMetaDocumentContext::setBuildId(
            xDocProps->getGenerator(), rImport.getImportInfo());
    } catch (uno::RuntimeException) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString::createFromAscii(
                "SvXMLMetaDocumentContext::initDocumentProperties: "
                "properties init exception"),
            rImport, makeAny(e));
    }
}

static void
lcl_initGenerator(SvXMLImport & rImport,
        uno::Reference<xml::sax::XDocumentHandler> const& xDocBuilder)
{
    uno::Reference< xml::dom::XSAXDocumentBuilder > const xDB(xDocBuilder,
        uno::UNO_QUERY_THROW);
    uno::Reference< xml::dom::XDocument > const xDoc(xDB->getDocument(),
        uno::UNO_SET_THROW);
    try {
        uno::Reference< xml::xpath::XXPathAPI > const xPath(
            rImport.getServiceFactory()->createInstance(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.xml.xpath.XPathAPI"))),
            uno::UNO_QUERY_THROW );
        xPath->registerNS(GetXMLToken(XML_NP_OFFICE),GetXMLToken(XML_N_OFFICE));
        xPath->registerNS(GetXMLToken(XML_NP_META), GetXMLToken(XML_N_META));

        ::rtl::OUString const expr(RTL_CONSTASCII_USTRINGPARAM(
            "string(/office:document-meta/office:meta/meta:generator)"));
        uno::Reference< xml::xpath::XXPathObject > const xObj(
            xPath->eval(xDoc.get(), expr), uno::UNO_SET_THROW);
        OUString const value(xObj->getString());
        SvXMLMetaDocumentContext::setBuildId(value, rImport.getImportInfo());
    } catch (uno::RuntimeException) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString::createFromAscii(
                "SvXMLMetaDocumentContext::initGenerator: exception"),
            rImport, makeAny(e));
    }
}

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
            sal_uInt16 nPrfx, const rtl::OUString& rLName,
            const uno::Reference<document::XDocumentProperties>& xDocProps,
            const uno::Reference<xml::sax::XDocumentHandler>& xDocBuilder) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mxDocProps(xDocProps),
    mxDocBuilder(xDocBuilder)
{
// #i103539#: must always read meta.xml for generator, xDocProps unwanted then
//    OSL_ENSURE(xDocProps.is(), "SvXMLMetaDocumentContext: no document props");
    OSL_ENSURE(xDocBuilder.is(), "SvXMLMetaDocumentContext: no document hdlr");
    // here are no attributes
}

SvXMLMetaDocumentContext::~SvXMLMetaDocumentContext()
{
}

SvXMLImportContext *SvXMLMetaDocumentContext::CreateChildContext(
             sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
             const uno::Reference<xml::sax::XAttributeList>& rAttrs)
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_META) )
    {
        return new XMLDocumentBuilderContext(
                GetImport(), nPrefix, rLocalName, rAttrs, mxDocBuilder);
    }
    else
    {
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
}


void SvXMLMetaDocumentContext::StartElement(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    mxDocBuilder->startDocument();
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->startElement(
        GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(),
            GetXMLToken(XML_DOCUMENT_META)), xAttrList);

}

void SvXMLMetaDocumentContext::EndElement()
{
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->endElement(
        GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(),
            GetXMLToken(XML_DOCUMENT_META)));
    mxDocBuilder->endDocument();
    if (mxDocProps.is())
    {
        lcl_initDocumentProperties(GetImport(), mxDocBuilder, mxDocProps);
    }
    else
    {
        lcl_initGenerator(GetImport(), mxDocBuilder);
    }
}

void SvXMLMetaDocumentContext::setBuildId(::rtl::OUString const& i_rBuildId, const uno::Reference<beans::XPropertySet>& xImportInfo )
{
    OUString sBuildId;
    // skip to second product
    sal_Int32 nBegin = i_rBuildId.indexOf( ' ' );
    if ( nBegin != -1 )
    {
        // skip to build information
        nBegin = i_rBuildId.indexOf( '/', nBegin );
        if ( nBegin != -1 )
        {
            sal_Int32 nEnd = i_rBuildId.indexOf( 'm', nBegin );
            if ( nEnd != -1 )
            {
                OUStringBuffer sBuffer(
                    i_rBuildId.copy( nBegin+1, nEnd-nBegin-1 ) );
                const OUString sBuildCompare(
                    RTL_CONSTASCII_USTRINGPARAM( "$Build-" ) );
                nBegin = i_rBuildId.indexOf( sBuildCompare, nEnd );
                if ( nBegin != -1 )
                {
                    sBuffer.append( (sal_Unicode)'$' );
                    sBuffer.append( i_rBuildId.copy(
                        nBegin + sBuildCompare.getLength() ) );
                    sBuildId = sBuffer.makeStringAndClear();
                }
            }
        }
    }

    if ( sBuildId.getLength() == 0 )
    {
        if ((i_rBuildId.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("StarOffice 7") ) == 0) ||
            (i_rBuildId.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("StarSuite 7") ) == 0)  ||
            (i_rBuildId.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("OpenOffice.org 1") ) == 0))
        {
            sBuildId = OUString::createFromAscii( "645$8687" );
        }
        if ((i_rBuildId.compareToAscii( RTL_CONSTASCII_STRINGPARAM("NeoOffice/2") ) == 0) )
        {
            sBuildId = OUString::createFromAscii( "680$9134" ); // fake NeoOffice as OpenOffice.org 2.2 release
        }
    }
    else
    {
        if ((i_rBuildId.compareToAscii( RTL_CONSTASCII_STRINGPARAM("LibreOffice/3") ) == 0) )
        {
            // #118558# fake LibreOffice3 as OpenOffice.org 3.3 release
            sBuildId = OUString::createFromAscii( "330$9567" ); 
        }
    }

    if ( sBuildId.getLength() ) try
    {
        if( xImportInfo.is() )
        {
            const OUString aPropName(RTL_CONSTASCII_USTRINGPARAM("BuildId"));
            uno::Reference< beans::XPropertySetInfo > xSetInfo(
                xImportInfo->getPropertySetInfo());
            if( xSetInfo.is() && xSetInfo->hasPropertyByName( aPropName ) )
                xImportInfo->setPropertyValue( aPropName, uno::makeAny( sBuildId ) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

