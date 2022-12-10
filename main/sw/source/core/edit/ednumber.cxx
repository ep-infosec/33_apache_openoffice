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
#include <editsh.hxx>
#include <edimp.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <swundo.hxx>
#include <numrule.hxx>

SV_IMPL_VARARR_SORT( _SwPamRanges, SwPamRange )


SwPamRanges::SwPamRanges( const SwPaM& rRing )
{
	const SwPaM* pTmp = &rRing;
	do {
		Insert( pTmp->GetMark()->nNode, pTmp->GetPoint()->nNode );
	} while( &rRing != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
}


void SwPamRanges::Insert( const SwNodeIndex& rIdx1, const SwNodeIndex& rIdx2 )
{
	SwPamRange aRg( rIdx1.GetIndex(), rIdx2.GetIndex() );
	if( aRg.nEnd < aRg.nStart )
	{	aRg.nStart = aRg.nEnd; aRg.nEnd = rIdx1.GetIndex(); }

	sal_uInt16 nPos = 0;
	const SwPamRange* pTmp;
	if( Count() && Seek_Entry( aRg, &nPos ))		// suche Insert Position
	{
		// ist der im Array stehende kleiner ??
		if( ( pTmp = GetData()+ nPos )->nEnd < aRg.nEnd )
		{
			aRg.nEnd = pTmp->nEnd;
			Remove( nPos, 1 );		// zusammenfassen
		}
		else
			return;		// ende, weil schon alle zusammengefasst waren
	}

	sal_Bool bEnde;
	do {
		bEnde = sal_True;

		// mit dem Vorgaenger zusammenfassen ??
		if( nPos > 0 )
		{
			if( ( pTmp = GetData()+( nPos-1 ))->nEnd == aRg.nStart
				|| pTmp->nEnd+1 == aRg.nStart )
			{
				aRg.nStart = pTmp->nStart;
				bEnde = sal_False;
				Remove( --nPos, 1 );		// zusammenfassen
			}
			// SSelection im Bereich ??
			else if( pTmp->nStart <= aRg.nStart && aRg.nEnd <= pTmp->nEnd )
				return;
		}
			// mit dem Nachfolger zusammenfassen ??
		if( nPos < Count() )
		{
			if( ( pTmp = GetData() + nPos )->nStart == aRg.nEnd ||
				pTmp->nStart == aRg.nEnd+1 )
			{
				aRg.nEnd = pTmp->nEnd;
				bEnde = sal_False;
				Remove( nPos, 1 );		// zusammenfassen
			}

			// SSelection im Bereich ??
			else if( pTmp->nStart <= aRg.nStart && aRg.nEnd <= pTmp->nEnd )
				return;
		}
	} while( !bEnde );

	_SwPamRanges::Insert( aRg );
}



SwPaM& SwPamRanges::SetPam( sal_uInt16 nArrPos, SwPaM& rPam )
{
	ASSERT_ID( nArrPos < Count(), ERR_VAR_IDX );
	const SwPamRange& rTmp = *(GetData() + nArrPos );
	rPam.GetPoint()->nNode = rTmp.nStart;
	rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
	rPam.SetMark();
	rPam.GetPoint()->nNode = rTmp.nEnd;
	rPam.GetPoint()->nContent.Assign( rPam.GetCntntNode(), 0 );
	return rPam;
}



// Numerierung Outline Regelwerk


void SwEditShell::SetOutlineNumRule(const SwNumRule& rRule)
{
	StartAllAction();		// Klammern fuers Updaten !!
	GetDoc()->SetOutlineNumRule(rRule);
	EndAllAction();
}


const SwNumRule* SwEditShell::GetOutlineNumRule() const
{
	return GetDoc()->GetOutlineNumRule();
}

// setzt, wenn noch keine Numerierung, sonst wird geaendert
// arbeitet mit alten und neuen Regeln, nur Differenzen aktualisieren

// Absaetze ohne Numerierung, aber mit Einzuegen

sal_Bool SwEditShell::NoNum()
{
	sal_Bool bRet = sal_True;
	StartAllAction();

	SwPaM* pCrsr = GetCrsr();
	if( pCrsr->GetNext() != pCrsr )			// Mehrfachselektion ?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( *pCrsr );
		SwPaM aPam( *pCrsr->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
			bRet = bRet && GetDoc()->NoNum( aRangeArr.SetPam( n, aPam ));
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
		bRet = GetDoc()->NoNum( *pCrsr );

	EndAllAction();
	return bRet;
}
// Loeschen, Splitten der Aufzaehlungsliste
sal_Bool SwEditShell::SelectionHasNumber() const
{
    sal_Bool bResult = HasNumber();
    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
	if (!bResult && pTxtNd && pTxtNd->Len()==0 && !pTxtNd->GetNumRule()) {
		SwPamRanges aRangeArr( *GetCrsr() );
		SwPaM aPam( *GetCrsr()->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
			{
					sal_uInt32 nStt = aPam.GetPoint()->nNode.GetIndex(),
						  nEnd = aPam.GetMark()->nNode.GetIndex();
					if( nStt > nEnd )
					{
						sal_uInt32 nTmp = nStt; nStt = nEnd; nEnd = nTmp;
					}
					for (sal_uInt32 nPos = nStt; nPos<=nEnd; nPos++) {
						SwTxtNode * pTxtNd = pDoc->GetNodes()[nPos]->GetTxtNode();
						if (pTxtNd && pTxtNd->Len()!=0)
						{
							bResult = pTxtNd->HasNumber();

							// --> OD 2005-10-26 #b6340308#
							// special case: outline numbered, not counted paragraph
							if ( bResult &&
								pTxtNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
								!pTxtNd->IsCountedInList() )
							{
								bResult = sal_False;
							}
							if (bResult==sal_False) {
								break;
							} 
							// <--
						}
					}
			}
        }

	}

    return bResult;
}
//add a new function to determine number on/off status
sal_Bool SwEditShell::SelectionHasBullet() const
{
    sal_Bool bResult = HasBullet();
    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();
	if (!bResult && pTxtNd && pTxtNd->Len()==0 && !pTxtNd->GetNumRule()) {
		SwPamRanges aRangeArr( *GetCrsr() );
		SwPaM aPam( *GetCrsr()->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
			{
					sal_uInt32 nStt = aPam.GetPoint()->nNode.GetIndex(),
						  nEnd = aPam.GetMark()->nNode.GetIndex();
					if( nStt > nEnd )
					{
						sal_uInt32 nTmp = nStt; nStt = nEnd; nEnd = nTmp;
					}
					for (sal_uInt32 nPos = nStt; nPos<=nEnd; nPos++) {
						SwTxtNode * pTxtNd = pDoc->GetNodes()[nPos]->GetTxtNode();
						if (pTxtNd && pTxtNd->Len()!=0)
						{
					        bResult = pTxtNd->HasBullet();

							if (bResult==sal_False) {
								break;
							} 
						}
					}
			}
        }

	}

    return bResult;
}
// -> #i29560#
sal_Bool SwEditShell::HasNumber() const
{
    sal_Bool bResult = sal_False;

    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        bResult = pTxtNd->HasNumber();

        // --> OD 2005-10-26 #b6340308#
        // special case: outline numbered, not counted paragraph
        if ( bResult &&
             pTxtNd->GetNumRule() == GetDoc()->GetOutlineNumRule() &&
             !pTxtNd->IsCountedInList() )
        {
            bResult = sal_False;
        }
        // <--
    }

    return bResult;
}

sal_Bool SwEditShell::HasBullet() const
{
    sal_Bool bResult = sal_False;

    const SwTxtNode * pTxtNd =
        GetCrsr()->GetPoint()->nNode.GetNode().GetTxtNode();

    if (pTxtNd)
    {
        bResult = pTxtNd->HasBullet();
    }

    return bResult;
}
// <- #i29560#

void SwEditShell::DelNumRules()
{
	StartAllAction();

	SwPaM* pCrsr = GetCrsr();
	if( pCrsr->GetNext() != pCrsr )			// Mehrfachselektion ?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( *pCrsr );
		SwPaM aPam( *pCrsr->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            GetDoc()->DelNumRules( aRangeArr.SetPam( n, aPam ) );
        }
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
        GetDoc()->DelNumRules( *pCrsr );

	// rufe das AttrChangeNotify auf der UI-Seite. Sollte eigentlich
	// ueberfluessig sein, aber VB hatte darueber eine Bugrep.
	CallChgLnk();

    // --> OD 2005-10-24 #126346# - cursor can not be anymore in
    // front of a label, because numbering/bullet is deleted.
    SetInFrontOfLabel( sal_False );
    // <--

	GetDoc()->SetModified();
	EndAllAction();
}

// Hoch-/Runterstufen


sal_Bool SwEditShell::NumUpDown( sal_Bool bDown )
{
	StartAllAction();

	sal_Bool bRet = sal_True;
	SwPaM* pCrsr = GetCrsr();
	if( pCrsr->GetNext() == pCrsr )			// keine Mehrfachselektion ?
		bRet = GetDoc()->NumUpDown( *pCrsr, bDown );
    else
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( *pCrsr );
		SwPaM aPam( *pCrsr->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
			bRet = bRet && GetDoc()->NumUpDown( aRangeArr.SetPam( n, aPam ), bDown );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
	GetDoc()->SetModified();

    // --> FME 2005-09-19 #i54693# Update marked numbering levels
    if ( IsInFrontOfLabel() )
        UpdateMarkedListLevel();
    // <--

    CallChgLnk();

	EndAllAction();
	return bRet;
}


bool SwEditShell::IsFirstOfNumRuleAtCrsrPos() const
{
    return GetDoc()->IsFirstOfNumRuleAtPos( *GetCrsr()->GetPoint() );
}


void SwEditShell::ChangeIndentOfAllListLevels( const short nDiff )
{
    StartAllAction();

    const SwNumRule *pCurNumRule = GetNumRuleAtCurrCrsrPos();
    if ( pCurNumRule != NULL )
    {
        SwNumRule aRule(*pCurNumRule);
        aRule.ChangeIndent( nDiff );

        SetCurNumRule( aRule, false );
    }

    EndAllAction();
}


void SwEditShell::SetIndent(
    short nIndent,
    const SwPosition & rPos )
{
    StartAllAction();

    SwNumRule *pCurNumRule = GetDoc()->GetNumRuleAtPos(rPos);

    if (pCurNumRule)
    {
        SwNumRule aRule(*pCurNumRule);
        if ( !IsMultiSelection() && IsFirstOfNumRuleAtCrsrPos() )
        {
            aRule.SetIndentOfFirstListLevelAndChangeOthers( nIndent );
        }
        else
        {
            const SwTxtNode* pTxtNode = rPos.nNode.GetNode().GetTxtNode();
            if ( pTxtNode != NULL
                 && pTxtNode->GetActualListLevel() >= 0 )
            {
                aRule.SetIndent( nIndent, static_cast< sal_uInt16 >( pTxtNode->GetActualListLevel() ) );
            }
        }

        // change numbering rule - changed numbering rule is not applied at <aPaM>
        SwPaM aPaM(rPos);
        GetDoc()->SetNumRule( aPaM, aRule, false, String(), false );
    }

    EndAllAction();
}

sal_Bool SwEditShell::MoveParagraph( long nOffset )
{
	StartAllAction();

	SwPaM *pCrsr = GetCrsr();
	if( !pCrsr->HasMark() )
	{
		// sorge dafuer, das Bound1 und Bound2 im gleichen Node stehen
		pCrsr->SetMark();
		pCrsr->DeleteMark();
	}

	sal_Bool bRet = GetDoc()->MoveParagraph( *pCrsr, nOffset );

	GetDoc()->SetModified();
	EndAllAction();
	return bRet;
}

//#outline level add by zhaojianwei
int SwEditShell::GetCurrentParaOutlineLevel( ) const
{
	int nLevel = 0;

	SwPaM* pCrsr = GetCrsr();
	const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();
	if( pTxtNd )
		nLevel = pTxtNd->GetAttrOutlineLevel();
	return nLevel;
}
//<-end,zhaojianwei

void SwEditShell::GetCurrentOutlineLevels( sal_uInt8& rUpper, sal_uInt8& rLower )
{
    SwPaM* pCrsr = GetCrsr();
	SwPaM aCrsr( *pCrsr->Start() );
	aCrsr.SetMark();
	if( pCrsr->HasMark() )
		*aCrsr.GetPoint() = *pCrsr->End();
    GetDoc()->GotoNextNum( *aCrsr.GetPoint(), sal_False,
                            &rUpper, &rLower );
}

sal_Bool SwEditShell::MoveNumParas( sal_Bool bUpperLower, sal_Bool bUpperLeft )
{
	StartAllAction();

	// auf alle Selektionen ??
	SwPaM* pCrsr = GetCrsr();
	SwPaM aCrsr( *pCrsr->Start() );
	aCrsr.SetMark();

	if( pCrsr->HasMark() )
		*aCrsr.GetPoint() = *pCrsr->End();

	sal_Bool bRet = sal_False;
	sal_uInt8 nUpperLevel, nLowerLevel;
	if( GetDoc()->GotoNextNum( *aCrsr.GetPoint(), sal_False,
								&nUpperLevel, &nLowerLevel ))
	{
		if( bUpperLower )
		{
			// ueber die naechste Nummerierung
			long nOffset = 0;
			const SwNode* pNd;

			if( bUpperLeft )		// verschiebe nach oben
			{
				SwPosition aPos( *aCrsr.GetMark() );
				if( GetDoc()->GotoPrevNum( aPos, sal_False ) )
					nOffset = aPos.nNode.GetIndex() -
							aCrsr.GetMark()->nNode.GetIndex();
				else
				{
					sal_uLong nStt = aPos.nNode.GetIndex(), nIdx = nStt - 1;
					while( nIdx && (
						( pNd = GetDoc()->GetNodes()[ nIdx ])->IsSectionNode() ||
                        ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode())))
						--nIdx;
					if( GetDoc()->GetNodes()[ nIdx ]->IsTxtNode() )
						nOffset = nIdx - nStt;
				}
			}
			else					// verschiebe nach unten
			{
				const SwNumRule* pOrig = aCrsr.GetNode(sal_False)->GetTxtNode()->GetNumRule();
				if( aCrsr.GetNode()->IsTxtNode() &&
					pOrig == aCrsr.GetNode()->GetTxtNode()->GetNumRule() )
				{
					sal_uLong nStt = aCrsr.GetPoint()->nNode.GetIndex(), nIdx = nStt+1;

                    while (nIdx < GetDoc()->GetNodes().Count()-1)
                    {
                        pNd = GetDoc()->GetNodes()[ nIdx ];

                        if (pNd->IsSectionNode() ||
                            ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode()) ||
                            ( pNd->IsTxtNode() && pOrig == ((SwTxtNode*)pNd)->GetNumRule() &&
                              ((SwTxtNode*)pNd)->GetActualListLevel() > nUpperLevel ))
                        {
                            ++nIdx;
                        }
                        // --> OD 2005-11-14 #i57856#
                        else
                        {
                            break;
                        }
                        // <--
                    }

					if( nStt == nIdx || !GetDoc()->GetNodes()[ nIdx ]->IsTxtNode() )
						nOffset = 1;
					else
						nOffset = nIdx - nStt;
				}
				else
					nOffset = 1;
			}

			if( nOffset )
			{
				aCrsr.Move( fnMoveBackward, fnGoNode );
				bRet = GetDoc()->MoveParagraph( aCrsr, nOffset );
			}
		}
		else if( bUpperLeft ? nUpperLevel : nLowerLevel+1 < MAXLEVEL )
		{
			aCrsr.Move( fnMoveBackward, fnGoNode );
			bRet = GetDoc()->NumUpDown( aCrsr, !bUpperLeft );
		}
	}

	GetDoc()->SetModified();
	EndAllAction();
	return bRet;
}

