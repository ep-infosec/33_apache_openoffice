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

#include <hintids.hxx>
#include <tools/urlobj.hxx>
#include <vcl/print.hxx>
#include <vcl/virdev.hxx>
#include <vcl/svapp.hxx>
#include <svtools/imapobj.hxx>
#include <svtools/imap.hxx>
#include <svl/urihelper.hxx>
#include <svtools/soerr.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/printer.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/window.hxx>
#include <fmturl.hxx>
#include <fmtsrnd.hxx>
#include <frmfmt.hxx>
#include <swrect.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <flyfrm.hxx>
#include <frmtool.hxx>
#include <viewopt.hxx>
#include <viewimp.hxx>
#include <pam.hxx>
#include <hints.hxx>
#include <rootfrm.hxx>
#include <dflyobj.hxx>
#include <pagefrm.hxx>
#include <notxtfrm.hxx>
#include <grfatr.hxx>
#include <charatr.hxx>
#include <fmtornt.hxx>
#include <ndnotxt.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <swregion.hxx>
#include <poolfmt.hxx>
#include <mdiexp.hxx>
#include <swwait.hxx>
#include <comcore.hrc>
#include <accessibilityoptions.hxx>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <svtools/embedhlp.hxx>
#include <svx/charthelper.hxx>
#include <dview.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <svtools/filter.hxx>

using namespace com::sun::star;

#define DEFTEXTSIZE  12

extern void ClrContourCache( const SdrObject *pObj ); // TxtFly.Cxx


inline sal_Bool GetRealURL( const SwGrfNode& rNd, String& rTxt )
{
	sal_Bool bRet = rNd.GetFileFilterNms( &rTxt, 0 );
	if( bRet )
		rTxt = URIHelper::removePassword( rTxt, INetURLObject::WAS_ENCODED,
			   							INetURLObject::DECODE_UNAMBIGUOUS);
	return bRet;
}

void lcl_PaintReplacement( const SwRect &rRect, const String &rText,
						   const ViewShell &rSh, const SwNoTxtFrm *pFrm,
						   sal_Bool bDefect )
{
	static Font *pFont = 0;
	if ( !pFont )
	{
		pFont = new Font();
		pFont->SetWeight( WEIGHT_BOLD );
		pFont->SetStyleName( aEmptyStr );
		pFont->SetName( String::CreateFromAscii(
                            RTL_CONSTASCII_STRINGPARAM( "Arial Unicode" )));
		pFont->SetFamily( FAMILY_SWISS );
		pFont->SetTransparent( sal_True );
	}

	Color aCol( COL_RED );
	FontUnderline eUnderline = UNDERLINE_NONE;
	const SwFmtURL &rURL = pFrm->FindFlyFrm()->GetFmt()->GetURL();
	if( rURL.GetURL().Len() || rURL.GetMap() )
	{
		sal_Bool bVisited = sal_False;
		if ( rURL.GetMap() )
		{
			ImageMap *pMap = (ImageMap*)rURL.GetMap();
			for( sal_uInt16 i = 0; i < pMap->GetIMapObjectCount(); i++ )
			{
				IMapObject *pObj = pMap->GetIMapObject( i );
				if( rSh.GetDoc()->IsVisitedURL( pObj->GetURL() ) )
				{
					bVisited = sal_True;
					break;
				}
			}
		}
		else if ( rURL.GetURL().Len() )
			bVisited = rSh.GetDoc()->IsVisitedURL( rURL.GetURL() );

		SwFmt *pFmt = rSh.GetDoc()->GetFmtFromPool( static_cast<sal_uInt16>
            (bVisited ? RES_POOLCHR_INET_VISIT : RES_POOLCHR_INET_NORMAL ) );
		aCol = pFmt->GetColor().GetValue();
		eUnderline = pFmt->GetUnderline().GetLineStyle();
	}

	pFont->SetUnderline( eUnderline );
	pFont->SetColor( aCol );

    const BitmapEx& rBmp = ViewShell::GetReplacementBitmap( bDefect != sal_False );
    Graphic::DrawEx( rSh.GetOut(), rText, *pFont, rBmp, rRect.Pos(), rRect.SSize() );
}

/*************************************************************************
|*
|*	  SwGrfFrm::SwGrfFrm(ViewShell * const,SwGrfNode *)
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 03. Mar. 93
|*
*************************************************************************/


SwNoTxtFrm::SwNoTxtFrm(SwNoTxtNode * const pNode, SwFrm* pSib )
	: SwCntntFrm( pNode, pSib )
{
	InitCtor();
}

// Initialisierung: z.Zt. Eintragen des Frames im Cache


void SwNoTxtFrm::InitCtor()
{
    nType = FRMC_NOTXT;
	// Das Gewicht der Grafik ist 0, wenn sie noch nicht
	// gelesen ist, < 0, wenn ein Lesefehler auftrat und
	// Ersatzdarstellung angewendet werden musste und >0,
	// wenn sie zur Verfuegung steht.
	nWeight = 0;
}

/*************************************************************************
|*
|*	  SwNoTxtNode::MakeFrm()
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 03. Mar. 93
|*
*************************************************************************/


SwCntntFrm *SwNoTxtNode::MakeFrm( SwFrm* pSib )
{
	return new SwNoTxtFrm(this, pSib);
}

/*************************************************************************
|*
|*	  SwNoTxtFrm::~SwNoTxtFrm()
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 30. Apr. 96
|*
*************************************************************************/

SwNoTxtFrm::~SwNoTxtFrm()
{
	StopAnimation();
}

