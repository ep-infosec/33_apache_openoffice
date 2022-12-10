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

#include "DomBuilderContext.hxx"

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlerror.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/XAttr.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>

#include <unotools/processfactory.hxx>


using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::xml::dom::XAttr;
using com::sun::star::xml::dom::XDocument;
using com::sun::star::xml::dom::XDocumentBuilder;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XElement;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::dom::NodeType_ELEMENT_NODE;
using rtl::OUString;


// helper functions; implemented below
Reference<XNode> lcl_createDomInstance();
Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_uInt16 nPrefix, 
                                    const OUString rLocalName,
                                    Reference<XNode> xParent);


DomBuilderContext::DomBuilderContext( SvXMLImport& rImport, 
                                      sal_uInt16 nPrefix,
                                      const OUString& rLocalName ) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mxNode( lcl_createElement( rImport, nPrefix, rLocalName, 
                               lcl_createDomInstance() ) )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    DBG_ASSERT( Reference<XElement>( mxNode, UNO_QUERY ).is(), "need element" );
    DBG_ASSERT( mxNode->getNodeType() == NodeType_ELEMENT_NODE, "need element" );
}

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport, 
                                      sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      Reference<XNode>& xParent ) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mxNode( lcl_createElement( rImport, nPrefix, rLocalName, xParent ) )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    DBG_ASSERT( Reference<XElement>( mxNode, UNO_QUERY ).is(), "need element" );
    DBG_ASSERT( mxNode->getNodeType() == NodeType_ELEMENT_NODE, "need element" );
}

DomBuilderContext::~DomBuilderContext()
{
}

Reference<XDocument> DomBuilderContext::getTree()
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    return mxNode->getOwnerDocument();
}

Reference<XNode> DomBuilderContext::getNode()
{
    return mxNode;
}


SvXMLImportContext* DomBuilderContext::CreateChildContext( 
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    // create DomBuilder for subtree
    return new DomBuilderContext( GetImport(), nPrefix, rLocalName, mxNode );
}


void DomBuilderContext::StartElement( 
    const Reference<XAttributeList>& xAttrList )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    DBG_ASSERT( mxNode->getOwnerDocument().is(), "XNode must have XDocument" );

    // add attribute nodes to new node
    sal_Int16 nAttributeCount = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nAttributeCount; i++ )
    {
        // get name & value for attribute
        const OUString& rName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        // namespace handling: determine namespace & namespace keykey
        OUString sNamespace;
        sal_uInt16 nNamespaceKey = 
            GetImport().GetNamespaceMap()._GetKeyByAttrName( 
                rName, NULL, NULL, &sNamespace );

        // create attribute node and set value
        Reference<XElement> xElement( mxNode, UNO_QUERY_THROW );
        switch( nNamespaceKey )
        {
        case XML_NAMESPACE_NONE:
            // no namespace: create a non-namespaced attribute
            xElement->setAttribute( rName, rValue );
            break;
        case XML_NAMESPACE_XMLNS:
            // namespace declaration: ignore, since the DOM tree handles these
            // declarations implicitly
            break;
        case XML_NAMESPACE_UNKNOWN:
            // unknown namespace: illegal input. Raise Warning.
            {
                Sequence<OUString> aSeq(2);
                aSeq[0] = rName;
                aSeq[1] = rValue;
                GetImport().SetError( 
                    XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, aSeq );
            }
            break;
        default:
            // a real and proper namespace: create namespaced attribute
            xElement->setAttributeNS( sNamespace, rName, rValue );
            break;
        }
    }
}

void DomBuilderContext::EndElement()
{
    // nothing to be done!
}

void DomBuilderContext::Characters( const OUString& rCharacters )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );

    // TODO: I assume adjacent text nodes should be joined, to preserve
    // processinf model? (I.e., if the SAX parser breaks a string into 2
    // Characters(..) calls, the DOM model would still see only one child.)

    // create text node and append to parent
    Reference<XNode> xNew( 
        mxNode->getOwnerDocument()->createTextNode( rCharacters ), 
        UNO_QUERY_THROW );
    mxNode->appendChild( xNew );
}


//
// helper function implementations
//

const sal_Char sDocumentBuilder[] = "com.sun.star.xml.dom.DocumentBuilder";

Reference<XNode> lcl_createDomInstance()
{
    Reference<XMultiServiceFactory> xFactory = utl::getProcessServiceFactory();
    DBG_ASSERT( xFactory.is(), "can't get service factory" );

    Reference<XDocumentBuilder> xBuilder(
        xFactory->createInstance( 
            OUString( RTL_CONSTASCII_USTRINGPARAM( sDocumentBuilder ) ) ), 
        UNO_QUERY_THROW );

    return Reference<XNode>( xBuilder->newDocument(), UNO_QUERY_THROW );
}

Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_uInt16 nPrefix, 
                                    const OUString rLocalName,
                                    Reference<XNode> xParent)
{
    DBG_ASSERT( xParent.is(), "need parent node" );

    Reference<XDocument> xDocument = xParent->getOwnerDocument();
    DBG_ASSERT( xDocument.is(), "no XDocument found!" );

    // TODO: come up with proper way of handling namespaces; re-creating the
    // namespace from the key is NOT a good idea, and will not work for
    // multiple prefixes for the same namespace. Fortunately, those are rare.

    Reference<XElement> xElement;
    switch( nPrefix )
    {
    case XML_NAMESPACE_NONE:
        // no namespace: use local name
        xElement = xDocument->createElement( rLocalName );
        break;
    case XML_NAMESPACE_XMLNS:
    case XML_NAMESPACE_UNKNOWN:
        // both cases are illegal; raise warning (and use only local name)
        xElement = xDocument->createElement( rLocalName );
        {
            Sequence<OUString> aSeq(1);
            aSeq[0] = rLocalName;
            rImport.SetError( 
                XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, aSeq );
        }
        break;
    default:
        // We are only given the prefix and the local name; thus we have to ask
        // the namespace map to create a qualified name for us. Technically,
        // this is a bug, since this will fail for multiple prefixes used for
        // the same namespace.
        xElement = xDocument->createElementNS(
            rImport.GetNamespaceMap().GetNameByKey( nPrefix ), 
            rImport.GetNamespaceMap().GetQNameByKey( nPrefix, rLocalName ) );
        break;
    }
    DBG_ASSERT( xElement.is(), "can't create element" );

    // add new element to parent and return
    Reference<XNode> xNode( xElement, UNO_QUERY_THROW ); 
    xParent->appendChild( xNode );
    return xNode;
}