sal_Bool SwEditShell::OutlineUpDown( short nOffset )
{
	StartAllAction();

	sal_Bool bRet = sal_True;
	SwPaM* pCrsr = GetCrsr();
	if( pCrsr->GetNext() == pCrsr )			// keine Mehrfachselektion ?
		bRet = GetDoc()->OutlineUpDown( *pCrsr, nOffset );
    else
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( *pCrsr );
		SwPaM aPam( *pCrsr->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
			bRet = bRet && GetDoc()->OutlineUpDown(
									aRangeArr.SetPam( n, aPam ), nOffset );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
	GetDoc()->SetModified();
	EndAllAction();
	return bRet;
}


sal_Bool SwEditShell::MoveOutlinePara( short nOffset )
{
	StartAllAction();
	sal_Bool bRet = GetDoc()->MoveOutlinePara( *GetCrsr(), nOffset );
	EndAllAction();
	return bRet;
}

// Outlines and SubOutline are ReadOnly?
sal_Bool SwEditShell::IsProtectedOutlinePara() const
{
	sal_Bool bRet = sal_False;
	const SwNode& rNd = GetCrsr()->Start()->nNode.GetNode();
	if( rNd.IsTxtNode() )
	{
		const SwOutlineNodes& rOutlNd = GetDoc()->GetNodes().GetOutLineNds();
		SwNodePtr pNd = (SwNodePtr)&rNd;
		sal_Bool bFirst = sal_True;
		sal_uInt16 nPos;
        int nLvl(0);
		if( !rOutlNd.Seek_Entry( pNd, &nPos ) && nPos )
			--nPos;

		for( ; nPos < rOutlNd.Count(); ++nPos )
		{
            SwNodePtr pTmpNd = rOutlNd[ nPos ];

			// --> OD 2008-04-02 #refactorlists#
//            sal_uInt8 nTmpLvl = GetRealLevel( pTmpNd->GetTxtNode()->
//                                    GetTxtColl()->GetOutlineLevel() );
 //           int nTmpLvl = pTmpNd->GetTxtNode()->GetOutlineLevel();//#outline level,zhaojianwei
            int nTmpLvl = pTmpNd->GetTxtNode()->GetAttrOutlineLevel();
 //           ASSERT( nTmpLvl >= 0 && nTmpLvl < MAXLEVEL,
            ASSERT( nTmpLvl >= 0 && nTmpLvl <= MAXLEVEL,			//<-end,zhaojianwei
                    "<SwEditShell::IsProtectedOutlinePara()>" );
            // <--
			if( bFirst )
			{
				nLvl = nTmpLvl;
				bFirst = sal_False;
			}
			else if( nLvl >= nTmpLvl )
				break;

            if( pTmpNd->IsProtect() )
			{
				bRet = sal_True;
				break;
			}
		}
	}
#ifdef DBG_UTIL
	else
	{
		ASSERT(sal_False, "Cursor not on an outline node" );
	}
#endif
	return bRet;
}

/** Test whether outline may be moved (bCopy == false)
 *                           or copied (bCopy == true)
 * Verify these conditions:
 * 1) outline must be within main body (and not in redline)
 * 2) outline must not be within table
 * 3) if bCopy is set, outline must not be write protected
 */
sal_Bool lcl_IsOutlineMoveAndCopyable( const SwDoc* pDoc, sal_uInt16 nIdx, bool bCopy )
{
	const SwNodes& rNds = pDoc->GetNodes();
    const SwNode* pNd = rNds.GetOutLineNds()[ nIdx ];
    return pNd->GetIndex() >= rNds.GetEndOfExtras().GetIndex() &&   // 1) body
            !pNd->FindTableNode() &&                                // 2) table
            ( bCopy || !pNd->IsProtect() );                         // 3) write
}

sal_Bool SwEditShell::IsOutlineMovable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, false );
}

