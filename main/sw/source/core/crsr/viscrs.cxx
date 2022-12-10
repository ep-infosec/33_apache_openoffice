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
#include "precompiled_sw.hxx"


#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif

#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include <viewopt.hxx>
#include <frmtool.hxx>
#include <viscrs.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <swtable.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <docary.hxx>
#include <extinput.hxx>
#include <ndtxt.hxx>
#include <txtfld.hxx>
#include <scriptinfo.hxx>
#include <mdiexp.hxx>
#ifndef _COMCORE_HRC
#include <comcore.hrc>			// ResId fuer Abfrage wenn zu Search & Replaces
#endif

#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/overlay/overlayselection.hxx>
#include <overlayrangesoutline.hxx>

#include <boost/scoped_ptr.hpp>

extern void SwCalcPixStatics( OutputDevice *pOut );

//Damit beim ShowCrsr nicht immer wieder die gleiche Size teuer ermittelt
//werden muss, hier statische Member, die beim Wechsel des MapModes
// angepasst werden

long SwSelPaintRects::nPixPtX = 0;
long SwSelPaintRects::nPixPtY = 0;
MapMode* SwSelPaintRects::pMapMode = 0;


#ifdef SHOW_BOOKMARKS
// #include <IMark.hxx>
//
// class SwBookmarkRects : public SwSelPaintRects
// {
// 	virtual void Paint( const Rectangle& rRect );
// 	virtual void FillRects();
//
// public:
// 	SwBookmarkRects( const SwCrsrShell& rSh ) : SwSelPaintRects( rSh ) {}
// };
//
// void SwBookmarkRects::Paint( const Rectangle& rRect )
// {
// 	Window* pWin = GetShell()->GetWin();
//
// 	RasterOp eOld( pWin->GetRasterOp() );
// 	sal_Bool bLCol = pWin->IsLineColor();
// 	Color aLCol( pWin->GetLineColor() );
// 	sal_Bool bFCol = pWin->IsFillColor();
// 	Color aFCol( pWin->GetFillColor() );
//
// 	pWin->SetRasterOp( ROP_XOR );
// 	Color aCol( RGB_COLORDATA( 0xF0, 0xC8, 0xF0 ) ^ COL_WHITE );
// 	pWin->SetFillColor( aCol );
// 	pWin->SetLineColor( aCol );
//
// 	pWin->DrawRect( rRect );
//
// 	if( bLCol ) pWin->SetLineColor( aLCol ); else pWin->SetLineColor();
// 	if( bFCol ) pWin->SetFillColor( aFCol ); else pWin->SetFillColor();
// 	pWin->SetRasterOp( eOld );
// }
//
// void SwBookmarkRects::FillRects()
// {
// 	SwRegionRects aReg( GetShell()->VisArea() );
//
//     const SwBookmarks& rBkmkTbl = GetShell()->getIDocumentMarkAccess()->getBookmarks();
// 	SwShellCrsr* pCrsr = 0;
// 	for( sal_uInt16 n = 0; n < rBkmkTbl.Count(); ++n )
// 	{
// 		const SwBookmark& rBkmk = *rBkmkTbl[ n ];
// 		if( rBkmk.IsBookMark() && rBkmk.GetOtherPos() )
// 		{
// 			if( !pCrsr )
// 			{
// 				pCrsr = new SwShellCrsr( *GetShell(), rBkmk.GetPos() );
// 				pCrsr->SetMark();
// 			}
// 			else
// 				*pCrsr->GetPoint() = rBkmk.GetPos();
// 			*pCrsr->GetMark() = *rBkmk.GetOtherPos();
// 			pCrsr->FillRects();
// 			for( sal_uInt16 i = 0; i < pCrsr->Count(); ++i )
// 				aReg -= (*pCrsr)[ i ];
//
// 			pCrsr->Remove( 0, i );
// 		}
// 	}
// 	if( pCrsr ) delete pCrsr;
//
// 	aReg.Invert();
// 	SwRects::Insert( &aReg, 0 );
// }
//
// SwBookmarkRects* pBookMarkRects = 0;
//
// void ShowBookmarks( const SwCrsrShell* pSh, int nAction, const SwRect* pRect = 0 )
// {
//     if( !pBookMarkRects && pSh->getIDocumentMarkAccess()->getBookmarks().Count() )
// 		pBookMarkRects = new SwBookmarkRects( *pSh );
//
// 	if( pBookMarkRects )
// 	{
// 		switch( nAction )
// 		{
// 		case 1: pBookMarkRects->Show(); break;
// 		case 2:	pBookMarkRects->Hide(); break;
// 		case 3: pBookMarkRects->Invalidate( *pRect ); break;
// 		}
//
// 		if( !pBookMarkRects->Count() )
// 			delete pBookMarkRects, pBookMarkRects = 0;
// 	}
// }
//
// #define SHOWBOOKMARKS1( nAct )			ShowBookmarks( GetShell(),nAct );
// #define SHOWBOOKMARKS2( nAct, pRect )	ShowBookmarks( GetShell(),nAct, pRect );

