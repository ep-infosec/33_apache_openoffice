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

#include <stdio.h>
#include <poll.h>

#include "vcl/salbtype.hxx"

#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salbmp.h"
#include "unx/salgdi.h"
#include "unx/salframe.h"
#include "unx/salvd.h"
#include "xrender_peer.hxx"

#include "printergfx.hxx"

#include "vcl/bmpacc.hxx"

#undef SALGDI2_TESTTRANS

// -=-= debugging =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#if 0

static void sal_PrintImage( char *s, XImage*p )
{
	fprintf( stderr, "%s %d %d %d\n", s, p->depth, p->width, p->height );
	int nW = Min( 64, p->width*p->bits_per_pixel >> 3 );
	for( int i = 0; i < Min( 16, p->height ); i++ )
	{
		for( int j = 0; j < nW; j++ )
			fprintf( stderr, "%02X", (UINT8)p->data[i*p->bytes_per_line+j] );
		fprintf( stderr, "\n" );
	}
}

#endif // DBG_UTIL

// -----------------------------------------------------------------------------

#if (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )								\
{																	\
	XCopyArea( pXDisp, _def_drawable, aDrawable, GetCopyGC(),		\
			   0, 0,												\
			   pPosAry->mnDestWidth, pPosAry->mnDestHeight,			\
			   0, 0 );												\
}
#else // (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS
#define DBG_TESTTRANS( _def_drawable )
#endif // (OSL_DEBUG_LEVEL > 1) && defined SALGDI2_TESTTRANS

// -=-= X11SalGraphics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::CopyScreenArea( Display* pDisplay,
                               Drawable aSrc, int nScreenSrc, int nSrcDepth,
                               Drawable aDest, int nScreenDest, int nDestDepth,
                               GC aDestGC,
                               int src_x, int src_y,
                               unsigned int w, unsigned int h,
                               int dest_x, int dest_y )
{
    if( nSrcDepth == nDestDepth )
    {
        if( nScreenSrc == nScreenDest )
            XCopyArea( pDisplay, aSrc, aDest, aDestGC,
                       src_x, src_y, w, h, dest_x, dest_y );
        else
        {
            SalXLib* pLib = GetX11SalData()->GetDisplay()->GetXLib();
            pLib->PushXErrorLevel( true );
            XImage* pImage = XGetImage( pDisplay, aSrc, src_x, src_y, w, h,
                                        AllPlanes, ZPixmap );
            if( pImage )
            {
                if( pImage->data )
                {
                    XPutImage( pDisplay, aDest, aDestGC, pImage,
                               0, 0, dest_x, dest_y, w, h );
                }
                XDestroyImage( pImage );
            }
            pLib->PopXErrorLevel();
        }
    }
    else
    {
        X11SalBitmap aBM;
        aBM.ImplCreateFromDrawable( aSrc, nScreenSrc, nSrcDepth, src_x, src_y, w, h );
        SalTwoRect aTwoRect;
        aTwoRect.mnSrcX = aTwoRect.mnSrcY = 0;
        aTwoRect.mnSrcWidth = aTwoRect.mnDestWidth = w;
        aTwoRect.mnSrcHeight = aTwoRect.mnDestHeight = h;
        aTwoRect.mnDestX = dest_x;
        aTwoRect.mnDestY = dest_y;
        aBM.ImplDraw( aDest, nScreenDest, nDestDepth, aTwoRect,aDestGC );
    }
}

