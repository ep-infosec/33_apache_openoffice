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
#include "precompiled_editeng.hxx"
#include <SvXMLAutoCorrectImport.hxx>
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;


static OUString sBlockList ( RTL_CONSTASCII_USTRINGPARAM ( "_block-list" ) );

// #110680#
SvXMLAutoCorrectImport::SvXMLAutoCorrectImport(
	const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
	SvxAutocorrWordList *pNewAutocorr_List,
	SvxAutoCorrect &rNewAutoCorrect,
    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rNewStorage)
:	SvXMLImport( xServiceFactory ),
	pAutocorr_List (pNewAutocorr_List),
	rAutoCorrect ( rNewAutoCorrect ),
    xStorage ( rNewStorage )
{
	GetNamespaceMap().Add(
			sBlockList,
			GetXMLToken ( XML_N_BLOCK_LIST),
			XML_NAMESPACE_BLOCKLIST );
}

SvXMLAutoCorrectImport::~SvXMLAutoCorrectImport ( void ) throw ()
{
}

SvXMLImportContext *SvXMLAutoCorrectImport::CreateContext(
		sal_uInt16 nPrefix,
		const OUString& rLocalName,
		const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
	SvXMLImportContext *pContext = 0;

	if( XML_NAMESPACE_BLOCKLIST == nPrefix &&
		IsXMLToken ( rLocalName, XML_BLOCK_LIST ) )
		pContext = new SvXMLWordListContext( *this, nPrefix, rLocalName, xAttrList );
	else
		pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
	return pContext;
}

SvXMLWordListContext::SvXMLWordListContext(
   SvXMLAutoCorrectImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & /*xAttrList*/ ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
}

SvXMLImportContext *SvXMLWordListContext::CreateChildContext(
	sal_uInt16 nPrefix,
	const OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
	SvXMLImportContext *pContext = 0;

	if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
		IsXMLToken ( rLocalName, XML_BLOCK ) )
		pContext = new SvXMLWordContext (rLocalRef, nPrefix, rLocalName, xAttrList);
	else
		pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
	return pContext;
}
SvXMLWordListContext::~SvXMLWordListContext ( void )
{
}

SvXMLWordContext::SvXMLWordContext(
   SvXMLAutoCorrectImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
	String sRight, sWrong;
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

	for (sal_Int16 i=0; i < nAttrCount; i++)
	{
		const OUString& rAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
		const OUString& rAttrValue = xAttrList->getValueByIndex( i );
		if (XML_NAMESPACE_BLOCKLIST == nAttrPrefix)
		{
			if ( IsXMLToken ( aLocalName, XML_ABBREVIATED_NAME ) )
			{
				sWrong = rAttrValue;
			}
			else if ( IsXMLToken ( aLocalName, XML_NAME ) )
			{
				sRight = rAttrValue;
			}
		}
	}
	if (!sWrong.Len() || !sRight.Len() )
		return;

//	const International& rInter = Application::GetAppInternational();
//	sal_Bool bOnlyTxt = COMPARE_EQUAL != rInter.Compare( sRight, sWrong, INTN_COMPARE_IGNORECASE );
	sal_Bool bOnlyTxt = sRight != sWrong;
	if( !bOnlyTxt )
	{
		String sLongSave( sRight );
        if( !rLocalRef.rAutoCorrect.GetLongText( rLocalRef.xStorage, String(), sWrong, sRight ) &&
            sLongSave.Len() )
		{
			sRight = sLongSave;
			bOnlyTxt = sal_True;
		}
	}
	SvxAutocorrWordPtr pNew = new SvxAutocorrWord( sWrong, sRight, bOnlyTxt );

	if( !rLocalRef.pAutocorr_List->Insert( pNew ) )
		delete pNew;
}

SvXMLWordContext::~SvXMLWordContext ( void )
{
}

// #110680#
SvXMLExceptionListImport::SvXMLExceptionListImport(
	const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
	SvStringsISortDtor & rNewList )
:	SvXMLImport( xServiceFactory ),
	rList (rNewList)
{
	GetNamespaceMap().Add(
			sBlockList,
			GetXMLToken ( XML_N_BLOCK_LIST),
			XML_NAMESPACE_BLOCKLIST );
}

SvXMLExceptionListImport::~SvXMLExceptionListImport ( void ) throw ()
{
}

SvXMLImportContext *SvXMLExceptionListImport::CreateContext(
		sal_uInt16 nPrefix,
		const OUString& rLocalName,
		const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
	SvXMLImportContext *pContext = 0;

	if( XML_NAMESPACE_BLOCKLIST==nPrefix &&
		IsXMLToken ( rLocalName, XML_BLOCK_LIST ) )
		pContext = new SvXMLExceptionListContext( *this, nPrefix, rLocalName, xAttrList );
	else
		pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
	return pContext;
}

SvXMLExceptionListContext::SvXMLExceptionListContext(
   SvXMLExceptionListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & /* xAttrList */ ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
}

SvXMLImportContext *SvXMLExceptionListContext::CreateChildContext(
	sal_uInt16 nPrefix,
	const OUString& rLocalName,
	const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
	SvXMLImportContext *pContext = 0;

	if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
		IsXMLToken ( rLocalName, XML_BLOCK ) )
		pContext = new SvXMLExceptionContext (rLocalRef, nPrefix, rLocalName, xAttrList);
	else
		pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
	return pContext;
}
SvXMLExceptionListContext::~SvXMLExceptionListContext ( void )
{
}

SvXMLExceptionContext::SvXMLExceptionContext(
   SvXMLExceptionListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
	String sWord;
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

	for (sal_Int16 i=0; i < nAttrCount; i++)
	{
		const OUString& rAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
		const OUString& rAttrValue = xAttrList->getValueByIndex( i );
		if (XML_NAMESPACE_BLOCKLIST == nAttrPrefix)
		{
			if ( IsXMLToken ( aLocalName, XML_ABBREVIATED_NAME ) )
			{
				sWord = rAttrValue;
			}
		}
	}
	if (!sWord.Len() )
		return;

	String * pNew = new String( sWord );

	if( !rLocalRef.rList.Insert( pNew ) )
		delete pNew;
}

SvXMLExceptionContext::~SvXMLExceptionContext ( void )
{
}
