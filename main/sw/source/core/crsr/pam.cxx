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
#include <editeng/protitem.hxx>
#include <cntfrm.hxx>
#include <pagefrm.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <pamtyp.hxx>
#include <txtfrm.hxx>
#include <section.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <swtable.hxx>
#include <crsskip.hxx>

// --> FME 2004-06-29 #114856# Formular view
#include <flyfrm.hxx>
#include <fmteiro.hxx>
#include <section.hxx>
#include <sectfrm.hxx>
// <--
#include <ndtxt.hxx> // #111827#

#include <IMark.hxx>
#include <hints.hxx>

// fuer den dummen ?MSC-? Compiler
inline xub_StrLen GetSttOrEnd( sal_Bool bCondition, const SwCntntNode& rNd )
{
    return bCondition ? 0 : rNd.Len();
}

/*************************************************************************
|*
|*  SwPosition
|*
|*  Beschreibung        PAM.DOC
|*  Ersterstellung      VB  4.3.91
|*  Letzte Aenderung    VB  4.3.91
|*
*************************************************************************/


SwPosition::SwPosition( const SwNodeIndex & rNodeIndex, const SwIndex & rCntnt )
    : nNode( rNodeIndex ), nContent( rCntnt )
{
}

SwPosition::SwPosition( const SwNodeIndex & rNodeIndex )
    : nNode( rNodeIndex ), nContent( nNode.GetNode().GetCntntNode() )
{
}

SwPosition::SwPosition( const SwNode& rNode )
    : nNode( rNode ), nContent( nNode.GetNode().GetCntntNode() )
{
}

SwPosition::SwPosition( SwCntntNode & rNode, const xub_StrLen nOffset )
    : nNode( rNode ), nContent( &rNode, nOffset )
{
}


SwPosition::SwPosition( const SwPosition & rPos )
    : nNode( rPos.nNode ), nContent( rPos.nContent )
{
}

SwPosition &SwPosition::operator=(const SwPosition &rPos)
{
    nNode = rPos.nNode;
    nContent = rPos.nContent;
    return *this;
}


sal_Bool SwPosition::operator<(const SwPosition &rPos) const
{
    if( nNode < rPos.nNode )
        return sal_True;
    if( nNode == rPos.nNode )
        return ( nContent < rPos.nContent );
    return sal_False;
}


sal_Bool SwPosition::operator>(const SwPosition &rPos) const
{
    if(nNode > rPos.nNode )
        return sal_True;
    if( nNode == rPos.nNode )
        return ( nContent > rPos.nContent );
    return sal_False;
}


sal_Bool SwPosition::operator<=(const SwPosition &rPos) const
{
    if(nNode < rPos.nNode )
        return sal_True;
    if( nNode == rPos.nNode )
        return ( nContent <= rPos.nContent );
    return sal_False;
}


sal_Bool SwPosition::operator>=(const SwPosition &rPos) const
{
    if(nNode > rPos.nNode )
        return sal_True;
    if( nNode == rPos.nNode )
        return ( nContent >= rPos.nContent );
    return sal_False;
}


sal_Bool SwPosition::operator==(const SwPosition &rPos) const
{
    return
        ( ( nNode == rPos.nNode ) && ( nContent == rPos.nContent ) ?
            sal_True: sal_False);
}


sal_Bool SwPosition::operator!=(const SwPosition &rPos) const
{
    if( nNode != rPos.nNode )
        return sal_True;
    return ( nContent != rPos.nContent );
}

SwDoc * SwPosition::GetDoc() const
{
    return nNode.GetNode().GetDoc();
}

SwComparePosition ComparePosition(
            const SwPosition& rStt1, const SwPosition& rEnd1,
            const SwPosition& rStt2, const SwPosition& rEnd2 )
{
    SwComparePosition nRet;
    if( rStt1 < rStt2 )
    {
        if( rEnd1 > rStt2 )
        {
            if( rEnd1 >= rEnd2 )
                nRet = POS_OUTSIDE;
            else
                nRet = POS_OVERLAP_BEFORE;

        }
        else if( rEnd1 == rStt2 )
            nRet = POS_COLLIDE_END;
        else
            nRet = POS_BEFORE;
    }
    else if( rEnd2 > rStt1 )
    {
        if( rEnd2 >= rEnd1 )
        {
            if( rEnd2 == rEnd1 && rStt2 == rStt1 )
                nRet = POS_EQUAL;
            else
                nRet = POS_INSIDE;
        }
        else
        {
            if (rStt1 == rStt2)
                nRet = POS_OUTSIDE;
            else
                nRet = POS_OVERLAP_BEHIND;
        }
    }
    else if( rEnd2 == rStt1 )
        nRet = POS_COLLIDE_START;
    else
        nRet = POS_BEHIND;
    return nRet;
}

