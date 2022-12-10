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
#include "precompiled_cppcanvas.hxx"
#include <rtl/instance.hxx>
#include <osl/getglobalmutex.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/rendering/InterpolationMode.hpp>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <vcl/canvastools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <cppcanvas/vclfactory.hxx>

#include <implbitmapcanvas.hxx>
#include <implspritecanvas.hxx>
#include <implpolypolygon.hxx>
#include <implbitmap.hxx>
#include <implrenderer.hxx>
#include <impltext.hxx>
#include <implsprite.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
	/* Singleton handling */
    struct InitInstance
    {
        VCLFactory* operator()()
        {
            return new VCLFactory();
        }
    };

    VCLFactory& VCLFactory::getInstance()
    {
        return *rtl_Instance< VCLFactory, InitInstance, ::osl::MutexGuard,
            ::osl::GetGlobalMutex >::create(
                InitInstance(), ::osl::GetGlobalMutex());
    }

    VCLFactory::VCLFactory()
    {
    }
    
    VCLFactory::~VCLFactory()
    {
    }

    BitmapCanvasSharedPtr VCLFactory::createCanvas( const ::Window& rVCLWindow )
    {        
        return BitmapCanvasSharedPtr( 
            new internal::ImplBitmapCanvas( 
                uno::Reference< rendering::XBitmapCanvas >(
                    rVCLWindow.GetCanvas(),
                    uno::UNO_QUERY) ) );
    }

    BitmapCanvasSharedPtr VCLFactory::createCanvas( const uno::Reference< rendering::XBitmapCanvas >& xCanvas )
    {
        return BitmapCanvasSharedPtr( 
            new internal::ImplBitmapCanvas( xCanvas ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const ::Window& rVCLWindow ) const
    {
        return SpriteCanvasSharedPtr( 
            new internal::ImplSpriteCanvas( 
                uno::Reference< rendering::XSpriteCanvas >(
                    rVCLWindow.GetSpriteCanvas(),
                    uno::UNO_QUERY) ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createSpriteCanvas( const uno::Reference< rendering::XSpriteCanvas >& xCanvas ) const
    {
        return SpriteCanvasSharedPtr( 
            new internal::ImplSpriteCanvas( xCanvas ) );
    }

    SpriteCanvasSharedPtr VCLFactory::createFullscreenSpriteCanvas( const ::Window& rVCLWindow, 
                                                                    const Size& 	rFullscreenSize ) const
    {
        return SpriteCanvasSharedPtr( 
            new internal::ImplSpriteCanvas( 
                uno::Reference< rendering::XSpriteCanvas >(
                    rVCLWindow.GetFullscreenSpriteCanvas( rFullscreenSize ),
                    uno::UNO_QUERY) ) );
    }

    PolyPolygonSharedPtr VCLFactory::createPolyPolygon( const CanvasSharedPtr& 	rCanvas, 
                                                        const ::Polygon& 		rPoly ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return PolyPolygonSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return PolyPolygonSharedPtr();

        return PolyPolygonSharedPtr( 
            new internal::ImplPolyPolygon( rCanvas, 
                                           ::vcl::unotools::xPolyPolygonFromPolygon(
                                               xCanvas->getDevice(),
                                               rPoly) ) );
    }

    PolyPolygonSharedPtr VCLFactory::createPolyPolygon( const CanvasSharedPtr& 	rCanvas, 
                                                        const ::PolyPolygon& 	rPolyPoly ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createPolyPolygon(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return PolyPolygonSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return PolyPolygonSharedPtr();

        return PolyPolygonSharedPtr( 
            new internal::ImplPolyPolygon( rCanvas, 
                                           ::vcl::unotools::xPolyPolygonFromPolyPolygon(
                                               xCanvas->getDevice(),
                                               rPolyPoly) ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr&	rCanvas, 
                                              const ::Size& 			rSize ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createBitmap(): Invalid canvas" );
        
        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( 
            new internal::ImplBitmap( rCanvas, 
                                      xCanvas->getDevice()->createCompatibleBitmap( 
                                          ::vcl::unotools::integerSize2DFromSize(rSize) ) ) );
    }

    BitmapSharedPtr VCLFactory::createAlphaBitmap( const CanvasSharedPtr&	rCanvas, 
                                                   const ::Size& 			rSize ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createBitmap(): Invalid canvas" );
        
        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( 
            new internal::ImplBitmap( rCanvas, 
                                      xCanvas->getDevice()->createCompatibleAlphaBitmap( 
                                          ::vcl::unotools::integerSize2DFromSize(rSize) ) ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr& 	rCanvas, 
                                              const ::Bitmap& 			rBitmap ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( new internal::ImplBitmap( rCanvas, 
                                                          ::vcl::unotools::xBitmapFromBitmap(
                                                              xCanvas->getDevice(),
                                                              rBitmap) ) );
    }

    BitmapSharedPtr VCLFactory::createBitmap( const CanvasSharedPtr& 	rCanvas, 
                                              const ::BitmapEx& 		rBmpEx ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createBitmap(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return BitmapSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return BitmapSharedPtr();

        return BitmapSharedPtr( new internal::ImplBitmap( rCanvas, 
                                                          ::vcl::unotools::xBitmapFromBitmapEx(
                                                              xCanvas->getDevice(),
                                                              rBmpEx) ) );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr& 		rCanvas, 
                                                  const ::Graphic& 				rGraphic,
                                                  const Renderer::Parameters& 	rParms ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createRenderer(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return RendererSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return RendererSharedPtr();

        if( rGraphic.GetType() == GRAPHIC_GDIMETAFILE )
            return RendererSharedPtr( new internal::ImplRenderer( rCanvas, 
                                                                  rGraphic.GetGDIMetaFile(),
                                                                  rParms ) );
        else
            return RendererSharedPtr( new internal::ImplRenderer( rCanvas, 
                                                                  rGraphic.GetBitmapEx(),
                                                                  rParms ) );
    }

    RendererSharedPtr VCLFactory::createRenderer( const CanvasSharedPtr& 		rCanvas, 
                                                  const ::GDIMetaFile& 			rMtf,
                                                  const Renderer::Parameters& 	rParms ) const
    {
        return RendererSharedPtr( new internal::ImplRenderer( rCanvas, 
                                                              rMtf,
                                                              rParms ) );
    }

    SpriteSharedPtr VCLFactory::createAnimatedSprite( const SpriteCanvasSharedPtr& rCanvas, const ::Animation& rAnim ) const
    {
        OSL_ENSURE( rCanvas.get() != NULL &&
                    rCanvas->getUNOCanvas().is(), 
                    "VCLFactory::createAnimatedSprite(): Invalid canvas" );

        if( rCanvas.get() == NULL )
            return SpriteSharedPtr();

        uno::Reference< rendering::XCanvas > xCanvas( rCanvas->getUNOCanvas() );
        if( !xCanvas.is() )
            return SpriteSharedPtr();

        uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas( rCanvas->getUNOSpriteCanvas() );
        if( !xSpriteCanvas.is() )
            return SpriteSharedPtr();

        if( rAnim.IsEmpty() )
            return SpriteSharedPtr();

        internal::ImplSpriteCanvas* pSpriteCanvas = dynamic_cast< internal::ImplSpriteCanvas* >( rCanvas.get() );
        if( !pSpriteCanvas )
            return SpriteSharedPtr();

        const sal_uInt16 nBitmaps( rAnim.Count() );
        uno::Sequence< uno::Reference< rendering::XBitmap > > aBitmapSequence( nBitmaps );
        uno::Reference< rendering::XBitmap >* pBitmaps = aBitmapSequence.getArray();

        unsigned int i;
        BitmapEx aBmpEx;
        BitmapEx aRestoreBuffer;
        aBmpEx.SetSizePixel( rAnim.GetDisplaySizePixel() );
        aRestoreBuffer.SetSizePixel( rAnim.GetDisplaySizePixel() );
        aBmpEx.Erase( ::Color( 255, 0,0,0 ) ); // clear alpha channel
        aRestoreBuffer = aBmpEx;
        const Point aEmptyPoint;

        for( i=0; i<nBitmaps; ++i )
        {
            const AnimationBitmap& rAnimBmp( rAnim.Get((sal_uInt16)i) );

            // Handle dispose according to GIF spec: a
            // DISPOSE_PREVIOUS does _not_ mean to revert to the
            // previous frame, but to revert to the last frame with
            // DISPOSE_NOT

            // dispose previous
        	if( rAnimBmp.eDisposal == DISPOSE_BACK )
            {
                // simply clear bitmap to transparent
                aBmpEx.Erase( ::Color( 255, 0,0,0 ) );
            }
    		else if( rAnimBmp.eDisposal == DISPOSE_PREVIOUS ) 
            {
                // copy in last known full frame
                aBmpEx = aRestoreBuffer;
            }
            // I have exactly _no_ idea what DISPOSE_FULL is supposed
            // to do. It's apparently not set anywhere in our code
            OSL_ENSURE( rAnimBmp.eDisposal!=DISPOSE_FULL,
                        "VCLFactory::createAnimatedSprite(): Somebody set the deprecated DISPOSE_FULL at the Animation" );

            // update display
            aBmpEx.CopyPixel( Rectangle( rAnimBmp.aPosPix, 
                                         rAnimBmp.aSizePix ), 
                              Rectangle( aEmptyPoint,
                                         rAnimBmp.aSizePix ), 
                              &rAnimBmp.aBmpEx );

            // store last DISPOSE_NOT frame, for later
            // DISPOSE_PREVIOUS updates
            if( rAnimBmp.eDisposal == DISPOSE_NOT )
                aRestoreBuffer = aBmpEx;

            pBitmaps[i] = ::vcl::unotools::xBitmapFromBitmapEx(
                xCanvas->getDevice(),
                aBmpEx);
        }

        return pSpriteCanvas->createSpriteFromBitmaps( aBitmapSequence,
                                                       rendering::InterpolationMode::NEAREST_NEIGHBOR );
    }

    TextSharedPtr VCLFactory::createText( const CanvasSharedPtr& rCanvas, const ::rtl::OUString& rText ) const
    {
        return TextSharedPtr( new internal::ImplText( rCanvas, 
                                                      rText ) );
    }

}
