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


#include "precompiled_rptui.hxx"
#include "EndMarker.hxx"
#include "ColorChanger.hxx"
#include "SectionWindow.hxx"
#include "helpids.hrc"
#include <vcl/svapp.hxx>
#include <vcl/gradient.hxx>
#include <vcl/lineinfo.hxx>


#define CORNER_SPACE	5
//=====================================================================
namespace rptui
{
//=====================================================================
	DBG_NAME( rpt_OEndMarker )
OEndMarker::OEndMarker(Window* _pParent	,const ::rtl::OUString& _sColorEntry)
: OColorListener(_pParent,_sColorEntry)
{
	DBG_CTOR( rpt_OEndMarker,NULL);	
	SetUniqueId(HID_RPT_ENDMARKER);
	ImplInitSettings();
}
// -----------------------------------------------------------------------------
OEndMarker::~OEndMarker()
{
	DBG_DTOR( rpt_OEndMarker,NULL);
}
// -----------------------------------------------------------------------------
void OEndMarker::Paint( const Rectangle& /*rRect*/ )
{
    Fraction aCornerSpace(long(CORNER_SPACE));
    aCornerSpace *= GetMapMode().GetScaleX();
    const long nCornerSpace = aCornerSpace;

	Size aSize = GetSizePixel();
    aSize.Width() += nCornerSpace;
	Rectangle aWholeRect(Point(-nCornerSpace,0),aSize);
    PolyPolygon aPoly;
    aPoly.Insert(Polygon(aWholeRect,nCornerSpace,nCornerSpace));

    Color aStartColor(m_nColor);
    aStartColor.IncreaseLuminance(10);
    sal_uInt16 nHue = 0;
    sal_uInt16 nSat = 0;
    sal_uInt16 nBri = 0;
    aStartColor.RGBtoHSB(nHue, nSat, nBri);
    nSat += 40;
    Color aEndColor(Color::HSBtoRGB(nHue, nSat, nBri));
    Gradient aGradient(GRADIENT_LINEAR,aStartColor,aEndColor);
    aGradient.SetSteps(static_cast<sal_uInt16>(aSize.Height()));

    DrawGradient(PixelToLogic(aPoly) ,aGradient);
	if ( m_bMarked )
	{
        Rectangle aRect( Point(-nCornerSpace,nCornerSpace),
		                 Size(aSize.Width()- nCornerSpace,aSize.Height() - nCornerSpace- nCornerSpace));
        ColorChanger aColors( this, COL_WHITE, COL_WHITE );
        DrawPolyLine(Polygon(PixelToLogic(aRect)),LineInfo(LINE_SOLID,2));
	}
}
// -----------------------------------------------------------------------
void OEndMarker::ImplInitSettings()
{
    EnableChildTransparentMode( sal_True );
    SetParentClipMode( PARENTCLIPMODE_NOCLIP );
    SetPaintTransparent( sal_True );

    SetBackground( Wallpaper( svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor)  );
	SetFillColor( Application::GetSettings().GetStyleSettings().GetShadowColor() );
}
// -----------------------------------------------------------------------
void OEndMarker::MouseButtonDown( const MouseEvent& rMEvt )
{
	if ( !rMEvt.IsLeft() && !rMEvt.IsRight())
		return;
    static_cast<OSectionWindow*>(GetParent())->showProperties();
}
// =======================================================================
}
// =======================================================================
