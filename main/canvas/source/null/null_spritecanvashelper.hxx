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



#ifndef _NULLCANVAS_SPRITECANVASHELPER_HXX_
#define _NULLCANVAS_SPRITECANVASHELPER_HXX_

#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>

#include <canvas/spriteredrawmanager.hxx>

#include "null_canvashelper.hxx"

namespace basegfx
{
    class B2IRange;
}

namespace nullcanvas
{
    class SpriteCanvas;

    class SpriteCanvasHelper : public CanvasHelper
    {
    public:
        SpriteCanvasHelper();

        void init( ::canvas::SpriteRedrawManager& rManager,
                   SpriteCanvas&                  rDevice,
                   const ::basegfx::B2ISize&      rSize,
                   bool                           bHasAlpha );
    
        /// Dispose all internal references
        void disposing();

        // XSpriteCanvas
        ::com::sun::star::uno::Reference< 
  			::com::sun::star::rendering::XAnimatedSprite > 		createSpriteFromAnimation( 
                const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimation >& animation );

        ::com::sun::star::uno::Reference< 
  			::com::sun::star::rendering::XAnimatedSprite > 		createSpriteFromBitmaps( 
                const ::com::sun::star::uno::Sequence< 
                	  ::com::sun::star::uno::Reference< 
                			::com::sun::star::rendering::XBitmap > >& animationBitmaps, 
                sal_Int8                                              interpolationMode );
    
        ::com::sun::star::uno::Reference< 
  			::com::sun::star::rendering::XCustomSprite > 		createCustomSprite( 
                const ::com::sun::star::geometry::RealSize2D& spriteSize );
    
        ::com::sun::star::uno::Reference< 
  			::com::sun::star::rendering::XSprite > 				createClonedSprite( 
                const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite >& original );

        /** Actually perform the screen update

            @param rCurrArea
            Current window area in absolute screen coordinates

        	@param bUpdateAll
            sal_True, if everything must be updated, not only changed
            sprites

            @param io_bSurfaceDirty
            In/out parameter, whether backbuffer surface is dirty (if
            yes, we're performing a full update, anyway)
         */
        sal_Bool updateScreen( const ::basegfx::B2IRange& rCurrArea,
                               sal_Bool                   bUpdateAll,
                               bool&                      io_bSurfaceDirty );


        // SpriteRedrawManager functor calls
        // -------------------------------------------------

        /** Gets called for simple background repaints
         */
        void backgroundPaint( const ::basegfx::B2DRange& rUpdateRect );

        /** Gets called when area can be handled by scrolling.

        	Called method must copy screen content from rMoveStart to
        	rMoveEnd, and restore the background in the uncovered
        	areas.

            @param rMoveStart
            Source rect of the scroll

            @param rMoveEnd
            Dest rect of the scroll

            @param rUpdateArea
            All info necessary, should rMoveStart be partially or
            fully outside the outdev
         */
        void scrollUpdate( const ::basegfx::B2DRange& 							rMoveStart, 
                           const ::basegfx::B2DRange& 							rMoveEnd,
                           const ::canvas::SpriteRedrawManager::UpdateArea& 	rUpdateArea );

        void opaqueUpdate( const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea );

        void genericUpdate( const ::canvas::SpriteRedrawManager::UpdateArea& rUpdateArea );

    private:
        /// Set from the SpriteCanvas: instance coordinating sprite redraw
    	::canvas::SpriteRedrawManager*	mpRedrawManager;
    };
}

#endif /* _NULLCANVAS_SPRITECANVASHELPER_HXX_ */