/*************************************************************************
|*
|*	  void SwNoTxtFrm::Modify( SwHint * pOld, SwHint * pNew )
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	JP 05.03.91
|*
*************************************************************************/

void SetOutDev( ViewShell *pSh, OutputDevice *pOut )
{
	pSh->pOut = pOut;
}




void lcl_ClearArea( const SwFrm &rFrm,
					OutputDevice &rOut, const SwRect& rPtArea,
					const SwRect &rGrfArea )
{
	SwRegionRects aRegion( rPtArea, 4, 4 );
	aRegion -= rGrfArea;

	if ( aRegion.Count() )
	{
		const SvxBrushItem *pItem; const Color *pCol; SwRect aOrigRect;

        //UUUU
        drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes;

		if ( rFrm.GetBackgroundBrush( aFillAttributes, pItem, pCol, aOrigRect, sal_False ) )
        {
            const bool bDone(::DrawFillAttributes(aFillAttributes, aOrigRect, rPtArea, rOut));

            if(!bDone)
            {
                for( sal_uInt16 i = 0; i < aRegion.Count(); ++i )
                {
                    ::DrawGraphic( pItem, &rOut, aOrigRect, aRegion[i] );
                }
            }
        }
		else
		{
            // OD 2004-04-23 #116347#
            rOut.Push( PUSH_FILLCOLOR|PUSH_LINECOLOR );
			rOut.SetFillColor( rFrm.getRootFrm()->GetCurrShell()->Imp()->GetRetoucheColor());
			rOut.SetLineColor();
			for( sal_uInt16 i = 0; i < aRegion.Count(); ++i )
				rOut.DrawRect( aRegion[i].SVRect() );
			rOut.Pop();
		}
	}
}

/*************************************************************************
|*
|*	  void SwNoTxtFrm::Paint()
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 10. Jan. 97
|*
*************************************************************************/

void SwNoTxtFrm::Paint(SwRect const& rRect, SwPrintData const*const) const
{
	if ( Frm().IsEmpty() )
		return;

	const ViewShell* pSh = getRootFrm()->GetCurrShell();
	if( !pSh->GetViewOptions()->IsGraphic() )
	{
		StopAnimation();
        // OD 10.01.2003 #i6467# - no paint of placeholder for page preview
        if ( pSh->GetWin() && !pSh->IsPreView() )
		{
			const SwNoTxtNode* pNd = GetNode()->GetNoTxtNode();
            String aTxt( pNd->GetTitle() );
			if ( !aTxt.Len() && pNd->IsGrfNode() )
				GetRealURL( *(SwGrfNode*)pNd, aTxt );
			if( !aTxt.Len() )
				aTxt = FindFlyFrm()->GetFmt()->GetName();
			lcl_PaintReplacement( Frm(), aTxt, *pSh, this, sal_False );
		}
		return;
	}

    if( pSh->GetAccessibilityOptions()->IsStopAnimatedGraphics() ||
    // --> FME 2004-06-21 #i9684# Stop animation during printing/pdf export
       !pSh->GetWin() )
    // <--
        StopAnimation();

	SfxProgress::EnterLock(); //Keine Progress-Reschedules im Paint (SwapIn)

	OutputDevice *pOut = pSh->GetOut();
	pOut->Push();
	sal_Bool bClip = sal_True;
	PolyPolygon aPoly;

    SwNoTxtNode& rNoTNd = *(SwNoTxtNode*)GetNode();
	SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
    if( pGrfNd )
		pGrfNd->SetFrameInPaint( sal_True );

    // OD 16.04.2003 #i13147# - add 2nd parameter with value <sal_True> to
    // method call <FindFlyFrm().GetContour(..)> to indicate that it is called
    // for paint in order to avoid load of the intrinsic graphic.
    if ( ( !pOut->GetConnectMetaFile() ||
           !pSh->GetWin() ) &&
         FindFlyFrm()->GetContour( aPoly, sal_True )
       )
	{
		pOut->SetClipRegion( aPoly );
		bClip = sal_False;
	}

	SwRect aOrigPaint( rRect );
	if ( HasAnimation() && pSh->GetWin() )
	{
		aOrigPaint = Frm(); aOrigPaint += Prt().Pos();
	}

	SwRect aGrfArea( Frm() );
	SwRect aPaintArea( aGrfArea );
	aPaintArea._Intersection( aOrigPaint );

	SwRect aNormal( Frm().Pos() + Prt().Pos(), Prt().SSize() );
	aNormal.Justify(); //Normalisiertes Rechteck fuer die Vergleiche

	if( aPaintArea.IsOver( aNormal ) )
	{
		// berechne die 4 zu loeschenden Rechtecke
		if( pSh->GetWin() )
			::lcl_ClearArea( *this, *pSh->GetOut(), aPaintArea, aNormal );

		// in der Schnittmenge vom PaintBereich und der Bitmap liegt
		// der absolut sichtbare Bereich vom Frame
		aPaintArea._Intersection( aNormal );

		if ( bClip )
			pOut->IntersectClipRegion( aPaintArea.SVRect() );
        /// OD 25.09.2002 #99739# - delete unused 3rd parameter
        PaintPicture( pOut, aGrfArea );
	}
	else
		// wenn nicht sichtbar, loesche einfach den angegebenen Bereich
		lcl_ClearArea( *this, *pSh->GetOut(), aPaintArea, SwRect() );
    if( pGrfNd )
		pGrfNd->SetFrameInPaint( sal_False );

	pOut->Pop();
	SfxProgress::LeaveLock();
}