SwComparePosition ComparePosition(
            const unsigned long nStt1, const unsigned long nEnd1,
            const unsigned long nStt2, const unsigned long nEnd2 )
{
    SwComparePosition nRet;
    if( nStt1 < nStt2 )
    {
        if( nEnd1 > nStt2 )
        {
            if( nEnd1 >= nEnd2 )
                nRet = POS_OUTSIDE;
            else
                nRet = POS_OVERLAP_BEFORE;

        }
        else if( nEnd1 == nStt2 )
            nRet = POS_COLLIDE_END;
        else
            nRet = POS_BEFORE;
    }
    else if( nEnd2 > nStt1 )
    {
        if( nEnd2 >= nEnd1 )
        {
            if( nEnd2 == nEnd1 && nStt2 == nStt1 )
                nRet = POS_EQUAL;
            else
                nRet = POS_INSIDE;
        }
        else
        {
            if (nStt1 == nStt2)
                nRet = POS_OUTSIDE;
            else
                nRet = POS_OVERLAP_BEHIND;
        }
    }
    else if( nEnd2 == nStt1 )
        nRet = POS_COLLIDE_START;
    else
        nRet = POS_BEHIND;
    return nRet;
}

/*  */

enum CHKSECTION { Chk_Both, Chk_One, Chk_None };


CHKSECTION lcl_TstIdx( sal_uLong nSttIdx, sal_uLong nEndIdx, const SwNode& rEndNd )
{
    sal_uLong nStt = rEndNd.StartOfSectionIndex(), nEnd = rEndNd.GetIndex();
    CHKSECTION eSec = nStt < nSttIdx && nEnd >= nSttIdx ? Chk_One : Chk_None;
    if( nStt < nEndIdx && nEnd >= nEndIdx )
        return( eSec == Chk_One ? Chk_Both : Chk_One );
    return eSec;
}


sal_Bool lcl_ChkOneRange( CHKSECTION eSec, sal_Bool bChkSections,
                    const SwNode& rBaseEnd, sal_uLong nStt, sal_uLong nEnd )
{
    if( eSec != Chk_Both )
        return sal_False;

    if( !bChkSections )
        return sal_True;

    // suche die umspannende Section
    const SwNodes& rNds = rBaseEnd.GetNodes();
    const SwNode *pTmp, *pNd = rNds[ nStt ];
    if( !pNd->IsStartNode() )
        pNd = pNd->StartOfSectionNode();

    if( pNd == rNds[ nEnd ]->StartOfSectionNode() )
        return sal_True;        // der gleiche StartNode, die selbe Section

    // steht schon auf einem GrundSection Node ? Fehler !!!
    if( !pNd->StartOfSectionIndex() )
        return sal_False;

    while( ( pTmp = pNd->StartOfSectionNode())->EndOfSectionNode() !=
            &rBaseEnd )
        pNd = pTmp;

    sal_uLong nSttIdx = pNd->GetIndex(), nEndIdx = pNd->EndOfSectionIndex();
    return nSttIdx <= nStt && nStt <= nEndIdx &&
           nSttIdx <= nEnd && nEnd <= nEndIdx ? sal_True : sal_False;
}


sal_Bool CheckNodesRange( const SwNodeIndex& rStt,
                        const SwNodeIndex& rEnd, sal_Bool bChkSection )
{
    const SwNodes& rNds = rStt.GetNodes();
    sal_uLong nStt = rStt.GetIndex(), nEnd = rEnd.GetIndex();
    CHKSECTION eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfContent() );
    if( Chk_None != eSec ) return eSec == Chk_Both ? sal_True : sal_False;

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfAutotext() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfAutotext(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfPostIts() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfPostIts(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfInserts() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfInserts(), nStt, nEnd );

    eSec = lcl_TstIdx( nStt, nEnd, rNds.GetEndOfRedlines() );
    if( Chk_None != eSec )
        return lcl_ChkOneRange( eSec, bChkSection,
                            rNds.GetEndOfRedlines(), nStt, nEnd );

    return sal_False;       // liegt irgendwo dazwischen, FEHLER
}


sal_Bool GoNext(SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode )
{
    if( pNd->IsCntntNode() )
        return ((SwCntntNode*)pNd)->GoNext( pIdx, nMode );
    return sal_False;
}


sal_Bool GoPrevious( SwNode* pNd, SwIndex * pIdx, sal_uInt16 nMode )
{
    if( pNd->IsCntntNode() )
        return ((SwCntntNode*)pNd)->GoPrevious( pIdx, nMode );
    return sal_False;
}


SwCntntNode* GoNextNds( SwNodeIndex* pIdx, sal_Bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwCntntNode* pNd = aIdx.GetNodes().GoNext( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != aIdx.GetIndex() - pIdx->GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, sal_True ) )
                pNd = 0;
        else
            *pIdx = aIdx;
    }
    return pNd;
}


