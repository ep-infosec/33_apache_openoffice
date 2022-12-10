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

#include <boost/bind.hpp>

#include "basebmp/scanlineformats.hxx"
#include "basebmp/color.hxx"

#include "basegfx/range/b2drectangle.hxx"
#include "basegfx/range/b2irange.hxx"
#include "basegfx/vector/b2ivector.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"

#include "vcl/svapp.hxx"

#include "aqua/salgdi.h"
#include "aqua/salframe.h"
#include "aqua/saldata.hxx"

// ----------------------------------------------------------------------

void AquaSalGraphics::SetWindowGraphics( AquaSalFrame* pFrame )
{
    mpFrame     = pFrame;

    mbWindow    = true;
    mbPrinter   = false;
    mbVirDev    = false;
}

void AquaSalGraphics::SetPrinterGraphics( CGContextRef xContext, long nDPIX, long nDPIY, double fScale )
{
    mbWindow    = false;
    mbPrinter   = true;
    mbVirDev    = false;
    
    mrContext   = xContext;
    mfFakeDPIScale = fScale;
    mnRealDPIX  = nDPIX;
    mnRealDPIY  = nDPIY;

    // a previously set clip path is now invalid
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }

    if( mrContext )
    {
        CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
        CGContextSaveGState( mrContext );
        SetState();
    }
}

void AquaSalGraphics::SetVirDevGraphics( CGLayerRef xLayer, CGContextRef xContext,
	int nBitmapDepth )
{
    mbWindow    = false;
    mbPrinter   = false;
    mbVirDev    = true;

	// set graphics properties
	mxLayer	= xLayer;
	mrContext = xContext;
	mnBitmapDepth = nBitmapDepth;

	// return early if the virdev is being destroyed
    if( !xContext )
    	return;

	// get new graphics properties
    if( !mxLayer )
    {
		mnWidth = CGBitmapContextGetWidth( mrContext );
		mnHeight = CGBitmapContextGetHeight( mrContext );
    }
    else
    {
		const CGSize aSize = CGLayerGetSize( mxLayer );
		mnWidth = static_cast<int>(aSize.width);
		mnHeight = static_cast<int>(aSize.height);
    }

	// prepare graphics for drawing
	const CGColorSpaceRef aCGColorSpace = GetSalData()->mxRGBSpace;
	CGContextSetFillColorSpace( mrContext, aCGColorSpace );
	CGContextSetStrokeColorSpace( mrContext, aCGColorSpace );

	// re-enable XorEmulation for the new context
	if( mpXorEmulation )
	{
		mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
		if( mpXorEmulation->IsEnabled() )
			mrContext = mpXorEmulation->GetMaskContext();
	}

	// initialize stack of CGContext states
	CGContextSaveGState( mrContext );
	SetState();
}

// ----------------------------------------------------------------------

void AquaSalGraphics::InvalidateContext()
{
    UnsetState();
    mrContext = 0;
}

// ----------------------------------------------------------------------

void AquaSalGraphics::UnsetState()
{
    if( mrContext )
    {
        CGContextRestoreGState( mrContext );
        mrContext = 0;
    }
    if( mxClipPath )
    {
        CGPathRelease( mxClipPath );
        mxClipPath = NULL;
    }
}

void AquaSalGraphics::SetState()
{
    CGContextRestoreGState( mrContext );
    CGContextSaveGState( mrContext );

	// setup clipping
	if( mxClipPath )
	{
		CGContextBeginPath( mrContext );            // discard any existing path
		CGContextAddPath( mrContext, mxClipPath );  // set the current path to the clipping path
		CGContextClip( mrContext );                 // use it for clipping
	}

	// set RGB colorspace and line and fill colors
	CGContextSetFillColor( mrContext, maFillColor.AsArray() );
	CGContextSetStrokeColor( mrContext, maLineColor.AsArray() );
    CGContextSetShouldAntialias( mrContext, false );
    if( mnXorMode == 2 )
        CGContextSetBlendMode( mrContext, kCGBlendModeDifference );
}

// ----------------------------------------------------------------------