GC X11SalGraphics::CreateGC( Drawable hDrawable, unsigned long nMask )
{
	XGCValues values;

	values.graphics_exposures	= False;
	values.foreground			= m_pColormap->GetBlackPixel()
		                          ^ m_pColormap->GetWhitePixel();
	values.function				= GXxor;
	values.line_width			= 1;
	values.fill_style			= FillStippled;
	values.stipple				= GetDisplay()->GetInvert50( m_nScreen );
    values.subwindow_mode		= ClipByChildren;

	return XCreateGC( GetXDisplay(), hDrawable, nMask | GCSubwindowMode, &values );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline GC X11SalGraphics::GetMonoGC( Pixmap hPixmap )
{
	if( !pMonoGC_ )
		pMonoGC_ = CreateGC( hPixmap );

	if( !bMonoGC_ )
	{
		SetClipRegion( pMonoGC_ );
		bMonoGC_ = sal_True;
	}

	return pMonoGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline GC X11SalGraphics::GetCopyGC()
{
	if( bXORMode_ ) return GetInvertGC();

	if( !pCopyGC_ )
		pCopyGC_ = CreateGC( GetDrawable() );

	if( !bCopyGC_ )
	{
		SetClipRegion( pCopyGC_ );
		bCopyGC_ = sal_True;
	}
	return pCopyGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::GetInvertGC()
{
	if( !pInvertGC_ )
		pInvertGC_ = CreateGC( GetDrawable(),
							   GCGraphicsExposures
							   | GCForeground
							   | GCFunction
							   | GCLineWidth );

	if( !bInvertGC_ )
	{
		SetClipRegion( pInvertGC_ );
		bInvertGC_ = sal_True;
	}
	return pInvertGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::GetInvert50GC()
{
	if( !pInvert50GC_ )
	{
		XGCValues values;

		values.graphics_exposures	= False;
		values.foreground			= m_pColormap->GetWhitePixel();
		values.background			= m_pColormap->GetBlackPixel();
		values.function				= GXinvert;
		values.line_width			= 1;
		values.line_style			= LineSolid;
		unsigned long nValueMask =
								  GCGraphicsExposures
								  | GCForeground
								  | GCBackground
								  | GCFunction
								  | GCLineWidth
								  | GCLineStyle
								  | GCFillStyle
								  | GCStipple;

		char* pEnv = getenv( "SAL_DO_NOT_USE_INVERT50" );
		if( pEnv && ! strcasecmp( pEnv, "true" ) )
		{
			values.fill_style = FillSolid;
			nValueMask &= ~ GCStipple;
		}
		else
		{
			values.fill_style			= FillStippled;
			values.stipple				= GetDisplay()->GetInvert50( m_nScreen );
		}

		pInvert50GC_ = XCreateGC( GetXDisplay(), GetDrawable(),
								  nValueMask,
								  &values );
	}

	if( !bInvert50GC_ )
	{
		SetClipRegion( pInvert50GC_ );
		bInvert50GC_ = sal_True;
	}
	return pInvert50GC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline GC X11SalGraphics::GetStippleGC()
{
	if( !pStippleGC_ )
		pStippleGC_ = CreateGC( GetDrawable(),
								GCGraphicsExposures
								| GCFillStyle
								| GCLineWidth );

	if( !bStippleGC_ )
	{
		XSetFunction( GetXDisplay(), pStippleGC_, bXORMode_ ? GXxor : GXcopy );
		SetClipRegion( pStippleGC_ );
		bStippleGC_ = sal_True;
	}

	return pStippleGC_;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int X11SalGraphics::Clip( XLIB_Region   pRegion,
								 int          &nX,
								 int          &nY,
								 unsigned int &nDX,
								 unsigned int &nDY,
								 int          &nSrcX,
								 int          &nSrcY ) const
{
	XRectangle aRect;
	XClipBox( pRegion, &aRect );

	if( int(nX + nDX) <= int(aRect.x) || nX >= int(aRect.x + aRect.width) )
		return RectangleOut;
	if( int(nY + nDY) <= int(aRect.y) || nY >= int(aRect.y + aRect.height) )
		return RectangleOut;

	if( nX < aRect.x )
	{
		nSrcX += aRect.x - nX;
		nDX   -= aRect.x - nX;
		nX     = aRect.x;
	}
	else if( int(nX + nDX) > int(aRect.x + aRect.width) )
		nDX = aRect.x + aRect.width - nX;

	if( nY < aRect.y )
	{
		nSrcY += aRect.y - nY;
		nDY   -= aRect.y - nY;
		nY     = aRect.y;
	}
	else if( int(nY + nDY) > int(aRect.y + aRect.height) )
		nDY = aRect.y + aRect.height - nY;

	return RectangleIn;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
int X11SalGraphics::Clip( int          &nX,
								 int          &nY,
								 unsigned int &nDX,
								 unsigned int &nDY,
								 int          &nSrcX,
								 int          &nSrcY ) const

{
	if( pPaintRegion_
		&& RectangleOut == Clip( pPaintRegion_, nX, nY, nDX, nDY, nSrcX, nSrcY ) )
		return RectangleOut;

	if( mpClipRegion
		&& RectangleOut == Clip( mpClipRegion,  nX, nY, nDX, nDY, nSrcX, nSrcY ) )
		return RectangleOut;

	int nPaint;
	if( pPaintRegion_ )
	{
		nPaint = XRectInRegion( pPaintRegion_, nX, nY, nDX, nDY );
		if( RectangleOut == nPaint )
			return RectangleOut;
	}
	else
		nPaint = RectangleIn;

	int nClip;
	if( mpClipRegion )
	{
		nClip = XRectInRegion( mpClipRegion, nX, nY, nDX, nDY );
		if( RectangleOut == nClip )
			return RectangleOut;
	}
	else
		nClip = RectangleIn;

	return RectangleIn == nClip && RectangleIn == nPaint
		   ? RectangleIn
		   : RectanglePart;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
GC X11SalGraphics::SetMask( int           &nX,
								   int           &nY,
								   unsigned int &nDX,
								   unsigned int &nDY,
								   int          &nSrcX,
								   int          &nSrcY,
								   Pixmap        hClipMask )
{
	int n = Clip( nX, nY, nDX, nDY, nSrcX, nSrcY );
	if( RectangleOut == n )
		return NULL;

	Display *pDisplay = GetXDisplay();

	if( !pMaskGC_ )
		pMaskGC_ = CreateGC( GetDrawable() );

	if( RectangleIn == n )
	{
		XSetClipMask( pDisplay, pMaskGC_, hClipMask );
		XSetClipOrigin( pDisplay, pMaskGC_, nX - nSrcX, nY - nSrcY );
		return pMaskGC_;
	}

	// - - - - create alternate clip pixmap for region clipping - - - -
	Pixmap hPixmap	= XCreatePixmap( pDisplay, hClipMask, nDX, nDY, 1 );

	if( !hPixmap )
	{
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
		fprintf( stderr, "X11SalGraphics::SetMask !hPixmap\n" );
#endif
		return NULL;
	}

	// - - - - reset pixmap; all 0 - - - - - - - - - - - - - - - - - - -
	XFillRectangle( pDisplay,
					hPixmap,
					GetDisplay()->GetMonoGC( m_nScreen ),
					0,   0,
					nDX, nDY );

	// - - - - copy pixmap only within region - - - - - - - - - - - - -
	GC pMonoGC = GetMonoGC( hPixmap );
	XSetClipOrigin( pDisplay, pMonoGC, -nX, -nY );
	XCopyArea( pDisplay,
			   hClipMask,			// Source
			   hPixmap,				// Destination
			   pMonoGC,
			   nSrcX, nSrcY,		// Source
			   nDX,   nDY,			// Width & Height
			   0,     0 );			// Destination

	XSetClipMask( pDisplay, pMaskGC_, hPixmap );
	XSetClipOrigin( pDisplay, pMaskGC_, nX, nY );

	XFreePixmap( pDisplay, hPixmap );
	return pMaskGC_;
}

// -=-= SalGraphics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

extern "C" 
{
    static Bool GraphicsExposePredicate( Display*, XEvent* pEvent, XPointer pFrameWindow )
    {
        Bool bRet = False;
        if( (pEvent->type == GraphicsExpose || pEvent->type == NoExpose) &&
            pEvent->xnoexpose.drawable == (Drawable)pFrameWindow )
        {
            bRet = True;
        }
        return bRet;
    }
}


void X11SalGraphics::YieldGraphicsExpose()
{
    // get frame if necessary
    SalFrame* pFrame    = m_pFrame;
    Display* pDisplay   = GetXDisplay();
    XLIB_Window aWindow = GetDrawable();
    if( ! pFrame )
    {
        const std::list< SalFrame* >& rFrames = GetX11SalData()->GetDisplay()->getFrames();
        for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end() && ! pFrame; ++it )
        {
            const SystemEnvData* pEnvData = (*it)->GetSystemData();
            if( Drawable(pEnvData->aWindow) == aWindow )
                pFrame = *it;
        }
        if( ! pFrame )
            return;
    }

    XEvent aEvent;
    while( XCheckTypedWindowEvent( pDisplay, aWindow, Expose, &aEvent ) )
    {
        SalPaintEvent aPEvt( aEvent.xexpose.x, aEvent.xexpose.y, aEvent.xexpose.width+1, aEvent.xexpose.height+1 );
        pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
    }

    do
    {
        if( ! GetDisplay()->XIfEventWithTimeout( &aEvent, (XPointer)aWindow, GraphicsExposePredicate ) )
            // this should not happen at all; still sometimes it happens
            break;

        if( aEvent.type == NoExpose )
            break;

        if( pFrame )
        {
            SalPaintEvent aPEvt( aEvent.xgraphicsexpose.x, aEvent.xgraphicsexpose.y, aEvent.xgraphicsexpose.width+1, aEvent.xgraphicsexpose.height+1 );
            pFrame->CallCallback( SALEVENT_PAINT, &aPEvt );
        }
    } while( aEvent.xgraphicsexpose.count != 0 );
}

void X11SalGraphics::copyBits( const SalTwoRect& rPosAry,
								  SalGraphics	   *pSSrcGraphics )
{
    X11SalGraphics* pSrcGraphics = pSSrcGraphics
        ? static_cast<X11SalGraphics*>(pSSrcGraphics)
        : this;
    
    if( rPosAry.mnSrcWidth <= 0
        || rPosAry.mnSrcHeight <= 0
        || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0 )
    {
        return;
    }
    
    int n;
    if( pSrcGraphics == this )
    {
        n = 2;
    }
    else if( pSrcGraphics->bWindow_ )
    {
        // window or compatible virtual device
        if( pSrcGraphics->GetDisplay() == GetDisplay() &&
            pSrcGraphics->m_nScreen == m_nScreen &&
            pSrcGraphics->GetVisual().GetDepth() == GetVisual().GetDepth()
            )
            n = 2; // same Display
        else
            n = 1; // printer or other display
    }
    else if( pSrcGraphics->bVirDev_ )
    {
        // printer compatible virtual device
        if( bPrinter_ )
            n = 2; // printer or compatible virtual device == same display
        else
            n = 1; // window or compatible virtual device
    }
    else
        n = 0;
    
    if( n == 2
        && rPosAry.mnSrcWidth	== rPosAry.mnDestWidth
        && rPosAry.mnSrcHeight == rPosAry.mnDestHeight
        )
    {
        // #i60699# Need to generate graphics exposures (to repaint
        // obscured areas beneath overlapping windows), src and dest
        // are the same window.
        const bool bNeedGraphicsExposures( pSrcGraphics == this &&
                                           !bVirDev_ && 
                                           pSrcGraphics->bWindow_ );

        GC pCopyGC;

        if( bXORMode_
            && !pSrcGraphics->bVirDev_
            && (GetDisplay()->GetProperties() & PROPERTY_BUG_XCopyArea_GXxor) )
        {
            Pixmap hPixmap = XCreatePixmap( GetXDisplay(),
                                            pSrcGraphics->GetDrawable(),		// source
                                            rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                                            pSrcGraphics->GetBitCount() );

            pCopyGC = GetDisplay()->GetCopyGC( m_nScreen );

            if( bNeedGraphicsExposures )
                XSetGraphicsExposures( GetXDisplay(),
                                       pCopyGC,
                                       True );

            XCopyArea( GetXDisplay(),
                       pSrcGraphics->GetDrawable(),		// source
                       hPixmap,							// destination
                       pCopyGC,							// no clipping
                       rPosAry.mnSrcX,     rPosAry.mnSrcY,
                       rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                       0,    				0 );			// destination
            XCopyArea( GetXDisplay(),
                       hPixmap,								// source
                       GetDrawable(),						// destination
                       GetInvertGC(),		// destination clipping
                       0,				    0,				// source
                       rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                       rPosAry.mnDestX,    rPosAry.mnDestY );
            XFreePixmap( GetXDisplay(), hPixmap );
        }
        else
        {
            pCopyGC = GetCopyGC();
            
            if( bNeedGraphicsExposures )
                XSetGraphicsExposures( GetXDisplay(),
                                       pCopyGC,
                                       True );

            XCopyArea( GetXDisplay(),
                       pSrcGraphics->GetDrawable(),		// source
                       GetDrawable(),					// destination
                       pCopyGC,							// destination clipping
                       rPosAry.mnSrcX,     rPosAry.mnSrcY,
                       rPosAry.mnSrcWidth, rPosAry.mnSrcHeight,
                       rPosAry.mnDestX,    rPosAry.mnDestY );
        }

        if( bNeedGraphicsExposures )
        {
            YieldGraphicsExpose();

            if( pCopyGC )
                XSetGraphicsExposures( GetXDisplay(),
                                       pCopyGC,
                                       False );
        }
    }
    else if( n )
    {
        // #i60699# No chance to handle graphics exposures - we copy
        // to a temp bitmap first, into which no repaints are
        // technically possible.
        SalBitmap *pDDB = pSrcGraphics->getBitmap( rPosAry.mnSrcX,
                                                   rPosAry.mnSrcY,
                                                   rPosAry.mnSrcWidth,
                                                   rPosAry.mnSrcHeight );
        
        if( !pDDB )
        {
            stderr0( "SalGraphics::CopyBits !pSrcGraphics->GetBitmap()\n" );
            return;
        }
        
        SalTwoRect aPosAry( rPosAry );
        
        aPosAry.mnSrcX = 0,	aPosAry.mnSrcY = 0;
        drawBitmap( aPosAry, *pDDB );
        
        delete pDDB;
    }
    else {
        stderr0( "X11SalGraphics::CopyBits from Printer not yet implemented\n" );
    }
}

// --------------------------------------------------------------------------

void X11SalGraphics::copyArea ( long nDestX,    long nDestY,
                                long nSrcX,     long nSrcY,
                                long nSrcWidth, long nSrcHeight,
                                sal_uInt16 )
{
    SalTwoRect aPosAry;
    
    aPosAry.mnDestX = nDestX;
    aPosAry.mnDestY = nDestY;
    aPosAry.mnDestWidth  = nSrcWidth;
    aPosAry.mnDestHeight = nSrcHeight;
    
    aPosAry.mnSrcX = nSrcX;
    aPosAry.mnSrcY = nSrcY;
    aPosAry.mnSrcWidth  = nSrcWidth;
    aPosAry.mnSrcHeight = nSrcHeight;
    
    copyBits ( aPosAry, 0 );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    const SalDisplay*   pSalDisp = GetDisplay();
    Display*			pXDisp = pSalDisp->GetDisplay();
    const Drawable		aDrawable( GetDrawable() );
    const SalColormap&	rColMap = pSalDisp->GetColormap( m_nScreen );
    const long			nDepth = GetDisplay()->GetVisual( m_nScreen ).GetDepth();
    GC					aGC( GetCopyGC() );
    XGCValues			aOldVal, aNewVal;
    int					nValues = GCForeground | GCBackground;
    
    if( rSalBitmap.GetBitCount() == 1 )
    {
        // set foreground/background values for 1Bit bitmaps
        XGetGCValues( pXDisp, aGC, nValues, &aOldVal );
        aNewVal.foreground = rColMap.GetWhitePixel(), aNewVal.background = rColMap.GetBlackPixel();
        XChangeGC( pXDisp, aGC, nValues, &aNewVal );
    }
    
    static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aDrawable, m_nScreen, nDepth, rPosAry, aGC );
    
    if( rSalBitmap.GetBitCount() == 1 )
        XChangeGC( pXDisp, aGC, nValues, &aOldVal );
    XFlush( pXDisp );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalGraphics::drawBitmap( const SalTwoRect& rPosAry,
                                 const SalBitmap& rSrcBitmap,
                                 const SalBitmap& rMaskBitmap )
{
    DBG_ASSERT( !bPrinter_, "Drawing of transparent bitmaps on printer devices is strictly forbidden" );

    // decide if alpha masking or transparency masking is needed
    BitmapBuffer* pAlphaBuffer = const_cast<SalBitmap&>(rMaskBitmap).AcquireBuffer( sal_True );
    if( pAlphaBuffer != NULL )
    {
        int nMaskFormat = pAlphaBuffer->mnFormat;
        const_cast<SalBitmap&>(rMaskBitmap).ReleaseBuffer( pAlphaBuffer, sal_True );
        if( nMaskFormat == BMP_FORMAT_8BIT_PAL )
            drawAlphaBitmap( rPosAry, rSrcBitmap, rMaskBitmap );
    }

    drawMaskedBitmap( rPosAry, rSrcBitmap, rMaskBitmap );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalGraphics::drawMaskedBitmap( const SalTwoRect& rPosAry,
                                       const SalBitmap& rSalBitmap,
                                       const SalBitmap& rTransBitmap )
{
    const SalDisplay*   pSalDisp = GetDisplay();
    Display*            pXDisp = pSalDisp->GetDisplay();
    Drawable            aDrawable( GetDrawable() );

    // figure work mode depth. If this is a VDev Drawable, use its
    // bitdepth to create pixmaps for, otherwise, XCopyArea will
    // refuse to work.
    const sal_uInt16	nDepth( m_pVDev ? 
                            m_pVDev->GetDepth() :
                            pSalDisp->GetVisual( m_nScreen ).GetDepth() );
    Pixmap			aFG( XCreatePixmap( pXDisp, aDrawable, rPosAry.mnDestWidth,
                                        rPosAry.mnDestHeight, nDepth ) );
    Pixmap			aBG( XCreatePixmap( pXDisp, aDrawable, rPosAry.mnDestWidth,
                                        rPosAry.mnDestHeight, nDepth ) );
    
    if( aFG && aBG )
    {
        GC					aTmpGC;
        XGCValues			aValues;
        const SalColormap&	rColMap = pSalDisp->GetColormap( m_nScreen );
        const int			nBlack = rColMap.GetBlackPixel(), nWhite = rColMap.GetWhitePixel();
        const int			nValues = GCFunction | GCForeground | GCBackground;
        SalTwoRect			aTmpRect( rPosAry ); aTmpRect.mnDestX = aTmpRect.mnDestY = 0;
        
        // draw paint bitmap in pixmap #1
        aValues.function = GXcopy, aValues.foreground = nWhite, aValues.background = nBlack;
        aTmpGC = XCreateGC( pXDisp, aFG, nValues, &aValues );
        static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aFG, m_nScreen, nDepth, aTmpRect, aTmpGC );
        DBG_TESTTRANS( aFG );

        // draw background in pixmap #2
        XCopyArea( pXDisp, aDrawable, aBG, aTmpGC,
                   rPosAry.mnDestX, rPosAry.mnDestY,
                   rPosAry.mnDestWidth, rPosAry.mnDestHeight,
                   0, 0 );

        DBG_TESTTRANS( aBG );
        
        // mask out paint bitmap in pixmap #1 (transparent areas 0)
        aValues.function = GXand, aValues.foreground = 0x00000000, aValues.background = 0xffffffff;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aFG, m_nScreen, 1, aTmpRect, aTmpGC );
        
        DBG_TESTTRANS( aFG );
        
        // #105055# For XOR mode, keep background behind bitmap intact
        if( !bXORMode_ )
        {
            // mask out background in pixmap #2 (nontransparent areas 0)
            aValues.function = GXand, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
            XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
            static_cast<const X11SalBitmap&>(rTransBitmap).ImplDraw( aBG, m_nScreen, 1, aTmpRect, aTmpGC );
            
            DBG_TESTTRANS( aBG );
        }
        
        // merge pixmap #1 and pixmap #2 in pixmap #2
        aValues.function = GXxor, aValues.foreground = 0xffffffff, aValues.background = 0x00000000;
        XChangeGC( pXDisp, aTmpGC, nValues, &aValues );
        XCopyArea( pXDisp, aFG, aBG, aTmpGC,
                   0, 0,
                   rPosAry.mnDestWidth, rPosAry.mnDestHeight,
				   0, 0 );
        DBG_TESTTRANS( aBG );
        
        // #105055# Disable XOR temporarily
        sal_Bool bOldXORMode( bXORMode_ );
        bXORMode_ = sal_False;
        
        // copy pixmap #2 (result) to background
        XCopyArea( pXDisp, aBG, aDrawable, GetCopyGC(),
                   0, 0,
                   rPosAry.mnDestWidth, rPosAry.mnDestHeight,
                   rPosAry.mnDestX, rPosAry.mnDestY );
        
        DBG_TESTTRANS( aBG );
        
        bXORMode_ = bOldXORMode;
        
        XFreeGC( pXDisp, aTmpGC );
        XFlush( pXDisp );
    }
    else
        drawBitmap( rPosAry, rSalBitmap );
    
    if( aFG )
        XFreePixmap( pXDisp, aFG );
    
    if( aBG )
        XFreePixmap( pXDisp, aBG );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool X11SalGraphics::drawAlphaBitmap( const SalTwoRect& rTR,
    const SalBitmap& rSrcBitmap, const SalBitmap& rAlphaBmp )
{
    // non 8-bit alpha not implemented yet
    if( rAlphaBmp.GetBitCount() != 8 )
        return false;

    // horizontal mirroring not implemented yet
    if( rTR.mnDestWidth < 0 )
        return false;
    
    // stretched conversion is not implemented yet
    if( rTR.mnDestWidth != rTR.mnSrcWidth )
        return false;
    if( rTR.mnDestHeight!= rTR.mnSrcHeight )
        return false;

    XRenderPeer& rPeer = XRenderPeer::GetInstance();
    if( rPeer.GetVersion() < 0x02 )
        return false;

    // create destination picture
    Picture aDstPic = GetXRenderPicture();
    if( !aDstPic )
        return false;

    const SalDisplay* pSalDisp = GetDisplay();
    const SalVisual& rSalVis = pSalDisp->GetVisual( m_nScreen );
    Display* pXDisplay = pSalDisp->GetDisplay();

    // create source Picture
    int nDepth = m_pVDev ? m_pVDev->GetDepth() : rSalVis.GetDepth();
    const X11SalBitmap& rSrcX11Bmp = static_cast<const X11SalBitmap&>( rSrcBitmap );
    ImplSalDDB* pSrcDDB = rSrcX11Bmp.ImplGetDDB( hDrawable_, m_nScreen, nDepth, rTR );
    if( !pSrcDDB )
        return false;

    //#i75249# workaround for ImplGetDDB() giving us back a different depth than
    // we requested. E.g. mask pixmaps are always compatible with the drawable
    // TODO: find an appropriate picture format for these cases
    //       then remove the workaround below and the one for #i75531#
    if( nDepth != pSrcDDB->ImplGetDepth() )
        return false;

    Pixmap aSrcPM = pSrcDDB->ImplGetPixmap();
    if( !aSrcPM )
        return false;

    // create source picture
    // TODO: use scoped picture
    Visual* pSrcXVisual = rSalVis.GetVisual();
    XRenderPictFormat* pSrcVisFmt = rPeer.FindVisualFormat( pSrcXVisual );
    if( !pSrcVisFmt )
        return false;
    Picture aSrcPic = rPeer.CreatePicture( aSrcPM, pSrcVisFmt, 0, NULL );
    if( !aSrcPic )
        return false;

    // create alpha Picture

    // TODO: use SalX11Bitmap functionality and caching for the Alpha Pixmap
    // problem is that they don't provide an 8bit Pixmap on a non-8bit display
    BitmapBuffer* pAlphaBuffer = const_cast<SalBitmap&>(rAlphaBmp).AcquireBuffer( sal_True );

    // an XImage needs its data top_down
    // TODO: avoid wrongly oriented images in upper layers!
    const int nImageSize = pAlphaBuffer->mnHeight * pAlphaBuffer->mnScanlineSize;
    const char* pSrcBits = (char*)pAlphaBuffer->mpBits;
    char* pAlphaBits = new char[ nImageSize ];
    if( BMP_SCANLINE_ADJUSTMENT( pAlphaBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
        memcpy( pAlphaBits, pSrcBits, nImageSize );
    else
    {
        char* pDstBits = pAlphaBits + nImageSize;
        const int nLineSize = pAlphaBuffer->mnScanlineSize;
        for(; (pDstBits -= nLineSize) >= pAlphaBits; pSrcBits += nLineSize )
            memcpy( pDstBits, pSrcBits, nLineSize );
    }

    // the alpha values need to be inverted for XRender
    // TODO: make upper layers use standard alpha
    long* pLDst = (long*)pAlphaBits;
    for( int i = nImageSize/sizeof(long); --i >= 0; ++pLDst )
        *pLDst = ~*pLDst;

    char* pCDst = (char*)pLDst;
    for( int i = nImageSize & (sizeof(long)-1); --i >= 0; ++pCDst )
        *pCDst = ~*pCDst;

    const XRenderPictFormat* pAlphaFormat = rPeer.GetStandardFormatA8();
    XImage* pAlphaImg = XCreateImage( pXDisplay, pSrcXVisual, 8, ZPixmap, 0,
        pAlphaBits, pAlphaBuffer->mnWidth, pAlphaBuffer->mnHeight,
        pAlphaFormat->depth, pAlphaBuffer->mnScanlineSize );

    Pixmap aAlphaPM = XCreatePixmap( pXDisplay, hDrawable_,
        rTR.mnDestWidth, rTR.mnDestHeight, 8 );

    XGCValues aAlphaGCV;
    aAlphaGCV.function = GXcopy;
    GC aAlphaGC = XCreateGC( pXDisplay, aAlphaPM, GCFunction, &aAlphaGCV );
    XPutImage( pXDisplay, aAlphaPM, aAlphaGC, pAlphaImg,
        rTR.mnSrcX, rTR.mnSrcY, 0, 0, rTR.mnDestWidth, rTR.mnDestHeight );
    XFreeGC( pXDisplay, aAlphaGC );
    XFree( pAlphaImg );
    if( pAlphaBits != (char*)pAlphaBuffer->mpBits )
        delete[] pAlphaBits;

    const_cast<SalBitmap&>(rAlphaBmp).ReleaseBuffer( pAlphaBuffer, sal_True );

    XRenderPictureAttributes aAttr;
    aAttr.repeat = true;
    Picture aAlphaPic = rPeer.CreatePicture( aAlphaPM, pAlphaFormat, CPRepeat, &aAttr );
    if( !aAlphaPic )
        return false;

    // set clipping
    if( mpClipRegion && !XEmptyRegion( mpClipRegion ) )
        rPeer.SetPictureClipRegion( aDstPic, mpClipRegion );
    
    // paint source * mask over destination picture
    rPeer.CompositePicture( PictOpOver, aSrcPic, aAlphaPic, aDstPic,
        rTR.mnSrcX, rTR.mnSrcY, 0, 0,
        rTR.mnDestX, rTR.mnDestY, rTR.mnDestWidth, rTR.mnDestHeight );

    // TODO: used ScopedPic
    rPeer.FreePicture( aAlphaPic );
    XFreePixmap(pXDisplay, aAlphaPM);
    rPeer.FreePicture( aSrcPic );
    return true;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool X11SalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    // here direct support for transformed bitmaps can be impemented
    (void)rNull; (void)rX; (void)rY; (void)rSourceBitmap; (void)pAlphaBitmap;
    return false;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
bool X11SalGraphics::drawAlphaRect( long nX, long nY, long nWidth, 
                                    long nHeight, sal_uInt8 nTransparency )
{
    if( ! m_pFrame && ! m_pVDev )
        return false;
    
    if( bPenGC_ || !bBrushGC_ || bXORMode_ )
        return false; // can only perform solid fills without XOR.
    
    if( m_pVDev && m_pVDev->GetDepth() < 8 )
        return false;

    XRenderPeer& rPeer = XRenderPeer::GetInstance();
    if( rPeer.GetVersion() < 0x02 ) // TODO: replace with better test
        return false;
    
    Picture aDstPic = GetXRenderPicture();
    if( !aDstPic )
        return false;

	const double fTransparency = (100 - nTransparency) * (1.0/100);
	const XRenderColor aRenderColor = GetXRenderColor( nBrushColor_ , fTransparency);
    
    rPeer.FillRectangle( PictOpOver,
                         aDstPic,
                         &aRenderColor,
                         nX, nY,
                         nWidth, nHeight );

    return true;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::drawBitmap( const SalTwoRect&,
                                 const SalBitmap&,
                                 SalColor )
{
    DBG_ERROR( "::DrawBitmap with transparent color not supported" );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::drawMask( const SalTwoRect& rPosAry,
                               const SalBitmap &rSalBitmap,
                               SalColor nMaskColor )
{
    const SalDisplay*   pSalDisp = GetDisplay();
    Display*	        pXDisp = pSalDisp->GetDisplay();
    Drawable            aDrawable( GetDrawable() );
    Pixmap              aStipple( XCreatePixmap( pXDisp, aDrawable,
                                                 rPosAry.mnDestWidth,
                                                 rPosAry.mnDestHeight, 1 ) );
    
    if( aStipple )
    {
        SalTwoRect	aTwoRect( rPosAry ); aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
        GC			aTmpGC;
        XGCValues	aValues;
        
        // create a stipple bitmap first (set bits are changed to unset bits and vice versa)
        aValues.function = GXcopyInverted;
        aValues.foreground = 1, aValues.background = 0;
        aTmpGC = XCreateGC( pXDisp, aStipple, GCFunction | GCForeground | GCBackground, &aValues );
        static_cast<const X11SalBitmap&>(rSalBitmap).ImplDraw( aStipple, m_nScreen, 1, aTwoRect, aTmpGC );
        
        XFreeGC( pXDisp, aTmpGC );
        
        // Set stipple and draw rectangle
        GC	aStippleGC( GetStippleGC() );
        int	nX = rPosAry.mnDestX, nY = rPosAry.mnDestY;
        
        XSetStipple( pXDisp, aStippleGC, aStipple );
        XSetTSOrigin( pXDisp, aStippleGC, nX, nY );
        XSetForeground( pXDisp, aStippleGC, GetPixel( nMaskColor ) );
        XFillRectangle( pXDisp, aDrawable, aStippleGC,
                        nX, nY,
                        rPosAry.mnDestWidth, rPosAry.mnDestHeight );
        XFreePixmap( pXDisp, aStipple );
        XFlush( pXDisp );
    }
    else
        drawBitmap( rPosAry, rSalBitmap );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalBitmap *X11SalGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
    if( bPrinter_ && !bVirDev_ )
        return NULL;
    
    bool bFakeWindowBG = false;

    // normalize
    if( nDX < 0 )
    {
        nX += nDX;
        nDX = -nDX;
    }
    if ( nDY < 0 )
    {
        nY += nDY;
        nDY = -nDY;
    }

    if( bWindow_ && !bVirDev_ )
    {        
        XWindowAttributes aAttrib;
        
        XGetWindowAttributes( GetXDisplay(), GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
            bFakeWindowBG = true;
        else
        {
            long nOrgDX = nDX, nOrgDY = nDY;

            // clip to window size
            if ( nX < 0 )
            {
                nDX += nX;
                nX   = 0;
            }
            if ( nY < 0 )
            {
                nDY += nY;
                nY   = 0;
            }
            if( nX + nDX > aAttrib.width )
                nDX = aAttrib.width  - nX;
            if( nY + nDY > aAttrib.height )
                nDY = aAttrib.height - nY;
        
            // inside ?
            if( nDX <= 0 || nDY <= 0 )
            {
                bFakeWindowBG = true;
                nDX = nOrgDX;
                nDY = nOrgDY;
            }
        }
    }
    
    X11SalBitmap*	pSalBitmap = new X11SalBitmap;
    sal_uInt16			nBitCount = GetBitCount();
    
    if( &GetDisplay()->GetColormap( m_nScreen ) != &GetColormap() )
        nBitCount = 1;
    
    if( ! bFakeWindowBG )
        pSalBitmap->ImplCreateFromDrawable( GetDrawable(), m_nScreen, nBitCount, nX, nY, nDX, nDY );
    else
        pSalBitmap->Create( Size( nDX, nDY ), (nBitCount > 8) ? 24 : nBitCount, BitmapPalette( nBitCount > 8 ? nBitCount : 0 ) );

    return pSalBitmap;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColor X11SalGraphics::getPixel( long nX, long nY )
{
    if( bWindow_ && !bVirDev_ )
    {
        XWindowAttributes aAttrib;
        
        XGetWindowAttributes( GetXDisplay(), GetDrawable(), &aAttrib );
        if( aAttrib.map_state != IsViewable )
        {
            stderr0( "X11SalGraphics::GetPixel drawable not viewable\n" );
            return 0;
        }
    }
    
    XImage *pXImage = XGetImage( GetXDisplay(),
                                     GetDrawable(),
                                 nX, nY,
                                 1,  1,
                                 AllPlanes,
                                 ZPixmap );
    if( !pXImage )
    {
        stderr0( "X11SalGraphics::GetPixel !XGetImage()\n" );
        return 0;
    }
    
    XColor aXColor;
    
    aXColor.pixel = XGetPixel( pXImage, 0, 0 );
    XDestroyImage( pXImage );
    
    return GetColormap().GetColor( aXColor.pixel );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::invert( long		nX,
								long		nY,
								long		nDX,
								long		nDY,
								SalInvert	nFlags )
{
    GC pGC;
    if( SAL_INVERT_50 & nFlags )
    {
        pGC = GetInvert50GC();
        XFillRectangle( GetXDisplay(), GetDrawable(), pGC, nX, nY, nDX, nDY );
    }
    else
    {
        if ( SAL_INVERT_TRACKFRAME & nFlags )
        {
            pGC = GetTrackingGC();
            XDrawRectangle( GetXDisplay(), GetDrawable(),  pGC, nX, nY, nDX, nDY );
        }
        else
        {
            pGC = GetInvertGC();
            XFillRectangle( GetXDisplay(), GetDrawable(),  pGC, nX, nY, nDX, nDY );
        }
    }
}

bool X11SalGraphics::supportsOperation( OutDevSupportType eType ) const
{
    bool bRet = false;
    switch( eType )
    {
    case OutDevSupport_TransparentRect:
    case OutDevSupport_B2DDraw:
        {
            XRenderPeer& rPeer = XRenderPeer::GetInstance();
            if( rPeer.GetVersion() >= 0x02 )
            {        
                const SalDisplay* pSalDisp = GetDisplay();
                const SalVisual& rSalVis = pSalDisp->GetVisual( m_nScreen );
            
                Visual* pDstXVisual = rSalVis.GetVisual();
                XRenderPictFormat* pDstVisFmt = rPeer.FindVisualFormat( pDstXVisual );
                if( pDstVisFmt )
                    bRet = true;
            }
        }
        break;
    default: break;
    }
    return bRet;
}

