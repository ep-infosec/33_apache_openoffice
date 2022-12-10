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
#include "precompiled_vcl.hxx"

#include <rtl/logfile.hxx>
#include <cppuhelper/compbase1.hxx>

#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>

#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>

// #i79917#
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <tools/poly.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/uuid.h>

#include <vcl/salbtype.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/bitmapex.hxx>

#include <canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <hash_map>


using namespace ::com::sun::star;

namespace vcl
{
    namespace unotools
    {
        // #i79917# removed helpers bezierSequenceFromPolygon and 
		// pointSequenceFromPolygon here
		// Also all helpers using tools Polygon and PolyPolygon will get mapped to the
		// B2DPolygon helpers for these cases, see comments with the same TaskID below.
		// TODO: Remove those wrapped methods

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolygon( const uno::Reference< rendering::XGraphicDevice >& 	xGraphicDevice,
                                                                             const ::Polygon&										inputPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolygon()" );

			// #i79917# map to basegfx
			const basegfx::B2DPolygon aB2DPolygon(inputPolygon.getB2DPolygon());
			return basegfx::unotools::xPolyPolygonFromB2DPolygon(xGraphicDevice, aB2DPolygon);
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XPolyPolygon2D > xPolyPolygonFromPolyPolygon( const uno::Reference< rendering::XGraphicDevice >& xGraphicDevice,
                                                                                 const ::PolyPolygon&								inputPolyPolygon )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xPolyPolygonFromPolyPolygon()" );

			// #i79917# map to basegfx
			const basegfx::B2DPolyPolygon aB2DPolyPolygon(inputPolyPolygon.getB2DPolyPolygon());
	        return basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(xGraphicDevice, aB2DPolyPolygon);
        }

        //---------------------------------------------------------------------------------------

        ::Polygon polygonFromPoint2DSequence( const uno::Sequence< geometry::RealPoint2D >& points )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::polygonFromPoint2DSequence()" );

            const sal_uInt16 nCurrSize( sal::static_int_cast<sal_uInt16>(points.getLength()) );

            ::Polygon aPoly( nCurrSize );

            sal_uInt16 nCurrPoint;
            for( nCurrPoint=0; nCurrPoint<nCurrSize; ++nCurrPoint )
                aPoly[nCurrPoint] = pointFromRealPoint2D( points[nCurrPoint] );

            return aPoly;
        }

        //---------------------------------------------------------------------------------------