#else

#define SHOWBOOKMARKS1( nAct )
#define SHOWBOOKMARKS2( nAct, pRect )

#endif

#ifdef SHOW_REDLINES
#include <redline.hxx>

class SwRedlineRects : public SwSelPaintRects
{
	sal_uInt16 nMode;
	sal_uInt16 nNm;

	virtual void Paint( const Rectangle& rRect );
	virtual void FillRects();

public:
	SwRedlineRects( const SwCrsrShell& rSh, sal_uInt16 nName, sal_uInt16 n )
		: SwSelPaintRects( rSh ), nMode( n ), nNm( nName )
	{}
};

void SwRedlineRects::Paint( const Rectangle& rRect )
{
	Window* pWin = GetShell()->GetWin();

	RasterOp eOld( pWin->GetRasterOp() );
	sal_Bool bLCol = pWin->IsLineColor();
	Color aLCol( pWin->GetLineColor() );
	sal_Bool bFCol = pWin->IsFillColor();
	Color aFCol( pWin->GetFillColor() );

	pWin->SetRasterOp( ROP_XOR );
	Color aCol;

	sal_uInt8 nVal = 0xc8 - ( (nMode / 4) * 16 );
	switch( nMode % 4 )
	{
	case 0: aCol = RGB_COLORDATA( nVal, nVal, 0xFF );	break;
	case 1: aCol = RGB_COLORDATA( 0xFF, 0xc8, nVal );	break;
	case 2: aCol = RGB_COLORDATA( nVal, 0xFF, nVal );	break;
	case 3: aCol = RGB_COLORDATA( 0xFF, nVal, nVal );	break;
	}
	aCol = aCol.GetColor() ^ COL_WHITE;

	pWin->SetFillColor( aCol );
	pWin->SetLineColor( aCol );

	pWin->DrawRect( rRect );

	if( bLCol ) pWin->SetLineColor( aLCol ); else pWin->SetLineColor();
	if( bFCol ) pWin->SetFillColor( aFCol ); else pWin->SetFillColor();
	pWin->SetRasterOp( eOld );
}

void SwRedlineRects::FillRects()
{
	SwRegionRects aReg( GetShell()->VisArea() );

	const SwRedlineTbl& rTbl = GetShell()->GetDoc()->GetRedlineTbl();
	SwShellCrsr* pCrsr = 0;
	for( sal_uInt16 n = 0; n < rTbl.Count(); ++n )
	{
		const SwRedline& rRed = *rTbl[ n ];
		if( rRed.HasMark() && (nMode % 4 ) == rRed.GetType() &&
			nNm == rRed.GetAuthor() )
		{
			if( !pCrsr )
			{
				pCrsr = new SwShellCrsr( *GetShell(), *rRed.GetPoint() );
				pCrsr->SetMark();
			}
			else
				*pCrsr->GetPoint() = *rRed.GetPoint();
			*pCrsr->GetMark() = *rRed.GetMark();
			pCrsr->FillRects();
			for( sal_uInt16 i = 0; i < pCrsr->Count(); ++i )
				aReg -= (*pCrsr)[ i ];

			pCrsr->Remove( 0, i );
		}
	}
	if( pCrsr ) delete pCrsr;

	aReg.Invert();
	SwRects::Insert( &aReg, 0 );
}

SwRedlineRects* aRedlines[ 10 * 4 ];
static int bFirstCall = sal_True;

void ShowRedlines( const SwCrsrShell* pSh, int nAction, const SwRect* pRect = 0 )
{
	if( bFirstCall )
	{
		memset( aRedlines, 0, sizeof(aRedlines));
		bFirstCall = sal_False;
	}

	const SwRedlineTbl& rTbl = pSh->GetDoc()->GetRedlineTbl();
	const SwRedlineAuthorTbl& rAuthorTbl = pSh->GetDoc()->GetRedlineAuthorTbl();

	for( sal_uInt16 n = 0; n < rAuthorTbl.Count(); ++n )
	{
		for( int i = 0; i < 4; ++i  )
		{
			SwRedlineRects** ppRedRect = &aRedlines[ n * 4 + i ];
			if( rTbl.Count() && !*ppRedRect )
				*ppRedRect = new SwRedlineRects( *pSh, n, n * 4 + i );

			if( *ppRedRect )
			{
				switch( nAction )
				{
				case 1: (*ppRedRect)->Show(); break;
				case 2:	(*ppRedRect)->Hide(); break;
				case 3: (*ppRedRect)->Invalidate( *pRect ); break;
				}

				if( !(*ppRedRect)->Count() )
					delete *ppRedRect, *ppRedRect = 0;
			}
		}
	}
}

