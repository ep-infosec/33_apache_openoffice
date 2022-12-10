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
#include <tools/debug.hxx>
#ifndef _SVSTDARR_STRINGSSORTDTOR_DECL
#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
#endif
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#ifndef _XMLOFF_XMLITMAP_HXX
//#include "xmlitmap.hxx"
#endif
#include <xmloff/xmluconv.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <xmloff/xmlaustp.hxx>
#ifndef _XMLOFF_STYLEEXP_HXX
#include <xmloff/styleexp.hxx>
#endif
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLEventExport.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::document::XEventsSupplier;

XMLStyleExport::XMLStyleExport(
		SvXMLExport& rExp,
		const ::rtl::OUString& rPoolStyleName,
		SvXMLAutoStylePoolP *pAutoStyleP ) :
	rExport( rExp ),
	sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
	sIsAutoUpdate( RTL_CONSTASCII_USTRINGPARAM( "IsAutoUpdate" ) ),
	sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) ),
	sNumberingStyleName( RTL_CONSTASCII_USTRINGPARAM( "NumberingStyleName" ) ),
	sOutlineLevel( RTL_CONSTASCII_USTRINGPARAM( "OutlineLevel" ) ),//#outline level,add by zhaojianwei
	sPoolStyleName( rPoolStyleName ),
	pAutoStylePool( pAutoStyleP  )
{
}

XMLStyleExport::~XMLStyleExport()
{
}

void XMLStyleExport::exportStyleAttributes( const Reference< XStyle >& )
{
}

void XMLStyleExport::exportStyleContent( const Reference< XStyle >& )
{
}

