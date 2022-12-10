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



#ifndef _SV_GDIMTF_HXX
#define _SV_GDIMTF_HXX

#include <vcl/dllapi.h>
#include <tools/gen.hxx>
#include <tools/list.hxx>
#include <tools/link.hxx>
#include <tools/string.hxx>
#include <vcl/mapmod.hxx>

class OutputDevice;
class ImpLabelList;
class MetaAction;
class SvStream;
class Color;
class BitmapEx;
class Polygon;
class PolyPolygon;
class Gradient;

// ---------------------
// - GDIMetaFile-Types -
// ---------------------

#define GDI_METAFILE_END                ((sal_uLong)0xFFFFFFFF)
#define GDI_METAFILE_LABEL_NOTFOUND     ((sal_uLong)0xFFFFFFFF)

#ifndef METAFILE_END
#define METAFILE_END					GDI_METAFILE_END
#endif

#ifndef METAFILE_LABEL_NOTFOUND
#define METAFILE_LABEL_NOTFOUND			GDI_METAFILE_LABEL_NOTFOUND
#endif

// -----------
// - Defines -
// -----------

#define MTF_MIRROR_NONE				0x00000000UL
#define MTF_MIRROR_HORZ				0x00000001UL
#define MTF_MIRROR_VERT				0x00000002UL

// ---------
// - Enums -
// ---------

enum MtfConversion
{
    MTF_CONVERSION_NONE = 0,
    MTF_CONVERSION_1BIT_THRESHOLD = 1,
    MTF_CONVERSION_8BIT_GREYS = 2
};

// -----------------------------
// - Color conversion routines -
// -----------------------------

//#if 0 // _SOLAR__PRIVATE

typedef Color (*ColorExchangeFnc)( const Color& rColor, const void* pColParam );
typedef BitmapEx (*BmpExchangeFnc)( const BitmapEx& rBmpEx, const void* pBmpParam );

//#endif // __PRIVATE

// ---------------
// - GDIMetaFile -
// ---------------

class VCL_DLLPUBLIC GDIMetaFile : protected List
{
private:

    MapMode         aPrefMapMode;
    Size            aPrefSize;
    Link            aHookHdlLink;
    GDIMetaFile*    pPrev;
    GDIMetaFile*    pNext;
    OutputDevice*   pOutDev;
    ImpLabelList*   pLabelList;
    sal_Bool            bPause;
    sal_Bool            bRecord;

//#if 0 // _SOLAR__PRIVATE

	SAL_DLLPRIVATE static Color    ImplColAdjustFnc( const Color& rColor, const void* pColParam );
	SAL_DLLPRIVATE static BitmapEx ImplBmpAdjustFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

	SAL_DLLPRIVATE static Color	   ImplColConvertFnc( const Color& rColor, const void* pColParam );
	SAL_DLLPRIVATE static BitmapEx ImplBmpConvertFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

	SAL_DLLPRIVATE static Color	   ImplColMonoFnc( const Color& rColor, const void* pColParam );
	SAL_DLLPRIVATE static BitmapEx ImplBmpMonoFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

	SAL_DLLPRIVATE static Color	   ImplColReplaceFnc( const Color& rColor, const void* pColParam );
	SAL_DLLPRIVATE static BitmapEx ImplBmpReplaceFnc( const BitmapEx& rBmpEx, const void* pBmpParam );

	SAL_DLLPRIVATE void			   ImplExchangeColors( ColorExchangeFnc pFncCol, const void* pColParam,
                                                       BmpExchangeFnc pFncBmp, const void* pBmpParam );

