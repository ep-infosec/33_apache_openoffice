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



#ifndef _NULLCANVAS_SPRITEHELPER_HXX
#define _NULLCANVAS_SPRITEHELPER_HXX

#include <com/sun/star/rendering/XCustomSprite.hpp>

#include <canvas/base/canvascustomspritehelper.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include "null_spritecanvas.hxx"


namespace nullcanvas
{
	/* Definition of SpriteHelper class */

    /** Helper class for canvas sprites.

    	This class implements all sprite-related functionality, like
    	that available on the XSprite interface.
     */
    class SpriteHelper : public ::canvas::CanvasCustomSpriteHelper
    {
    public:
        /** Create sprite helper
         */
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

            @param rDevice
            DX device to use

            @param rSpriteSurface
            The surface of the sprite (not the DX texture, but the
            persistent target of content rendering)

            @param bShowSpriteBounds
            When true, little debug bound rects for sprites are shown
         */
        void init( const ::com::sun::star::geometry::RealSize2D&	rSpriteSize,
                   const SpriteCanvasRef&							rSpriteCanvas );

        void disposing();

        /** Repaint sprite content via hardware to associated sprite
            canvas

            @param io_bSurfaceDirty
            Input/output parameter, whether the sprite content is
            dirty or not. If texture was updated, set to false
         */
        void redraw( bool& io_bSurfaceDirty ) const;
        
    private:
        virtual ::basegfx::B2DPolyPolygon polyPolygonFromXPolyPolygon2D( 
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XPolyPolygon2D >& xPoly ) const;


        SpriteCanvasRef				mpSpriteCanvas;
        mutable bool                mbTextureDirty;  // when true, texture needs update
    };
}

#endif /* _NULLCANVAS_SPRITEHELPER_HXX */
