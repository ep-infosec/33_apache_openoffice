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

#define ENABLE_BYTESTRING_STREAM_OPERATORS
#include <vcl/animate.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <rtl/crc.h>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <impanmvw.hxx>
#include <vcl/dibtools.hxx>

DBG_NAME( Animation )

// -----------
// - Defines -
// -----------

#define MIN_TIMEOUT 2L
#define INC_TIMEOUT 0L

// -----------
// - statics -
// -----------

sal_uLong Animation::mnAnimCount = 0UL;

// -------------------
// - AnimationBitmap -
// -------------------

sal_uLong AnimationBitmap::GetChecksum() const
{
	sal_uInt32	nCrc = aBmpEx.GetChecksum();
	SVBT32		aBT32;

	UInt32ToSVBT32( aPosPix.X(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( aPosPix.Y(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( aSizePix.Width(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( aSizePix.Height(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( (long) nWait, aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( (long) eDisposal, aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( (long) bUserInput, aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	return nCrc;
}

// -------------
// - Animation -
// -------------

Animation::Animation() :
	mnLoopCount 		( 0 ),
	mnLoops 			( 0 ),
	mnPos				( 0 ),
	meCycleMode 		( CYCLE_NORMAL ),
	mbIsInAnimation 	( sal_False ),
	mbLoopTerminated	( sal_False ),
	mbIsWaiting 		( sal_False )
{
	DBG_CTOR( Animation, NULL );
	maTimer.SetTimeoutHdl( LINK( this, Animation, ImplTimeoutHdl ) );
	mpViewList = new List;
}

// -----------------------------------------------------------------------

Animation::Animation( const Animation& rAnimation ) :
	maBitmapEx			( rAnimation.maBitmapEx ),
	maGlobalSize		( rAnimation.maGlobalSize ),
	mnLoopCount 		( rAnimation.mnLoopCount ),
	mnPos				( rAnimation.mnPos ),
	meCycleMode 		( rAnimation.meCycleMode ),
	mbIsInAnimation 	( sal_False ),
	mbLoopTerminated	( rAnimation.mbLoopTerminated ),
	mbIsWaiting 		( rAnimation.mbIsWaiting )
{
	DBG_CTOR( Animation, NULL );

	for( long i = 0, nCount = rAnimation.maList.Count(); i < nCount; i++ )
		maList.Insert( new AnimationBitmap( *(AnimationBitmap*) rAnimation.maList.GetObject( i ) ), LIST_APPEND );

	maTimer.SetTimeoutHdl( LINK( this, Animation, ImplTimeoutHdl ) );
	mpViewList = new List;
	mnLoops = mbLoopTerminated ? 0 : mnLoopCount;
}

// -----------------------------------------------------------------------

Animation::~Animation()
{
	DBG_DTOR( Animation, NULL );

	if( mbIsInAnimation )
		Stop();

	for( void* pStepBmp = maList.First(); pStepBmp; pStepBmp = maList.Next() )
		delete (AnimationBitmap*) pStepBmp;

	for( void* pView = mpViewList->First(); pView; pView = mpViewList->Next() )
		delete (ImplAnimView*) pView;

	delete mpViewList;
}

// -----------------------------------------------------------------------

Animation& Animation::operator=( const Animation& rAnimation )
{
	Clear();

	for( long i = 0, nCount = rAnimation.maList.Count(); i < nCount; i++ )
		maList.Insert( new AnimationBitmap( *(AnimationBitmap*) rAnimation.maList.GetObject( i ) ), LIST_APPEND );

	maGlobalSize = rAnimation.maGlobalSize;
	maBitmapEx = rAnimation.maBitmapEx;
	meCycleMode = rAnimation.meCycleMode;
	mnLoopCount = rAnimation.mnLoopCount;
	mnPos = rAnimation.mnPos;
	mbLoopTerminated = rAnimation.mbLoopTerminated;
	mbIsWaiting = rAnimation.mbIsWaiting;
	mnLoops = mbLoopTerminated ? 0 : mnLoopCount;

	return *this;
}

// -----------------------------------------------------------------------

sal_Bool Animation::operator==( const Animation& rAnimation ) const
{
	const sal_uLong nCount = maList.Count();
	sal_Bool		bRet = sal_False;

	if( rAnimation.maList.Count() == nCount &&
		rAnimation.maBitmapEx == maBitmapEx &&
		rAnimation.maGlobalSize == maGlobalSize &&
		rAnimation.meCycleMode == meCycleMode )
	{
		bRet = sal_True;

		for( sal_uLong n = 0; n < nCount; n++ )
		{
			if( ( *(AnimationBitmap*) maList.GetObject( n ) ) !=
				( *(AnimationBitmap*) rAnimation.maList.GetObject( n ) ) )
			{
				bRet = sal_False;
				break;
			}
		}
	}

	return bRet;
}

// ------------------------------------------------------------------

sal_Bool Animation::IsEqual( const Animation& rAnimation ) const
{
	const sal_uLong nCount = maList.Count();
	sal_Bool		bRet = sal_False;

	if( rAnimation.maList.Count() == nCount &&
		rAnimation.maBitmapEx.IsEqual( maBitmapEx ) &&
		rAnimation.maGlobalSize == maGlobalSize &&
		rAnimation.meCycleMode == meCycleMode )
	{
		for( sal_uLong n = 0; ( n < nCount ) && !bRet; n++ )
			if( ( (AnimationBitmap*) maList.GetObject( n ) )->IsEqual( *(AnimationBitmap*) rAnimation.maList.GetObject( n ) ) )
				bRet = sal_True;
	}

	return bRet;
}

// ------------------------------------------------------------------

sal_Bool Animation::IsEmpty() const
{
	return( maBitmapEx.IsEmpty() && !maList.Count() );
}

// ------------------------------------------------------------------

void Animation::SetEmpty()
{
	maTimer.Stop();
	mbIsInAnimation = sal_False;
	maGlobalSize = Size();
	maBitmapEx.SetEmpty();

	for( void* pStepBmp = maList.First(); pStepBmp; pStepBmp = maList.Next() )
		delete (AnimationBitmap*) pStepBmp;
	maList.Clear();

	for( void* pView = mpViewList->First(); pView; pView = mpViewList->Next() )
		delete (ImplAnimView*) pView;
	mpViewList->Clear();
}

// -----------------------------------------------------------------------

void Animation::Clear()
{
	SetEmpty();
}

// -----------------------------------------------------------------------

sal_Bool Animation::IsTransparent() const
{
	Point		aPoint;
	Rectangle	aRect( aPoint, maGlobalSize );
	sal_Bool		bRet = sal_False;

	// Falls irgendein 'kleines' Bildchen durch den Hintergrund
	// ersetzt werden soll, muessen wir 'transparent' sein, um
	// richtig dargestellt zu werden, da die Appl. aus Optimierungsgruenden
	// kein Invalidate auf nicht-transp. Grafiken ausfuehren
	for( long i = 0, nCount = maList.Count(); i < nCount; i++ )
	{
		const AnimationBitmap* pAnimBmp = (AnimationBitmap*) maList.GetObject( i );

		if( DISPOSE_BACK == pAnimBmp->eDisposal && Rectangle( pAnimBmp->aPosPix, pAnimBmp->aSizePix ) != aRect )
		{
			bRet = sal_True;
			break;
		}
	}

	if( !bRet )
		bRet = maBitmapEx.IsTransparent();

	return bRet;
}

// -----------------------------------------------------------------------

sal_uLong Animation::GetSizeBytes() const
{
	sal_uLong nSizeBytes = GetBitmapEx().GetSizeBytes();

	for( long i = 0, nCount = maList.Count(); i < nCount; i++ )
	{
		const AnimationBitmap* pAnimBmp = (AnimationBitmap*) maList.GetObject( i );
		nSizeBytes += pAnimBmp->aBmpEx.GetSizeBytes();
	}

	return nSizeBytes;
}

// -----------------------------------------------------------------------

sal_uLong Animation::GetChecksum() const
{
	SVBT32		aBT32;
	sal_uInt32	nCrc = GetBitmapEx().GetChecksum();

	UInt32ToSVBT32( maList.Count(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( maGlobalSize.Width(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( maGlobalSize.Height(), aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	UInt32ToSVBT32( (long) meCycleMode, aBT32 );
	nCrc = rtl_crc32( nCrc, aBT32, 4 );

	for( long i = 0, nCount = maList.Count(); i < nCount; i++ )
	{
		UInt32ToSVBT32( ( (AnimationBitmap*) maList.GetObject( i ) )->GetChecksum(), aBT32 );
		nCrc = rtl_crc32( nCrc, aBT32, 4 );
	}

	return nCrc;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Start( OutputDevice* pOut, const Point& rDestPt, long nExtraData,
					   OutputDevice* pFirstFrameOutDev )
{
	return Start( pOut, rDestPt, pOut->PixelToLogic( maGlobalSize ), nExtraData, pFirstFrameOutDev );
}

// -----------------------------------------------------------------------

sal_Bool Animation::Start( OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz, long nExtraData,
					   OutputDevice* pFirstFrameOutDev )
{
	sal_Bool bRet = sal_False;

	if( maList.Count() )
	{
		if( ( pOut->GetOutDevType() == OUTDEV_WINDOW ) && !mbLoopTerminated &&
			( ANIMATION_TIMEOUT_ON_CLICK != ( (AnimationBitmap*) maList.GetObject( mnPos ) )->nWait ) )
		{
			ImplAnimView*	pView;
			ImplAnimView*	pMatch = NULL;

			for( pView = (ImplAnimView*) mpViewList->First(); pView; pView = (ImplAnimView*) mpViewList->Next() )
			{
				if( pView->ImplMatches( pOut, nExtraData ) )
				{
					if( pView->ImplGetOutPos() == rDestPt &&
						pView->ImplGetOutSizePix() == pOut->LogicToPixel( rDestSz ) )
					{
						pView->ImplRepaint();
						pMatch = pView;
					}
					else
					{
						delete (ImplAnimView*) mpViewList->Remove( pView );
						pView = NULL;
					}

					break;
				}
			}

			if( !mpViewList->Count() )
			{
				maTimer.Stop();
				mbIsInAnimation = sal_False;
				mnPos = 0UL;
			}

			if( !pMatch )
				mpViewList->Insert( new ImplAnimView( this, pOut, rDestPt, rDestSz, nExtraData, pFirstFrameOutDev ), LIST_APPEND );

			if( !mbIsInAnimation )
			{
				ImplRestartTimer( ( (AnimationBitmap*) maList.GetObject( mnPos ) )->nWait );
				mbIsInAnimation = sal_True;
			}
		}
		else
			Draw( pOut, rDestPt, rDestSz );

		bRet = sal_True;
	}

	return bRet;
}

// -----------------------------------------------------------------------

void Animation::Stop( OutputDevice* pOut, long nExtraData )
{
	ImplAnimView* pView = (ImplAnimView*) mpViewList->First();

	while( pView )
	{
		if( pView->ImplMatches( pOut, nExtraData ) )
		{
			delete (ImplAnimView*) mpViewList->Remove( pView );
			pView = (ImplAnimView*) mpViewList->GetCurObject();
		}
		else
			pView = (ImplAnimView*) mpViewList->Next();
	}

	if( !mpViewList->Count() )
	{
		maTimer.Stop();
		mbIsInAnimation = sal_False;
	}
}

// -----------------------------------------------------------------------

void Animation::Draw( OutputDevice* pOut, const Point& rDestPt ) const
{
	Draw( pOut, rDestPt, pOut->PixelToLogic( maGlobalSize ) );
}

// -----------------------------------------------------------------------

void Animation::Draw( OutputDevice* pOut, const Point& rDestPt, const Size& rDestSz ) const
{
	const sal_uLong nCount = maList.Count();

	if( nCount )
	{
		AnimationBitmap* pObj = (AnimationBitmap*) maList.GetObject( Min( mnPos, (long) nCount - 1L ) );

		if( pOut->GetConnectMetaFile() || ( pOut->GetOutDevType() == OUTDEV_PRINTER ) )
			( (AnimationBitmap*) maList.GetObject( 0 ) )->aBmpEx.Draw( pOut, rDestPt, rDestSz );
		else if( ANIMATION_TIMEOUT_ON_CLICK == pObj->nWait )
			pObj->aBmpEx.Draw( pOut, rDestPt, rDestSz );
		else
		{
			const sal_uLong nOldPos = mnPos;
			( (Animation*) this )->mnPos = mbLoopTerminated ? ( nCount - 1UL ) : mnPos;
			delete new ImplAnimView( (Animation*) this, pOut, rDestPt, rDestSz, 0 );
			( (Animation*) this )->mnPos = nOldPos;
		}
	}
}

// -----------------------------------------------------------------------

void Animation::ImplRestartTimer( sal_uLong nTimeout )
{
	maTimer.SetTimeout( Max( nTimeout, (sal_uLong)(MIN_TIMEOUT + ( mnAnimCount - 1 ) * INC_TIMEOUT) ) * 10L );
	maTimer.Start();
}

// -----------------------------------------------------------------------

IMPL_LINK( Animation, ImplTimeoutHdl, Timer*, EMPTYARG )
{
	const sal_uLong nAnimCount = maList.Count();

	if( nAnimCount )
	{
		ImplAnimView*	pView;
		sal_Bool			bGlobalPause = sal_True;

		if( maNotifyLink.IsSet() )
		{
			AInfo* pAInfo;

			// create AInfo-List
			for( pView = (ImplAnimView*) mpViewList->First(); pView; pView = (ImplAnimView*) mpViewList->Next() )
				maAInfoList.Insert( pView->ImplCreateAInfo() );

			maNotifyLink.Call( this );

			// set view state from AInfo structure
			for( pAInfo = (AInfo*) maAInfoList.First(); pAInfo; pAInfo = (AInfo*) maAInfoList.Next() )
			{
				if( !pAInfo->pViewData )
				{
					pView = new ImplAnimView( this, pAInfo->pOutDev,
											  pAInfo->aStartOrg, pAInfo->aStartSize, pAInfo->nExtraData );

					mpViewList->Insert( pView, LIST_APPEND );
				}
				else
					pView = (ImplAnimView*) pAInfo->pViewData;

				pView->ImplPause( pAInfo->bPause );
				pView->ImplSetMarked( sal_True );
			}

			// delete AInfo structures
			for( pAInfo = (AInfo*) maAInfoList.First(); pAInfo; pAInfo = (AInfo*) maAInfoList.Next() )
				delete (AInfo*) pAInfo;
			maAInfoList.Clear();

			// delete all unmarked views and reset marked state
			pView = (ImplAnimView*) mpViewList->First();
			while( pView )
			{
				if( !pView->ImplIsMarked() )
				{
					delete (ImplAnimView*) mpViewList->Remove( pView );
					pView = (ImplAnimView*) mpViewList->GetCurObject();
				}
				else
				{
					if( !pView->ImplIsPause() )
						bGlobalPause = sal_False;

					pView->ImplSetMarked( sal_False );
					pView = (ImplAnimView*) mpViewList->Next();
				}
			}
		}
		else
			bGlobalPause = sal_False; 

		if( !mpViewList->Count() )
			Stop();
		else if( bGlobalPause )
			ImplRestartTimer( 10 );
		else
		{
			AnimationBitmap* pStepBmp = (AnimationBitmap*) maList.GetObject( ++mnPos );

			if( !pStepBmp )
			{
				if( mnLoops == 1 )
				{
					Stop();
					mbLoopTerminated = sal_True;
					mnPos = nAnimCount - 1UL;
					maBitmapEx = ( (AnimationBitmap*) maList.GetObject( mnPos ) )->aBmpEx;
					return 0L;
				}
				else
				{
					if( mnLoops )
						mnLoops--;

					mnPos = 0;
					pStepBmp = (AnimationBitmap*) maList.GetObject( mnPos );
				}
			}

			// Paint all views; after painting check, if view is
			// marked; in this case remove view, because area of output
			// lies out of display area of window; mark state is
			// set from view itself
			pView = (ImplAnimView*) mpViewList->First();
			while( pView )
			{
				pView->ImplDraw( mnPos );

				if( pView->ImplIsMarked() )
				{
					delete (ImplAnimView*) mpViewList->Remove( pView );
					pView = (ImplAnimView*) mpViewList->GetCurObject();
				}
				else
					pView = (ImplAnimView*) mpViewList->Next();
			}

			// stop or restart timer
			if( !mpViewList->Count() )
				Stop();
			else
				ImplRestartTimer( pStepBmp->nWait );
		}
	}
	else
		Stop();

	return 0L;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Insert( const AnimationBitmap& rStepBmp )
{
	sal_Bool bRet = sal_False;

	if( !IsInAnimation() )
	{
		Point		aPoint;
		Rectangle	aGlobalRect( aPoint, maGlobalSize );

		maGlobalSize = aGlobalRect.Union( Rectangle( rStepBmp.aPosPix, rStepBmp.aSizePix ) ).GetSize();
		maList.Insert( new AnimationBitmap( rStepBmp ), LIST_APPEND );

		// zunaechst nehmen wir die erste BitmapEx als Ersatz-BitmapEx
		if( maList.Count() == 1 )
			maBitmapEx = rStepBmp.aBmpEx;

		bRet = sal_True;
	}

	return bRet;
}

// -----------------------------------------------------------------------

const AnimationBitmap& Animation::Get( sal_uInt16 nAnimation ) const
{
	DBG_ASSERT( ( nAnimation < maList.Count() ), "No object at this position" );
	return *(AnimationBitmap*) maList.GetObject( nAnimation );
}

// -----------------------------------------------------------------------

void Animation::Replace( const AnimationBitmap& rNewAnimationBitmap, sal_uInt16 nAnimation )
{
	DBG_ASSERT( ( nAnimation < maList.Count() ), "No object at this position" );

	delete (AnimationBitmap*) maList.Replace( new AnimationBitmap( rNewAnimationBitmap ), nAnimation );

	// Falls wir an erster Stelle einfuegen,
	// muessen wir natuerlich auch,
	// auch die Ersatzdarstellungs-BitmapEx
	// aktualisieren;
	if ( ( !nAnimation && ( !mbLoopTerminated || ( maList.Count() == 1 ) ) ) ||
		 ( ( nAnimation == maList.Count() - 1 ) && mbLoopTerminated ) )
	{
		maBitmapEx = rNewAnimationBitmap.aBmpEx;
	}
}

// -----------------------------------------------------------------------

void Animation::SetLoopCount( const sal_uLong nLoopCount )
{
	mnLoopCount = nLoopCount;
	ResetLoopCount();
}

// -----------------------------------------------------------------------

void Animation::ResetLoopCount()
{
	mnLoops = mnLoopCount;
	mbLoopTerminated = sal_False;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Convert( BmpConversion eConversion )
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
			bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Convert( eConversion );

		maBitmapEx.Convert( eConversion );
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool Animation::ReduceColors( sal_uInt16 nNewColorCount, BmpReduce eReduce )
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
			bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.ReduceColors( nNewColorCount, eReduce );

		maBitmapEx.ReduceColors( nNewColorCount, eReduce );
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Invert()
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
			bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Invert();

		maBitmapEx.Invert();
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Mirror( sal_uLong nMirrorFlags )
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool	bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		if( nMirrorFlags )
		{
			for( AnimationBitmap* pStepBmp = (AnimationBitmap*) maList.First();
				 pStepBmp && bRet;
				 pStepBmp = (AnimationBitmap*) maList.Next() )
			{
				if( ( bRet = pStepBmp->aBmpEx.Mirror( nMirrorFlags ) ) == sal_True )
				{
					if( nMirrorFlags & BMP_MIRROR_HORZ )
						pStepBmp->aPosPix.X() = maGlobalSize.Width() - pStepBmp->aPosPix.X() - pStepBmp->aSizePix.Width();

					if( nMirrorFlags & BMP_MIRROR_VERT )
						pStepBmp->aPosPix.Y() = maGlobalSize.Height() - pStepBmp->aPosPix.Y() - pStepBmp->aSizePix.Height();
				}
			}

			maBitmapEx.Mirror( nMirrorFlags );
		}
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Dither( sal_uLong nDitherFlags )
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
			bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Dither( nDitherFlags );

		maBitmapEx.Dither( nDitherFlags );
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Adjust( short nLuminancePercent, short nContrastPercent,
			 short nChannelRPercent, short nChannelGPercent, short nChannelBPercent,
			 double fGamma, sal_Bool bInvert )
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
		{
			bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Adjust( nLuminancePercent, nContrastPercent,
																	nChannelRPercent, nChannelGPercent, nChannelBPercent,
																	fGamma, bInvert );
		}

		maBitmapEx.Adjust( nLuminancePercent, nContrastPercent,
						   nChannelRPercent, nChannelGPercent, nChannelBPercent,
						   fGamma, bInvert );
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

sal_Bool Animation::Filter( BmpFilter eFilter, const BmpFilterParam* pFilterParam, const Link* pProgress )
{
	DBG_ASSERT( !IsInAnimation(), "Animation modified while it is animated" );

	sal_Bool bRet;

	if( !IsInAnimation() && maList.Count() )
	{
		bRet = sal_True;

		for( void* pStepBmp = maList.First(); pStepBmp && bRet; pStepBmp = maList.Next() )
			bRet = ( ( AnimationBitmap*) pStepBmp )->aBmpEx.Filter( eFilter, pFilterParam, pProgress );

		maBitmapEx.Filter( eFilter, pFilterParam, pProgress );
	}
	else
		bRet = sal_False;

	return bRet;
}

// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Animation& rAnimation )
{
	const sal_uInt16 nCount = rAnimation.Count();

	if( nCount )
	{
		const ByteString	aDummyStr;
		const sal_uInt32		nDummy32 = 0UL;

		// Falls keine BitmapEx gesetzt wurde, schreiben wir
		// einfach die erste Bitmap der Animation
		if( !rAnimation.GetBitmapEx().GetBitmap() )
			WriteDIBBitmapEx(rAnimation.Get( 0 ).aBmpEx, rOStm);
		else
			WriteDIBBitmapEx(rAnimation.GetBitmapEx(), rOStm);

		// Kennung schreiben ( SDANIMA1 )
		rOStm << (sal_uInt32) 0x5344414e << (sal_uInt32) 0x494d4931;

		for( sal_uInt16 i = 0; i < nCount; i++ )
		{
			const AnimationBitmap&	rAnimBmp = rAnimation.Get( i );
			const sal_uInt16			nRest = nCount - i - 1;

			// AnimationBitmap schreiben
			WriteDIBBitmapEx(rAnimBmp.aBmpEx, rOStm);
			rOStm << rAnimBmp.aPosPix;
			rOStm << rAnimBmp.aSizePix;
			rOStm << rAnimation.maGlobalSize;
			rOStm << (sal_uInt16) ( ( ANIMATION_TIMEOUT_ON_CLICK == rAnimBmp.nWait ) ? 65535 : rAnimBmp.nWait );
			rOStm << (sal_uInt16) rAnimBmp.eDisposal;
			rOStm << (sal_uInt8) rAnimBmp.bUserInput;
			rOStm << (sal_uInt32) rAnimation.mnLoopCount;
			rOStm << nDummy32;	// unbenutzt
			rOStm << nDummy32;	// unbenutzt
			rOStm << nDummy32;	// unbenutzt
			rOStm << aDummyStr; // unbenutzt
			rOStm << nRest; 	// Anzahl der Strukturen, die noch _folgen_
		}
	}

	return rOStm;
}

// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Animation& rAnimation )
{
	Bitmap	aBmp;
	sal_uLong	nStmPos = rIStm.Tell();
	sal_uInt32	nAnimMagic1, nAnimMagic2;
	sal_uInt16	nOldFormat = rIStm.GetNumberFormatInt();
	sal_Bool	bReadAnimations = sal_False;

	rIStm.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
	nStmPos = rIStm.Tell();
	rIStm >> nAnimMagic1 >> nAnimMagic2;

	rAnimation.Clear();

	// Wenn die BitmapEx am Anfang schon gelesen
	// wurde ( von Graphic ), koennen wir direkt die Animationsbitmaps einlesen
	if( ( nAnimMagic1 == 0x5344414e ) && ( nAnimMagic2 == 0x494d4931 ) && !rIStm.GetError() )
		bReadAnimations = sal_True;
	// ansonsten versuchen wir erstmal die Bitmap(-Ex) zu lesen
	else
	{
		rIStm.Seek( nStmPos );
        ReadDIBBitmapEx(rAnimation.maBitmapEx, rIStm);
		nStmPos = rIStm.Tell();
		rIStm >> nAnimMagic1 >> nAnimMagic2;

		if( ( nAnimMagic1 == 0x5344414e ) && ( nAnimMagic2 == 0x494d4931 ) && !rIStm.GetError() )
			bReadAnimations = sal_True;
		else
			rIStm.Seek( nStmPos );
	}

	// ggf. Animationsbitmaps lesen
	if( bReadAnimations )
	{
		AnimationBitmap aAnimBmp;
		BitmapEx		aBmpEx;
		ByteString		aDummyStr;
		sal_uInt32			nTmp32;
		sal_uInt16			nTmp16;
		sal_uInt8			cTmp;

		do
		{
            ReadDIBBitmapEx(aAnimBmp.aBmpEx, rIStm);
			rIStm >> aAnimBmp.aPosPix;
			rIStm >> aAnimBmp.aSizePix;
			rIStm >> rAnimation.maGlobalSize;
			rIStm >> nTmp16; aAnimBmp.nWait = ( ( 65535 == nTmp16 ) ? ANIMATION_TIMEOUT_ON_CLICK : nTmp16 );
			rIStm >> nTmp16; aAnimBmp.eDisposal = ( Disposal) nTmp16;
			rIStm >> cTmp; aAnimBmp.bUserInput = (sal_Bool) cTmp;
			rIStm >> nTmp32; rAnimation.mnLoopCount = (sal_uInt16) nTmp32;
			rIStm >> nTmp32;	// unbenutzt
			rIStm >> nTmp32;	// unbenutzt
			rIStm >> nTmp32;	// unbenutzt
			rIStm >> aDummyStr; // unbenutzt
			rIStm >> nTmp16;	// Rest zu lesen

			rAnimation.Insert( aAnimBmp );
		}
		while( nTmp16 && !rIStm.GetError() );

		rAnimation.ResetLoopCount();
	}

	rIStm.SetNumberFormatInt( nOldFormat );

	return rIStm;
}
