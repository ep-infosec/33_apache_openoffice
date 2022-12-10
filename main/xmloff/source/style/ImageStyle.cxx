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
#include "xmloff/ImageStyle.hxx"
#include <com/sun/star/awt/XBitmap.hpp>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include"xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/xmltkmap.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;

enum SvXMLTokenMapAttrs
{
	XML_TOK_IMAGE_NAME,
	XML_TOK_IMAGE_DISPLAY_NAME,
	XML_TOK_IMAGE_URL,
	XML_TOK_IMAGE_TYPE,
	XML_TOK_IMAGE_SHOW,
	XML_TOK_IMAGE_ACTUATE,
	/* XML_TOK_IMAGE_SIZEW,
	   XML_TOK_IMAGE_SIZEH,*/
	XML_TOK_TABSTOP_END=XML_TOK_UNKNOWN
};


XMLImageStyle::XMLImageStyle()
{
}

XMLImageStyle::~XMLImageStyle()
{
}

#ifndef SVX_LIGHT

sal_Bool XMLImageStyle::exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue, SvXMLExport& rExport )
{
	return ImpExportXML( rStrName, rValue, rExport );
}

sal_Bool XMLImageStyle::ImpExportXML( const OUString& rStrName, const uno::Any& rValue, SvXMLExport& rExport )
{
	sal_Bool bRet = sal_False;

	OUString sImageURL;

	if( rStrName.getLength() )
	{
		if( rValue >>= sImageURL )
		{
			OUString aStrValue;
			OUStringBuffer aOut;

			// Name
			sal_Bool bEncoded = sal_False;
			rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, 
								  rExport.EncodeStyleName( rStrName,
														   &bEncoded ) );
			if( bEncoded )
				rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, 
									  rStrName );
			
			// uri
			const OUString aStr( rExport.AddEmbeddedGraphicObject( sImageURL ) );
			if( aStr.getLength() )
			{
				rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aStr );
				rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
				rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
				rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
			}
/*
			// size			
			awt::Size aSize = xBitmap->getSize();

			rUnitConverter.convertNumber( aOut, aSize.Width );
			aStrValue = aOut.makeStringAndClear();
			AddAttribute( XML_NAMESPACE_SVG, XML_WIDTH, aStrValue );

			rUnitConverter.convertNumber( aOut, aSize.Height );
			aStrValue = aOut.makeStringAndClear();
			AddAttribute( XML_NAMESPACE_SVG, XML_HEIGHT, aStrValue );
*/			
			// Do Write
			SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_FILL_IMAGE, sal_True, sal_True );

			if( sImageURL.getLength() )
			{
				// optional office:binary-data
				rExport.AddEmbeddedGraphicObjectAsBase64( sImageURL );
			}
		}
	}

	return bRet;
}

#endif // #ifndef SVX_LIGHT

sal_Bool XMLImageStyle::importXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Any& rValue, OUString& rStrName, SvXMLImport& rImport )
{
	return ImpImportXML( xAttrList, rValue, rStrName, rImport );
}

sal_Bool XMLImageStyle::ImpImportXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, 
									  uno::Any& rValue, OUString& rStrName,
									  SvXMLImport& rImport )
{
	sal_Bool bRet     = sal_False;
	sal_Bool bHasHRef = sal_False;
	sal_Bool bHasName = sal_False;
	OUString aStrURL;
	OUString aDisplayName;

    {
        static __FAR_DATA SvXMLTokenMapEntry aHatchAttrTokenMap[] =
{
	{ XML_NAMESPACE_DRAW, XML_NAME, XML_TOK_IMAGE_NAME },
	{ XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, XML_TOK_IMAGE_DISPLAY_NAME },
	{ XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_IMAGE_URL },
	{ XML_NAMESPACE_XLINK, XML_TYPE, XML_TOK_IMAGE_TYPE },
	{ XML_NAMESPACE_XLINK, XML_SHOW, XML_TOK_IMAGE_SHOW },
	{ XML_NAMESPACE_XLINK, XML_ACTUATE, XML_TOK_IMAGE_ACTUATE },
	/*{ XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_IMAGE_URL },
	{ XML_NAMESPACE_XLINK, XML_HREF, XML_TOK_IMAGE_URL },*/
	XML_TOKEN_MAP_END 
};

	SvXMLTokenMap aTokenMap( aHatchAttrTokenMap );

	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		const OUString& rFullAttrName = xAttrList->getNameByIndex( i );
		OUString aStrAttrName;
		sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rFullAttrName, &aStrAttrName );
		const OUString& rStrValue = xAttrList->getValueByIndex( i );

		switch( aTokenMap.Get( nPrefix, aStrAttrName ) )
		{
			case XML_TOK_IMAGE_NAME:
				{
					rStrName = rStrValue;
					bHasName = sal_True;
				}			
				break;
			case XML_TOK_IMAGE_DISPLAY_NAME:
				{
					aDisplayName = rStrValue;
				}			
				break;
			case XML_TOK_IMAGE_URL:
				{
					aStrURL = rImport.ResolveGraphicObjectURL( rStrValue, sal_False );
					bHasHRef = sal_True;
				}			
				break;
			case XML_TOK_IMAGE_TYPE:
				// ignore
				break;
			case XML_TOK_IMAGE_SHOW:
				// ignore
				break;
			case XML_TOK_IMAGE_ACTUATE:
				// ignore
				break;
			default:
				DBG_WARNING( "Unknown token at import fill bitmap style" )
				;
		}
	}

	rValue <<= aStrURL;

	if( aDisplayName.getLength() )
	{
		rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_FILL_IMAGE_ID, 
									 rStrName, aDisplayName );
		rStrName = aDisplayName;
	}

	bRet = bHasName && bHasHRef;

    }

	return bRet;
}