sal_Bool SwEditShell::IsOutlineCopyable( sal_uInt16 nIdx ) const
{
    return lcl_IsOutlineMoveAndCopyable( GetDoc(), nIdx, true );
}


sal_Bool SwEditShell::NumOrNoNum(
    sal_Bool bNumOn,
    sal_Bool bChkStart )
{
    sal_Bool bRet = sal_False;

    if ( !IsMultiSelection()
         && !HasSelection()
         && ( !bChkStart || IsSttPara() ) )
    {
        StartAllAction();
        bRet = GetDoc()->NumOrNoNum( GetCrsr()->GetPoint()->nNode, !bNumOn );
        EndAllAction();
    }
    return bRet;
}


sal_Bool SwEditShell::IsNoNum( sal_Bool bChkStart ) const
{
    sal_Bool bResult = sal_False;

    if ( !IsMultiSelection()
         && !HasSelection()
         && ( !bChkStart || IsSttPara() ) )
    {
        const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
        if ( pTxtNd != NULL )
        {
            bResult =  !pTxtNd->IsCountedInList();
        }
    }

    return bResult;
}

sal_uInt8 SwEditShell::GetNumLevel() const
{
    // gebe die akt. Ebene zurueck, auf der sich der Point vom Cursor befindet
    //sal_uInt8 nLevel = NO_NUMBERING;	//#outline level,zhaojianwei
    sal_uInt8 nLevel = MAXLEVEL;		//end,zhaojianwei

    SwPaM* pCrsr = GetCrsr();
    const SwTxtNode* pTxtNd = pCrsr->GetNode()->GetTxtNode();

    ASSERT( pTxtNd != NULL, "GetNumLevel() without text node" )
    if ( pTxtNd == NULL )
        return nLevel;

    const SwNumRule* pRule = pTxtNd->GetNumRule();
    if ( pRule != NULL )
    {
        const int nListLevelOfTxtNode( pTxtNd->GetActualListLevel() );
        if ( nListLevelOfTxtNode >= 0 )
        {
            nLevel = static_cast<sal_uInt8>( nListLevelOfTxtNode );
        }
    }

    return nLevel;
}

