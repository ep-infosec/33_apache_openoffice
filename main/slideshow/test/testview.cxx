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

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/make_shared_from_uno.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include "tests.hxx"
#include "view.hxx"
#include "unoview.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

#include <vector>
#include <exception>


namespace target = slideshow::internal;
using namespace ::com::sun::star;

// our test view subject
typedef ::cppu::WeakComponentImplHelper1< presentation::XSlideShowView > ViewBase;
class ImplTestView : public TestView,
                     private cppu::BaseMutex,
                     public ViewBase
{
    mutable std::vector<std::pair<basegfx::B2DVector,double> > maCreatedSprites;
    mutable std::vector<TestViewSharedPtr>                     maViewLayers;
    basegfx::B2DRange                                  maBounds;
    basegfx::B1DRange                                  maPriority;
    bool                                               mbIsClipSet;
    bool                                               mbIsClipEmptied;
    bool                                               mbIsClearCalled;
    bool                                               mbIsSoundEnabled;
    bool                                               mbDisposed;


public:
    ImplTestView() :
        ViewBase(m_aMutex),
        maCreatedSprites(),
        maViewLayers(),
        maBounds(),
        maPriority(),
        mbIsClipSet(false),
        mbIsClipEmptied(false),
        mbIsClearCalled(false),
        mbIsSoundEnabled(false),
        mbDisposed( false )
    {
    }

    virtual ~ImplTestView()
    {
    }

    // XSlideShowView
    virtual uno::Reference< rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) throw (uno::RuntimeException)
    {
        return uno::Reference< rendering::XSpriteCanvas >();
    }

    virtual ::com::sun::star::awt::Rectangle SAL_CALL getCanvasArea(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        // FIXME:
        ::com::sun::star::awt::Rectangle r;
        r.X = 0;
        r.Y = 0;
        r.Width = 0;
        r.Height = 0;
        return r;
    }

    virtual void SAL_CALL clear(  ) throw (uno::RuntimeException)
    {
    }

    virtual geometry::AffineMatrix2D SAL_CALL getTransformation(  ) throw (uno::RuntimeException)
    {
        return geometry::AffineMatrix2D();
    }

    virtual void SAL_CALL addTransformationChangedListener( const uno::Reference< util::XModifyListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeTransformationChangedListener( const uno::Reference< util::XModifyListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addPaintListener( const uno::Reference< awt::XPaintListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removePaintListener( const uno::Reference< awt::XPaintListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addMouseListener( const uno::Reference< awt::XMouseListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeMouseListener( const uno::Reference< awt::XMouseListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL setMouseCursor( ::sal_Int16 ) throw (uno::RuntimeException)
    {
    }

    // TestView
    virtual bool isClearCalled() const
    {
        return mbIsClearCalled;
    }

    virtual bool isSoundEnabled() const
    {
        return mbIsSoundEnabled;
    }

    virtual void setIsSoundEnabled(const bool bValue)
    {
        mbIsSoundEnabled = bValue;
    }

    virtual std::vector<std::pair<basegfx::B2DVector,double> > getCreatedSprites() const
    {
        return maCreatedSprites;
    }

    virtual basegfx::B1DRange getPriority() const
    {
        return maPriority;
    }

    virtual bool wasClipSet() const
    {
        return mbIsClipEmptied;
    }

    virtual basegfx::B2DRange getBounds() const
    {
        return maBounds;
    }

    virtual std::vector<boost::shared_ptr<TestView> > getViewLayers() const
    {
        return maViewLayers;
    }

    // ViewLayer
    virtual bool isOnView(target::ViewSharedPtr const& /*rView*/) const
    {
        return true;
    }

    virtual ::cppcanvas::CanvasSharedPtr getCanvas() const
    {
        return ::cppcanvas::CanvasSharedPtr();
    }

    virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& rSpriteSizePixel,
                                                             double                    nPriority ) const
    {
        maCreatedSprites.push_back( std::make_pair(rSpriteSizePixel,nPriority) );

        return ::cppcanvas::CustomSpriteSharedPtr();
    }

    virtual void setPriority( const basegfx::B1DRange& rRange )
    {
        maPriority = rRange;
    }

    virtual ::basegfx::B2DHomMatrix getTransformation() const
    {
        return ::basegfx::B2DHomMatrix();
    }

    virtual ::basegfx::B2DHomMatrix getSpriteTransformation() const
    {
        return ::basegfx::B2DHomMatrix();
    }

    virtual void setClip( const ::basegfx::B2DPolyPolygon& rClip )
    {
        if( !mbIsClipSet )
        {
            if( rClip.count() > 0 )
                mbIsClipSet = true;
        }
        else if( !mbIsClipEmptied )
        {
            if( rClip.count() == 0 )
                mbIsClipEmptied = true;
        }
        else if( rClip.count() > 0 )
        {
            mbIsClipSet = true;
            mbIsClipEmptied = false;
        }
        else
        {
            // unexpected call
            throw std::exception();
        }
    }

    virtual bool resize( const basegfx::B2DRange& rArea )
    {
        const bool bRet( maBounds != rArea );
        maBounds = rArea;
        return bRet;
    }

    virtual target::ViewLayerSharedPtr createViewLayer(
        const basegfx::B2DRange& rLayerBounds ) const
    {
        maViewLayers.push_back( TestViewSharedPtr(new ImplTestView()));
        maViewLayers.back()->resize( rLayerBounds );

        return maViewLayers.back();
    }

    virtual bool updateScreen() const
    {
        // misusing updateScreen for state reporting
        return !mbDisposed;
    }

    virtual bool paintScreen() const
    {
        // misusing updateScreen for state reporting
        return !mbDisposed;
    }

    virtual void clear() const
    {
    }

    virtual void clearAll() const
    {
    }

    virtual void setViewSize( const ::basegfx::B2DSize& )
    {
    }

    virtual void setCursorShape( sal_Int16 /*nPointerShape*/ )
    {
    }

    virtual uno::Reference< presentation::XSlideShowView > getUnoView() const
    {
        return uno::Reference< presentation::XSlideShowView >( const_cast<ImplTestView*>(this) );
    }

    virtual void _dispose()
    {
        mbDisposed = true;
    }
};


TestViewSharedPtr createTestView()
{
    return TestViewSharedPtr( 
        comphelper::make_shared_from_UNO(
            new ImplTestView()) );
}
