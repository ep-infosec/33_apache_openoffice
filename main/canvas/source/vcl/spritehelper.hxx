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



#ifndef _VCLCANVAS_SPRITEHELPER_HXX
#define _VCLCANVAS_SPRITEHELPER_HXX

#include <com/sun/star/rendering/XCustomSprite.hpp>

#include <vcl/virdev.hxx>

#include <canvas/base/canvascustomspritehelper.hxx>
#include <canvas/base/spritesurface.hxx>
#include <canvas/vclwrapper.hxx>

#include "backbuffer.hxx"
#include "impltools.hxx"
#include "spritecanvas.hxx"


namespace vclcanvas
{
	/* Definition of SpriteHelper class */

    /** Helper class for canvas sprites.

    	This class implements all sprite-related functionality, like
    	that available on the XSprite interface.
     */
    class SpriteHelper : public ::canvas::CanvasCustomSpriteHelper
    {
    public:
        SpriteHelper();

        // make CanvasCustomSpriteHelper::init visible for name lookup
        using ::canvas::CanvasCustomSpriteHelper::init;

        /** Late-init the sprite helper

        	@param rSpriteSize
            Size of the sprite

            @param rSpriteCanvas
            Sprite canvas this sprite is part of. Object stores
            ref-counted reference to it, thus, don't forget to pass on
            disposing()!

            @param rBackBuffer
            Buffer of the sprite content (non-alpha part)

            @param rBackBufferMask
            Buffer of the sprite content (alpha part)
         */
        void init( const ::com::sun::star::geometry::RealSize2D& rSpriteSize,
                   const ::canvas::SpriteSurface::Reference&     rOwningSpriteCanvas,
                   const BackBufferSharedPtr&					 rBackBuffer,
                   const BackBufferSharedPtr&					 rBackBufferMask,
                   bool											 bShowSpriteBounds );

        void disposing();

        /** Repaint sprite content to associated sprite canvas

        	@param rPos
            Output position (sprite's own position is disregarded)

            @param io_bSurfacesDirty
            When true, the referenced sprite surfaces (backBuffer and
            backBufferMask) have been modified since last call.

            @param bBufferedUpdate
            When true, the redraw does <em>not</em> happen directly on
            the front buffer, but within a VDev. Used to speed up
            drawing.
         */
        void redraw( OutputDevice&              rOutDev,
                     const ::basegfx::B2DPoint& rPos,
                     bool& 						bSurfacesDirty,
                     bool                       bBufferedUpdate ) const;

    private:
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D( 
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPoly ) const;

        // for the redraw
        BackBufferSharedPtr									mpBackBuffer;
        BackBufferSharedPtr									mpBackBufferMask;

        /// Cached bitmap for the current sprite content
        mutable ::canvas::vcltools::VCLObject<BitmapEx>		maContent;

        /// When true, line sprite corners in red
        bool                                                mbShowSpriteBounds;

    };
}

#endif /* _VCLCANVAS_SPRITEHELPER_HXX */
