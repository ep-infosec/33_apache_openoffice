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



#ifndef _SVP_SVPGDI_HXX

#include <basebmp/bitmapdevice.hxx>
#include <basebmp/color.hxx>

#include <salgdi.hxx>
#include <sallayout.hxx>

class ServerFont;

class SvpSalGraphics : public SalGraphics
{
    basebmp::BitmapDeviceSharedPtr       m_aDevice;
    basebmp::BitmapDeviceSharedPtr       m_aOrigDevice;
    basebmp::BitmapDeviceSharedPtr       m_aClipMap;

    bool                                 m_bUseLineColor;
    basebmp::Color                       m_aLineColor;
    bool                                 m_bUseFillColor;
    basebmp::Color                       m_aFillColor;    
    basebmp::Color                       m_aTextColor;

    basebmp::DrawMode                    m_aDrawMode;

    ServerFont*                          m_pServerFont[ MAX_FALLBACK ];
    sal_uInt32                           m_eTextFmt;

protected:
    virtual bool drawAlphaBitmap( const SalTwoRect&, const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap );
    virtual bool drawTransformedBitmap(
        const basegfx::B2DPoint& rNull,
        const basegfx::B2DPoint& rX,
        const basegfx::B2DPoint& rY,
        const SalBitmap& rSourceBitmap,
        const SalBitmap* pAlphaBitmap);
    virtual bool drawAlphaRect( long nX, long nY, long nWidth, long nHeight, sal_uInt8 nTransparency );

public:
    SvpSalGraphics();
    virtual	~SvpSalGraphics();

    const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aDevice; }
    void setDevice( basebmp::BitmapDeviceSharedPtr& rDevice );

    // overload all pure virtual methods
    virtual void			GetResolution( sal_Int32& rDPIX, sal_Int32& rDPIY );
    virtual sal_uInt16			GetBitCount();
    virtual long			GetGraphicsWidth() const;

    virtual void			ResetClipRegion();
    virtual bool            setClipRegion( const Region& );

    virtual void			SetLineColor();
    virtual void			SetLineColor( SalColor nSalColor );
    virtual void			SetFillColor();

    virtual void                        SetFillColor( SalColor nSalColor );

    virtual void			SetXORMode( bool bSet, bool );

    virtual void			SetROPLineColor( SalROPColor nROPColor );
    virtual void			SetROPFillColor( SalROPColor nROPColor );

    virtual void			SetTextColor( SalColor nSalColor );
    virtual sal_uInt16                  SetFont( ImplFontSelectData*, int nFallbackLevel );
    virtual void			GetFontMetric( ImplFontMetricData*, int nFallbackLevel );
    virtual sal_uLong			GetKernPairs( sal_uLong nPairs, ImplKernPairData* pKernPairs );
    virtual const ImplFontCharMap* GetImplFontCharMap() const;
    virtual void			GetDevFontList( ImplDevFontList* );
    virtual void			GetDevFontSubstList( OutputDevice* );
    virtual bool			AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName );
    virtual sal_Bool			CreateFontSubset( const rtl::OUString& rToFile,
                                              const ImplFontData*,
                                              sal_GlyphId* pGlyphIds,
                                              sal_uInt8* pEncoding,
                                              sal_Int32* pWidths,
                                              int nGlyphs,
                                              FontSubsetInfo& rInfo
                                              );
    virtual const Ucs2SIntMap* GetFontEncodingVector( const ImplFontData*, const Ucs2OStrMap** ppNonEncoded );
    virtual const void*	GetEmbedFontData( const ImplFontData*,
                                          const sal_Ucs* pUnicodes,
                                          sal_Int32* pWidths,
                                          FontSubsetInfo& rInfo,
                                          long* pDataLen );
    virtual void			FreeEmbedFontData( const void* pData, long nDataLen );
    virtual void            GetGlyphWidths( const ImplFontData*,
                                            bool bVertical,
                                            Int32Vector& rWidths,
                                            Ucs2UIntMap& rUnicodeEnc );
    virtual bool			GetGlyphBoundRect( sal_GlyphId, Rectangle& );
    virtual bool			GetGlyphOutline( sal_GlyphId, ::basegfx::B2DPolyPolygon& );
    virtual SalLayout*		GetTextLayout( ImplLayoutArgs&, int nFallbackLevel );
    virtual void			DrawServerFontLayout( const ServerFontLayout& );
    virtual bool            supportsOperation( OutDevSupportType ) const;
    virtual void			drawPixel( long nX, long nY );
    virtual void			drawPixel( long nX, long nY, SalColor nSalColor );
    virtual void			drawLine( long nX1, long nY1, long nX2, long nY2 );
    virtual void			drawRect( long nX, long nY, long nWidth, long nHeight );
    virtual bool            drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double fTransparency );
    virtual bool            drawPolyLine( 
        const ::basegfx::B2DPolygon&, 
        double fTransparency, 
        const ::basegfx::B2DVector& rLineWidths, 
        basegfx::B2DLineJoin,
        com::sun::star::drawing::LineCap);
    virtual void			drawPolyLine( sal_uInt32 nPoints, const SalPoint* pPtAry );
    virtual void			drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry );
    virtual void			drawPolyPolygon( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             PCONSTSALPOINT* pPtAry );
    virtual sal_Bool		drawPolyLineBezier( sal_uInt32 nPoints,
                                                const SalPoint* pPtAry,
                                                const sal_uInt8* pFlgAry );
    virtual sal_Bool		drawPolygonBezier( sal_uInt32 nPoints,
                                               const SalPoint* pPtAry,
                                               const sal_uInt8* pFlgAry );
    virtual sal_Bool		drawPolyPolygonBezier( sal_uInt32 nPoly,
                                                   const sal_uInt32* pPoints,
                                                   const SalPoint* const* pPtAry,
                                                   const sal_uInt8* const* pFlgAry );

    virtual void			copyArea( long nDestX,
                                      long nDestY,
                                      long nSrcX,
                                      long nSrcY,
                                      long nSrcWidth,
                                      long nSrcHeight,
                                      sal_uInt16 nFlags );
    virtual void			copyBits( const SalTwoRect& rPosAry,
                                      SalGraphics* pSrcGraphics );
    virtual void			drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap );
    virtual void			drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        SalColor nTransparentColor );
    virtual void			drawBitmap( const SalTwoRect& rPosAry,
                                        const SalBitmap& rSalBitmap,
                                        const SalBitmap& rTransparentBitmap );
    virtual void			drawMask( const SalTwoRect& rPosAry,
                                      const SalBitmap& rSalBitmap,
                                      SalColor nMaskColor );
    virtual SalBitmap*		getBitmap( long nX, long nY, long nWidth, long nHeight );
    virtual SalColor		getPixel( long nX, long nY );
    virtual void			invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags );
    virtual void			invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags );

    virtual sal_Bool			drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize );

    virtual SystemGraphicsData GetGraphicsData() const;
    virtual SystemFontData     GetSysFontData( int nFallbacklevel ) const;
};

#endif