SwCntntNode* GoPreviousNds( SwNodeIndex * pIdx, sal_Bool bChk )
{
    SwNodeIndex aIdx( *pIdx );
    SwCntntNode* pNd = aIdx.GetNodes().GoPrevious( &aIdx );
    if( pNd )
    {
        if( bChk && 1 != pIdx->GetIndex() - aIdx.GetIndex() &&
            !CheckNodesRange( *pIdx, aIdx, sal_True ) )
                pNd = 0;
        else
            *pIdx = aIdx;
    }
    return pNd;
}

// ----------------------------------------------------------------------

/*************************************************************************
|*
|*  SwPointAndMark
|*
|*  Beschreibung        PAM.DOC
|*  Ersterstellung      VB  4.3.91
|*  Letzte Aenderung    JP  6.5.91
|*
*************************************************************************/

SwPaM::SwPaM( const SwPosition& rPos, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rPos )
    , m_Bound2( rPos.nNode.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( m_pPoint )
    , m_bIsInFrontOfLabel( false )
{
}

SwPaM::SwPaM( const SwPosition& rMark, const SwPosition& rPoint, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
}

SwPaM::SwPaM( const SwNodeIndex& rMark, const SwNodeIndex& rPoint,
              long nMarkOffset, long nPointOffset, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    if ( nMarkOffset )
    {
        m_pMark->nNode += nMarkOffset;
    }
    if ( nPointOffset )
    {
        m_pPoint->nNode += nPointOffset;
    }

    m_Bound1.nContent.Assign( m_Bound1.nNode.GetNode().GetCntntNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.nNode.GetNode().GetCntntNode(), 0 );
}

SwPaM::SwPaM( const SwNode& rMark, const SwNode& rPoint,
              long nMarkOffset, long nPointOffset, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    if ( nMarkOffset )
    {
        m_pMark->nNode += nMarkOffset;
    }
    if ( nPointOffset )
    {
        m_pPoint->nNode += nPointOffset;
    }

    m_Bound1.nContent.Assign( m_Bound1.nNode.GetNode().GetCntntNode(), 0 );
    m_Bound2.nContent.Assign( m_Bound2.nNode.GetNode().GetCntntNode(), 0 );
}

SwPaM::SwPaM( const SwNodeIndex& rMark , xub_StrLen nMarkCntnt,
              const SwNodeIndex& rPoint, xub_StrLen nPointCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( rPoint.GetNode().GetCntntNode(), nPointCntnt);
    m_pMark ->nContent.Assign( rMark .GetNode().GetCntntNode(), nMarkCntnt );
}

SwPaM::SwPaM( const SwNode& rMark , xub_StrLen nMarkCntnt,
              const SwNode& rPoint, xub_StrLen nPointCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rMark )
    , m_Bound2( rPoint )
    , m_pPoint( &m_Bound2 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->nNode.GetNode().GetCntntNode(),
        nPointCntnt);
    m_pMark ->nContent.Assign( m_pMark ->nNode.GetNode().GetCntntNode(),
        nMarkCntnt );
}

SwPaM::SwPaM( const SwNode& rNode, xub_StrLen nCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNode )
    , m_Bound2( m_Bound1.nNode.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( m_pPoint->nNode.GetNode().GetCntntNode(),
        nCntnt );
}

SwPaM::SwPaM( const SwNodeIndex& rNodeIdx, xub_StrLen nCntnt, SwPaM* pRing )
    : Ring( pRing )
    , m_Bound1( rNodeIdx )
    , m_Bound2( rNodeIdx.GetNode().GetNodes() ) // default initialize
    , m_pPoint( &m_Bound1 )
    , m_pMark( &m_Bound1 )
    , m_bIsInFrontOfLabel( false )
{
    m_pPoint->nContent.Assign( rNodeIdx.GetNode().GetCntntNode(), nCntnt );
}

SwPaM::~SwPaM() {}

// @@@ semantic: no copy ctor.
SwPaM::SwPaM( SwPaM &rPam )
    : Ring( &rPam )
    , m_Bound1( *(rPam.m_pPoint) )
    , m_Bound2( *(rPam.m_pMark)  )
    , m_pPoint( &m_Bound1 ), m_pMark( rPam.HasMark() ? &m_Bound2 : m_pPoint )
    , m_bIsInFrontOfLabel( false )
{
}

// @@@ semantic: no copy assignment for super class Ring.
SwPaM &SwPaM::operator=( const SwPaM &rPam )
{
    *m_pPoint = *( rPam.m_pPoint );
    if ( rPam.HasMark() )
    {
        SetMark();
        *m_pMark = *( rPam.m_pMark );
    }
    else
    {
        DeleteMark();
    }
    return *this;
}

void SwPaM::SetMark()
{
    if (m_pPoint == &m_Bound1)
    {
        m_pMark = &m_Bound2;
    }
    else
    {
        m_pMark = &m_Bound1;
    }
    (*m_pMark) = (*m_pPoint);
}