const SwNumRule* SwEditShell::GetNumRuleAtCurrCrsrPos() const
{
	return GetDoc()->GetNumRuleAtPos( *GetCrsr()->GetPoint() );
}

const SwNumRule* SwEditShell::GetNumRuleAtCurrentSelection() const
{
    const SwNumRule* pNumRuleAtCurrentSelection = NULL;

    const SwPaM* pCurrentCrsr = GetCrsr();
    bool bDifferentNumRuleFound = false;
    const SwPaM* pCrsr = pCurrentCrsr;
    do
    {
        const SwNodeIndex aEndNode = pCrsr->End()->nNode;

        for ( SwNodeIndex aNode = pCrsr->Start()->nNode; aNode <= aEndNode; aNode++ )
        {
            const SwNumRule* pNumRule = GetDoc()->GetNumRuleAtPos( SwPosition( aNode ) );
            if ( pNumRule == NULL )
            {
                continue;
            }
            else if ( pNumRule != pNumRuleAtCurrentSelection )
            {
                if ( pNumRuleAtCurrentSelection == NULL )
                {
                    pNumRuleAtCurrentSelection = pNumRule;
                }
                else
                {
                    pNumRuleAtCurrentSelection = NULL;
                    bDifferentNumRuleFound = true;
                    break;
                }
            }
        }

        pCrsr = static_cast< const SwPaM* >(pCrsr->GetNext());
    } while ( !bDifferentNumRuleFound && pCrsr != pCurrentCrsr );

    return pNumRuleAtCurrentSelection;
}


