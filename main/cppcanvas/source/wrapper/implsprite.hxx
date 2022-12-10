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



#ifndef _CPPCANVAS_IMPLSPRITE_HXX
#define _CPPCANVAS_IMPLSPRITE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/rendering/XSprite.hpp>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#include <cppcanvas/sprite.hxx>

#include <implspritecanvas.hxx>


namespace cppcanvas
{
    namespace internal
    {
        class ImplSprite : public virtual Sprite
        {
        public:
            ImplSprite( const ::com::sun::star::uno::Reference< 
                              	::com::sun::star::rendering::XSpriteCanvas >& 	rParentCanvas,
                        const ::com::sun::star::uno::Reference< 
                              	::com::sun::star::rendering::XSprite >& 		rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&			rTransformArbiter );
            ImplSprite( const ::com::sun::star::uno::Reference< 
                              	::com::sun::star::rendering::XSpriteCanvas >& 	rParentCanvas,
                        const ::com::sun::star::uno::Reference< 
                              	::com::sun::star::rendering::XAnimatedSprite >& rSprite,
                        const ImplSpriteCanvas::TransformationArbiterSharedPtr&			rTransformArbiter );
            virtual ~ImplSprite();

            virtual void setAlpha( const double& rAlpha );
            virtual void movePixel( const ::basegfx::B2DPoint& rNewPos );
            virtual void move( const ::basegfx::B2DPoint& rNewPos );
            virtual void transform( const ::basegfx::B2DHomMatrix& rMatrix );
            virtual void setClipPixel( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void setClip( const ::basegfx::B2DPolyPolygon& rClipPoly );
            virtual void setClip();
        
            virtual void show();
            virtual void hide();

            virtual void setPriority( double fPriority );

            virtual ::com::sun::star::uno::Reference< 
                ::com::sun::star::rendering::XSprite > 	getUNOSprite() const;

            ::com::sun::star::uno::Reference< 
				::com::sun::star::rendering::XGraphicDevice >
													            getGraphicDevice() const;

        private:
            // default: disabled copy/assignment
            ImplSprite(const ImplSprite&);
            ImplSprite& operator=( const ImplSprite& );

            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice > 		mxGraphicDevice;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSprite > 			mxSprite;
            const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XAnimatedSprite > 	mxAnimatedSprite;
            ImplSpriteCanvas::TransformationArbiterSharedPtr										mpTransformArbiter;
		};
	}
}

#endif /* _CPPCANVAS_IMPLSPRITE_HXX */