#ifdef DBG_UTIL

void SwPaM::Exchange()
{
    if (m_pPoint != m_pMark)
    {
        SwPosition *pTmp = m_pPoint;
        m_pPoint = m_pMark;
        m_pMark = pTmp;
    }
}
#endif

// Bewegen des Cursors


sal_Bool SwPaM::Move( SwMoveFn fnMove, SwGoInDoc fnGo )
{
    sal_Bool bRet = (*fnGo)( *this, fnMove );

    m_bIsInFrontOfLabel = false;

    return bRet;
}


/*************************************************************************
|*
|*    void SwPaM::MakeRegion( SwMoveFn, SwPaM*, const SwPaM* )
|*
|*    Beschreibung      Setzt den 1. SwPaM auf den uebergebenen SwPaM
|*                      oder setzt auf den Anfang oder Ende vom Document.
|*                      SPoint bleibt auf der Position stehen, GetMark aendert
|*                      sich entsprechend !
|*
|*    Parameter         SwDirection     gibt an, ob an Anfang / Ende
|*                      SwPaM *         der zu setzende Bereich
|*                      const SwPaM&    der enventuell vorgegeben Bereich
|*    Return-Werte      SwPaM*          der entsprehend neu gesetzte Bereich
|*
|*    Ersterstellung    JP 26.04.91
|*    Letzte Aenderung  JP 26.04.91
|*
*************************************************************************/


SwPaM* SwPaM::MakeRegion( SwMoveFn fnMove, const SwPaM * pOrigRg )
{
    SwPaM* pPam;
    if( pOrigRg == 0 )
    {
        pPam = new SwPaM( *m_pPoint );
        pPam->SetMark();                    // setze Anfang fest
        pPam->Move( fnMove, fnGoSection);       // an Anfang / Ende vom Node

        // stelle SPoint wieder auf alte Position, GetMark auf das "Ende"
        pPam->Exchange();
    }
    else
    {
        pPam = new SwPaM( *(SwPaM*)pOrigRg );   // die Suchregion ist vorgegeben
        // sorge dafuer, dass SPoint auf dem "echten" StartPunkt steht
        // FORWARD  --> SPoint immer kleiner  als GetMark
        // BACKWARD --> SPoint immer groesser als GetMark
        if( (pPam->GetMark()->*fnMove->fnCmpOp)( *pPam->GetPoint() ) )
            pPam->Exchange();
    }
    return pPam;
}

SwPaM & SwPaM::Normalize(sal_Bool bPointFirst)
{
    if (HasMark())
        if ( ( bPointFirst && *m_pPoint > *m_pMark) ||
             (!bPointFirst && *m_pPoint < *m_pMark) )
        {
            Exchange();
        }

    return *this;
}

sal_uInt16 SwPaM::GetPageNum( sal_Bool bAtPoint, const Point* pLayPos )
{
    // return die Seitennummer am Cursor
    // (fuer Reader + Seitengebundene Rahmen)
    const SwCntntFrm* pCFrm;
    const SwPageFrm *pPg;
    const SwCntntNode *pNd ;
    const SwPosition* pPos = bAtPoint ? m_pPoint : m_pMark;

    if( 0 != ( pNd = pPos->nNode.GetNode().GetCntntNode() ) &&
        0 != ( pCFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), pLayPos, pPos, sal_False )) &&
        0 != ( pPg = pCFrm->FindPageFrm() ))
        return pPg->GetPhyPageNum();
    return 0;
}

// --> FME 2004-06-29 #114856# Formular view
// See also SwCrsrShell::IsCrsrReadonly()
const SwFrm* lcl_FindEditInReadonlyFrm( const SwFrm& rFrm )
{
    const SwFrm* pRet = 0;

    const SwFlyFrm* pFly;
    const SwSectionFrm* pSectionFrm;

    if( rFrm.IsInFly() &&
       (pFly = rFrm.FindFlyFrm())->GetFmt()->GetEditInReadonly().GetValue() &&
        pFly->Lower() &&
       !pFly->Lower()->IsNoTxtFrm() )
    {
       pRet = pFly;
    }
    else if ( rFrm.IsInSct() &&
              0 != ( pSectionFrm = rFrm.FindSctFrm() )->GetSection() &&
              pSectionFrm->GetSection()->IsEditInReadonlyFlag() )
    {
        pRet = pSectionFrm;
    }

    return pRet;
}
// <--