/*************************************************************************
|*
|*    void lcl_CalcRect( Point & aPt, Size & aDim,
|*                   sal_uInt16 nMirror )
|*
|*    Beschreibung      Errechne die Position und die Groesse der Grafik im
|*                      Frame, entsprechen der aktuellen Grafik-Attribute
|*
|*    Parameter         Point&  die Position im Frame  ( auch Return-Wert )
|*                      Size&   die Groesse der Grafik ( auch Return-Wert )
|*                      MirrorGrf   akt. Spiegelungs-Attribut
|*    Ersterstellung    JP 04.03.91
|*    Letzte Aenderung  JP 31.08.94
|*
*************************************************************************/


void lcl_CalcRect( Point& rPt, Size& rDim, sal_uInt16 nMirror )
{
    if( nMirror == RES_MIRROR_GRAPH_VERT || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        rPt.X() += rDim.Width() -1;
        rDim.Width() = -rDim.Width();
    }

    if( nMirror == RES_MIRROR_GRAPH_HOR || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        rPt.Y() += rDim.Height() -1;
        rDim.Height() = -rDim.Height();
    }
}

/*************************************************************************
|*
|*	  void SwNoTxtFrm::GetGrfArea()
|*
|*	  Beschreibung		Errechne die Position und die Groesse der Bitmap
|*						innerhalb des uebergebenem Rechtecks.
|*
|*	  Ersterstellung	JP 03.09.91
|*	  Letzte Aenderung	MA 11. Oct. 94
|*
*************************************************************************/

void SwNoTxtFrm::GetGrfArea( SwRect &rRect, SwRect* pOrigRect,
							 sal_Bool ) const
{
    // JP 23.01.2001: currently only used for scaling, cropping and mirroring
    // the contour of graphics!
	//					all other is handled by the GraphicObject

	//In rRect wird das sichbare Rechteck der Grafik gesteckt.
	//In pOrigRect werden Pos+Size der Gesamtgrafik gesteck.

	const SwAttrSet& rAttrSet = GetNode()->GetSwAttrSet();
	const SwCropGrf& rCrop = rAttrSet.GetCropGrf();
    sal_uInt16 nMirror = rAttrSet.GetMirrorGrf().GetValue();

    if( rAttrSet.GetMirrorGrf().IsGrfToggle() )
    {
        if( !(FindPageFrm()->GetVirtPageNum() % 2) )
        {
            switch ( nMirror )
            {
                case RES_MIRROR_GRAPH_DONT: nMirror = RES_MIRROR_GRAPH_VERT; break;
                case RES_MIRROR_GRAPH_VERT: nMirror = RES_MIRROR_GRAPH_DONT; break;
                case RES_MIRROR_GRAPH_HOR: nMirror = RES_MIRROR_GRAPH_BOTH; break;
                default: nMirror = RES_MIRROR_GRAPH_HOR; break;
            }
        }
    }

	//Grafik wird vom Node eingelesen falls notwendig. Kann aber schiefgehen.
	long nLeftCrop, nRightCrop, nTopCrop, nBottomCrop;
	Size aOrigSz( ((SwNoTxtNode*)GetNode())->GetTwipSize() );
	if ( !aOrigSz.Width() )
	{
		aOrigSz.Width() = Prt().Width();
		nLeftCrop  = -rCrop.GetLeft();
		nRightCrop = -rCrop.GetRight();
	}
	else
	{
		nLeftCrop = Max( aOrigSz.Width() -
							(rCrop.GetRight() + rCrop.GetLeft()), long(1) );
		const double nScale = double(Prt().Width())  / double(nLeftCrop);
		nLeftCrop  = long(nScale * -rCrop.GetLeft() );
		nRightCrop = long(nScale * -rCrop.GetRight() );
	}

    // crop values have to be mirrored too
    if( nMirror == RES_MIRROR_GRAPH_VERT || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        long nTmpCrop = nLeftCrop;
        nLeftCrop = nRightCrop;
        nRightCrop= nTmpCrop;
    }

	if( !aOrigSz.Height() )
	{
		aOrigSz.Height() = Prt().Height();
		nTopCrop   = -rCrop.GetTop();
		nBottomCrop= -rCrop.GetBottom();
	}
	else
	{
		nTopCrop = Max( aOrigSz.Height() - (rCrop.GetTop() + rCrop.GetBottom()), long(1) );
		const double nScale = double(Prt().Height()) / double(nTopCrop);
		nTopCrop   = long(nScale * -rCrop.GetTop() );
		nBottomCrop= long(nScale * -rCrop.GetBottom() );
	}

    // crop values have to be mirrored too
    if( nMirror == RES_MIRROR_GRAPH_HOR || nMirror == RES_MIRROR_GRAPH_BOTH )
    {
        long nTmpCrop = nTopCrop;
        nTopCrop   = nBottomCrop;
        nBottomCrop= nTmpCrop;
    }

	Size  aVisSz( Prt().SSize() );
	Size  aGrfSz( aVisSz );
	Point aVisPt( Frm().Pos() + Prt().Pos() );
	Point aGrfPt( aVisPt );

	//Erst das 'sichtbare' Rect einstellen.
	if ( nLeftCrop > 0 )
	{
		aVisPt.X() 	+= nLeftCrop;
		aVisSz.Width() -= nLeftCrop;
	}
	if ( nTopCrop > 0 )
	{
		aVisPt.Y() 	 += nTopCrop;
		aVisSz.Height() -= nTopCrop;
	}
	if ( nRightCrop > 0 )
		aVisSz.Width() -= nRightCrop;
	if ( nBottomCrop > 0 )
		aVisSz.Height() -= nBottomCrop;

	rRect.Pos  ( aVisPt );
	rRect.SSize( aVisSz );

	//Ggf. Die Gesamtgrafik berechnen
	if ( pOrigRect )
	{
		Size aTmpSz( aGrfSz );
		aGrfPt.X()    += nLeftCrop;
		aTmpSz.Width() -= nLeftCrop + nRightCrop;
		aGrfPt.Y()	    += nTopCrop;
		aTmpSz.Height()-= nTopCrop + nBottomCrop;

        if( RES_MIRROR_GRAPH_DONT != nMirror )
            lcl_CalcRect( aGrfPt, aTmpSz, nMirror );

		pOrigRect->Pos  ( aGrfPt );
		pOrigRect->SSize( aTmpSz );
	}
}