sal_Bool XMLStyleExport::exportStyle(
		const Reference< XStyle >& rStyle,
	  	const OUString& rXMLFamily,
		const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
		const Reference< XNameAccess >& xStyles,		//#outline level,add by zhaojianwei
		const OUString* pPrefix )
{
	Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
	Reference< XPropertySetInfo > xPropSetInfo =
			xPropSet->getPropertySetInfo();
	Any aAny;

	// Don't export styles that aren't existing really. This may be the
	// case for StarOffice Writer's pool styles.
	if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
	{
		aAny = xPropSet->getPropertyValue( sIsPhysical );
		if( !*(sal_Bool *)aAny.getValue() )
			return sal_False;
	}

	// <style:style ...>
	GetExport().CheckAttrList();

	// style:name="..."
	OUString sName;

	if(pPrefix)
		sName = *pPrefix;
	sName += rStyle->getName();

	sal_Bool bEncoded = sal_False;
	const OUString sEncodedStyleName(GetExport().EncodeStyleName( sName, &bEncoded ));
	GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, sEncodedStyleName );

	if( bEncoded )
		GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
							  	 sName);

	// style:family="..."
	if( rXMLFamily.getLength() > 0 )
		GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, rXMLFamily);

	// style:parent-style-name="..."
	OUString sParentString(rStyle->getParentStyle());
	OUString sParent;

	if(sParentString.getLength())
	{
		if(pPrefix)
			sParent = *pPrefix;
		sParent += sParentString;
	}
	else
		sParent = sPoolStyleName;

	if( sParent.getLength() )
		GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_PARENT_STYLE_NAME,
					  			  GetExport().EncodeStyleName( sParent ) );

	// style:next-style-name="..." (paragraph styles only)
	if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
	{
		aAny = xPropSet->getPropertyValue( sFollowStyle );
		OUString sNextName;
		aAny >>= sNextName;
		if( sName != sNextName )
		{
			GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NEXT_STYLE_NAME,
						  GetExport().EncodeStyleName( sNextName ) );
		}
	}

	// style:auto-update="..." (SW only)
	if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
	{
		aAny = xPropSet->getPropertyValue( sIsAutoUpdate );
		if( *(sal_Bool *)aAny.getValue() )
			GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_AUTO_UPDATE,
                                      XML_TRUE );
	}

	// style:default-outline-level"..." //#outline level, add by zhaojianwei.0802
	sal_Int32 nOutlineLevel = 0;
	if( xPropSetInfo->hasPropertyByName( sOutlineLevel ) )
	{
		Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
		if( PropertyState_DIRECT_VALUE == xPropState->getPropertyState( sOutlineLevel ) )
		{
			aAny = xPropSet->getPropertyValue( sOutlineLevel );
			aAny >>= nOutlineLevel;
			if( nOutlineLevel > 0 )
			{
				OUStringBuffer sTmp;
				sTmp.append( static_cast<sal_Int32>(nOutlineLevel));
				GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DEFAULT_OUTLINE_LEVEL,
                                          sTmp.makeStringAndClear() );
			}
			else
			{
                // --> OD 2009-12-29 #i104889#
                // empty value for style:default-outline-level does exist
                // since ODF 1.2. Thus, suppress its export for former versions.
                if ( ( GetExport().getExportFlags() & EXPORT_OASIS ) != 0 &&
                     GetExport().getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
                // <--
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              XML_DEFAULT_OUTLINE_LEVEL,
                                              OUString( RTL_CONSTASCII_USTRINGPARAM( "" )));
                }
			}
		}
	}//<-end,zhaojianwei

	// style:list-style-name="..." (SW paragarph styles only)
	if( xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
	{
		Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
		if( PropertyState_DIRECT_VALUE ==
				xPropState->getPropertyState( sNumberingStyleName  ) )
		{
			aAny = xPropSet->getPropertyValue( sNumberingStyleName );
			if( aAny.hasValue() )
			{
				OUString sListName;
				aAny >>= sListName;

                // --> OD 2006-09-21 #i69523#
                // An direct set empty list style has to be written. Otherwise,
                // this information is lost and causes an error, if the parent
                // style has a list style set.
                if ( !sListName.getLength() )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              XML_LIST_STYLE_NAME,
                                              sListName /* empty string */);
                }
                else
                {
                    // --> OD 2006-09-27 #i69627#
                    bool bSuppressListStyle( false );
                    {
                        if ( !GetExport().writeOutlineStyleAsNormalListStyle() )
                        {
                            Reference< XChapterNumberingSupplier > xCNSupplier
                                (GetExport().GetModel(), UNO_QUERY);

                            OUString sOutlineName;
                            if (xCNSupplier.is())
                            {
                                Reference< XIndexReplace > xNumRule
                                    ( xCNSupplier->getChapterNumberingRules() );
                                DBG_ASSERT( xNumRule.is(), "no chapter numbering rules" );

                                if (xNumRule.is())
                                {
                                    Reference< XPropertySet > xNumRulePropSet
                                        (xNumRule, UNO_QUERY);
                                    xNumRulePropSet->getPropertyValue(
                                        OUString(RTL_CONSTASCII_USTRINGPARAM("Name")) )
                                        >>= sOutlineName;
                                    bSuppressListStyle = ( sListName == sOutlineName );
                                }
                            }
                        }
                    }

                    if ( sListName.getLength() && !bSuppressListStyle )
                    // <--
                    {
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_LIST_STYLE_NAME,
                                  GetExport().EncodeStyleName( sListName ) );
                    }
                }
                // <--
			}
		}
		//#outline level, add by zhaojianwei.0802
		else if( nOutlineLevel > 0 )
		{

			bool bNoInheritedListStyle( true );

			/////////////////////////////////////////////////
			Reference<XStyle> xStyle( xPropState, UNO_QUERY );
			while ( xStyle.is() )
			{
				OUString aParentStyle( xStyle->getParentStyle() );
				if ( aParentStyle.getLength() == 0 ||
					!xStyles->hasByName( aParentStyle ) )
				{
					break;
				}
				else
				{
					xPropState = Reference< XPropertyState >( xStyles->getByName( aParentStyle ), UNO_QUERY );
					if ( !xPropState.is() )
					{
						break;
					}
					if ( xPropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
					{
						bNoInheritedListStyle = false;
						break;
					}
					else
					{
						xStyle = Reference<XStyle>( xPropState, UNO_QUERY );
					}
				}
			}
			/////////////////////////////////////////////////
			if ( bNoInheritedListStyle )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_LIST_STYLE_NAME,
                                          OUString( RTL_CONSTASCII_USTRINGPARAM( "" )));
		}
		//<-end,zhaojianwei
	}


	// style:pool-id="..." is not required any longer since we use
	// english style names only
	exportStyleAttributes( rStyle );

	// TODO: style:help-file-name="..." and style:help-id="..." can neither
	// be modified by UI nor by API and that for, have not to be exported
	// currently.

	{
		// <style:style>
		SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_STYLE,
								  sal_True, sal_True );

		rPropMapper->SetStyleName( sName );

		// <style:properties>
		::std::vector< XMLPropertyState > xPropStates =
			rPropMapper->Filter( xPropSet );
		rPropMapper->exportXML( GetExport(), xPropStates,
                                XML_EXPORT_FLAG_IGN_WS );

		rPropMapper->SetStyleName( OUString() );

		exportStyleContent( rStyle );

		// <script:events>, if they are supported by this style
		Reference<XEventsSupplier> xEventsSupp(rStyle, UNO_QUERY);
		GetExport().GetEventExport().Export(xEventsSupp);
	}
	return sal_True;
}