// steht in etwas geschuetztem oder in die Selektion umspannt
// etwas geschuetztes.
sal_Bool SwPaM::HasReadonlySel( const bool bFormView ) const
{
    sal_Bool bRet = sal_False;

    const SwCntntNode* pNd = GetPoint()->nNode.GetNode().GetCntntNode();
    const SwCntntFrm *pFrm = NULL;
    if ( pNd != NULL )
    {
        Point aTmpPt;
        pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aTmpPt, GetPoint(), sal_False );
    }

    // Will be set if point are inside edit-in-readonly environment
    const SwFrm* pPointEditInReadonlyFrm = NULL;
    if ( pFrm != NULL
         && ( pFrm->IsProtected()
              || ( bFormView
                   && 0 == ( pPointEditInReadonlyFrm = lcl_FindEditInReadonlyFrm( *pFrm ) ) ) ) )
    {
        bRet = sal_True;
    }
    else if( pNd != NULL )
    {
        const SwSectionNode* pSNd = pNd->GetSectionNode();
        if ( pSNd != NULL
             && ( pSNd->GetSection().IsProtectFlag()
                  || ( bFormView
                       && !pSNd->GetSection().IsEditInReadonlyFlag()) ) )
        {
            bRet = sal_True;
        }
    }

    if ( !bRet
         && HasMark()
         && GetPoint()->nNode != GetMark()->nNode )
    {
        pNd = GetMark()->nNode.GetNode().GetCntntNode();
        pFrm = NULL;
        if ( pNd != NULL )
        {
            Point aTmpPt;
            pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aTmpPt, GetMark(), sal_False );
        }

        const SwFrm* pMarkEditInReadonlyFrm = NULL;
        if ( pFrm != NULL
             && ( pFrm->IsProtected()
                  || ( bFormView
                       && 0 == ( pMarkEditInReadonlyFrm = lcl_FindEditInReadonlyFrm( *pFrm ) ) ) ) )
        {
            bRet = sal_True;
        }
        else if( pNd != NULL )
        {
            const SwSectionNode* pSNd = pNd->GetSectionNode();
            if ( pSNd != NULL
                 && ( pSNd->GetSection().IsProtectFlag()
                      || ( bFormView
                           && !pSNd->GetSection().IsEditInReadonlyFlag()) ) )
            {
                bRet = sal_True;
            }
        }

        if ( !bRet && bFormView )
        {
           // Check if start and end frame are inside the _same_
           // edit-in-readonly-environment. Otherwise we better return 'true'
           if ( pPointEditInReadonlyFrm != pMarkEditInReadonlyFrm )
                bRet = sal_True;
        }

        // check for protected section inside the selection
        if( !bRet )
        {
            sal_uLong nSttIdx = GetMark()->nNode.GetIndex(),
                    nEndIdx = GetPoint()->nNode.GetIndex();
            if( nEndIdx <= nSttIdx )
            {
                sal_uLong nTmp = nSttIdx;
                nSttIdx = nEndIdx;
                nEndIdx = nTmp;
            }

            // wenn ein geschuetzter Bereich zwischen den Nodes stehen soll,
            // muss die Selektion selbst schon x Nodes umfassen.
            // (TxtNd, SectNd, TxtNd, EndNd, TxtNd )
            if( nSttIdx + 3 < nEndIdx )
            {
                const SwSectionFmts& rFmts = GetDoc()->GetSections();
                for( sal_uInt16 n = rFmts.Count(); n;  )
                {
                    const SwSectionFmt* pFmt = rFmts[ --n ];
                    if( pFmt->GetProtect().IsCntntProtected() )
                    {
                        const SwFmtCntnt& rCntnt = pFmt->GetCntnt(sal_False);
                        ASSERT( rCntnt.GetCntntIdx(), "wo ist der SectionNode?" );
                        sal_uLong nIdx = rCntnt.GetCntntIdx()->GetIndex();
                        if( nSttIdx <= nIdx && nEndIdx >= nIdx &&
                            rCntnt.GetCntntIdx()->GetNode().GetNodes().IsDocNodes() )
                        {
                            bRet = sal_True;
                            break;
                        }
                    }
                }

#ifdef CHECK_CELL_READONLY
//JP 22.01.99: bisher wurden Tabelle, die in der Text-Selektion standen
//              nicht beachtet. Wollte man das haben, dann muss dieser
//              Code freigeschaltet werden

                if( !bRet )
                {
                    // dann noch ueber alle Tabellen
                    const SwFrmFmts& rFmts = *GetDoc()->GetTblFrmFmts();
                    for( n = rFmts.Count(); n ;  )
                    {
                        SwFrmFmt* pFmt = (SwFrmFmt*)rFmts[ --n ];
                        const SwTable* pTbl = SwTable::FindTable( pFmt );
                        sal_uLong nIdx = pTbl ? pTbl->GetTabSortBoxes()[0]->GetSttIdx()
                                          : 0;
                        if( nSttIdx <= nIdx && nEndIdx >= nIdx )
                        {
                            // dann teste mal alle Boxen
                            const SwTableSortBoxes& rBoxes = pTbl->GetTabSortBoxes();

                            for( sal_uInt16 i =  rBoxes.Count(); i; )
                                if( rBoxes[ --i ]->GetFrmFmt()->GetProtect().
                                    IsCntntProtected() )
                                {
                                    bRet = sal_True;
                                    break;
                                }

                            if( bRet )
                                break;
                        }
                    }
                }
#endif
            }
        }
    }

    //FIXME FieldBk
    // TODO: Form Protection when Enhanced Fields are enabled
    if (!bRet)
    {
        const SwDoc *pDoc = GetDoc();
        sw::mark::IMark* pA = NULL;
        sw::mark::IMark* pB = NULL;
        if ( pDoc )
        {
            const IDocumentMarkAccess* pMarksAccess = pDoc->getIDocumentMarkAccess( );
            pA = GetPoint() ? pMarksAccess->getFieldmarkFor( *GetPoint( ) ) : NULL;
            pB = GetMark( ) ? pMarksAccess->getFieldmarkFor( *GetMark( ) ) : pA;
            bRet = ( pA != pB );
        }
        bool bProtectForm = pDoc->get( IDocumentSettingAccess::PROTECT_FORM );
        if ( bProtectForm )
            bRet |= ( pA == NULL || pB == NULL );
    }

    return bRet;
}

