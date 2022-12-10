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

#include <comphelper/extract.hxx>
#include "ximp3dobject.hxx"
#include <xmloff/XMLShapeStyleContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include "xexptran.hxx"
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <xmloff/families.hxx>
#include "ximpstyl.hxx"
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#include <com/sun/star/drawing/DoubleSequence.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DObjectContext, SdXMLShapeContext );

SdXML3DObjectContext::SdXML3DObjectContext(
	SvXMLImport& rImport,
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:	SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
	mbSetTransform( sal_False )
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for(sal_Int16 i=0; i < nAttrCount; i++)
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		OUString sValue = xAttrList->getValueByIndex( i );
		const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DObjectAttrTokenMap();

		switch(rAttrTokenMap.Get(nPrefix, aLocalName))
		{
			case XML_TOK_3DOBJECT_DRAWSTYLE_NAME:
			{
				maDrawStyleName = sValue;
				break;
			}
			case XML_TOK_3DOBJECT_TRANSFORM:
			{
				SdXMLImExTransform3D aTransform(sValue, GetImport().GetMM100UnitConverter());
				if(aTransform.NeedsAction())
					mbSetTransform = aTransform.GetFullHomogenTransform(mxHomMat);
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DObjectContext::~SdXML3DObjectContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DObjectContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
	uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
	if(xPropSet.is())
	{
		// set parameters
		if(mbSetTransform)
		{
			uno::Any aAny;
			aAny <<= mxHomMat;
			xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DTransformMatrix")), aAny);
		}

		// call parent
		SdXMLShapeContext::StartElement(xAttrList);
	}
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DObjectContext::EndElement()
{
	// call parent
	SdXMLShapeContext::EndElement();
}

//////////////////////////////////////////////////////////////////////////////
/*
void SdXML3DObjectContext::AddShape(uno::Reference< drawing::XShape >& xShape)
{
	if(xShape.is() && mxShapes.is())
	{
		// set shape local
		mxShape = xShape;

		// add new shape to parent
		mxShapes->add( xShape );
	}
}
*/
//////////////////////////////////////////////////////////////////////////////
/*
void SdXML3DObjectContext::SetStyle()
{
	// set style on shape
	if(maDrawStyleName.getLength() && mxShape.is())
	{
		const SvXMLStyleContext* pStyle = 0L;
		sal_Bool bAutoStyle(sal_False);

		if(GetImport().GetShapeImport()->GetAutoStylesContext())
			pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(
			XML_STYLE_FAMILY_SD_GRAPHICS_ID, maDrawStyleName);

		if(pStyle)
			bAutoStyle = sal_True;

		if(!pStyle && GetImport().GetShapeImport()->GetStylesContext())
			pStyle = GetImport().GetShapeImport()->GetStylesContext()->
			FindStyleChildContext(XML_STYLE_FAMILY_SD_GRAPHICS_ID, maDrawStyleName);

		if(pStyle && pStyle->ISA(XMLShapeStyleContext))
		{
			uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
			if(xPropSet.is())
			{
				XMLShapeStyleContext* pDocStyle = (XMLShapeStyleContext*)pStyle;

				if(pDocStyle->GetStyle().is())
				{
					// set style on object
					uno::Any aAny;
					aAny <<= pDocStyle->GetStyle();
					xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Style")), aAny);
				}

				if(bAutoStyle)
				{
					// set PropertySet on object
					pDocStyle->FillPropertySet(xPropSet);
				}
			}
		}
	}
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DCubeObjectShapeContext, SdXML3DObjectContext);

SdXML3DCubeObjectShapeContext::SdXML3DCubeObjectShapeContext(
	SvXMLImport& rImport,
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:	SdXML3DObjectContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
	maMinEdge(-2500.0, -2500.0, -2500.0),
	maMaxEdge(2500.0, 2500.0, 2500.0),
	mbMinEdgeUsed(sal_False),
	mbMaxEdgeUsed(sal_False)
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for(sal_Int16 i=0; i < nAttrCount; i++)
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		OUString sValue = xAttrList->getValueByIndex( i );
		const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DCubeObjectAttrTokenMap();

		switch(rAttrTokenMap.Get(nPrefix, aLocalName))
		{
			case XML_TOK_3DCUBEOBJ_MINEDGE:
			{
				::basegfx::B3DVector aNewVec;
				GetImport().GetMM100UnitConverter().convertB3DVector(aNewVec, sValue);

				if(aNewVec != maMinEdge)
				{
					maMinEdge = aNewVec;
					mbMinEdgeUsed = sal_True;
				}
				break;
			}
			case XML_TOK_3DCUBEOBJ_MAXEDGE:
			{
				::basegfx::B3DVector aNewVec;
				GetImport().GetMM100UnitConverter().convertB3DVector(aNewVec, sValue);

				if(aNewVec != maMaxEdge)
				{
					maMaxEdge = aNewVec;
					mbMaxEdgeUsed = sal_True;
				}
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DCubeObjectShapeContext::~SdXML3DCubeObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DCubeObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
	// create shape
	AddShape( "com.sun.star.drawing.Shape3DCubeObject" );
	if(mxShape.is())
	{
		// add, set style and properties from base shape
		SetStyle();
		SdXML3DObjectContext::StartElement(xAttrList);

		// set local parameters on shape
		uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
		if(xPropSet.is())
		{
			// set parameters
			drawing::Position3D aPosition3D;
			drawing::Direction3D aDirection3D;

			// convert from min, max to size to be set
			maMaxEdge = maMaxEdge - maMinEdge;

			aPosition3D.PositionX = maMinEdge.getX();
			aPosition3D.PositionY = maMinEdge.getY();
			aPosition3D.PositionZ = maMinEdge.getZ();

			aDirection3D.DirectionX = maMaxEdge.getX();
			aDirection3D.DirectionY = maMaxEdge.getY();
			aDirection3D.DirectionZ = maMaxEdge.getZ();

			uno::Any aAny;
			aAny <<= aPosition3D;
			xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")), aAny);
			aAny <<= aDirection3D;
			xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")), aAny);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DCubeObjectShapeContext::EndElement()
{
	// call parent
	SdXML3DObjectContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DSphereObjectShapeContext, SdXML3DObjectContext);

SdXML3DSphereObjectShapeContext::SdXML3DSphereObjectShapeContext(
	SvXMLImport& rImport,
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:	SdXML3DObjectContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
	maCenter(0.0, 0.0, 0.0),
	maSize(5000.0, 5000.0, 5000.0),
	mbCenterUsed(sal_False),
	mbSizeUsed(sal_False)
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for(sal_Int16 i=0; i < nAttrCount; i++)
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		OUString sValue = xAttrList->getValueByIndex( i );
		const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DSphereObjectAttrTokenMap();

		switch(rAttrTokenMap.Get(nPrefix, aLocalName))
		{
			case XML_TOK_3DSPHEREOBJ_CENTER:
			{
				::basegfx::B3DVector aNewVec;
				GetImport().GetMM100UnitConverter().convertB3DVector(aNewVec, sValue);

				if(aNewVec != maCenter)
				{
					maCenter = aNewVec;
					mbCenterUsed = sal_True;
				}
				break;
			}
			case XML_TOK_3DSPHEREOBJ_SIZE:
			{
				::basegfx::B3DVector aNewVec;
				GetImport().GetMM100UnitConverter().convertB3DVector(aNewVec, sValue);

				if(aNewVec != maSize)
				{
					maSize = aNewVec;
					mbSizeUsed = sal_True;
				}
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DSphereObjectShapeContext::~SdXML3DSphereObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSphereObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
	// create shape
	AddShape( "com.sun.star.drawing.Shape3DSphereObject" );
	if(mxShape.is())
	{
		// add, set style and properties from base shape
		SetStyle();
		SdXML3DObjectContext::StartElement(xAttrList);

		// set local parameters on shape
		uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
		if(xPropSet.is())
		{
			// set parameters
			drawing::Position3D aPosition3D;
			drawing::Direction3D aDirection3D;

			aPosition3D.PositionX = maCenter.getX();
			aPosition3D.PositionY = maCenter.getY();
			aPosition3D.PositionZ = maCenter.getZ();

			aDirection3D.DirectionX = maSize.getX();
			aDirection3D.DirectionY = maSize.getY();
			aDirection3D.DirectionZ = maSize.getZ();

			uno::Any aAny;
			aAny <<= aPosition3D;
			xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")), aAny);
			aAny <<= aDirection3D;
			xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")), aAny);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSphereObjectShapeContext::EndElement()
{
	// call parent
	SdXML3DObjectContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DPolygonBasedShapeContext, SdXML3DObjectContext );

SdXML3DPolygonBasedShapeContext::SdXML3DPolygonBasedShapeContext(
	SvXMLImport& rImport,
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:	SdXML3DObjectContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
	sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
	for(sal_Int16 i=0; i < nAttrCount; i++)
	{
		OUString sAttrName = xAttrList->getNameByIndex( i );
		OUString aLocalName;
		sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
		OUString sValue = xAttrList->getValueByIndex( i );
		const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DPolygonBasedAttrTokenMap();

		switch(rAttrTokenMap.Get(nPrefix, aLocalName))
		{
			case XML_TOK_3DPOLYGONBASED_VIEWBOX:
			{
				maViewBox = sValue;
				break;
			}
			case XML_TOK_3DPOLYGONBASED_D:
			{
				maPoints = sValue;
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DPolygonBasedShapeContext::~SdXML3DPolygonBasedShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DPolygonBasedShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);

    if(xPropSet.is())
    {
        // set parameters
        if(maPoints.getLength() && maViewBox.getLength())
        {
            // import 2d PolyPolygon from svg:d
            basegfx::B2DPolyPolygon aPolyPolygon;

            if(basegfx::tools::importFromSvgD(aPolyPolygon, maPoints, true, 0))
            {
                // convert to 3D PolyPolygon
                const basegfx::B3DPolyPolygon aB3DPolyPolygon(
                    basegfx::tools::createB3DPolyPolygonFromB2DPolyPolygon(
                        aPolyPolygon));

                // convert to UNO API class PolyPolygonShape3D
                drawing::PolyPolygonShape3D xPolyPolygon3D;
                basegfx::tools::B3DPolyPolygonToUnoPolyPolygonShape3D(
                    aB3DPolyPolygon, 
                    xPolyPolygon3D);

                // set polygon data
                uno::Any aAny;
                aAny <<= xPolyPolygon3D;
                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPolyPolygon3D")), aAny);
            }
            else
            {
                OSL_ENSURE(false, "Error on importing svg:d for 3D PolyPolygon (!)");
            }
        }

        // call parent
        SdXML3DObjectContext::StartElement(xAttrList);
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DPolygonBasedShapeContext::EndElement()
{
	// call parent
	SdXML3DObjectContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DLatheObjectShapeContext, SdXML3DPolygonBasedShapeContext);

SdXML3DLatheObjectShapeContext::SdXML3DLatheObjectShapeContext(
	SvXMLImport& rImport,
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:	SdXML3DPolygonBasedShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DLatheObjectShapeContext::~SdXML3DLatheObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DLatheObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
	// create shape
	AddShape( "com.sun.star.drawing.Shape3DLatheObject" );
	if(mxShape.is())
	{
		// add, set style and properties from base shape
		SetStyle();
		SdXML3DPolygonBasedShapeContext::StartElement(xAttrList);
	}
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DLatheObjectShapeContext::EndElement()
{
	// call parent
	SdXML3DPolygonBasedShapeContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DExtrudeObjectShapeContext, SdXML3DPolygonBasedShapeContext);

SdXML3DExtrudeObjectShapeContext::SdXML3DExtrudeObjectShapeContext(
	SvXMLImport& rImport,
	sal_uInt16 nPrfx,
	const OUString& rLocalName,
	const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
	uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:	SdXML3DPolygonBasedShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DExtrudeObjectShapeContext::~SdXML3DExtrudeObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DExtrudeObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
	AddShape( "com.sun.star.drawing.Shape3DExtrudeObject" );
	if(mxShape.is())
	{
		// add, set style and properties from base shape
		SetStyle();
		SdXML3DPolygonBasedShapeContext::StartElement(xAttrList);
	}
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DExtrudeObjectShapeContext::EndElement()
{
	// call parent
	SdXML3DPolygonBasedShapeContext::EndElement();
}

// EOF