#define SHOWREDLINES1( nAct )			ShowRedlines( GetShell(),nAct );
#define SHOWREDLINES2( nAct, pRect )	ShowRedlines( GetShell(),nAct, pRect );

#else

#define SHOWREDLINES1( nAct )
#define SHOWREDLINES2( nAct, pRect )

#endif

#ifdef JP_REDLINE
	if( GetDoc()->GetRedlineTbl().Count() )
	{
		SwRedlineTbl& rRedlineTbl = (SwRedlineTbl&)GetDoc()->GetRedlineTbl();
		for( sal_uInt16 i = 0; i < rRedlineTbl.Count(); ++i )
			rRedlineTbl[ i ]->HideRects( *GetShell() );
	}
#endif

// --------  Ab hier Klassen / Methoden fuer den nicht Text-Cursor ------

SwVisCrsr::SwVisCrsr( const SwCrsrShell * pCShell )
	: pCrsrShell( pCShell )
{
	pCShell->GetWin()->SetCursor( &aTxtCrsr );
	bIsVisible = aTxtCrsr.IsVisible();
	bIsDragCrsr = sal_False;
	aTxtCrsr.SetWidth( 0 );

#ifdef SW_CRSR_TIMER
	bTimerOn = sal_True;
	SetTimeout( 50 );       // 50msec Verzoegerung
#endif
}



SwVisCrsr::~SwVisCrsr()
{
#ifdef SW_CRSR_TIMER
	if( bTimerOn )
		Stop();		// Timer stoppen
#endif

	if( bIsVisible && aTxtCrsr.IsVisible() )
		aTxtCrsr.Hide();

	pCrsrShell->GetWin()->SetCursor( 0 );
}




void SwVisCrsr::Show()
{
	if( !bIsVisible )
	{
		bIsVisible = sal_True;

		// muss ueberhaupt angezeigt werden ?
		if( pCrsrShell->VisArea().IsOver( pCrsrShell->aCharRect ) )
#ifdef SW_CRSR_TIMER
		{
			if( bTimerOn )
				Start();            // Timer aufsetzen
			else
			{
				if( IsActive() )
					Stop();         // Timer Stoppen

				_SetPosAndShow();
			}
		}
#else
			_SetPosAndShow();
#endif
	}
}



void SwVisCrsr::Hide()
{
	if( bIsVisible )
	{
		bIsVisible = sal_False;

#ifdef SW_CRSR_TIMER
		if( IsActive() )
			Stop();         // Timer Stoppen
#endif

		if( aTxtCrsr.IsVisible() )		// sollten die Flags nicht gueltig sein?
			aTxtCrsr.Hide();
	}
}

#ifdef SW_CRSR_TIMER

void __EXPORT SwVisCrsr::Timeout()
{
	ASSERT( !bIsDragCrsr, "Timer vorher abschalten" );
	if( bIsVisible )
	{
		if ( !pCrsrShell->GetWin() ) //SwFrmFmt::GetGraphic setzt das Win temp aus!
			Start();
		else
			_SetPosAndShow();
	}
}

sal_Bool SwCrsrShell::ChgCrsrTimerFlag( sal_Bool bTimerOn )
{
	return pVisCrsr->ChgTimerFlag( bTimerOn );
}


sal_Bool SwVisCrsr::ChgTimerFlag( sal_Bool bFlag )
{
	bOld = bTimerOn;
	if( !bFlag && bIsVisible && IsActive() )
	{
		Stop();			// Timer Stoppen
		_SetPosAndShow();
	}
	bTimerOn = bFlag;
	return bOld;
}

#endif