//--------------------  Suche nach Formaten( FormatNamen ) -----------------

// die Funktion gibt in Suchrichtung den folgenden Node zurueck.
// Ist in der Richtung keiner mehr vorhanden oder ist dieser ausserhalb
// des Bereiches, wird ein 0 Pointer returnt.
// Das rbFirst gibt an, ob es man zu erstenmal einen Node holt. Ist das der
// Fall, darf die Position vom Pam nicht veraendert werden!


SwCntntNode* GetNode( SwPaM & rPam, sal_Bool& rbFirst, SwMoveFn fnMove,
                        sal_Bool bInReadOnly )
{
    SwCntntNode * pNd = 0;
    SwCntntFrm* pFrm;
    if( ((*rPam.GetPoint()).*fnMove->fnCmpOp)( *rPam.GetMark() ) ||
        ( *rPam.GetPoint() == *rPam.GetMark() && rbFirst ) )
    {
        if( rbFirst )
        {
            rbFirst = sal_False;
            pNd = rPam.GetCntntNode();
            if( pNd )
            {
                if(
                    (
                        0 == ( pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout() ) ) ||
                        ( !bInReadOnly && pFrm->IsProtected() ) ||
                        (pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsHiddenNow())
                    ) ||
                    ( !bInReadOnly && pNd->FindSectionNode() &&
                        pNd->FindSectionNode()->GetSection().IsProtect()
                    )
                  )
                    {
                        pNd = 0;
                    }
            }
        }

        if( !pNd )          // steht Cursor auf keinem ContentNode ?
        {
            SwPosition aPos( *rPam.GetPoint() );
            sal_Bool bSrchForward = fnMove == fnMoveForward;
            SwNodes& rNodes = aPos.nNode.GetNodes();

            // zum naechsten / vorherigen ContentNode
// Funktioniert noch alles, wenn die Uerbpruefung vom ueberspringen der
// Sektions herausgenommen wird ??
//          if( (*fnMove->fnNds)( rNodes, &aPos.nNode ) )
            while( sal_True )
            {
                pNd = bSrchForward
                        ? rNodes.GoNextSection( &aPos.nNode, sal_True, !bInReadOnly )
                        : rNodes.GoPrevSection( &aPos.nNode, sal_True, !bInReadOnly );
                if( pNd )
                {
                    aPos.nContent.Assign( pNd, ::GetSttOrEnd( bSrchForward,*pNd ));
                    // liegt Position immer noch im Bereich ?
                    if( (aPos.*fnMove->fnCmpOp)( *rPam.GetMark() ) )
                    {
                        // nur in der AutoTextSection koennen Node stehen, die
                        // nicht angezeigt werden !!
                        if( 0 == ( pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout() ) ) ||
                            ( !bInReadOnly && pFrm->IsProtected() ) ||
                            ( pFrm->IsTxtFrm() &&
                                ((SwTxtFrm*)pFrm)->IsHiddenNow() ) )

//                          rNodes[ rNodes.EndOfAutotext ]->StartOfSection().GetIndex()
//                          < aPos.nNode.GetIndex() && aPos.nNode.GetIndex()
//                          < rNodes.EndOfAutotext.GetIndex() &&
//                          0 == ( pFrm = pNd->GetFrm()) &&
//                          pFrm->IsProtected() )
                        {
                            pNd = 0;
                            continue;       // suche weiter
                        }
                        *(SwPosition*)rPam.GetPoint() = aPos;
                    }
                    else
                        pNd = 0;            // kein gueltiger Node
                    break;
                }
                break;
            }
        }
    }
    return pNd;
}

// ----------------------------------------------------------------------

// hier folgen die Move-Methoden ( Foward, Backward; Content, Node, Doc )


void GoStartDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = *rNodes.GetEndOfContent().StartOfSectionNode();
    // es muss immer ein ContentNode gefunden werden !!
    SwCntntNode* pCNd = rNodes.GoNext( &pPos->nNode );
    if( pCNd )
        pCNd->MakeStartIndex( &pPos->nContent );
}


void GoEndDoc( SwPosition * pPos )
{
    SwNodes& rNodes = pPos->nNode.GetNodes();
    pPos->nNode = rNodes.GetEndOfContent();
    SwCntntNode* pCNd = GoPreviousNds( &pPos->nNode, sal_True );
    if( pCNd )
        pCNd->MakeEndIndex( &pPos->nContent );
}


void GoStartSection( SwPosition * pPos )
{
    // springe zum Anfang der Section
    SwNodes& rNodes = pPos->nNode.GetNodes();
    sal_uInt16 nLevel = rNodes.GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { rNodes.GoStartOfSection( &pPos->nNode ); } while( nLevel-- );

    // steht jetzt schon auf einem CntntNode
    pPos->nNode.GetNode().GetCntntNode()->MakeStartIndex( &pPos->nContent );
}

// gehe an das Ende der akt. Grund-Section


void GoEndSection( SwPosition * pPos )
{
    // springe zum Anfang/Ende der Section
    SwNodes& rNodes = pPos->nNode.GetNodes();
    sal_uInt16 nLevel = rNodes.GetSectionLevel( pPos->nNode );
    if( pPos->nNode < rNodes.GetEndOfContent().StartOfSectionIndex() )
        nLevel--;
    do { rNodes.GoEndOfSection( &pPos->nNode ); } while( nLevel-- );

    // steht jetzt auf einem EndNode, also zum vorherigen CntntNode
    if( GoPreviousNds( &pPos->nNode, sal_True ) )
        pPos->nNode.GetNode().GetCntntNode()->MakeEndIndex( &pPos->nContent );
}



sal_Bool GoInDoc( SwPaM & rPam, SwMoveFn fnMove )
{
    (*fnMove->fnDoc)( rPam.GetPoint() );
    return sal_True;
}


sal_Bool GoInSection( SwPaM & rPam, SwMoveFn fnMove )
{
    (*fnMove->fnSections)( (SwPosition*)rPam.GetPoint() );
    return sal_True;
}


sal_Bool GoInNode( SwPaM & rPam, SwMoveFn fnMove )
{
    SwCntntNode *pNd = (*fnMove->fnNds)( &rPam.GetPoint()->nNode, sal_True );
    if( pNd )
        rPam.GetPoint()->nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return 0 != pNd;
}


sal_Bool GoInCntnt( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                        &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS ))
        return sal_True;
    return GoInNode( rPam, fnMove );
}

sal_Bool GoInCntntCells( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                         &rPam.GetPoint()->nContent, CRSR_SKIP_CELLS ))
        return sal_True;
    return GoInNode( rPam, fnMove );
}

sal_Bool GoInCntntSkipHidden( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                        &rPam.GetPoint()->nContent, CRSR_SKIP_CHARS | CRSR_SKIP_HIDDEN ) )
        return sal_True;
    return GoInNode( rPam, fnMove );
}

sal_Bool GoInCntntCellsSkipHidden( SwPaM & rPam, SwMoveFn fnMove )
{
    if( (*fnMove->fnNd)( &rPam.GetPoint()->nNode.GetNode(),
                         &rPam.GetPoint()->nContent, CRSR_SKIP_CELLS | CRSR_SKIP_HIDDEN ) )
        return sal_True;
    return GoInNode( rPam, fnMove );
}



// --------- Funktionsdefinitionen fuer die SwCrsrShell --------------


sal_Bool GoPrevPara( SwPaM & rPam, SwPosPara aPosPara )
{
    if( rPam.Move( fnMoveBackward, fnGoNode ) )
    {
        // steht immer auf einem ContentNode !
        SwPosition& rPos = *rPam.GetPoint();
        SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
        rPos.nContent.Assign( pNd,
                            ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
        return sal_True;
    }
    return sal_False;
}


sal_Bool GoCurrPara( SwPaM & rPam, SwPosPara aPosPara )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
    if( pNd )
    {
        xub_StrLen nOld = rPos.nContent.GetIndex(),
                   nNew = aPosPara == fnMoveForward ? 0 : pNd->Len();
        // stand er schon auf dem Anfang/Ende dann zum naechsten/vorherigen
        if( nOld != nNew )
        {
            rPos.nContent.Assign( pNd, nNew );
            return sal_True;
        }
    }
    // den Node noch etwas bewegen ( auf den naechsten/vorh. CntntNode)
    if( ( aPosPara==fnParaStart && 0 != ( pNd =
            GoPreviousNds( &rPos.nNode, sal_True ))) ||
        ( aPosPara==fnParaEnd && 0 != ( pNd =
            GoNextNds( &rPos.nNode, sal_True ))) )
    {
        rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ));
        return sal_True;
    }
    return sal_False;
}