sal_Bool XMLStyleExport::exportDefaultStyle(
		const Reference< XPropertySet >& xPropSet,
	  	const OUString& rXMLFamily,
		const UniReference < SvXMLExportPropertyMapper >& rPropMapper )
{
	Reference< XPropertySetInfo > xPropSetInfo =
			xPropSet->getPropertySetInfo();

	Any aAny;

	// <style:default-style ...>
	GetExport().CheckAttrList();

	{
		// style:family="..."
		if( rXMLFamily.getLength() > 0 )
			GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY,
                                      rXMLFamily );
		// <style:style>
		SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
								  XML_DEFAULT_STYLE,
								  sal_True, sal_True );
		// <style:properties>
		//::std::vector< XMLPropertyState > xPropStates =
		//	rPropMapper->FilterDefaults( xPropSet );
		::std::vector< XMLPropertyState > xPropStates =
			rPropMapper->FilterDefaults( xPropSet );
		rPropMapper->exportXML( GetExport(), xPropStates,
							   	  XML_EXPORT_FLAG_IGN_WS );
//		exportStyleContent( rStyle );
	}
	return sal_True;
}

#if 0
void XMLStyleExport::exportStyleFamily(
	const sal_Char *pFamily,
	const OUString& rXMLFamily,
	const UniReference < XMLPropertySetMapper >& rPropMapper,
	sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
	const OUString sFamily(OUString::createFromAscii(pFamily ));
	UniReference < SvXMLExportPropertyMapper > xExpPropMapper =
		new SvXMLExportPropertyMapper( rPropMapper );
	exportStyleFamily( sFamily, rXMLFamily, xExpPropMapper, bUsed, nFamily,
					   pPrefix);
}

void XMLStyleExport::exportStyleFamily(
	const OUString& rFamily, const OUString& rXMLFamily,
	const UniReference < XMLPropertySetMapper >& rPropMapper,
	sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
	UniReference < SvXMLExportPropertyMapper > xExpPropMapper =
		new SvXMLExportPropertyMapper( rPropMapper );
	exportStyleFamily( rFamily, rXMLFamily, xExpPropMapper, bUsed, nFamily,
					   pPrefix);
}
#endif

void XMLStyleExport::exportStyleFamily(
	const sal_Char *pFamily,
	const OUString& rXMLFamily,
	const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
	sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
	const OUString sFamily(OUString::createFromAscii(pFamily ));
	exportStyleFamily( sFamily, rXMLFamily, rPropMapper, bUsed, nFamily,
					   pPrefix);
}