void SwVisCrsr::_SetPosAndShow()
{
	SwRect aRect;
    long nTmpY = pCrsrShell->aCrsrHeight.Y();
    if( 0 > nTmpY )
	{
        nTmpY = -nTmpY;
        aTxtCrsr.SetOrientation( 900 );
		aRect = SwRect( pCrsrShell->aCharRect.Pos(),
           Size( pCrsrShell->aCharRect.Height(), nTmpY ) );
		aRect.Pos().X() += pCrsrShell->aCrsrHeight.X();
        if( pCrsrShell->IsOverwriteCrsr() )
            aRect.Pos().Y() += aRect.Width();
	}
	else
    {
        aTxtCrsr.SetOrientation( 0 );
		aRect = SwRect( pCrsrShell->aCharRect.Pos(),
           Size( pCrsrShell->aCharRect.Width(), nTmpY ) );
		aRect.Pos().Y() += pCrsrShell->aCrsrHeight.X();
    }

    // check if cursor should show the current cursor bidi level
    aTxtCrsr.SetDirection( CURSOR_DIRECTION_NONE );
    const SwCursor* pTmpCrsr = pCrsrShell->_GetCrsr();

    if ( pTmpCrsr && !pCrsrShell->IsOverwriteCrsr() )
    {
        SwNode& rNode = pTmpCrsr->GetPoint()->nNode.GetNode();
        if( rNode.IsTxtNode() )
        {
            const SwTxtNode& rTNd = *rNode.GetTxtNode();
            const SwFrm* pFrm = rTNd.getLayoutFrm( pCrsrShell->GetLayout(), 0, 0, sal_False );
            if ( pFrm )
            {
                const SwScriptInfo* pSI = ((SwTxtFrm*)pFrm)->GetScriptInfo();
                 // cursor level has to be shown
                if ( pSI && pSI->CountDirChg() > 1 )
                {
                    aTxtCrsr.SetDirection(
                        ( pTmpCrsr->GetCrsrBidiLevel() % 2 ) ?
                          CURSOR_DIRECTION_RTL :
                          CURSOR_DIRECTION_LTR );
                }

                if ( pFrm->IsRightToLeft() )
                {
                    const OutputDevice *pOut = pCrsrShell->GetOut();
                    if ( pOut )
                    {
                        long nSize = pOut->GetSettings().GetStyleSettings().GetCursorSize();
                        Size aSize( nSize, nSize );
                        aSize = pOut->PixelToLogic( aSize );
                        aRect.Left( aRect.Left() - aSize.Width() );
                    }
                }
            }
        }
    }

    if( aRect.Height() )
    {
        ::SwCalcPixStatics( pCrsrShell->GetOut() );
        ::SwAlignRect( aRect, (ViewShell*)pCrsrShell );
    }
    if( !pCrsrShell->IsOverwriteCrsr() || bIsDragCrsr ||
        pCrsrShell->IsSelection() )
        aRect.Width( 0 );

	aTxtCrsr.SetSize( aRect.SSize() );

	aTxtCrsr.SetPos( aRect.Pos() );
    if ( !pCrsrShell->IsCrsrReadonly()  || pCrsrShell->GetViewOptions()->IsSelectionInReadonly() )
	{
		if ( pCrsrShell->GetDrawView() )
			((SwDrawView*)pCrsrShell->GetDrawView())->SetAnimationEnabled(
					!pCrsrShell->IsSelection() );

		sal_uInt16 nStyle = bIsDragCrsr ? CURSOR_SHADOW : 0;
		if( nStyle != aTxtCrsr.GetStyle() )
		{
			aTxtCrsr.SetStyle( nStyle );
			aTxtCrsr.SetWindow( bIsDragCrsr ? pCrsrShell->GetWin() : 0 );
		}

		aTxtCrsr.Show();
	}
}

//////////////////////////////////////////////////////////////////////////////

SwSelPaintRects::SwSelPaintRects( const SwCrsrShell& rCSh )
    : SwRects( 0 )
    , pCShell( &rCSh )
    , mpCursorOverlay( 0 )
    , mbShowTxtInputFldOverlay( true )
    , mpTxtInputFldOverlay( NULL )
{
}

SwSelPaintRects::~SwSelPaintRects()
{
	Hide();
}

void SwSelPaintRects::swapContent(SwSelPaintRects& rSwap)
{
    SwRects aTempRects;
    aTempRects.Insert(this, 0);

    Remove(0, Count());
    Insert(&rSwap, 0);

    rSwap.Remove(0, rSwap.Count());
    rSwap.Insert(&aTempRects, 0);

    // #i75172# also swap mpCursorOverlay
    sdr::overlay::OverlayObject* pTempOverlay = getCursorOverlay();
    setCursorOverlay(rSwap.getCursorOverlay());
    rSwap.setCursorOverlay(pTempOverlay);

    const bool bTempShowTxtInputFldOverlay = mbShowTxtInputFldOverlay;
    mbShowTxtInputFldOverlay = rSwap.mbShowTxtInputFldOverlay;
    rSwap.mbShowTxtInputFldOverlay = bTempShowTxtInputFldOverlay;

    sw::overlay::OverlayRangesOutline* pTempTxtInputFldOverlay = mpTxtInputFldOverlay;
    mpTxtInputFldOverlay = rSwap.mpTxtInputFldOverlay;
    rSwap.mpTxtInputFldOverlay = pTempTxtInputFldOverlay;
}

