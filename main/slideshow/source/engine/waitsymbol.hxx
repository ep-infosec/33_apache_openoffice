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



#if ! defined(WAITSYMBOL_HXX_INCLUDED)
#define WAITSYMBOL_HXX_INCLUDED

#include <com/sun/star/rendering/XBitmap.hpp>
#include <cppcanvas/customsprite.hxx>

#include "vieweventhandler.hxx"
#include "screenupdater.hxx"
#include "eventmultiplexer.hxx"
#include "unoview.hxx"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp> // for noncopyable
#include <vector>

namespace slideshow {
namespace internal {

class EventMultiplexer;
typedef boost::shared_ptr<class WaitSymbol> WaitSymbolSharedPtr;

class WaitSymbol : public ViewEventHandler,
                   private ::boost::noncopyable
{
public:
    static WaitSymbolSharedPtr create( const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XBitmap>& xBitmap,
                                       ScreenUpdater&                               rScreenUpdater,
                                       EventMultiplexer&                            rEventMultiplexer,
                                       const UnoViewContainer&                      rViewContainer );
    
    /** Shows the wait symbol.
     */
    void show() { setVisible(true); }

    /** Hides the wait symbol.
     */
    void hide() { setVisible(false); }

private:
    WaitSymbol( const ::com::sun::star::uno::Reference<
                      ::com::sun::star::rendering::XBitmap>& xBitmap,
                ScreenUpdater&                               rScreenUpdater,
                const UnoViewContainer&                      rViewContainer );

    // ViewEventHandler
    virtual void viewAdded( const UnoViewSharedPtr& rView );
    virtual void viewRemoved( const UnoViewSharedPtr& rView );
    virtual void viewChanged( const UnoViewSharedPtr& rView );
    virtual void viewsChanged();

    void setVisible( const bool bVisible );
    ::basegfx::B2DPoint calcSpritePos( UnoViewSharedPtr const & rView ) const;

    template <typename func_type>
    void for_each_sprite( func_type const & func ) const
    {
        ViewsVecT::const_iterator iPos( maViews.begin() );
        const ViewsVecT::const_iterator iEnd( maViews.end() );
        for ( ; iPos != iEnd; ++iPos )
            if( iPos->second )
                func( iPos->second );
    }

    typedef ::std::vector<
        ::std::pair<UnoViewSharedPtr,
                    cppcanvas::CustomSpriteSharedPtr> > ViewsVecT;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XBitmap>  mxBitmap;

    ViewsVecT                                  maViews;
    ScreenUpdater&                             mrScreenUpdater;
    bool                                       mbVisible;
};

} // namespace internal
} // namespace presentation

#endif