void XMLStyleExport::exportStyleFamily(
	const OUString& rFamily, const OUString& rXMLFamily,
	const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
	sal_Bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
	DBG_ASSERT( GetExport().GetModel().is(), "There is the model?" );
	Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(), UNO_QUERY );
	if( !xFamiliesSupp.is() )
		return; // family not available in current model

	Reference< XNameAccess > xStyleCont;

	Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
	if( xFamilies->hasByName( rFamily ) )
		xFamilies->getByName( rFamily ) >>= xStyleCont;

	if( !xStyleCont.is() )
		return;

	Reference< XNameAccess > xStyles( xStyleCont, UNO_QUERY );
   	// If next styles are supported and used styles should be exported only,
	// the next style may be unused but has to be exported, too. In this case
	// the names of all exported styles are remembered.
	SvStringsSortDtor *pExportedStyles = 0;
	sal_Bool bFirstStyle = sal_True;

    const uno::Sequence< ::rtl::OUString> aSeq = xStyles->getElementNames();
    const ::rtl::OUString* pIter = aSeq.getConstArray();
    const ::rtl::OUString* pEnd	  = pIter + aSeq.getLength();
    for(;pIter != pEnd;++pIter)
    {
		Reference< XStyle > xStyle;
		try
		{
			xStyles->getByName( *pIter ) >>= xStyle;
		}
		catch( lang::IndexOutOfBoundsException )
		{
			// due to bugs in prior versions it is possible that
			// a binary file is missing some critical styles.
			// The only possible way to deal with this is to
			// not export them here and remain silent.
			continue;
		}

		DBG_ASSERT( xStyle.is(), "Style not found for export!" );
		if( xStyle.is() )
		{
			if( !bUsed || xStyle->isInUse() )
			{
				sal_Bool bExported = exportStyle( xStyle, rXMLFamily, rPropMapper,
											  xStyles,pPrefix );
				if( bUsed && bFirstStyle && bExported  )
				{
					// If this is the first style, find out whether next styles
					// are supported.
					Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
					Reference< XPropertySetInfo > xPropSetInfo =
						xPropSet->getPropertySetInfo();

					if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
						pExportedStyles = new SvStringsSortDtor;
					bFirstStyle = sal_False;
				}

				if( pExportedStyles && bExported )
				{
					// If next styles are supported, remember this style's name.
					String *pTmp = new String( xStyle->getName() );
					if( !pExportedStyles->Insert( pTmp ) )
						delete pTmp;
				}
			}

			// if an auto style pool is given, remember this style's name as a
			// style name that must not be used by automatic styles.
			if( pAutoStylePool )
				pAutoStylePool->RegisterName( nFamily, xStyle->getName() );
		}
	}

	if( pExportedStyles )
	{
		// if next styles are supported, export all next styles that are
		// unused and that for, haven't been exported in the first loop.
        pIter = aSeq.getConstArray();
		for(;pIter != pEnd;++pIter)
		{
			Reference< XStyle > xStyle;
			xStyles->getByName( *pIter ) >>= xStyle;

			DBG_ASSERT( xStyle.is(), "Style not found for export!" );
			if( xStyle.is() )
			{
				Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
				Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

				// styles that aren't existing really are ignored.
				if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
				{
					Any aAny( xPropSet->getPropertyValue( sIsPhysical ) );
					if( !*(sal_Bool *)aAny.getValue() )
						continue;
				}

				if( !xStyle->isInUse() )
					continue;

				if( !xPropSetInfo->hasPropertyByName( sFollowStyle ) )
				{
					DBG_ASSERT( 0==sFollowStyle.getLength(),
								"no follow style???" );
					continue;
				}

				OUString sNextName;
				xPropSet->getPropertyValue( sFollowStyle ) >>= sNextName;
				String sTmp( sNextName );
				// if the next style hasn't been exported by now, export it now
				// and remember its name.
				if( xStyle->getName() != sNextName &&
					!pExportedStyles->Seek_Entry( &sTmp ) )
				{
					xStyleCont->getByName( sNextName ) >>= xStyle;
					DBG_ASSERT( xStyle.is(), "Style not found for export!" );

					if( xStyle.is() && exportStyle( xStyle, rXMLFamily, rPropMapper, xStyles,pPrefix ) )
						pExportedStyles->Insert( new String( sTmp ) );
				}
			}
		}
	}

	delete pExportedStyles;
}