void SwSelPaintRects::Hide()
{
    if(mpCursorOverlay)
    {
        delete mpCursorOverlay;
        mpCursorOverlay = 0;
    }

    if ( mpTxtInputFldOverlay != NULL )
    {
        delete mpTxtInputFldOverlay;
        mpTxtInputFldOverlay = NULL;
    }

    SwRects::Remove( 0, Count() );
}

void SwSelPaintRects::Show()
{
    SdrView* pView = (SdrView*)pCShell->GetDrawView();

    if(pView && pView->PaintWindowCount())
    {
        // reset rects
        SwRects::Remove( 0, SwRects::Count() );
        FillRects();

        // get new rects
        std::vector< basegfx::B2DRange > aNewRanges;

        for(sal_uInt16 a(0); a < Count(); a++)
        {
            const SwRect aNextRect((*this)[a]);
            const Rectangle aPntRect(aNextRect.SVRect());

            aNewRanges.push_back(basegfx::B2DRange(
                aPntRect.Left(), aPntRect.Top(),
                aPntRect.Right() + 1, aPntRect.Bottom() + 1));
        }

        if(mpCursorOverlay)
        {
            if(aNewRanges.size())
            {
                static_cast< sdr::overlay::OverlaySelection* >(mpCursorOverlay)->setRanges(aNewRanges);
            }
            else
            {
                delete mpCursorOverlay;
                mpCursorOverlay = 0;
            }
        }
        else if(Count())
        {
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

            if(pTargetOverlay)
            {
                // get the system's hilight color
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                const Color aHighlight(aSvtOptionsDrawinglayer.getHilightColor());

                // create correct selection
                mpCursorOverlay = new sdr::overlay::OverlaySelection(
                    sdr::overlay::OVERLAY_TRANSPARENT,
                    aHighlight, 
                    aNewRanges,
                    true);

                pTargetOverlay->add(*mpCursorOverlay);
            }
        }

        HighlightInputFld();
    }
}


void SwSelPaintRects::HighlightInputFld()
{
    std::vector< basegfx::B2DRange > aInputFldRanges;

    if ( mbShowTxtInputFldOverlay )
    {
        SwTxtInputFld* pCurTxtInputFldAtCrsr =
            dynamic_cast<SwTxtInputFld*>(GetShell()->GetTxtFldAtPos( GetShell()->GetCrsr()->Start(), false ));
        if ( pCurTxtInputFldAtCrsr != NULL )
        {
            SwTxtNode* pTxtNode = pCurTxtInputFldAtCrsr->GetpTxtNode();
            ::boost::scoped_ptr<SwShellCrsr> pCrsrForInputTxtFld(
                new SwShellCrsr( *GetShell(), SwPosition( *pTxtNode, *(pCurTxtInputFldAtCrsr->GetStart()) ) ) );
            pCrsrForInputTxtFld->SetMark();
            pCrsrForInputTxtFld->GetMark()->nNode = *pTxtNode;
            pCrsrForInputTxtFld->GetMark()->nContent.Assign( pTxtNode, *(pCurTxtInputFldAtCrsr->End()) );

            pCrsrForInputTxtFld->FillRects();

            for( sal_uInt16 a(0); a < pCrsrForInputTxtFld->Count(); ++a )
            {
                const SwRect aNextRect((*pCrsrForInputTxtFld)[a]);
                const Rectangle aPntRect(aNextRect.SVRect());

                aInputFldRanges.push_back(basegfx::B2DRange(
                    aPntRect.Left(), aPntRect.Top(),
                    aPntRect.Right() + 1, aPntRect.Bottom() + 1));
            }
        }
    }

    if ( aInputFldRanges.size() > 0 )
    {
        if ( mpTxtInputFldOverlay != NULL )
        {
            mpTxtInputFldOverlay->setRanges( aInputFldRanges );
        }
        else
        {
            SdrView* pView = (SdrView*)GetShell()->GetDrawView();
            SdrPaintWindow* pCandidate = pView->GetPaintWindow(0);
            sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

            if(pTargetOverlay)
            {
                // use system's hilight color with decreased luminance as highlight color
                const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
                Color aHighlight(aSvtOptionsDrawinglayer.getHilightColor());
                aHighlight.DecreaseLuminance( 128 );

                mpTxtInputFldOverlay = new sw::overlay::OverlayRangesOutline( aHighlight, aInputFldRanges );
                pTargetOverlay->add( *mpTxtInputFldOverlay );
            }
        }
    }
    else
    {
        if ( mpTxtInputFldOverlay != NULL )
        {
            delete mpTxtInputFldOverlay;
            mpTxtInputFldOverlay = NULL;
        }
    }
}


