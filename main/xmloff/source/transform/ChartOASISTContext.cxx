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
#include "ChartOASISTContext.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#ifndef _XMLOFF_TRANSFORMERBASE_HXX
#include "TransformerBase.hxx"
#endif

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

// -----------------------------------------------------------------------------

TYPEINIT1( XMLChartOASISTransformerContext, XMLTransformerContext );

XMLChartOASISTransformerContext::XMLChartOASISTransformerContext( 
		XMLTransformerBase& rImp, 
		const OUString& rQName ) :
	XMLTransformerContext( rImp, rQName )
{
}

XMLChartOASISTransformerContext::~XMLChartOASISTransformerContext()
{
}

void XMLChartOASISTransformerContext::StartElement( 
	const Reference< XAttributeList >& rAttrList )
{
	XMLTransformerActions *pActions =
		GetTransformer().GetUserDefinedActions( OASIS_CHART_ACTIONS );
	OSL_ENSURE( pActions, "go no actions" );
	
	OUString aAddInName;
	Reference< XAttributeList > xAttrList( rAttrList );
	XMLMutableAttributeList *pMutableAttrList = 0;
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for( sal_Int16 i=0; i < nAttrCount; i++ )
	{
		const OUString& rAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix =
			GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName, 
																 &aLocalName );
		XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
		XMLTransformerActions::const_iterator aIter =
			pActions->find( aKey );
		if( !(aIter == pActions->end() ) )
		{
			if( !pMutableAttrList )
			{
				pMutableAttrList = 
						new XMLMutableAttributeList( xAttrList );
				xAttrList = pMutableAttrList;
			}
			const OUString& rAttrValue = xAttrList->getValueByIndex( i );
			switch( (*aIter).second.m_nActionType )
			{
			case XML_ATACTION_IN2INCH:
				{
					OUString aAttrValue( rAttrValue );
					if( XMLTransformerBase::ReplaceSingleInWithInch( 
								aAttrValue ) )
						pMutableAttrList->SetValueByIndex( i, aAttrValue );
				}
				break;
			case XML_ATACTION_DECODE_STYLE_NAME_REF:
				{
					OUString aAttrValue( rAttrValue );
					if( GetTransformer().DecodeStyleName(aAttrValue) )
						pMutableAttrList->SetValueByIndex( i, aAttrValue );
				}
				break;
			case XML_ATACTION_REMOVE_ANY_NAMESPACE_PREFIX:
				OSL_ENSURE( IsXMLToken( aLocalName, XML_CLASS ),
					   		"unexpected class token" );
				{
					OUString aChartClass;
					sal_uInt16 nValuePrefix =
						GetTransformer().GetNamespaceMap().GetKeyByAttrName(
                            rAttrValue,
                            &aChartClass );
					if( XML_NAMESPACE_CHART == nValuePrefix )
					{
						pMutableAttrList->SetValueByIndex( i, aChartClass );
					}
					else if ( XML_NAMESPACE_OOO == nValuePrefix )
					{
						pMutableAttrList->SetValueByIndex( i,
												GetXMLToken(XML_ADD_IN ) );
						aAddInName = aChartClass;
					}
				}
				break;
			default:
				OSL_ENSURE( sal_False, "unknown action" );
				break;
			}
		}
	}

	if( aAddInName.getLength() > 0 )
	{
		OUString aAttrQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
								XML_NAMESPACE_CHART,
								GetXMLToken( XML_ADD_IN_NAME ) ) );
		pMutableAttrList->AddAttribute( aAttrQName, aAddInName );
	}

	XMLTransformerContext::StartElement( xAttrList );
}