sal_Bool GoNextPara( SwPaM & rPam, SwPosPara aPosPara )
{
    if( rPam.Move( fnMoveForward, fnGoNode ) )
    {
        // steht immer auf einem ContentNode !
        SwPosition& rPos = *rPam.GetPoint();
        SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();
        rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( aPosPara == fnMoveForward, *pNd ) );
        return sal_True;
    }
    return sal_False;
}



sal_Bool GoCurrSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos );        // eine Vergleichsposition
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    (rNds.*fnMove->fnSection)( &rPos.nNode );
    SwCntntNode *pNd;
    if( 0 == ( pNd = rPos.nNode.GetNode().GetCntntNode()) &&
        0 == ( pNd = (*fnMove->fnNds)( &rPos.nNode, sal_True )) )
    {
        rPos = aSavePos;        // Cusror nicht veraendern
        return sal_False;
    }

    rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return aSavePos != rPos;
}


sal_Bool GoNextSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos );        // eine Vergleichsposition
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    rNds.GoEndOfSection( &rPos.nNode );

    // kein weiterer ContentNode vorhanden ?
    if( !GoInCntnt( rPam, fnMoveForward ) )
    {
        rPos = aSavePos;        // Cusror nicht veraendern
        return sal_False;
    }
    (rNds.*fnMove->fnSection)( &rPos.nNode );
    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();
    rPos.nContent.Assign( pNd,
                        ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ) );
    return sal_True;
}


sal_Bool GoPrevSection( SwPaM & rPam, SwMoveFn fnMove )
{
    SwPosition& rPos = *rPam.GetPoint();
    SwPosition aSavePos( rPos );        // eine Vergleichsposition
    SwNodes& rNds = aSavePos.nNode.GetNodes();
    rNds.GoStartOfSection( &rPos.nNode );

    // kein weiterer ContentNode vorhanden ?
    if( !GoInCntnt( rPam, fnMoveBackward ))
    {
        rPos = aSavePos;        // Cusror nicht veraendern
        return sal_False;
    }
    (rNds.*fnMove->fnSection)( &rPos.nNode );
    SwCntntNode *pNd = rPos.nNode.GetNode().GetCntntNode();
    rPos.nContent.Assign( pNd,
                            ::GetSttOrEnd( fnMove == fnMoveForward, *pNd ));
    return sal_True;
}

// #111827#
String SwPaM::GetTxt() const
{
    String aResult;

    SwNodeIndex aNodeIndex = Start()->nNode;

    /* The first node can be the end node. A first end node must be
       handled, too. There fore do ... while and no incrementing of
       aNodeIndex in the first pass.
     */
    bool bFirst = true;
    do
    {
        if (! bFirst)
        {
            aNodeIndex++;
        }

        bFirst = false;

        SwTxtNode * pTxtNode = aNodeIndex.GetNode().GetTxtNode();

        if (pTxtNode != NULL)
        {
            const String & aTmpStr = pTxtNode->GetTxt();

            if (aNodeIndex == Start()->nNode)
            {
                xub_StrLen nEnd;
                if (End()->nNode == aNodeIndex)
                    nEnd = End()->nContent.GetIndex();
                else
                    nEnd = aTmpStr.Len();

                aResult += aTmpStr.Copy(Start()->nContent.GetIndex(),
                                        nEnd - Start()->nContent.GetIndex()) ;
            }
            else if (aNodeIndex == End()->nNode)
                aResult += aTmpStr.Copy(0, End()->nContent.GetIndex());
            else
                aResult += aTmpStr;
        }
    }
    while (aNodeIndex != End()->nNode);

    return aResult;
}

sal_Bool SwPaM::Overlap(const SwPaM & a, const SwPaM & b)
{
    return !(*b.End() <= *a.Start() || *a.End() <= *b.End());
}

void SwPaM::InvalidatePaM()
{
    const SwNode *_pNd=this->GetNode();
    const SwTxtNode *_pTxtNd=(_pNd!=NULL?_pNd->GetTxtNode():NULL);
    if (_pTxtNd!=NULL)
    {
        // pretent that the PaM marks inserted text to recalc the portion...
        SwInsTxt aHint( Start()->nContent.GetIndex(),
                        End()->nContent.GetIndex() - Start()->nContent.GetIndex() + 1 );
        SwModify *_pModify=(SwModify*)_pTxtNd;
        _pModify->ModifyNotification( 0, &aHint);
    }
}

sal_Bool SwPaM::LessThan(const SwPaM & a, const SwPaM & b)
{
    return (*a.Start() < *b.Start()) || (*a.Start() == *b.Start() && *a.End() < *b.End());
}