/*************************************************************************
|*
|*	  Size SwNoTxtFrm::GetSize()
|*
|*	  Beschreibung		Gebe die Groesse des umgebenen FLys und
|*						damit die der Grafik zurueck.
|*	  Ersterstellung	JP 04.03.91
|*	  Letzte Aenderung	JP 31.08.94
|*
*************************************************************************/


const Size& SwNoTxtFrm::GetSize() const
{
	// gebe die Groesse des Frames zurueck
	const SwFrm *pFly = FindFlyFrm();
	if( !pFly )
		pFly = this;
	return pFly->Prt().SSize();
}

/*************************************************************************
|*
|*	  SwNoTxtFrm::MakeAll()
|*
|*	  Ersterstellung	MA 29. Nov. 96
|*	  Letzte Aenderung	MA 29. Nov. 96
|*
*************************************************************************/


void SwNoTxtFrm::MakeAll()
{
	SwCntntNotify aNotify( this );
	SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
	const SwBorderAttrs &rAttrs = *aAccess.Get();

	while ( !bValidPos || !bValidSize || !bValidPrtArea )
	{
		MakePos();

		if ( !bValidSize )
			Frm().Width( GetUpper()->Prt().Width() );

		MakePrtArea( rAttrs );

		if ( !bValidSize )
		{	bValidSize = sal_True;
			Format();
		}
	}
}

/*************************************************************************
|*
|*	  SwNoTxtFrm::Format()
|*
|*	  Beschreibung		Errechne die Groesse der Bitmap, wenn noetig
|*	  Ersterstellung	JP 11.03.91
|*	  Letzte Aenderung	MA 13. Mar. 96
|*
*************************************************************************/


void SwNoTxtFrm::Format( const SwBorderAttrs * )
{
	const Size aNewSize( GetSize() );

	// hat sich die Hoehe geaendert?
	SwTwips nChgHght = IsVertical() ?
		(SwTwips)(aNewSize.Width() - Prt().Width()) :
		(SwTwips)(aNewSize.Height() - Prt().Height());
    if( nChgHght > 0)
        Grow( nChgHght );
	else if( nChgHght < 0)
        Shrink( Min(Prt().Height(), -nChgHght) );
}

/*************************************************************************
|*
|*	  SwNoTxtFrm::GetCharRect()
|*
|*	  Beschreibung
|*	  Ersterstellung	SS 29-Apr-1991
|*	  Letzte Aenderung	MA 10. Oct. 94
|*
|*************************************************************************/


sal_Bool SwNoTxtFrm::GetCharRect( SwRect &rRect, const SwPosition& rPos,
							  SwCrsrMoveState *pCMS ) const
{
	if ( &rPos.nNode.GetNode() != (SwNode*)GetNode() )
		return sal_False;

	Calc();
	SwRect aFrameRect( Frm() );
	rRect = aFrameRect;
	rRect.Pos( Frm().Pos() + Prt().Pos() );
	rRect.SSize( Prt().SSize() );

	rRect.Justify();

	// liegt die Bitmap ueberhaupt im sichtbaren Berich ?
	if( !aFrameRect.IsOver( rRect ) )
	{
		// wenn nicht dann steht der Cursor auf dem Frame
		rRect = aFrameRect;
		rRect.Width( 1 );
	}
	else
		rRect._Intersection( aFrameRect );

	if ( pCMS )
	{
		if ( pCMS->bRealHeight )
		{
			pCMS->aRealHeight.Y() = rRect.Height();
			pCMS->aRealHeight.X() = 0;
		}
	}

	return sal_True;
}


sal_Bool SwNoTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& ,
                             SwCrsrMoveState* ) const
{
	SwCntntNode* pCNd = (SwCntntNode*)GetNode();
	pPos->nNode = *pCNd;
	pPos->nContent.Assign( pCNd, 0 );
	return sal_True;
}

#define CLEARCACHE( pNd ) {\
	(pNd)->ReleaseGraphicFromCache();\
	SwFlyFrm* pFly = FindFlyFrm();\
	if( pFly && pFly->GetFmt()->GetSurround().IsContour() )\
	{\
		ClrContourCache( pFly->GetVirtDrawObj() );\
		pFly->NotifyBackground( FindPageFrm(), Prt(), PREP_FLY_ATTR_CHG );\
	}\
}

void SwNoTxtFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
	sal_uInt16 nWhich = pNew ? pNew->Which() : pOld ? pOld->Which() : 0;

    // --> OD 2007-03-06 #i73788#
    // no <SwCntntFrm::Modify(..)> for RES_LINKED_GRAPHIC_STREAM_ARRIVED
    if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
		 RES_GRAPHIC_ARRIVED != nWhich &&
         RES_GRF_REREAD_AND_INCACHE != nWhich &&
         RES_LINKED_GRAPHIC_STREAM_ARRIVED != nWhich )
    // <--
    {
		SwCntntFrm::Modify( pOld, pNew );
    }

	sal_Bool bComplete = sal_True;

	switch( nWhich )
	{
	case RES_OBJECTDYING:
		break;

	case RES_GRF_REREAD_AND_INCACHE:
		if( ND_GRFNODE == GetNode()->GetNodeType() )
		{
			bComplete = sal_False;
			SwGrfNode* pNd = (SwGrfNode*) GetNode();

			ViewShell *pVSh = 0;
			pNd->GetDoc()->GetEditShell( &pVSh );
			if( pVSh )
			{
				GraphicAttr aAttr;
				if( pNd->GetGrfObj().IsCached( pVSh->GetOut(), Point(),
							Prt().SSize(), &pNd->GetGraphicAttr( aAttr, this ) ))
				{
					ViewShell *pSh = pVSh;
					do {
						SET_CURR_SHELL( pSh );
						if( pSh->GetWin() )
						{
							if( pSh->IsPreView() )
								::RepaintPagePreview( pSh, Frm().SVRect() );
							else
								pSh->GetWin()->Invalidate( Frm().SVRect() );
						}
					} while( pVSh != (pSh = (ViewShell*)pSh->GetNext() ));
				}
				else
					pNd->SwapIn();
			}
		}
		break;

	case RES_UPDATE_ATTR:
	case RES_FMT_CHG:
		CLEARCACHE( (SwGrfNode*) GetNode() )
		break;

	case RES_ATTRSET_CHG:
		{
			sal_uInt16 n;
			for( n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
				if( SFX_ITEM_SET == ((SwAttrSetChg*)pOld)->GetChgSet()->
								GetItemState( n, sal_False ))
				{
					CLEARCACHE( (SwGrfNode*) GetNode() )
					break;
				}
			if( RES_GRFATR_END == n )			// not found
				return ;
		}
		break;

	case RES_GRAPHIC_PIECE_ARRIVED:
	case RES_GRAPHIC_ARRIVED:
    // --> OD 2007-03-06 #i73788#
    // handle RES_LINKED_GRAPHIC_STREAM_ARRIVED as RES_GRAPHIC_ARRIVED
    case RES_LINKED_GRAPHIC_STREAM_ARRIVED:
    // <--
		if ( GetNode()->GetNodeType() == ND_GRFNODE )
		{
			bComplete = sal_False;
			SwGrfNode* pNd = (SwGrfNode*) GetNode();

			CLEARCACHE( pNd )

			SwRect aRect( Frm() );

			ViewShell *pVSh = 0;
			pNd->GetDoc()->GetEditShell( &pVSh );
            if( !pVSh )
				break;

			ViewShell *pSh = pVSh;
			do {
				SET_CURR_SHELL( pSh );
				if( pSh->IsPreView() )
				{
					if( pSh->GetWin() )
						::RepaintPagePreview( pSh, aRect );
				}
                else if ( pSh->VisArea().IsOver( aRect ) &&
                   OUTDEV_WINDOW == pSh->GetOut()->GetOutDevType() )
				{
                    // OD 27.11.2002 #105519# - invalidate instead of painting
                    pSh->GetWin()->Invalidate( aRect.SVRect() );
				}

				pSh = (ViewShell *)pSh->GetNext();
			} while( pSh != pVSh );
		}
		break;

	default:
        if ( !pNew || !isGRFATR(nWhich) )
			return;
	}

	if( bComplete )
	{
		InvalidatePrt();
		SetCompletePaint();
	}
}

