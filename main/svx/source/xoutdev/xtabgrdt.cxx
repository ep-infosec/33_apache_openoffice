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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>
#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"
#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <vcl/svapp.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtGradient[]	= {'s','o','g'};
//char const aChckGradient[]  = { 0x04, 0x00, 'S','O','G','L'};	// < 5.2
//char const aChckGradient0[] = { 0x04, 0x00, 'S','O','G','0'};	// = 5.2
//char const aChckXML[]       = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// --------------------
// class XGradientList
// --------------------

XGradientList::XGradientList( const String& rPath )
:   XPropertyList(rPath )
{
}

XGradientList::~XGradientList()
{
}

XGradientEntry* XGradientList::Replace(XGradientEntry* pEntry, long nIndex )
{
	return( (XGradientEntry*) XPropertyList::Replace( pEntry, nIndex ) );
}

XGradientEntry* XGradientList::Remove(long nIndex)
{
	return( (XGradientEntry*) XPropertyList::Remove( nIndex ) );
}

XGradientEntry* XGradientList::GetGradient(long nIndex) const
{
	return( (XGradientEntry*) XPropertyList::Get( nIndex ) );
}

bool XGradientList::Load()
{
	if( mbListDirty )
	{
		mbListDirty = false;

		INetURLObject aURL( maPath );

		if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
		{
			DBG_ASSERT( !maPath.Len(), "invalid URL" );
			return false;
		}

		aURL.Append( maName );

		if( !aURL.getExtension().getLength() )
			aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

		uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );

	}

    return false;
}

bool XGradientList::Save()
{
	INetURLObject aURL( maPath );

	if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
	{
		DBG_ASSERT( !maPath.Len(), "invalid URL" );
		return false;
	}

	aURL.Append( maName );

	if( !aURL.getExtension().getLength() )
		aURL.setExtension( rtl::OUString( pszExtGradient, 3 ) );

	uno::Reference< container::XNameContainer > xTable( SvxUnoXGradientTable_createInstance( this ), uno::UNO_QUERY );
	return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

bool XGradientList::Create()
{
	XubString aStr( SVX_RES( RID_SVXSTR_GRADIENT ) );
	xub_StrLen nLen;

	aStr.AppendAscii(" 1");
	nLen = aStr.Len() - 1;
	Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLACK  ),RGB_Color(COL_WHITE	),XGRAD_LINEAR	  ,    0,10,10, 0,100,100),aStr));
	aStr.SetChar(nLen, sal_Unicode('2'));
	Insert(new XGradientEntry(XGradient(RGB_Color(COL_BLUE	 ),RGB_Color(COL_RED	),XGRAD_AXIAL	  ,  300,20,20,10,100,100),aStr));
	aStr.SetChar(nLen, sal_Unicode('3'));
	Insert(new XGradientEntry(XGradient(RGB_Color(COL_RED	 ),RGB_Color(COL_YELLOW ),XGRAD_RADIAL	  ,  600,30,30,20,100,100),aStr));
	aStr.SetChar(nLen, sal_Unicode('4'));
	Insert(new XGradientEntry(XGradient(RGB_Color(COL_YELLOW ),RGB_Color(COL_GREEN	),XGRAD_ELLIPTICAL,  900,40,40,30,100,100),aStr));
	aStr.SetChar(nLen, sal_Unicode('5'));
	Insert(new XGradientEntry(XGradient(RGB_Color(COL_GREEN  ),RGB_Color(COL_MAGENTA),XGRAD_SQUARE	  , 1200,50,50,40,100,100),aStr));
	aStr.SetChar(nLen, sal_Unicode('6'));
	Insert(new XGradientEntry(XGradient(RGB_Color(COL_MAGENTA),RGB_Color(COL_YELLOW ),XGRAD_RECT	  , 1900,60,60,50,100,100),aStr));

	return true;
}

Bitmap XGradientList::CreateBitmapForUI(long nIndex)
{
    Bitmap aRetval;
    OSL_ENSURE(nIndex < Count(), "OOps, access out of range (!)");

    if(nIndex < Count()) 
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();

        // prepare polygon geometry for rectangle
        const basegfx::B2DPolygon aRectangle(
            basegfx::tools::createPolygonFromRect(
                basegfx::B2DRange(0.0, 0.0, rSize.Width(), rSize.Height())));

        const XGradient& rGradient = GetGradient(nIndex)->GetGradient();
        const sal_uInt16 nStartIntens(rGradient.GetStartIntens());
        basegfx::BColor aStart(rGradient.GetStartColor().getBColor());

        if(nStartIntens != 100)
        {
            const basegfx::BColor aBlack;
            aStart = interpolate(aBlack, aStart, (double)nStartIntens * 0.01);
        }

        const sal_uInt16 nEndIntens(rGradient.GetEndIntens());
        basegfx::BColor aEnd(rGradient.GetEndColor().getBColor());

        if(nEndIntens != 100)
        {
            const basegfx::BColor aBlack;
            aEnd = interpolate(aBlack, aEnd, (double)nEndIntens * 0.01);
        }

        drawinglayer::attribute::GradientStyle aGradientStyle(drawinglayer::attribute::GRADIENTSTYLE_RECT);

        switch(rGradient.GetGradientStyle())
        {
            case XGRAD_LINEAR : 
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_LINEAR;
                break;
            }
            case XGRAD_AXIAL : 
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_AXIAL;
                break;
            }
            case XGRAD_RADIAL : 
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RADIAL;
                break;
            }
            case XGRAD_ELLIPTICAL : 
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_ELLIPTICAL;
                break;
            }
            case XGRAD_SQUARE : 
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_SQUARE; 
                break;
            }
            default : 
            {
                aGradientStyle = drawinglayer::attribute::GRADIENTSTYLE_RECT; // XGRAD_RECT
                break;
            }
        }

        const sal_uInt16 nSteps((rSize.Width() + rSize.Height()) / 3);
        const drawinglayer::attribute::FillGradientAttribute aFillGradient(
            aGradientStyle,
            (double)rGradient.GetBorder() * 0.01,
            (double)rGradient.GetXOffset() * 0.01,
            (double)rGradient.GetYOffset() * 0.01,
            (double)rGradient.GetAngle() * F_PI1800,
            aStart,
            aEnd,
            nSteps);

        const drawinglayer::primitive2d::Primitive2DReference aGradientPrimitive(
            new drawinglayer::primitive2d::PolyPolygonGradientPrimitive2D(
                basegfx::B2DPolyPolygon(aRectangle),
                aFillGradient));

        const basegfx::BColor aBlack(0.0, 0.0, 0.0);
        const drawinglayer::primitive2d::Primitive2DReference aBlackRectanglePrimitive(
            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                aRectangle,
                aBlack));

        // prepare VirtualDevice
        VirtualDevice aVirtualDevice;
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

        aVirtualDevice.SetOutputSizePixel(rSize);
        aVirtualDevice.SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);

        // create processor and draw primitives
        drawinglayer::processor2d::BaseProcessor2D* pProcessor2D = drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            aVirtualDevice, 
            aNewViewInformation2D);

        if(pProcessor2D)
        {
            drawinglayer::primitive2d::Primitive2DSequence aSequence(2);

            aSequence[0] = aGradientPrimitive;
            aSequence[1] = aBlackRectanglePrimitive;

            pProcessor2D->process(aSequence);
            delete pProcessor2D;
        }

        // get result bitmap and scale
        aRetval = aVirtualDevice.GetBitmap(Point(0, 0), aVirtualDevice.GetOutputSizePixel());
    }

    return aRetval;
}

//////////////////////////////////////////////////////////////////////////////
// eof