void SwEditShell::SetCurNumRule( const SwNumRule& rRule,
                                 const bool bCreateNewList,
                                 const String sContinuedListId,
                                 const bool bResetIndentAttrs )
{
    StartAllAction();

    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );

    SwPaM* pCrsr = GetCrsr();
    if( IsMultiSelection() )
    {
        SwPamRanges aRangeArr( *pCrsr );
        SwPaM aPam( *pCrsr->GetPoint() );
        for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
        {
            aRangeArr.SetPam( n, aPam );
            GetDoc()->SetNumRule( aPam, rRule,
                                  bCreateNewList, sContinuedListId,
                                  true, bResetIndentAttrs );
            GetDoc()->SetCounted( aPam, true );
        }
    }
    else
    {
        GetDoc()->SetNumRule( *pCrsr, rRule,
                              bCreateNewList, sContinuedListId,
                              true, bResetIndentAttrs );
        GetDoc()->SetCounted( *pCrsr, true );
    }
    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );

    EndAllAction();
}


String SwEditShell::GetUniqueNumRuleName( const String* pChkStr, sal_Bool bAutoNum ) const
{
	return GetDoc()->GetUniqueNumRuleName( pChkStr, bAutoNum );
}

void SwEditShell::ChgNumRuleFmts( const SwNumRule& rRule )
{
	StartAllAction();
	GetDoc()->ChgNumRuleFmts( rRule );
	EndAllAction();
}