void SwSelPaintRects::Invalidate( const SwRect& rRect )
{
	sal_uInt16 nSz = Count();
	if( !nSz )
		return;

	SwRegionRects aReg( GetShell()->VisArea() );
	aReg.Remove( 0, aReg.Count() );
	aReg.Insert( this, 0 );
	aReg -= rRect;
	SwRects::Remove( 0, nSz );
	SwRects::Insert( &aReg, 0 );

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Liegt die Selection rechts oder unten ausserhalb des sichtbaren
	// Bereiches, so ist diese nie auf eine Pixel rechts/unten aligned.
	// Das muss hier erkannt und ggf. das Rechteckt erweitert werden.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if( GetShell()->bVisPortChgd && 0 != ( nSz = Count()) )
	{
		SwSelPaintRects::Get1PixelInLogic( *GetShell() );
		SwRect* pRect = (SwRect*)GetData();
		for( ; nSz--; ++pRect )
		{
			if( pRect->Right() == GetShell()->aOldRBPos.X() )
				pRect->Right( pRect->Right() + nPixPtX );
			if( pRect->Bottom() == GetShell()->aOldRBPos.Y() )
				pRect->Bottom( pRect->Bottom() + nPixPtY );
		}
	}
}

void SwSelPaintRects::Paint( const Rectangle& /*rRect*/ )
{
    // nothing to do with overlays
}


// check current MapMode of the shell and set possibly the static members.
// Optional set the parameters pX, pY
void SwSelPaintRects::Get1PixelInLogic( const ViewShell& rSh,
										long* pX, long* pY )
{
	const OutputDevice* pOut = rSh.GetWin();
	if ( ! pOut )
		pOut = rSh.GetOut();

	const MapMode& rMM = pOut->GetMapMode();
	if( pMapMode->GetMapUnit() != rMM.GetMapUnit() ||
		pMapMode->GetScaleX() != rMM.GetScaleX() ||
		pMapMode->GetScaleY() != rMM.GetScaleY() )
	{
		*pMapMode = rMM;
		Size aTmp( 1, 1 );
		aTmp = pOut->PixelToLogic( aTmp );
		nPixPtX = aTmp.Width();
		nPixPtY = aTmp.Height();
	}
	if( pX )
		*pX = nPixPtX;
	if( pY )
		*pY = nPixPtY;
}


/*  */

SwShellCrsr::SwShellCrsr(
    const SwCrsrShell& rCShell,
    const SwPosition &rPos )
    : SwCursor(rPos,0,false)
    , SwSelPaintRects(rCShell)
    , pPt(SwPaM::GetPoint())
{}


SwShellCrsr::SwShellCrsr(
    const SwCrsrShell& rCShell,
    const SwPosition &rPos,
    const Point& rPtPos,
    SwPaM* pRing )
    : SwCursor(rPos, pRing, false)
    , SwSelPaintRects(rCShell)
    , aMkPt(rPtPos)
    , aPtPt(rPtPos)
    , pPt(SwPaM::GetPoint())
{}


SwShellCrsr::SwShellCrsr( SwShellCrsr& rICrsr )
    : SwCursor(rICrsr)
    , SwSelPaintRects(*rICrsr.GetShell())
    , aMkPt(rICrsr.GetMkPos())
    , aPtPt(rICrsr.GetPtPos())
    , pPt(SwPaM::GetPoint())
{}

SwShellCrsr::~SwShellCrsr()
{}


bool SwShellCrsr::IsReadOnlyAvailable() const
{
    return GetShell()->IsReadOnlyAvailable();
}

void SwShellCrsr::SetMark()
{
	if( SwPaM::GetPoint() == pPt )
		aMkPt = aPtPt;
	else
		aPtPt = aMkPt;
	SwPaM::SetMark();
}

void SwShellCrsr::FillRects()
{
    // die neuen Rechtecke berechnen
    if ( HasMark()
         && GetPoint()->nNode.GetNode().IsCntntNode()
         && GetPoint()->nNode.GetNode().GetCntntNode()->getLayoutFrm( GetShell()->GetLayout() )
         && ( GetMark()->nNode == GetPoint()->nNode
              || ( GetMark()->nNode.GetNode().IsCntntNode()
                   && GetMark()->nNode.GetNode().GetCntntNode()->getLayoutFrm( GetShell()->GetLayout() ) ) ) )
    {
        GetShell()->GetLayout()->CalcFrmRects( *this );
    }
}