void lcl_correctlyAlignRect( SwRect& rAlignedGrfArea, const SwRect& rInArea, OutputDevice* pOut )
{

    if(!pOut)
        return;
    Rectangle aPxRect = pOut->LogicToPixel( rInArea.SVRect() );
    Rectangle aNewPxRect( aPxRect );
    while( aNewPxRect.Left() < aPxRect.Left() )
    {
        rAlignedGrfArea.Left( rAlignedGrfArea.Left()+1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Top() < aPxRect.Top() )
    {
        rAlignedGrfArea.Top( rAlignedGrfArea.Top()+1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Bottom() > aPxRect.Bottom() )
    {
        rAlignedGrfArea.Bottom( rAlignedGrfArea.Bottom()-1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
    while( aNewPxRect.Right() > aPxRect.Right() )
    {
        rAlignedGrfArea.Right( rAlignedGrfArea.Right()-1 );
        aNewPxRect = pOut->LogicToPixel( rAlignedGrfArea.SVRect() );
    }
}

bool paintUsingPrimitivesHelper(
    OutputDevice& rOutputDevice,
    const drawinglayer::primitive2d::Primitive2DSequence& rSequence,
    const basegfx::B2DRange& rSourceRange,
    const basegfx::B2DRange& rTargetRange)
{
    if(rSequence.hasElements() && !basegfx::fTools::equalZero(rSourceRange.getWidth()) && !basegfx::fTools::equalZero(rSourceRange.getHeight()))
    {
        if(!basegfx::fTools::equalZero(rTargetRange.getWidth()) && !basegfx::fTools::equalZero(rTargetRange.getHeight()))
        {
            // map graphic range to target range. This will e.g. automatically include
            // tme mapping from 1/100th mm content to twips if needed when the target
            // range is defined in twips
            const basegfx::B2DHomMatrix aMappingTransform(
                basegfx::tools::createSourceRangeTargetRangeTransform(
                    rSourceRange,
                    rTargetRange));

            // Fill ViewInformation. Use MappingTransform here, so there is no need to
            // embed the primitives to it. Use original TargetRange here so there is also
            // no need to embed the primitives to a MaskPrimitive for cropping. This works 
            // only in this case where the graphic object cannot be rotated, though.
            const drawinglayer::geometry::ViewInformation2D aViewInformation2D(
                aMappingTransform, 
                rOutputDevice.GetViewTransformation(), 
                rTargetRange, 
                0,
                0.0, 
                uno::Sequence< beans::PropertyValue >());

            // get a primitive processor for rendering
            drawinglayer::processor2d::BaseProcessor2D* pProcessor2D = drawinglayer::processor2d::createProcessor2DFromOutputDevice(
                rOutputDevice, 
                aViewInformation2D);

            if(pProcessor2D)
            {
                // render and cleanup
                pProcessor2D->process(rSequence);
                delete pProcessor2D;
                return true;
            }
        }
    }

    return false;
}

// Ausgabe der Grafik. Hier wird entweder eine QuickDraw-Bmp oder
// eine Grafik vorausgesetzt. Ist nichts davon vorhanden, wird
// eine Ersatzdarstellung ausgegeben.
/// OD 25.09.2002 #99739# - delete unused 3rd parameter.
/// OD 25.09.2002 #99739# - use aligned rectangle for drawing graphic.
/// OD 25.09.2002 #99739# - pixel-align coordinations for drawing graphic.
void SwNoTxtFrm::PaintPicture( OutputDevice* pOut, const SwRect &rGrfArea ) const
{
	ViewShell* pShell = getRootFrm()->GetCurrShell();

	SwNoTxtNode& rNoTNd = *(SwNoTxtNode*)GetNode();
	SwGrfNode* pGrfNd = rNoTNd.GetGrfNode();
	SwOLENode* pOLENd = rNoTNd.GetOLENode();

    const sal_Bool bPrn = pOut == rNoTNd.getIDocumentDeviceAccess()->getPrinter( false ) ||
                          pOut->GetConnectMetaFile();

    const bool bIsChart = pOLENd && ChartHelper::IsChart( pOLENd->GetOLEObj().GetObject() );

    /// OD 25.09.2002 #99739# - calculate aligned rectangle from parameter <rGrfArea>.
    ///     Use aligned rectangle <aAlignedGrfArea> instead of <rGrfArea> in
    ///     the following code.
    SwRect aAlignedGrfArea = rGrfArea;
    ::SwAlignRect( aAlignedGrfArea,  pShell );

    if( !bIsChart )
    {
        /// OD 25.09.2002 #99739#
        /// Because for drawing a graphic left-top-corner and size coordinations are
        /// used, these coordinations have to be determined on pixel level.
        ::SwAlignGrfRect( &aAlignedGrfArea, *pOut );
    }
    else //if( bIsChart )
    {
        //#i78025# charts own borders are not completely visible
        //the above pixel correction is not correct - at least not for charts
        //so a different pixel correction is chosen here
        //this might be a good idea for all other OLE objects also,
        //but as I cannot oversee the consequences I fix it only for charts for now
        lcl_correctlyAlignRect( aAlignedGrfArea, rGrfArea, pOut );
    }

    if( pGrfNd )
	{
		sal_Bool bForceSwap = sal_False, bContinue = sal_True;
		const GraphicObject& rGrfObj = pGrfNd->GetGrfObj();

		GraphicAttr aGrfAttr;
		pGrfNd->GetGraphicAttr( aGrfAttr, this );

		if( !bPrn )
		{
            // --> OD 2007-01-02 #i73788#
            if ( pGrfNd->IsLinkedInputStreamReady() )
            {
                pGrfNd->UpdateLinkWithInputStream();
            }
            // <--
            // --> OD 2008-01-30 #i85717#
            // --> OD 2008-07-21 #i90395# - check, if asynchronous retrieval
            // if input stream for the graphic is possible
//            else if( GRAPHIC_DEFAULT == rGrfObj.GetType() &&
            else if ( ( rGrfObj.GetType() == GRAPHIC_DEFAULT ||
                        rGrfObj.GetType() == GRAPHIC_NONE ) &&
                      pGrfNd->IsLinkedFile() &&
                      pGrfNd->IsAsyncRetrieveInputStreamPossible() )
            // <--
			{
				Size aTmpSz;
                ::sfx2::SvLinkSource* pGrfObj = pGrfNd->GetLink()->GetObj();
				if( !pGrfObj ||
					!pGrfObj->IsDataComplete() ||
					!(aTmpSz = pGrfNd->GetTwipSize()).Width() ||
					!aTmpSz.Height() || !pGrfNd->GetAutoFmtLvl() )
				{
                    // --> OD 2006-12-22 #i73788#
                    pGrfNd->TriggerAsyncRetrieveInputStream();
                    // <--
				}
                String aTxt( pGrfNd->GetTitle() );
				if ( !aTxt.Len() )
					GetRealURL( *pGrfNd, aTxt );
                ::lcl_PaintReplacement( aAlignedGrfArea, aTxt, *pShell, this, sal_False );
				bContinue = sal_False;
			}
		}

		if( bContinue )
		{
			const sal_Bool bSwapped = rGrfObj.IsSwappedOut();
			const sal_Bool bSwappedIn = 0 != pGrfNd->SwapIn( bPrn );
			if( bSwappedIn && rGrfObj.GetGraphic().IsSupportedGraphic())
			{
                const sal_Bool bAnimate = rGrfObj.IsAnimated() &&
                                         !pShell->IsPreView() &&
                                         !pShell->GetAccessibilityOptions()->IsStopAnimatedGraphics() &&
                // --> FME 2004-06-21 #i9684# Stop animation during printing/pdf export
                                          pShell->GetWin();
                // <--

				if( bAnimate &&
					FindFlyFrm() != ::GetFlyFromMarked( 0, pShell ))
				{
					OutputDevice* pVout;
					if( pOut == pShell->GetOut() && SwRootFrm::FlushVout() )
						pVout = pOut, pOut = pShell->GetOut();
					else if( pShell->GetWin() &&
							 OUTDEV_VIRDEV == pOut->GetOutDevType() )
						pVout = pOut, pOut = pShell->GetWin();
					else
						pVout = 0;

                    ASSERT( OUTDEV_VIRDEV != pOut->GetOutDevType() ||
                            pShell->GetViewOptions()->IsPDFExport(),
                            "pOut sollte kein virtuelles Device sein" );

                    pGrfNd->StartGraphicAnimation(pOut, aAlignedGrfArea.Pos(),
                                        aAlignedGrfArea.SSize(), long(this),
										0, GRFMGR_DRAW_STANDARD, pVout );
				}
                else
                {
                    // unify using GraphicPrimitive2D
                    // -> the primitive handles all crop and mirror stuff
                    // -> the primitive renderer will create the needed pdf export data
                    // -> if bitmap conent, it will be cached system-dependent
                    const basegfx::B2DRange aTargetRange(
                        aAlignedGrfArea.Left(), aAlignedGrfArea.Top(), 
                        aAlignedGrfArea.Right(), aAlignedGrfArea.Bottom());
                    const basegfx::B2DHomMatrix aTargetTransform(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            aTargetRange.getRange(),
                            aTargetRange.getMinimum()));
                    drawinglayer::primitive2d::Primitive2DSequence aContent(1);
                    bool bDone(false);

                    // #i125171# The mechanism to get lossless jpegs into pdf is based on having the original
                    // file data (not the bitmap data) at the Graphic in the GfxLink (which has *nothing* to
                    // do with the graphic being linked). This works well for DrawingLayer GraphicObjects (linked
                    // and unlinked) but fails for linked Writer GraphicObjects. These have the URL in the
                    // GraphicObject, but no GfxLink with the original file data when it's a linked graphic.
                    // Since this blows up PDF size by a factor of 10 (the graphics get embedded as pixel maps
                    // then) it is okay to add this workarund: In the needed case, load the graphic in a way to
                    // get the GfxLink in the needed form and use that Graphic temporarily. Do this only when
                    // - we have PDF export
                    // - the GraphicObject is linked
                    // - the Graphic has no GfxLink
                    // - LosslessCompression is activated
                    // - it's indeed a jpeg graphic (could be checked by the url ending, but is more reliable to check later)
                    // In all other cases (normal repaint, print, etc...) use the available Graphic with the
                    // already loaded pixel graphic as before this change.
                    if(pOut->GetExtOutDevData() && rGrfObj.HasLink() && !rGrfObj.GetGraphic().IsLink())
                    {
                        const vcl::PDFExtOutDevData* pPDFExt = dynamic_cast< const vcl::PDFExtOutDevData* >(pOut->GetExtOutDevData());

                        if(pPDFExt && pPDFExt->GetIsLosslessCompression())
                        {
                            Graphic aTempGraphic;
                            INetURLObject aURL(rGrfObj.GetLink());

                            if(GRFILTER_OK == GraphicFilter::GetGraphicFilter()->ImportGraphic(aTempGraphic, aURL))
                            {
                                if(aTempGraphic.IsLink() && GFX_LINK_TYPE_NATIVE_JPG == aTempGraphic.GetLink().GetType())
                                {
                                    aContent[0] = new drawinglayer::primitive2d::GraphicPrimitive2D(
                                        aTargetTransform,
                                        aTempGraphic,
                                        aGrfAttr);
                                    bDone = true;
                                }
                            }
                        }
                    }

                    if(!bDone)
                    {
                        aContent[0] = new drawinglayer::primitive2d::GraphicPrimitive2D(
                            aTargetTransform,
                            rGrfObj.GetGraphic(),
                            aGrfAttr);
                    }

                    paintUsingPrimitivesHelper(
                        *pOut,
                        aContent,
                        aTargetRange,
                        aTargetRange);

                    // need to reset the timer manually (was in original paints at GraphicManager)
                    rGrfObj.restartSwapOutTimer();
                }
			}
			else
			{
				sal_uInt16 nResId = 0;
				if( bSwappedIn )
				{
					if( GRAPHIC_NONE == rGrfObj.GetType() )
						nResId = STR_COMCORE_READERROR;
					else if ( !rGrfObj.GetGraphic().IsSupportedGraphic() )
						nResId = STR_COMCORE_CANT_SHOW;
				}
				((SwNoTxtFrm*)this)->nWeight = -1;
				String aText;
				if ( !nResId &&
                     !(aText = pGrfNd->GetTitle()).Len() &&
					 (!GetRealURL( *pGrfNd, aText ) || !aText.Len()))
				{
					nResId = STR_COMCORE_READERROR;
				}
				if ( nResId )
					aText = SW_RESSTR( nResId );

                ::lcl_PaintReplacement( aAlignedGrfArea, aText, *pShell, this, sal_True );
			}

			//Beim Drucken duerfen wir nicht die Grafiken sammeln...
			if( bSwapped && bPrn )
				bForceSwap = sal_True;
		}
		if( bForceSwap )
			pGrfNd->SwapOut();
	}
    else // bIsChart || pOLENd
    {
        // --> OD 2009-03-05 #i99665#
        // Adjust AntiAliasing mode at output device for chart OLE
        const sal_uInt16 nFormerAntialiasingAtOutput( pOut->GetAntialiasing() );
        if ( pOLENd->IsChart() &&
                pShell->Imp()->GetDrawView()->IsAntiAliasing() )
        {
            const sal_uInt16 nAntialiasingForChartOLE =
                    nFormerAntialiasingAtOutput | ANTIALIASING_PIXELSNAPHAIRLINE;
            pOut->SetAntialiasing( nAntialiasingForChartOLE );
        }
        // <--

        bool bDone(false);

        if(bIsChart)
        {
            const uno::Reference< frame::XModel > aXModel(pOLENd->GetOLEObj().GetOleRef()->getComponent(), uno::UNO_QUERY);

            if(aXModel.is())
            {
                basegfx::B2DRange aSourceRange;

                const drawinglayer::primitive2d::Primitive2DSequence aSequence(
                    ChartHelper::tryToGetChartContentAsPrimitive2DSequence(
                        aXModel,
                        aSourceRange));

                if(aSequence.hasElements() && !aSourceRange.isEmpty())
                {
                    const basegfx::B2DRange aTargetRange(
                        aAlignedGrfArea.Left(), aAlignedGrfArea.Top(), 
                        aAlignedGrfArea.Right(), aAlignedGrfArea.Bottom());

                    bDone = paintUsingPrimitivesHelper(
                        *pOut,
                        aSequence,
                        aSourceRange,
                        aTargetRange);
                }
            }
        }

        if(!bDone && pOLENd)
	    {
            Point aPosition(aAlignedGrfArea.Pos());
            Size aSize(aAlignedGrfArea.SSize());

            // Im BrowseModus gibt es nicht unbedingt einen Drucker und
            // damit kein JobSetup, also legen wir eines an ...
            const JobSetup* pJobSetup = pOLENd->getIDocumentDeviceAccess()->getJobsetup();
            sal_Bool bDummyJobSetup = 0 == pJobSetup;
            if( bDummyJobSetup )
                pJobSetup = new JobSetup();

		    // #i42323#
		    // The reason for #114233# is gone, so i remove it again
            //TODO/LATER: is it a problem that the JopSetup isn't used?
            //xRef->DoDraw( pOut, aAlignedGrfArea.Pos(), aAlignedGrfArea.SSize(), *pJobSetup );

            // get hi-contrast image, but never for printing
		    Graphic* pGraphic = NULL;
            if (pOut && !bPrn && Application::GetSettings().GetStyleSettings().GetHighContrastMode() )
			    pGraphic = pOLENd->GetHCGraphic();

		    // when it is not possible to get HC-representation, the original image should be used
		    if ( !pGraphic )
           	    pGraphic = pOLENd->GetGraphic();

            if ( pGraphic && pGraphic->GetType() != GRAPHIC_NONE )
		    {
			    pGraphic->Draw( pOut, aPosition, aSize );

			    // shade the representation if the object is activated outplace
			    uno::Reference < embed::XEmbeddedObject > xObj = pOLENd->GetOLEObj().GetOleRef();
			    if ( xObj.is() && xObj->getCurrentState() == embed::EmbedStates::ACTIVE )
			    {
				    ::svt::EmbeddedObjectRef::DrawShading( Rectangle( aPosition, aSize ), pOut );
			    }
		    }
            else
                ::svt::EmbeddedObjectRef::DrawPaintReplacement( Rectangle( aPosition, aSize ), pOLENd->GetOLEObj().GetCurrentPersistName(), pOut );

            if( bDummyJobSetup )
                delete pJobSetup;  // ... und raeumen wieder auf.

            sal_Int64 nMiscStatus = pOLENd->GetOLEObj().GetOleRef()->getStatus( pOLENd->GetAspect() );
            if ( !bPrn && pShell->ISA( SwCrsrShell ) &&
                    nMiscStatus & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE )
            {
                const SwFlyFrm *pFly = FindFlyFrm();
                ASSERT( pFly, "OLE not in FlyFrm" );
                ((SwFEShell*)pShell)->ConnectObj( pOLENd->GetOLEObj().GetObject(), pFly->Prt(), pFly->Frm());
            }
        }

        // --> OD 2009-03-05 #i99665#
        if ( pOLENd->IsChart() &&
                pShell->Imp()->GetDrawView()->IsAntiAliasing() )
        {
            pOut->SetAntialiasing( nFormerAntialiasingAtOutput );
        }
        // <--
    }
}


sal_Bool SwNoTxtFrm::IsTransparent() const
{
	const ViewShell* pSh = getRootFrm()->GetCurrShell();
	if ( !pSh || !pSh->GetViewOptions()->IsGraphic() )
		return sal_True;

	const SwGrfNode *pNd;
	if( 0 != (pNd = GetNode()->GetGrfNode()) )
		return pNd->IsTransparent();

	//#29381# OLE sind immer Transparent.
	return sal_True;
}


void SwNoTxtFrm::StopAnimation( OutputDevice* pOut ) const
{
	//animierte Grafiken anhalten
	const SwGrfNode* pGrfNd = dynamic_cast< const SwGrfNode* >(GetNode()->GetGrfNode());

    if( pGrfNd && pGrfNd->IsAnimated() )
    {
		const_cast< SwGrfNode* >(pGrfNd)->StopGraphicAnimation( pOut, long(this) );
    }
}


sal_Bool SwNoTxtFrm::HasAnimation() const
{
	const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();
	return pGrfNd && pGrfNd->IsAnimated();
}



