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
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <xmloff/xmltoken.hxx>
#include <comphelper/extract.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "ximpshow.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;
using namespace ::xmloff::token;

///////////////////////////////////////////////////////////////////////

class ShowsImpImpl
{
public:
	Reference< XSingleServiceFactory > mxShowFactory;
	Reference< XNameContainer > mxShows;
	Reference< XPropertySet > mxPresProps;
	Reference< XNameAccess > mxPages;
	OUString maCustomShowName;
	SdXMLImport& mrImport;

	ShowsImpImpl( SdXMLImport& rImport )
	:	mrImport( rImport )
	{}
};

///////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLShowsContext, SvXMLImportContext );

SdXMLShowsContext::SdXMLShowsContext( SdXMLImport& rImport,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList )
:	SvXMLImportContext(rImport, nPrfx, rLocalName)
{
	mpImpl = new ShowsImpImpl( rImport );

	Reference< XCustomPresentationSupplier > xShowsSupplier( rImport.GetModel(), UNO_QUERY );
	if( xShowsSupplier.is() )
	{
		mpImpl->mxShows = xShowsSupplier->getCustomPresentations();
		mpImpl->mxShowFactory = Reference< XSingleServiceFactory >::query( mpImpl->mxShows );
	}

	Reference< XDrawPagesSupplier > xDrawPagesSupplier( rImport.GetModel(), UNO_QUERY );
	if( xDrawPagesSupplier.is() )
		mpImpl->mxPages = Reference< XNameAccess >::query( xDrawPagesSupplier->getDrawPages() );

	Reference< XPresentationSupplier > xPresentationSupplier( rImport.GetModel(), UNO_QUERY );
	if( xPresentationSupplier.is() )
		mpImpl->mxPresProps = Reference< XPropertySet >::query( xPresentationSupplier->getPresentation() );


	if( mpImpl->mxPresProps.is() )
	{
		sal_Bool bAll = sal_True;
		uno::Any aAny;

		// read attributes
		const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
		for(sal_Int16 i=0; i < nAttrCount; i++)
		{
			OUString sAttrName = xAttrList->getNameByIndex( i );
			OUString aLocalName;
			sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
			OUString sValue = xAttrList->getValueByIndex( i );

			switch( nPrefix )
			{
			case XML_NAMESPACE_PRESENTATION:
				if( IsXMLToken( aLocalName, XML_START_PAGE ) )
				{
					aAny <<= sValue;
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FirstPage" ) ), aAny );
					bAll = sal_False;
				}
				else if( IsXMLToken( aLocalName, XML_SHOW ) )
				{
					mpImpl->maCustomShowName = sValue;
					bAll = sal_False;
				}
				else if( IsXMLToken( aLocalName, XML_PAUSE ) )
				{
					DateTime aTime;
					if( !SvXMLUnitConverter::convertTime( aTime,  sValue ) )
						continue;

					const sal_Int32 nMS = ( aTime.Hours * 60 + aTime.Minutes ) * 60 + aTime.Seconds;
					aAny <<= nMS;
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Pause" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_ANIMATIONS ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_ENABLED ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AllowAnimations" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_STAY_ON_TOP ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAlwaysOnTop" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_FORCE_MANUAL ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAutomatic" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_ENDLESS ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsEndless" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_FULL_SCREEN ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsFullScreen" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_MOUSE_VISIBLE ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsMouseVisible" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_START_WITH_NAVIGATOR ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "StartWithNavigator" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_MOUSE_AS_PEN ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UsePen" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_TRANSITION_ON_CLICK ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_ENABLED ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsTransitionOnClick" ) ), aAny );
				}
				else if( IsXMLToken( aLocalName, XML_SHOW_LOGO ) )
				{
					aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
					mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowLogo" ) ), aAny );
				}
			}
		}
		aAny = bool2any( bAll );
		mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "IsShowAll" ) ), aAny );
	}
}

SdXMLShowsContext::~SdXMLShowsContext()
{
	if( mpImpl && mpImpl->maCustomShowName.getLength() )
	{
		uno::Any aAny;
		aAny <<= mpImpl->maCustomShowName;
		mpImpl->mxPresProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CustomShow" ) ), aAny );
	}

	delete mpImpl;
}

SvXMLImportContext * SdXMLShowsContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
{
	if( mpImpl && p_nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_SHOW ) )
	{
		OUString aName;
		OUString aPages;

		// read attributes
		const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
		for(sal_Int16 i=0; i < nAttrCount; i++)
		{
			OUString sAttrName = xAttrList->getNameByIndex( i );
			OUString aLocalName;
			sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
			OUString sValue = xAttrList->getValueByIndex( i );

			switch( nPrefix )
			{
			case XML_NAMESPACE_PRESENTATION:
				if( IsXMLToken( aLocalName, XML_NAME ) )
				{
					aName = sValue;
				}
				else if( IsXMLToken( aLocalName, XML_PAGES ) )
				{
					aPages = sValue;
				}
			}
		}

		if( aName.getLength() != 0 && aPages.getLength() != 0 )
		{
			Reference< XIndexContainer > xShow( mpImpl->mxShowFactory->createInstance(), UNO_QUERY );
			if( xShow.is() )
			{
				SvXMLTokenEnumerator aPageNames( aPages, sal_Unicode(',') );
				OUString sPageName;
				Any aAny;

				while( aPageNames.getNextToken( sPageName ) )
				{
					if( !mpImpl->mxPages->hasByName( sPageName ) )
						continue;

					Reference< XDrawPage > xPage;
					mpImpl->mxPages->getByName( sPageName ) >>= xPage;
					if( xPage.is() )
					{
						aAny <<= xPage;
						xShow->insertByIndex( xShow->getCount(), aAny );
					}
				}

				aAny <<= xShow;

				if( mpImpl->mxShows->hasByName( aName ) )
				{
					mpImpl->mxShows->replaceByName( aName, aAny );
				}
				else
				{
					mpImpl->mxShows->insertByName( aName, aAny );
				}
			}
		}
	}

	return new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
}