void SwShellCrsr::Show()
{
	SwShellCrsr * pTmp = this;
	do {
		pTmp->SwSelPaintRects::Show();
    } while( this != ( pTmp = dynamic_cast<SwShellCrsr*>(pTmp->GetNext()) ) );

	SHOWBOOKMARKS1( 1 )
	SHOWREDLINES1( 1 )
}


	// Dieses Rechteck wird neu gepaintet, also ist die SSelection in
	// dem Bereich ungueltig
void SwShellCrsr::Invalidate( const SwRect& rRect )
{
	SwShellCrsr * pTmp = this;

	do
    {
		pTmp->SwSelPaintRects::Invalidate( rRect );

        // --> FME 2005-08-18 #125102#
        // skip any non SwShellCrsr objects in the ring
        // (see:SwAutoFormat::DeleteSel()
        // <--
        Ring* pTmpRing = pTmp;
        pTmp = 0;
        do
        {
            pTmpRing = pTmpRing->GetNext();
            pTmp = dynamic_cast<SwShellCrsr*>(pTmpRing);
        }
        while ( !pTmp );
	}
    while( this != pTmp );

	SHOWBOOKMARKS2( 3, &rRect )
	SHOWREDLINES2( 3, &rRect )
}


void SwShellCrsr::Hide()
{
	SwShellCrsr * pTmp = this;
	do {
		pTmp->SwSelPaintRects::Hide();
    } while( this != ( pTmp = dynamic_cast<SwShellCrsr*>(pTmp->GetNext()) ) );

	SHOWBOOKMARKS1( 2 )
	SHOWREDLINES1( 2 )
}

SwCursor* SwShellCrsr::Create( SwPaM* pRing ) const
{
	return new SwShellCrsr( *GetShell(), *GetPoint(), GetPtPos(), pRing );
}


short SwShellCrsr::MaxReplaceArived()
{
    short nRet = RET_YES;
	Window* pDlg = LAYOUT_THIS_WINDOW (::GetSearchDialog());
	if( pDlg )
	{
		// alte Actions beenden; die Tabellen-Frames werden angelegt und
		// eine SSelection kann erzeugt werden
		SvUShorts aArr;
		sal_uInt16 nActCnt;
		ViewShell *pShell = const_cast< SwCrsrShell* >( GetShell() ),
				  *pSh = pShell;
		do {
			for( nActCnt = 0; pSh->ActionPend(); ++nActCnt )
				pSh->EndAction();
			aArr.Insert( nActCnt, aArr.Count() );
		} while( pShell != ( pSh = (ViewShell*)pSh->GetNext() ) );

		{
			nRet = QueryBox( pDlg, SW_RES( MSG_COMCORE_ASKSEARCH )).Execute();
		}

		for( sal_uInt16 n = 0; n < aArr.Count(); ++n )
		{
            for( nActCnt = aArr[n]; nActCnt--; )
				pSh->StartAction();
			pSh = (ViewShell*)pSh->GetNext();
		}	//swmod 071107//swmod 071225
	}
	else
		// ansonsten aus dem Basic, und dann auf RET_YES schalten
		nRet = RET_YES;

    return nRet;
}

void SwShellCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
	((SwCrsrShell*)GetShell())->SaveTblBoxCntnt( pPos );
}

sal_Bool SwShellCrsr::UpDown( sal_Bool bUp, sal_uInt16 nCnt )
{
	return SwCursor::UpDown( bUp, nCnt,
							&GetPtPos(), GetShell()->GetUpDownX() );
}

#ifdef DBG_UTIL

// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//				am sichtbaren Cursor

sal_Bool SwShellCrsr::IsSelOvr( int eFlags )
{
	return SwCursor::IsSelOvr( eFlags );
}

#endif

// sal_True: an die Position kann der Cursor gesetzt werden
sal_Bool SwShellCrsr::IsAtValidPos( sal_Bool bPoint ) const
{
	if( GetShell() && ( GetShell()->IsAllProtect() ||
		GetShell()->GetViewOptions()->IsReadonly() ||
		( GetShell()->Imp()->GetDrawView() &&
		  GetShell()->Imp()->GetDrawView()->GetMarkedObjectList().GetMarkCount() )))
		return sal_True;

	return SwCursor::IsAtValidPos( bPoint );
}

/*  */

SwShellTableCrsr::SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
									const SwPosition& rPos )
	: SwCursor(rPos,0,false), SwShellCrsr(rCrsrSh, rPos), SwTableCursor(rPos)
{
}