sal_Bool SwEditShell::ReplaceNumRule( const String& rOldRule, const String& rNewRule )
{
	StartAllAction();
	sal_Bool bRet = GetDoc()->ReplaceNumRule( *GetCrsr()->GetPoint(), rOldRule, rNewRule );
	EndAllAction();
	return bRet;
}

void SwEditShell::SetNumRuleStart( sal_Bool bFlag )
{
	StartAllAction();

	SwPaM* pCrsr = GetCrsr();
	if( pCrsr->GetNext() != pCrsr )			// Mehrfachselektion ?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( *pCrsr );
		SwPaM aPam( *pCrsr->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
			GetDoc()->SetNumRuleStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), bFlag );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
		GetDoc()->SetNumRuleStart( *pCrsr->GetPoint(), bFlag );

	EndAllAction();
}

sal_Bool SwEditShell::IsNumRuleStart() const
{
    sal_Bool bResult = sal_False;
	const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
	if( pTxtNd )
        bResult = pTxtNd->IsListRestart() ? sal_True : sal_False;
	return bResult;
}

void SwEditShell::SetNodeNumStart( sal_uInt16 nStt )
{
	StartAllAction();

	SwPaM* pCrsr = GetCrsr();
	if( pCrsr->GetNext() != pCrsr )			// Mehrfachselektion ?
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_START, NULL );
		SwPamRanges aRangeArr( *pCrsr );
		SwPaM aPam( *pCrsr->GetPoint() );
		for( sal_uInt16 n = 0; n < aRangeArr.Count(); ++n )
			GetDoc()->SetNodeNumStart( *aRangeArr.SetPam( n, aPam ).GetPoint(), nStt );
        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_END, NULL );
    }
    else
		GetDoc()->SetNodeNumStart( *pCrsr->GetPoint(), nStt );

	EndAllAction();
}

sal_uInt16 SwEditShell::GetNodeNumStart() const
{
	const SwTxtNode* pTxtNd = GetCrsr()->GetNode()->GetTxtNode();
    // --> OD 2008-02-28 #refactorlists#
    // correction: check, if list restart value is set at text node and
    // use new method <SwTxtNode::GetAttrListRestartValue()>.
    // return USHRT_MAX, if no list restart value is found.
    if ( pTxtNd && pTxtNd->HasAttrListRestartValue() )
    {
        return static_cast<sal_uInt16>(pTxtNd->GetAttrListRestartValue());
    }
    return USHRT_MAX;
    // <--
}

/*-- 26.08.2005 14:47:17---------------------------------------------------

  -----------------------------------------------------------------------*/
// --> OD 2008-03-18 #refactorlists#
const SwNumRule * SwEditShell::SearchNumRule( const bool bForward,
                                              const bool bNum,
                                              const bool bOutline,
                                              int nNonEmptyAllowed,
                                              String& sListId )
{
    return GetDoc()->SearchNumRule( *(bForward ? GetCrsr()->End() : GetCrsr()->Start()),
                                    bForward, bNum, bOutline, nNonEmptyAllowed,
                                    sListId );
}
// <--