bool AquaSalGraphics::CheckContext()
{
    if( mbWindow && mpFrame && mpFrame->getNSWindow() )
    {
        const unsigned int nWidth = mpFrame->maGeometry.nWidth;
        const unsigned int nHeight = mpFrame->maGeometry.nHeight;

        CGContextRef rReleaseContext = 0;
        CGLayerRef   rReleaseLayer = NULL;

		// check if a new drawing context is needed (e.g. after a resize)
		if( (unsigned(mnWidth) != nWidth) || (unsigned(mnHeight) != nHeight) )
		{
			mnWidth = nWidth;
			mnHeight = nHeight;
			// prepare to release the corresponding resources
			rReleaseContext = mrContext;
			rReleaseLayer   = mxLayer;
			mrContext = NULL;
			mxLayer = NULL;
		}

        if( !mrContext )
        {
	        const CGSize aLayerSize = CGSizeMake( nWidth, nHeight);
	        NSGraphicsContext* pNSGContext = [NSGraphicsContext graphicsContextWithWindow: mpFrame->getNSWindow()];
	        CGContextRef xCGContext = reinterpret_cast<CGContextRef>([pNSGContext graphicsPort]);
    	    mxLayer = CGLayerCreateWithContext( xCGContext, aLayerSize, NULL );
			if( mxLayer )
				mrContext = CGLayerGetContext( mxLayer );

            if( mrContext )
            {
                // copy original layer to resized layer
                if( rReleaseLayer )
                	CGContextDrawLayerAtPoint( mrContext, CGPointZero, rReleaseLayer );

                CGContextTranslateCTM( mrContext, 0, nHeight );
                CGContextScaleCTM( mrContext, 1.0, -1.0 );
                CGContextSetFillColorSpace( mrContext, GetSalData()->mxRGBSpace );
                CGContextSetStrokeColorSpace( mrContext, GetSalData()->mxRGBSpace );
                CGContextSaveGState( mrContext );
                SetState();

				// re-enable XOR emulation for the new context
				if( mpXorEmulation )
					mpXorEmulation->SetTarget( mnWidth, mnHeight, mnBitmapDepth, mrContext, mxLayer );
            }
        }

		if( rReleaseLayer )
			CGLayerRelease( rReleaseLayer );
        else if( rReleaseContext )
            CGContextRelease( rReleaseContext );
    }

	DBG_ASSERT( mrContext || mbPrinter, "<<<WARNING>>> AquaSalGraphics::CheckContext() FAILED!!!!\n" );
	return (mrContext != NULL);
}


void AquaSalGraphics::RefreshRect(float lX, float lY, float lWidth, float lHeight)
{
    if( ! mbWindow ) // view only on Window graphics
        return;

    if( mpFrame )
    {
        // update a little more around the designated rectangle
        // this helps with antialiased rendering
        // Rounding down x and width can accumulate a rounding error of up to 2
        // The decrementing of x, the rounding error and the antialiasing border
        // require that the width and the height need to be increased by four
        const Rectangle aVclRect(Point(static_cast<long int>(lX-1),
					static_cast<long int>(lY-1) ),
				 Size( 	static_cast<long int>(lWidth+4), 
					static_cast<long int>(lHeight+4) ) );
        mpFrame->maInvalidRect.Union( aVclRect );
    }
}

CGPoint* AquaSalGraphics::makeCGptArray(sal_uLong nPoints, const SalPoint*  pPtAry)
{
    CGPoint *CGpoints = new CGPoint[ nPoints];
    if ( CGpoints ) 
      {
        for(sal_uLong i=0;i<nPoints;i++)
          {
            CGpoints[i].x = (float)(pPtAry[i].mnX);  
            CGpoints[i].y = (float)(pPtAry[i].mnY);  
          }
      }    
    return CGpoints;
}

// -----------------------------------------------------------------------

void AquaSalGraphics::UpdateWindow( NSRect& )
{
	if( !mpFrame )
		return;
    NSGraphicsContext* pContext = [NSGraphicsContext currentContext];
	if( (mxLayer != NULL) && (pContext != NULL) )
	{
        CGContextRef rCGContext = reinterpret_cast<CGContextRef>([pContext graphicsPort]);

        CGMutablePathRef rClip = mpFrame->getClipPath();
        if( rClip )
        {
            CGContextSaveGState( rCGContext );
            CGContextBeginPath( rCGContext );
            CGContextAddPath( rCGContext, rClip );
            CGContextClip( rCGContext );
        }

		ApplyXorContext();
		CGContextDrawLayerAtPoint( rCGContext, CGPointZero, mxLayer );
        if( rClip ) // cleanup clipping
            CGContextRestoreGState( rCGContext );
	}
    else
        DBG_ASSERT( mpFrame->mbInitShow, "UpdateWindow called on uneligible graphics" );
}

// -----------------------------------------------------------------------