        ::PolyPolygon polyPolygonFromPoint2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >& points )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::polyPolygonFromPoint2DSequenceSequence()" );

            ::PolyPolygon aRes;

            int nCurrPoly;
            for( nCurrPoly=0; nCurrPoly<points.getLength(); ++nCurrPoly )
            {
                aRes.Insert( polygonFromPoint2DSequence( points[nCurrPoly] ) );
            }

            return aRes;
        }

        //---------------------------------------------------------------------------------------

        ::Polygon polygonFromBezier2DSequence( const uno::Sequence< geometry::RealBezierSegment2D >& curves )
        {
			// #i79917# map to basegfx
			const basegfx::B2DPolygon aB2DPolygon(basegfx::unotools::polygonFromBezier2DSequence(curves));
			return ::Polygon(aB2DPolygon);
        }

        //---------------------------------------------------------------------------------------

        ::PolyPolygon polyPolygonFromBezier2DSequenceSequence( const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >& curves )
        {
			// #i79917# map to basegfx
	        const basegfx::B2DPolyPolygon aB2DPolyPolygon(basegfx::unotools::polyPolygonFromBezier2DSequenceSequence(curves));
			return ::PolyPolygon(aB2DPolyPolygon);
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmap( const uno::Reference< rendering::XGraphicDevice >& 	/*xGraphicDevice*/,
                                                                const ::Bitmap&										inputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xBitmapFromBitmap()" );

            return new vcl::unotools::VclCanvasBitmap( BitmapEx( inputBitmap ) );
        }

        //---------------------------------------------------------------------------------------

        uno::Reference< rendering::XBitmap > xBitmapFromBitmapEx( const uno::Reference< rendering::XGraphicDevice >& 	/*xGraphicDevice*/,
                                                                  const ::BitmapEx&										inputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::xBitmapFromBitmapEx()" );

            return new vcl::unotools::VclCanvasBitmap( inputBitmap );
        }

        //---------------------------------------------------------------------------------------

        const uno::Sequence< sal_Int8 > getTunnelIdentifier( TunnelIdentifierType eType )
        {
            static std::hash_map< int, uno::Sequence< sal_Int8 > > aIds;
            std::hash_map< int, uno::Sequence< sal_Int8 > >::iterator it =
                aIds.find( eType );
            if( it == aIds.end() )
            {
                uno::Sequence< sal_Int8 > aNewId( 16 );
                rtl_createUuid( (sal_uInt8*)aNewId.getArray(), NULL, sal_True );
                aIds[ eType ] = aNewId;
                it = aIds.find( eType );
            }
            return it->second;
        }

        //---------------------------------------------------------------------------------------

        namespace
        {
            inline bool operator==( const rendering::IntegerBitmapLayout& rLHS, 
                                    const rendering::IntegerBitmapLayout& rRHS )
            {
                return 
                    rLHS.ScanLineBytes       == rRHS.ScanLineBytes &&
                    rLHS.ScanLineStride      == rRHS.ScanLineStride &&
                    rLHS.PlaneStride         == rRHS.PlaneStride &&
                    rLHS.ColorSpace          == rRHS.ColorSpace &&
                    rLHS.Palette             == rRHS.Palette &&
                    rLHS.IsMsbFirst          == rRHS.IsMsbFirst;
            }

            bool readBmp( sal_Int32                                                  nWidth,
                          sal_Int32                                                  nHeight,
                          const rendering::IntegerBitmapLayout&                      rLayout,
                          const uno::Reference< rendering::XIntegerReadOnlyBitmap >& xInputBitmap,
                          ScopedBitmapWriteAccess&                                   rWriteAcc,
                          ScopedBitmapWriteAccess&                                   rAlphaAcc )
            {
                rendering::IntegerBitmapLayout      aCurrLayout;
                geometry::IntegerRectangle2D        aRect;
                uno::Sequence<sal_Int8>             aPixelData;
                uno::Sequence<rendering::RGBColor>  aRGBColors;
                uno::Sequence<rendering::ARGBColor> aARGBColors;

                for( aRect.Y1=0; aRect.Y1<nHeight; ++aRect.Y1 )
                {
                    aRect.X1 = 0; aRect.X2 = nWidth; aRect.Y2 = aRect.Y1+1;
                    try
                    {
                        aPixelData = xInputBitmap->getData(aCurrLayout,aRect);
                    }
                    catch( rendering::VolatileContentDestroyedException& )
                    {
                        // re-read bmp from the start
                        return false;
                    }
                    if( !(aCurrLayout == rLayout) )
                        return false; // re-read bmp from the start

                    if( rAlphaAcc.get() )
                    {
                        // read ARGB color
                        aARGBColors = rLayout.ColorSpace->convertIntegerToARGB(aPixelData);

                        if( rWriteAcc->HasPalette() )
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::ARGBColor& rColor=aARGBColors[x];
                                rWriteAcc->SetPixelIndex( aRect.Y1, x,
                                                     (sal_uInt8) rWriteAcc->GetBestPaletteIndex(
                                                         BitmapColor( toByteColor(rColor.Red),
                                                                      toByteColor(rColor.Green),
                                                                      toByteColor(rColor.Blue))) );
                                rAlphaAcc->SetPixel( aRect.Y1, x,
                                                     BitmapColor( 255 - toByteColor(rColor.Alpha) ));
                            }
                        }
                        else
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::ARGBColor& rColor=aARGBColors[x];
                                rWriteAcc->SetPixel( aRect.Y1, x,
                                                     BitmapColor( toByteColor(rColor.Red),
                                                                  toByteColor(rColor.Green),
                                                                  toByteColor(rColor.Blue) ));
                                rAlphaAcc->SetPixel( aRect.Y1, x,
                                                     BitmapColor( 255 - toByteColor(rColor.Alpha) ));
                            }
                        }
                    }
                    else
                    {
                        // read RGB color
                        aRGBColors = rLayout.ColorSpace->convertIntegerToRGB(aPixelData);
                        if( rWriteAcc->HasPalette() )
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::RGBColor& rColor=aRGBColors[x];
                                rWriteAcc->SetPixelIndex( aRect.Y1, x,
                                                     (sal_uInt8) rWriteAcc->GetBestPaletteIndex(
                                                         BitmapColor( toByteColor(rColor.Red),
                                                                      toByteColor(rColor.Green),
                                                                      toByteColor(rColor.Blue))) );
                            }
                        }
                        else
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::RGBColor& rColor=aRGBColors[x];
                                rWriteAcc->SetPixel( aRect.Y1, x,
                                                     BitmapColor( toByteColor(rColor.Red),
                                                                  toByteColor(rColor.Green),
                                                                  toByteColor(rColor.Blue) ));
                            }
                        }
                    }
                }

                return true;
            }
        }

        ::BitmapEx VCL_DLLPUBLIC bitmapExFromXBitmap( const uno::Reference< rendering::XIntegerReadOnlyBitmap >& xInputBitmap )
        {
            RTL_LOGFILE_CONTEXT( aLog, "::vcl::unotools::bitmapExFromXBitmap()" );

            if( !xInputBitmap.is() )
                return ::BitmapEx();

            // tunnel directly for known implementation
            // ----------------------------------------------------------------
            VclCanvasBitmap* pImplBitmap = dynamic_cast<VclCanvasBitmap*>(xInputBitmap.get());
            if( pImplBitmap )
                return pImplBitmap->getBitmapEx();

            // retrieve data via UNO interface
            // ----------------------------------------------------------------

            // volatile bitmaps are a bit more complicated to read
            // from..
            uno::Reference<rendering::XVolatileBitmap> xVolatileBitmap(
                xInputBitmap, uno::UNO_QUERY);

            // loop a few times, until successfully read (for XVolatileBitmap)
            for( int i=0; i<10; ++i )
            {
                sal_Int32 nDepth=0;
                sal_Int32 nAlphaDepth=0;
                const rendering::IntegerBitmapLayout aLayout(
                    xInputBitmap->getMemoryLayout());

                OSL_ENSURE(aLayout.ColorSpace.is(),
                           "Cannot convert image without color space!");
                if( !aLayout.ColorSpace.is() )
                    return ::BitmapEx();                

                nDepth = aLayout.ColorSpace->getBitsPerPixel();

                if( xInputBitmap->hasAlpha() )
                {
                    // determine alpha channel depth
                    const uno::Sequence<sal_Int8> aTags( 
                        aLayout.ColorSpace->getComponentTags() );
                    const uno::Sequence<sal_Int32> aDepths( 
                        aLayout.ColorSpace->getComponentBitCounts() );
                    const sal_Int8* pStart(aTags.getConstArray());
                    const sal_Size  nLen(aTags.getLength());
                    const sal_Int8* pEnd(pStart+nLen);

                    const std::ptrdiff_t nAlphaIndex =
                        std::find(pStart,pEnd,
                                  rendering::ColorComponentTag::ALPHA) - pStart;

                    if( nAlphaIndex < sal::static_int_cast<std::ptrdiff_t>(nLen) )
                    {
                        nAlphaDepth = aLayout.ColorSpace->getComponentBitCounts()[nAlphaIndex] > 1 ? 8 : 1;
                        nDepth -= nAlphaDepth;
                    }
                }

                BitmapPalette aPalette;
                if( aLayout.Palette.is() )
                {
                    uno::Reference< rendering::XColorSpace > xPaletteColorSpace(
                        aLayout.Palette->getColorSpace());
                    ENSURE_OR_THROW(xPaletteColorSpace.is(),
                                    "Palette without color space");

                    const sal_Int32 nEntryCount( aLayout.Palette->getNumberOfEntries() );
                    if( nEntryCount <= 256 )
                    {
                        if( nEntryCount <= 2 )
                            nDepth = 1;
                        else
                            nDepth = 8;

                        const sal_uInt16 nPaletteEntries( 
                            sal::static_int_cast<sal_uInt16>(
                                std::min(sal_Int32(255), nEntryCount)));

                        // copy palette entries
                        aPalette.SetEntryCount(nPaletteEntries);
                        uno::Reference<rendering::XBitmapPalette> xPalette( aLayout.Palette );
                        uno::Reference<rendering::XColorSpace>    xPalColorSpace( xPalette->getColorSpace() );

                        uno::Sequence<double> aPaletteEntry;
                        for( sal_uInt16 j=0; j<nPaletteEntries; ++j )
                        {
                            if( !xPalette->getIndex(aPaletteEntry,j) && 
                                nAlphaDepth == 0 )
                            {
                                nAlphaDepth = 1;
                            }
                            uno::Sequence<rendering::RGBColor> aColors=xPalColorSpace->convertToRGB(aPaletteEntry);
                            ENSURE_OR_THROW(aColors.getLength() == 1,
                                            "Palette returned more or less than one entry");
                            const rendering::RGBColor& rColor=aColors[0];
                            aPalette[j] = BitmapColor(toByteColor(rColor.Red),
                                                      toByteColor(rColor.Green),
                                                      toByteColor(rColor.Blue));
                        }
                    }
                }

                const ::Size aPixelSize( 
                    sizeFromIntegerSize2D(xInputBitmap->getSize()));

                // normalize bitcount
                nDepth = 
                    ( nDepth <= 1 ) ? 1 :
                    ( nDepth <= 4 ) ? 4 :
                    ( nDepth <= 8 ) ? 8 : 24; 

                ::Bitmap aBitmap( aPixelSize,
                                  sal::static_int_cast<sal_uInt16>(nDepth),
                                  aLayout.Palette.is() ? &aPalette : NULL );
                ::Bitmap aAlpha;
                if( nAlphaDepth )
                    aAlpha = ::Bitmap( aPixelSize, 
                                       sal::static_int_cast<sal_uInt16>(nAlphaDepth),
                                       &::Bitmap::GetGreyPalette(
                                           sal::static_int_cast<sal_uInt16>(1L << nAlphaDepth)) );

                { // limit scoped access 
                    ScopedBitmapWriteAccess pWriteAccess( aBitmap.AcquireWriteAccess(),
                                                          aBitmap );
                    ScopedBitmapWriteAccess pAlphaWriteAccess( nAlphaDepth ? aAlpha.AcquireWriteAccess() : NULL,
                                                               aAlpha );

                    ENSURE_OR_THROW(pWriteAccess.get() != NULL,
                                    "Cannot get write access to bitmap");

                    const sal_Int32 nWidth(aPixelSize.Width());
                    const sal_Int32 nHeight(aPixelSize.Height());

                    if( !readBmp(nWidth,nHeight,aLayout,xInputBitmap,
                                 pWriteAccess,pAlphaWriteAccess) )
                        continue;
                } // limit scoped access 

                if( nAlphaDepth )
                    return ::BitmapEx( aBitmap,
                                       AlphaMask( aAlpha ) );
                else
                    return ::BitmapEx( aBitmap );
            }

            // failed to read data 10 times - bail out
            return ::BitmapEx();
        }

        //---------------------------------------------------------------------------------------

        geometry::RealSize2D size2DFromSize( const Size& rSize )
        {
            return geometry::RealSize2D( rSize.Width(),
                                         rSize.Height() );
        }

        geometry::RealPoint2D point2DFromPoint( const Point& rPoint )
        {
            return geometry::RealPoint2D( rPoint.X(),
                                          rPoint.Y() );
        }

        geometry::RealRectangle2D rectangle2DFromRectangle( const Rectangle& rRect )
        {
            return geometry::RealRectangle2D( rRect.Left(), rRect.Top(),
                                              rRect.Right(), rRect.Bottom() );
        }

        Size sizeFromRealSize2D( const geometry::RealSize2D& rSize )
        {
            return Size( static_cast<long>(rSize.Width + .5),
                         static_cast<long>(rSize.Height + .5) );
        }

        Point pointFromRealPoint2D( const geometry::RealPoint2D& rPoint )
        {
            return Point( static_cast<long>(rPoint.X + .5),
                          static_cast<long>(rPoint.Y + .5) );
        }

        Rectangle rectangleFromRealRectangle2D( const geometry::RealRectangle2D& rRect )
        {
            return Rectangle( static_cast<long>(rRect.X1 + .5),
                              static_cast<long>(rRect.Y1 + .5),
                              static_cast<long>(rRect.X2 + .5),
                              static_cast<long>(rRect.Y2 + .5) );
        }

        ::Size sizeFromB2DSize( const ::basegfx::B2DVector& rVec )
        {
            return ::Size( FRound( rVec.getX() ),
                           FRound( rVec.getY() ) );
        }

        ::Point pointFromB2DPoint( const ::basegfx::B2DPoint& rPoint )
        {
            return ::Point( FRound( rPoint.getX() ),
                            FRound( rPoint.getY() ) );
        }

        ::Rectangle rectangleFromB2DRectangle( const ::basegfx::B2DRange& rRect )
        {
            return ::Rectangle( FRound( rRect.getMinX() ),
                                FRound( rRect.getMinY() ),
                                FRound( rRect.getMaxX() ),
                                FRound( rRect.getMaxY() ) );
        }

        Size sizeFromB2ISize( const ::basegfx::B2IVector& rVec )
        {
            return ::Size( rVec.getX(),
                           rVec.getY() );
        }

        Point pointFromB2IPoint( const ::basegfx::B2IPoint& rPoint )
        {
            return ::Point( rPoint.getX(),
                            rPoint.getY() );
        }

        Rectangle rectangleFromB2IRectangle( const ::basegfx::B2IRange& rRect )
        {
            return ::Rectangle( rRect.getMinX(),
                                rRect.getMinY(),
                                rRect.getMaxX(),
                                rRect.getMaxY() );
        }

        ::basegfx::B2DVector b2DSizeFromSize( const ::Size& rSize )
        {
            return ::basegfx::B2DVector( rSize.Width(),
                                         rSize.Height() );
        }

        ::basegfx::B2DPoint b2DPointFromPoint( const ::Point& rPoint )
        {
            return ::basegfx::B2DPoint( rPoint.X(),
                                        rPoint.Y() );
        }

        ::basegfx::B2DRange b2DRectangleFromRectangle( const ::Rectangle& rRect )
        {
            return ::basegfx::B2DRange( rRect.Left(),
                                        rRect.Top(),
                                        rRect.Right(),
                                        rRect.Bottom() );
        }

        basegfx::B2IVector b2ISizeFromSize( const Size& rSize )
        {
            return ::basegfx::B2IVector( rSize.Width(),
                                         rSize.Height() );
        }

        basegfx::B2IPoint b2IPointFromPoint( const Point& rPoint )
        {
            return ::basegfx::B2IPoint( rPoint.X(),
                                        rPoint.Y() );
        }

        basegfx::B2IRange b2IRectangleFromRectangle( const Rectangle& rRect )
        {
            return ::basegfx::B2IRange( rRect.Left(),
                                        rRect.Top(),
                                        rRect.Right(),
                                        rRect.Bottom() );
        }

        geometry::IntegerSize2D integerSize2DFromSize( const Size& rSize )
        {
            return geometry::IntegerSize2D( rSize.Width(),
                                            rSize.Height() );
        }

        geometry::IntegerPoint2D integerPoint2DFromPoint( const Point& rPoint )
        {
            return geometry::IntegerPoint2D( rPoint.X(),
                                             rPoint.Y() );
        }

        geometry::IntegerRectangle2D integerRectangle2DFromRectangle( const Rectangle& rRectangle )
        {
            return geometry::IntegerRectangle2D( rRectangle.Left(), rRectangle.Top(),
                                                 rRectangle.Right(), rRectangle.Bottom() );
        }

        Size sizeFromIntegerSize2D( const geometry::IntegerSize2D& rSize )
        {
            return Size( rSize.Width,
                         rSize.Height );
        }

        Point pointFromIntegerPoint2D( const geometry::IntegerPoint2D& rPoint )
        {
            return Point( rPoint.X,
                          rPoint.Y );
        }

        Rectangle rectangleFromIntegerRectangle2D( const geometry::IntegerRectangle2D& rRectangle )
        {
            return Rectangle( rRectangle.X1, rRectangle.Y1,
                              rRectangle.X2, rRectangle.Y2 );
        }

        namespace
        {
            class StandardColorSpace : public cppu::WeakImplHelper1< com::sun::star::rendering::XColorSpace >
            {
            private:
                uno::Sequence< sal_Int8 > m_aComponentTags;

                virtual ::sal_Int8 SAL_CALL getType(  ) throw (uno::RuntimeException)
                {
                    return rendering::ColorSpaceType::RGB;
                }
                virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) throw (uno::RuntimeException)
                {
                    return m_aComponentTags;
                }
                virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (uno::RuntimeException)
                {
                    return rendering::RenderingIntent::PERCEPTUAL;
                }
                virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) throw (uno::RuntimeException)
                {
                    return uno::Sequence< beans::PropertyValue >();
                }
                virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor, 
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) throw (lang::IllegalArgumentException, 
                                                                                                                                                      uno::RuntimeException)
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
                virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const sal_Size nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0, 
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for( sal_Size i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::RGBColor(pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const sal_Size nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0, 
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( sal_Size i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(pIn[3],pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const sal_Size nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0, 
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( sal_Size i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(pIn[3],pIn[3]*pIn[0],pIn[3]*pIn[1],pIn[3]*pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
                {
                    const rendering::RGBColor* pIn( rgbColor.getConstArray() );
                    const sal_Size             nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( sal_Size i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red;
                        *pColors++ = pIn->Green;
                        *pColors++ = pIn->Blue;
                        *pColors++ = 1.0;
                        ++pIn;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const sal_Size              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( sal_Size i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red;
                        *pColors++ = pIn->Green;
                        *pColors++ = pIn->Blue;
                        *pColors++ = pIn->Alpha;
                        ++pIn;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) throw (lang::IllegalArgumentException, uno::RuntimeException)
                {
                    const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
                    const sal_Size              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( sal_Size i=0; i<nLen; ++i )
                    {
                        *pColors++ = pIn->Red/pIn->Alpha;
                        *pColors++ = pIn->Green/pIn->Alpha;
                        *pColors++ = pIn->Blue/pIn->Alpha;
                        *pColors++ = pIn->Alpha;
                        ++pIn;
                    }
                    return aRes;
                }

            public:
                StandardColorSpace() : m_aComponentTags(4)
                {
                    sal_Int8* pTags = m_aComponentTags.getArray();
                    pTags[0] = rendering::ColorComponentTag::RGB_RED;
                    pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
                    pTags[2] = rendering::ColorComponentTag::RGB_BLUE;
                    pTags[3] = rendering::ColorComponentTag::ALPHA;
                }
            };
        }

        uno::Reference<rendering::XColorSpace> VCL_DLLPUBLIC createStandardColorSpace()
        {
            return new StandardColorSpace();
        }

        //---------------------------------------------------------------------------------------

        uno::Sequence< double > colorToStdColorSpaceSequence( const Color& rColor )
        {
            uno::Sequence< double > aRet(4);
            double* pRet = aRet.getArray();

            pRet[0] = toDoubleColor(rColor.GetRed());
            pRet[1] = toDoubleColor(rColor.GetGreen());
            pRet[2] = toDoubleColor(rColor.GetBlue());

            // VCL's notion of alpha is different from the rest of the world's
            pRet[3] = 1.0 - toDoubleColor(rColor.GetTransparency());

            return aRet;
        }

        Color stdColorSpaceSequenceToColor( const uno::Sequence< double >& rColor		 )
        {
            ENSURE_ARG_OR_THROW( rColor.getLength() == 4,
                                 "color must have 4 channels" );

            Color aColor;

            aColor.SetRed  ( toByteColor(rColor[0]) );
            aColor.SetGreen( toByteColor(rColor[1]) );
            aColor.SetBlue ( toByteColor(rColor[2]) );
            // VCL's notion of alpha is different from the rest of the world's
            aColor.SetTransparency( 255 - toByteColor(rColor[3]) );

            return aColor;
        }

        uno::Sequence< double > VCL_DLLPUBLIC colorToDoubleSequence( 
            const Color&                                    rColor,
            const uno::Reference< rendering::XColorSpace >& xColorSpace )
        {
            uno::Sequence<rendering::ARGBColor> aSeq(1);
            aSeq[0] = rendering::ARGBColor(
                    1.0-toDoubleColor(rColor.GetTransparency()),
                    toDoubleColor(rColor.GetRed()),
                    toDoubleColor(rColor.GetGreen()),
                    toDoubleColor(rColor.GetBlue()) );

            return xColorSpace->convertFromARGB(aSeq);
        }

        Color VCL_DLLPUBLIC doubleSequenceToColor( 
            const uno::Sequence< double >                   rColor,
            const uno::Reference< rendering::XColorSpace >& xColorSpace )
        {
            const rendering::ARGBColor& rARGBColor(
                xColorSpace->convertToARGB(rColor)[0]);

            return Color( 255-toByteColor(rARGBColor.Alpha),
                          toByteColor(rARGBColor.Red),
                          toByteColor(rARGBColor.Green),
                          toByteColor(rARGBColor.Blue) );
        }

        //---------------------------------------------------------------------------------------

    } // namespace vcltools

} // namespace canvas

// eof
