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



#include "buildlistcontext.hxx"
#include <rtl/ustring.hxx>
#include "oox/helper/attributelist.hxx"


using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using ::rtl::OUString;

namespace oox { namespace ppt {

    BuildListContext::BuildListContext( ContextHandler& rParent,
                const Reference< XFastAttributeList >& /*xAttribs*/,
                TimeNodePtrList & aTimeNodeList)
        : ContextHandler( rParent )
		, maTimeNodeList( aTimeNodeList )
		, mbInBldGraphic( false )
		,	mbBuildAsOne( false )
	{
	}

	BuildListContext::~BuildListContext( )
	{
	}

	void SAL_CALL BuildListContext::endFastElement( sal_Int32 aElement ) throw ( SAXException, RuntimeException)
	{
		switch( aElement )
		{
		case PPT_TOKEN( bldGraphic ):
			mbInBldGraphic = false;
			break;
		default:
			break;
		}
	}

	Reference< XFastContextHandler > SAL_CALL BuildListContext::createFastChildContext( ::sal_Int32 aElementToken,
																																										 const Reference< XFastAttributeList >& xAttribs )
		throw ( SAXException, RuntimeException )
	{
		Reference< XFastContextHandler > xRet;

		switch( aElementToken )
		{
		case PPT_TOKEN( bldAsOne ):
			if( mbInBldGraphic )
			{
				mbBuildAsOne = true;
			}
			break;
		case PPT_TOKEN( bldSub ):
			if( mbInBldGraphic )
			{
			}
			break;
		case PPT_TOKEN( bldGraphic ):
		{
			mbInBldGraphic = true;
			AttributeList attribs( xAttribs );
			OUString sShapeId = xAttribs->getOptionalValue( XML_spid );
// TODO
//		bool uiExpand = attribs.getBool( XML_uiExpand, true );
				/* this is unsigned */
//		sal_uInt32 nGroupId =  attribs.getUnsignedInteger( XML_grpId, 0 );
			break;
		}
		case A_TOKEN( bldDgm ):
		case A_TOKEN( bldOleChart ):
		case A_TOKEN( bldP ):

			break;
		default:
			break;
		}

		if( !xRet.is() )
			xRet.set(this);

		return xRet;
	}


} }
