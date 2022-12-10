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
#include <vcl/svapp.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

#define GLOBALOVERFLOW

using namespace com::sun::star;
using namespace rtl;

sal_Unicode const pszExtLineEnd[]	= {'s','o','e'};
//static char const aChckLEnd[]  = { 0x04, 0x00, 'S','O','E','L'};	// < 5.2
//static char const aChckLEnd0[] = { 0x04, 0x00, 'S','O','E','0'};	// = 5.2
//static char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// --------------------
// class XLineEndList
// --------------------

XLineEndList::XLineEndList(const String& rPath)
:	XPropertyList(rPath)
{
}

XLineEndList::~XLineEndList()
{
}

XLineEndEntry* XLineEndList::Replace(XLineEndEntry* pEntry, long nIndex )
{
	return (XLineEndEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XLineEndEntry* XLineEndList::Remove(long nIndex)
{
	return (XLineEndEntry*) XPropertyList::Remove(nIndex);
}

XLineEndEntry* XLineEndList::GetLineEnd(long nIndex) const
{
	return (XLineEndEntry*) XPropertyList::Get(nIndex);
}

bool XLineEndList::Load()
{
	if( mbListDirty )
	{
		mbListDirty = false;

		INetURLObject aURL( maPath );

		if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
		{
			OSL_ENSURE( !maPath.Len(), "invalid URL" );
			return false;
		}

		aURL.Append( maName );

		if( !aURL.getExtension().getLength() )
			aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

		uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
	}

    return false;
}

bool XLineEndList::Save()
{
	INetURLObject aURL( maPath );

	if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
	{
		OSL_ENSURE( !maPath.Len(), "invalid URL" );
		return false;
	}

	aURL.Append( maName );

	if( !aURL.getExtension().getLength() )
		aURL.setExtension( rtl::OUString( pszExtLineEnd, 3 ) );

	uno::Reference< container::XNameContainer > xTable( SvxUnoXLineEndTable_createInstance( this ), uno::UNO_QUERY );
	return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

bool XLineEndList::Create()
{
	basegfx::B2DPolygon aTriangle;
	aTriangle.append(basegfx::B2DPoint(10.0, 0.0));
	aTriangle.append(basegfx::B2DPoint(0.0, 30.0));
	aTriangle.append(basegfx::B2DPoint(20.0, 30.0));
	aTriangle.setClosed(true);
	Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aTriangle), SVX_RESSTR( RID_SVXSTR_ARROW ) ) );

	basegfx::B2DPolygon aSquare;
	aSquare.append(basegfx::B2DPoint(0.0, 0.0));
	aSquare.append(basegfx::B2DPoint(10.0, 0.0));
	aSquare.append(basegfx::B2DPoint(10.0, 10.0));
	aSquare.append(basegfx::B2DPoint(0.0, 10.0));
	aSquare.setClosed(true);
	Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aSquare), SVX_RESSTR( RID_SVXSTR_SQUARE ) ) );

	basegfx::B2DPolygon aCircle(basegfx::tools::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 100.0));
	Insert( new XLineEndEntry( basegfx::B2DPolyPolygon(aCircle), SVX_RESSTR( RID_SVXSTR_CIRCLE ) ) );

	return true;
}

Bitmap XLineEndList::CreateBitmapForUI( long nIndex )
{
    Bitmap aRetval;
    OSL_ENSURE(nIndex < Count(), "OOps, access out of range (!)");

    if(nIndex < Count())
    {
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
        const Size& rSize = rStyleSettings.GetListBoxPreviewDefaultPixelSize();
        const Size aSize(rSize.Width() * 2, rSize.Height());

        // prepare line geometry
        basegfx::B2DPolygon aLine;
        const double fBorderDistance(aSize.Height() * 0.1);

        aLine.append(basegfx::B2DPoint(fBorderDistance, aSize.Height() / 2));
        aLine.append(basegfx::B2DPoint(aSize.Width() - fBorderDistance, aSize.Height() / 2));

        // prepare LineAttribute
        const basegfx::BColor aLineColor(rStyleSettings.GetFieldTextColor().getBColor());
        const double fLineWidth(rStyleSettings.GetListBoxPreviewDefaultLineWidth() * 1.1);
        const drawinglayer::attribute::LineAttribute aLineAttribute(
            aLineColor,
            fLineWidth);

        const basegfx::B2DPolyPolygon aLineEnd(GetLineEnd(nIndex)->GetLineEnd());
        const double fArrowHeight(aSize.Height() - (2.0 * fBorderDistance));
        const drawinglayer::attribute::LineStartEndAttribute aLineStartEndAttribute(
            fArrowHeight,
            aLineEnd,
            false);

        // prepare line primitive
        const drawinglayer::primitive2d::Primitive2DReference aLineStartEndPrimitive(
            new drawinglayer::primitive2d::PolygonStrokeArrowPrimitive2D(
                aLine,
                aLineAttribute,
                aLineStartEndAttribute,
                aLineStartEndAttribute));

        // prepare VirtualDevice
        VirtualDevice aVirtualDevice;
        const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;

        aVirtualDevice.SetOutputSizePixel(aSize);
        aVirtualDevice.SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);

        if(rStyleSettings.GetPreviewUsesCheckeredBackground())
        {
            const Point aNull(0, 0);
            static const sal_uInt32 nLen(8);
            static const Color aW(COL_WHITE);
            static const Color aG(0xef, 0xef, 0xef);

            aVirtualDevice.DrawCheckered(aNull, aSize, nLen, aW, aG);
        }
        else
        {
            aVirtualDevice.SetBackground(rStyleSettings.GetFieldColor());
            aVirtualDevice.Erase();
        }

        // create processor and draw primitives
        drawinglayer::processor2d::BaseProcessor2D* pProcessor2D = drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            aVirtualDevice, 
            aNewViewInformation2D);

        if(pProcessor2D)
        {
            const drawinglayer::primitive2d::Primitive2DSequence aSequence(&aLineStartEndPrimitive, 1);

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