    SAL_DLLPRIVATE Point           ImplGetRotatedPoint( const Point& rPt, const Point& rRotatePt,
                                                        const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE Polygon         ImplGetRotatedPolygon( const Polygon& rPoly, const Point& rRotatePt,
                                                          const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE PolyPolygon     ImplGetRotatedPolyPolygon( const PolyPolygon& rPoly, const Point& rRotatePt,
                                                              const Size& rOffset, double fSin, double fCos );
    SAL_DLLPRIVATE void            ImplAddGradientEx( GDIMetaFile&          rMtf,
                                                      const OutputDevice&   rMapDev,
                                                      const PolyPolygon&    rPolyPoly,
                                                      const Gradient&	  	rGrad 		);

//#endif // __PRIVATE

protected:

    virtual void    Linker( OutputDevice* pOut, sal_Bool bLink );
    virtual long    Hook();

public:
                    GDIMetaFile();
					GDIMetaFile( const GDIMetaFile& rMtf );
    virtual			~GDIMetaFile();

    using List::operator==;
    using List::operator!=;
    GDIMetaFile&    operator=( const GDIMetaFile& rMtf );
	sal_Bool			operator==( const GDIMetaFile& rMtf ) const;
	sal_Bool			operator!=( const GDIMetaFile& rMtf ) const { return !( *this == rMtf ); }

    void            Clear();
	sal_Bool		IsEqual( const GDIMetaFile& rMtf ) const;
    sal_Bool            Mirror( sal_uLong nMirrorFlags );
    void            Move( long nX, long nY );
    // additional Move method getting specifics how to handle MapMode( MAP_PIXEL )
    void            Move( long nX, long nY, long nDPIX, long nDPIY );
    void            Scale( double fScaleX, double fScaleY );
    void            Scale( const Fraction& rScaleX, const Fraction& rScaleY );
    void            Rotate( long nAngle10 );
    void            Clip( const Rectangle& );
    /* get the bound rect of the contained actions
     * caveats:
     * - clip actions will limit the contained actions,
     *   but the current clipregion of the passed OutputDevice will not
     * - coordinates of actions will be transformed to preferred mapmode
     * - the returned rectangle is relative to the preferred mapmode of the metafile
    */
    Rectangle       GetBoundRect( OutputDevice& i_rReference, Rectangle* pHairline = 0 ) const;

	void			Adjust( short nLuminancePercent = 0, short nContrastPercent = 0,
							short nChannelRPercent = 0, short nChannelGPercent = 0, 
							short nChannelBPercent = 0, double fGamma = 1.0, sal_Bool bInvert = sal_False );
	void			Convert( MtfConversion eConversion );
    void			ReplaceColors( const Color& rSearchColor, const Color& rReplaceColor, sal_uLong nTol = 0 );
    void			ReplaceColors( const Color* pSearchColors, const Color* rReplaceColors, 
								   sal_uLong nColorCount, sal_uLong* pTols = NULL );

    GDIMetaFile     GetMonochromeMtf( const Color& rCol ) const;

    void            Record( OutputDevice* pOutDev );
    sal_Bool            IsRecord() const { return bRecord; }

    void            Play( GDIMetaFile& rMtf, sal_uLong nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, sal_uLong nPos = GDI_METAFILE_END );
    void            Play( OutputDevice* pOutDev, const Point& rPos,
                          const Size& rSize, sal_uLong nPos = GDI_METAFILE_END );

    void            Pause( sal_Bool bPause );
    sal_Bool            IsPause() const { return bPause; }

    void            Stop();

    void            WindStart();
    void            WindEnd();
    void            Wind( sal_uLong nAction );
    void            WindPrev();
    void            WindNext();

    sal_uLong           GetActionCount() const { return Count(); }
    void            AddAction( MetaAction* pAction );
    void            AddAction( MetaAction* pAction, sal_uLong nPos );
    void 			RemoveAction( sal_uLong nPos );
    MetaAction*     CopyAction( sal_uLong nPos ) const;
    MetaAction*     GetCurAction() const { return (MetaAction*) GetCurObject(); }
    MetaAction*     GetAction( sal_uLong nAction ) const { return (MetaAction*) GetObject( nAction ); }
    MetaAction*     FirstAction() {	return (MetaAction*) First(); }
    MetaAction*     NextAction() { 	return (MetaAction*) Next(); }
    MetaAction*     ReplaceAction( MetaAction* pAction, sal_uLong nAction ) { return (MetaAction*) Replace( pAction, nAction ); }

    sal_uLong           GetActionPos( const String& rLabel );
    sal_Bool            InsertLabel( const String& rLabel, sal_uLong nActionPos );
    void            RemoveLabel( const String& rLabel );
    void            RenameLabel( const String& rLabel, const String& rNewLabel );
    sal_uLong           GetLabelCount() const;
    String          GetLabel( sal_uLong nLabel );

    sal_Bool            SaveStatus();

    const Size&     GetPrefSize() const { return aPrefSize; }
    void            SetPrefSize( const Size& rSize ) { aPrefSize = rSize; }

    const MapMode&  GetPrefMapMode() const { return aPrefMapMode; }
    void            SetPrefMapMode( const MapMode& rMapMode ) { aPrefMapMode = rMapMode; }

    void            SetHookHdl( const Link& rLink ) { aHookHdlLink = rLink; }
    const Link&     GetHookHdl() const { return aHookHdlLink; }

	sal_uLong			GetChecksum() const;
    sal_uLong           GetSizeBytes() const;

	// Methoden zum Lesen und Schreiben des neuen Formats;
	// die Read-Methode kann auch das alte Format lesen
	SvStream&		Read( SvStream& rIStm );
	SvStream&		Write( SvStream& rOStm );

	// Stream-Operatoren schreiben das alte Format (noch)
	// und lesen sowohl das alte wie auch das neue Format
    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, GDIMetaFile& rGDIMetaFile );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const GDIMetaFile& rGDIMetaFile );

	sal_Bool           CreateThumbnail( sal_uInt32 nMaximumExtent, BitmapEx& rBmpEx, const BitmapEx* pOverlay = NULL, const Rectangle* pOverlayRect = NULL ) const;
};

#endif // _SV_GDIMTF_HXX