SwShellTableCrsr::SwShellTableCrsr( const SwCrsrShell& rCrsrSh,
					const SwPosition& rMkPos, const Point& rMkPt,
					const SwPosition& rPtPos, const Point& rPtPt )
	: SwCursor(rPtPos,0,false), SwShellCrsr(rCrsrSh, rPtPos), SwTableCursor(rPtPos)
{
	SetMark();
	*GetMark() = rMkPos;
	GetMkPos() = rMkPt;
	GetPtPos() = rPtPt;
}

SwShellTableCrsr::~SwShellTableCrsr() {}

void SwShellTableCrsr::SetMark() 				{ SwShellCrsr::SetMark(); }

SwCursor* SwShellTableCrsr::Create( SwPaM* pRing ) const
{
	return SwShellCrsr::Create( pRing );
}
short SwShellTableCrsr::MaxReplaceArived()
{
	return SwShellCrsr::MaxReplaceArived();
}
void SwShellTableCrsr::SaveTblBoxCntnt( const SwPosition* pPos )
{
	SwShellCrsr::SaveTblBoxCntnt( pPos );
}


void SwShellTableCrsr::FillRects()
{
	// die neuen Rechtecke berechnen
	// JP 16.01.98: wenn der Cursor noch "geparkt" ist nichts machen!!
	if( !aSelBoxes.Count() || bParked ||
		!GetPoint()->nNode.GetIndex() )
		return;

	SwRegionRects aReg( GetShell()->VisArea() );
	SwNodes& rNds = GetDoc()->GetNodes();
	for( sal_uInt16 n = 0; n < aSelBoxes.Count(); ++n )
	{
        const SwStartNode* pSttNd = (*(aSelBoxes.GetData() + n ))->GetSttNd();
        const SwTableNode* pSelTblNd = pSttNd->FindTableNode();

        SwNodeIndex aIdx( *pSttNd );
       	SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, sal_True, sal_False );

        // TABLE IN TABLE
        // (see also lcl_FindTopLevelTable in unoobj2.cxx for a different
        // version to do this)
        const SwTableNode* pCurTblNd = pCNd->FindTableNode();
        while ( pSelTblNd != pCurTblNd && pCurTblNd )
        {
            aIdx = pCurTblNd->EndOfSectionIndex();
            pCNd = rNds.GoNextSection( &aIdx, sal_True, sal_False );
            pCurTblNd = pCNd->FindTableNode();
        }

		if( !pCNd )
			continue;

		SwFrm* pFrm = pCNd->getLayoutFrm( GetShell()->GetLayout(), &GetSttPos() );
		while( pFrm && !pFrm->IsCellFrm() )
			pFrm = pFrm->GetUpper();

        ASSERT( pFrm, "Node nicht in einer Tabelle" );

        while ( pFrm )
        {
    		if( pFrm && aReg.GetOrigin().IsOver( pFrm->Frm() ) )
	    		aReg -= pFrm->Frm();

            pFrm = pFrm->GetNextCellLeaf( MAKEPAGE_NONE );
        }
    }
	aReg.Invert();
	Insert( &aReg, 0 );
}


// Pruefe, ob sich der SPoint innerhalb der Tabellen-SSelection befindet
sal_Bool SwShellTableCrsr::IsInside( const Point& rPt ) const
{
	// die neuen Rechtecke berechnen
	// JP 16.01.98: wenn der Cursor noch "geparkt" ist nichts machen!!
	if( !aSelBoxes.Count() || bParked ||
		!GetPoint()->nNode.GetIndex()  )
		return sal_False;

	SwNodes& rNds = GetDoc()->GetNodes();
	for( sal_uInt16 n = 0; n < aSelBoxes.Count(); ++n )
	{
		SwNodeIndex aIdx( *(*(aSelBoxes.GetData() + n ))->GetSttNd() );
		SwCntntNode* pCNd = rNds.GoNextSection( &aIdx, sal_True, sal_False );
		if( !pCNd )
			continue;

		SwFrm* pFrm = pCNd->getLayoutFrm( GetShell()->GetLayout(), &GetPtPos() );
		while( pFrm && !pFrm->IsCellFrm() )
			pFrm = pFrm->GetUpper();
		ASSERT( pFrm, "Node nicht in einer Tabelle" );
		if( pFrm && pFrm->Frm().IsInside( rPt ) )
			return sal_True;
	}
	return sal_False;
}

#ifdef DBG_UTIL

// JP 05.03.98: zum Testen des UNO-Crsr Verhaltens hier die Implementierung
//				am sichtbaren Cursor
sal_Bool SwShellTableCrsr::IsSelOvr( int eFlags )
{
	return SwShellCrsr::IsSelOvr( eFlags );
}

#endif

sal_Bool SwShellTableCrsr::IsAtValidPos( sal_Bool bPoint ) const
{
	return SwShellCrsr::IsAtValidPos( bPoint );
}

