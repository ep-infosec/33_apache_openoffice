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

#include <ctype.h>
#include <hintids.hxx>
#include <hints.hxx>
#include <svtools/filter.hxx>

#include <vcl/graph.hxx>
#include <svl/urihelper.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crsditem.hxx>
#include <svl/stritem.hxx>
#include <unotools/charclass.hxx>
#include <txtftn.hxx>
#include <fmtpdsc.hxx>
#include <fmtftn.hxx>
#include <fmtanchr.hxx>
#include <fmtrfmrk.hxx>
#include <fmtclds.hxx>
#include <fmtfld.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtcntnt.hxx>
#include <redline.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <ndtxt.hxx>
#include <frmatr.hxx>
#include <fldbas.hxx>			// RES_SETEXPFLD
#include <charatr.hxx>			// class SwFmtRefMark
#include <swtable.hxx>			// class SwTableLines, ...
#include <tox.hxx>
#include <expfld.hxx>			// SwExpField
#include <section.hxx>			// class SwSection
#include <tblsel.hxx>			// class SwSelBoxes
#include <pagedesc.hxx>
#include <docsh.hxx>			// class SwDocSh
#include <fltshell.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>


#define MAX_FIELDLEN 64000

using namespace com::sun::star;

static SwCntntNode* GetCntntNode(SwDoc* pDoc, SwNodeIndex& rIdx, sal_Bool bNext)
{
    SwCntntNode * pCNd = rIdx.GetNode().GetCntntNode();
	if(!pCNd && 0 == (pCNd = bNext ? pDoc->GetNodes().GoNext(&rIdx)
									 : pDoc->GetNodes().GoPrevious(&rIdx)))
	{
		pCNd = bNext ? pDoc->GetNodes().GoPrevious(&rIdx)
					 : pDoc->GetNodes().GoNext(&rIdx);
		ASSERT(pCNd, "kein ContentNode gefunden");
	}
	return pCNd;
}

// ------ Stack-Eintrag fuer die gesamten - Attribute vom Text -----------
SwFltStackEntry::SwFltStackEntry(const SwPosition& rStartPos, SfxPoolItem* pHt ) :
	nMkNode(rStartPos.nNode, -1),
	//Modify here for #119405, by easyfan, 2012-05-24
	nPtNode(nMkNode),mnStartCP(-1),mnEndCP(-1),bIsParaEnd(false)
	//End of modification, by easyfan
{
	// Anfang vom Bereich merken
	nMkCntnt = rStartPos.nContent.GetIndex();
	pAttr = pHt;		// speicher eine Kopie vom Attribut
	bOld	= sal_False;	// used for marking Attributes *before* skipping field results
	bLocked = sal_True; 	// locke das Attribut --> darf erst
	bCopied = sal_False;	// gesetzt werden, wenn es wieder geunlocked ist
	bConsumedByField = sal_False;
}

SwFltStackEntry::SwFltStackEntry(const SwFltStackEntry& rEntry) :
	nMkNode(rEntry.nMkNode),
	nPtNode(rEntry.nPtNode)
{
	pAttr	= rEntry.pAttr->Clone();
	nMkCntnt= rEntry.nMkCntnt;
	bOld	= rEntry.bOld;
	bLocked	= bCopied = sal_True; // when rEntry were NOT bLocked we would never have been called
	bConsumedByField = rEntry.bConsumedByField;
	//Modify here for #119405, by chengjh, 2012-08-16
	mnStartCP= rEntry.mnStartCP;
	mnEndCP = rEntry.mnEndCP;
	bIsParaEnd = rEntry.bIsParaEnd;
	//End
}


SwFltStackEntry::~SwFltStackEntry()
{
	// Attribut kam zwar als Pointer, wird aber hier geloescht
	if (pAttr)
		delete pAttr;
}

void SwFltStackEntry::SetEndPos(const SwPosition& rEndPos)
{
	// Attribut freigeben und das Ende merken.
	// Alles mit sal_uInt16's, weil sonst beim Einfuegen von neuem Text an der
	// Cursor-Position auch der Bereich vom Attribut weiter
	// verschoben wird.
	// Das ist aber nicht das gewollte!
	bLocked = sal_False;					// freigeben und das ENDE merken
	nPtNode = rEndPos.nNode.GetIndex()-1;
	nPtCntnt = rEndPos.nContent.GetIndex();
}
//Modify here for #119405, by chengjh, 2012-08-16
//The only position of 0x0D will not be able to make regin in the old logic
//because it is beyond the length of para...need special consideration here.
bool SwFltStackEntry::IsAbleMakeRegion()
{
    SwCntntNode *const pCntntNode( SwNodeIndex(nMkNode, +1).GetNode().GetCntntNode() );
    if ( (nMkNode.GetIndex() == nPtNode.GetIndex())
         && (nMkCntnt == nPtCntnt)
         && ( (0 != nPtCntnt)
              || ( pCntntNode
                   && ( 0 != pCntntNode->Len() ) ) )
         && ( RES_TXTATR_FIELD != pAttr->Which()
              && RES_TXTATR_ANNOTATION != pAttr->Which()
              && RES_TXTATR_INPUTFIELD != pAttr->Which() )
         && !( bIsParaEnd
               && pCntntNode
               && pCntntNode->IsTxtNode()
               && 0 != pCntntNode->Len() ) )
    {
        return false;
    }

    return true;
}
//End
sal_Bool SwFltStackEntry::MakeRegion(SwDoc* pDoc, SwPaM& rRegion, sal_Bool bCheck )
{
    // does this range actually contain something?
    // empty range is allowed if at start of empty paragraph
    // fields are special: never have range, so leave them
	//Modify here for #119405, by chengjh, 2012-08-16
	//Revised the code and move the code segment to defined function
    	if ( !IsAbleMakeRegion() )
	{
		return sal_False;
	}
	//End
	// !!! Die Content-Indizies beziehen sich immer auf den Node !!!
	rRegion.GetPoint()->nNode = nMkNode.GetIndex() + 1;
	SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, sal_True);
	rRegion.GetPoint()->nContent.Assign(pCNd, nMkCntnt);
	rRegion.SetMark();
	if( nMkNode != nPtNode )
	{
		rRegion.GetPoint()->nNode = nPtNode.GetIndex() + 1;
		pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, sal_False);
	}
	rRegion.GetPoint()->nContent.Assign(pCNd, nPtCntnt);
#if OSL_DEBUG_LEVEL > 1
	ASSERT( CheckNodesRange( rRegion.Start()->nNode,
							 rRegion.End()->nNode, sal_True ),
			 "Attribut oder AEhnliches ueber Bereichs-Grenzen" );
#endif
	if( bCheck )
		return CheckNodesRange( rRegion.Start()->nNode,
								rRegion.End()->nNode, sal_True );
	else
		return sal_True;
}

SwFltControlStack::SwFltControlStack(SwDoc* pDo, sal_uLong nFieldFl)
  : nFieldFlags(nFieldFl), bHasSdOD(true)
  ,bSdODChecked(false), pDoc(pDo), bIsEndStack(false)
	//End

{
}


SwFltControlStack::~SwFltControlStack()
{
	ASSERT(!Count(), "noch Attribute auf dem Stack");
}

// MoveAttrs() ist fuer folgendes Problem:
// Wenn ueber den Stack ein Feld wie z.B. "Variable setzen" gesetzt wird,
// verschiebt sich der Text um ein \xff - Zeichen, und alle folgenden
// Attribute stimmen in ihrer Position nicht mehr.
// Dann muss MoveAttrs() nach dem Setzen des Attributes ins Doc gerufen werden,
// so dass alle Attribut-Positionen,
// die im selben Absatz weiter hinten stehen, um 1 Zeichen weiter
// nach rechts verschoben werden.
void SwFltControlStack::MoveAttrs( const SwPosition& rPos )
{
    sal_uInt16 nCnt = static_cast< sal_uInt16 >(Count());
	SwFltStackEntry* pEntry;
	sal_uLong nPosNd = rPos.nNode.GetIndex();
	sal_uInt16 nPosCt = rPos.nContent.GetIndex() - 1;

	for (sal_uInt16 i=0; i < nCnt; i++){
		pEntry = (*this)[ i ];
		if(( pEntry->nMkNode.GetIndex() + 1 == nPosNd )
		   &&( pEntry->nMkCntnt >= nPosCt )){
			pEntry->nMkCntnt++;
			ASSERT( pEntry->nMkCntnt
				<= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
					"Attribut-Anfang hinter Zeilenende" );
		}
		if(( pEntry->nPtNode.GetIndex() + 1 == nPosNd )
		   &&( pEntry->nPtCntnt >= nPosCt )){
			pEntry->nPtCntnt++;
			ASSERT( pEntry->nPtCntnt
				<= pDoc->GetNodes()[nPosNd]->GetCntntNode()->Len(),
					"Attribut-Ende hinter Zeilenende" );
		}
	}
}

void SwFltControlStack::MarkAllAttrsOld()
{
    sal_uInt16 nCnt = static_cast< sal_uInt16 >(Count());
	for (sal_uInt16 i=0; i < nCnt; i++)
		(*this)[ i ]->bOld = sal_True;
}

void SwFltControlStack::NewAttr(const SwPosition& rPos, const SfxPoolItem & rAttr )
{
	SwFltStackEntry *pTmp = new SwFltStackEntry(rPos, rAttr.Clone() );
	//Modify here for #119405, by easyfan, 2012-05-24
	pTmp->SetStartCP(GetCurrAttrCP());
	//End of modification, by easyfan
	sal_uInt16 nWhich = pTmp->pAttr->Which();
	SetAttr(rPos, nWhich);// Ende von evtl. gleichen Attributen auf dem Stack
								// Setzen, damit sich die Attribute nicht auf
								// dem Stack haeufen
	maEntries.push_back(pTmp);
}

void SwFltControlStack::DeleteAndDestroy(Entries::size_type nCnt)
{
	ASSERT(nCnt < maEntries.size(), "Out of range!");
	if (nCnt < maEntries.size())
	{
	    myEIter aElement = maEntries.begin() + nCnt;
	    delete *aElement;
	    maEntries.erase(aElement);
	}
	//Modify for #119405 by chengjh, 2012-08-16
	//Clear the para end position recorded in reader intermittently for the least impact on loading performance
	//Because the attributes handled based on the unit of para
	if ( Count() == 0 )
	{
		ClearParaEndPosition();
		bHasSdOD = true;
		bSdODChecked = false;
	}
	//End
}

// SwFltControlStack::StealAttr() loescht Attribute des angegebenen Typs vom Stack.
// Als nAttrId sind erlaubt: 0 fuer alle, oder ein spezieller Typ.
// Damit erscheinen sie nicht in der Doc-Struktur. Dabei werden nur die
// Attribute entfernt, die im selben Absatz wie pPos stehen.
// Wird fuer Grafik-Apos -> Grafiken benutzt.
void SwFltControlStack::StealAttr(const SwPosition* pPos, sal_uInt16 nAttrId /* = 0 */)
{
    sal_uInt16 nCnt = static_cast< sal_uInt16 >(Count());

	SwFltStackEntry* pEntry;

	while (nCnt)
	{
		nCnt --;
		pEntry = (*this)[ nCnt ];
		if (pEntry->nPtNode.GetIndex()+1 == pPos->nNode.GetIndex() &&
			(!nAttrId || nAttrId == pEntry->pAttr->Which()))
			DeleteAndDestroy(nCnt);		// loesche aus dem Stack
	}
}

// SwFltControlStack::KillUnlockedAttr() loescht alle Attribute vom Stack,
// welche punktuell auf pPos aufgespannt sind.
// Damit erscheinen sie nicht in der Doc-Struktur.
// Wird im WW Import benoetigt zum ignorieren der auf dem 0x0c Section-
// Break-Symbol gesetzten Attribute.
void SwFltControlStack::KillUnlockedAttrs(const SwPosition& pPos)
{
	SwNodeIndex aAktNode( pPos.nNode, -1 );
	sal_uInt16 nAktIdx = pPos.nContent.GetIndex();

    sal_uInt16 nCnt = static_cast< sal_uInt16 >(Count());
	SwFltStackEntry* pEntry;
	while( nCnt )
	{
		nCnt --;
		pEntry = (*this)[ nCnt ];
		if(    !pEntry->bOld
			&& !pEntry->bLocked
			&& (pEntry->nMkNode  == aAktNode)
			&& (pEntry->nMkCntnt == nAktIdx )
			&& (pEntry->nPtNode  == aAktNode)
			&& (pEntry->nPtCntnt == nAktIdx ))
		{
			DeleteAndDestroy( nCnt ); // loesche aus dem Stack
		}
	}
}

// Alle gelockten Attribute freigeben (unlocken) und das Ende setzen,
// alle anderen im Document setzen und wieder aus dem Stack loeschen
// Returned, ob das gesuchte Attribut / die gesuchten Attribute
// ueberhaupt auf dem Stack standen
void SwFltControlStack::SetAttr(const SwPosition& rPos, sal_uInt16 nAttrId,
								sal_Bool bTstEnde, long nHand, sal_Bool consumedByField )
{
	ASSERT(!nAttrId ||
		(POOLATTR_BEGIN <= nAttrId && POOLATTR_END > nAttrId) ||
		(RES_FLTRATTR_BEGIN <= nAttrId && RES_FLTRATTR_END > nAttrId),
		"Falsche Id fuers Attribut")

    sal_uInt16 nCnt = static_cast< sal_uInt16 >(Count());

	SwFltStackEntry* pEntry;

	for (sal_uInt16 i=0; i < nCnt; i++)
	{
		pEntry = (*this)[ i ];
		if (pEntry->bLocked)
		{
			// setze das Ende vom Attribut
			bool bF = false;
			if (!nAttrId ){
				bF = true;
			}else if( nAttrId == pEntry->pAttr->Which()){
				if( nAttrId != RES_FLTR_BOOKMARK ){		// Handle abfragen
					bF = true;
				}else if( nHand == ((SwFltBookmark*)(pEntry->pAttr))->GetHandle() )
				{
					bF = true;
				}
			}
			if (bF) {
				pEntry->bConsumedByField = consumedByField;
				pEntry->SetEndPos(rPos);
				//Modify here for #119405, by easyfan, 2012-05-24
				pEntry->SetEndCP(GetCurrAttrCP());
				//End of modification, by easyfan
			}
			continue;
		}

		// ist die Endposition die Cursor-Position, dann noch nicht
		// ins Dokument setzen, es muss noch Text folgen;
		// ausser am Dokumentende. (Attribut-Expandierung !!)
		// Beim Ende-Stack niemals ausser am DocEnde reinsetzen
		if (bTstEnde)
		{
			if (bIsEndStack || pEntry->nPtNode.GetIndex()+1 ==
						rPos.nNode.GetIndex())
			continue;
		}
		SetAttrInDoc(rPos, pEntry);
		DeleteAndDestroy(i);		// loesche aus dem Stack
		i--; nCnt--;		// Danach rutschen alle folgenden nach unten
	}
}

static void MakePoint(SwFltStackEntry* pEntry, SwDoc* pDoc, SwPaM& rRegion)
{
			// der Anker ist der Point vom Pam. Dieser wird beim Einfugen
			// von Text usw. veraendert; darum wird er auf dem Stack
			// gespeichert. Das Attribut muss nur noch im Format
			// gesetzt werden.
	rRegion.DeleteMark();
	rRegion.GetPoint()->nNode = pEntry->nMkNode.GetIndex() + 1;
	SwCntntNode* pCNd = GetCntntNode(pDoc, rRegion.GetPoint()->nNode, sal_True);
	rRegion.GetPoint()->nContent.Assign(pCNd, pEntry->nMkCntnt);
}

// MakeBookRegionOrPoint() ist wie MakeRegionOrPoint, aber die besonderen
// Beschraenkungen von Bookmarks in Tabellen werden beachtet.
// ( Anfang und Ende muessen in selber Zelle sein )
static void MakeBookRegionOrPoint(SwFltStackEntry* pEntry, SwDoc* pDoc,
					SwPaM& rRegion, sal_Bool bCheck )
{
	if (pEntry->MakeRegion(pDoc, rRegion, bCheck )){
//		sal_Bool b1 = rNds[rRegion.GetPoint()->nNode]->FindTableNode() != 0;
//		const SwStartNode* p1 =	rNds[rRegion.GetPoint()->nNode]->FindTableBoxStartNode();
//		const SwStartNode* p2 = rNds[rRegion.GetMark()->nNode]->FindTableBoxStartNode();
        if (rRegion.GetPoint()->nNode.GetNode().FindTableBoxStartNode()
              != rRegion.GetMark()->nNode.GetNode().FindTableBoxStartNode())
        {
			rRegion.Exchange();			// Ungueltiger Bereich
			rRegion.DeleteMark();		// -> beide auf Mark
		}
	}else{
		MakePoint(pEntry, pDoc, rRegion);
	}
}

#if OSL_DEBUG_LEVEL > 1
extern sal_Bool CheckNodesRange( const SwNodeIndex& rStt,
					const SwNodeIndex& rEnd, sal_Bool bChkSection );
#endif

// IterateNumrulePiece() sucht von rTmpStart bis rEnd den ersten
// fuer Numrules gueltigen Bereich heraus.
//
// rNds sind die Doc-Nodes
// rEnd ist Bereichs-Ende,
// rTmpStart ist ReinRaus-Parameter: Anfang des zu untersuchenden Bereiches rein,
//									 Anfang des gueltigen Bereichs raus
// rTmpEnd ist raus-Parameter
// Return-Bool ist sal_True fuer gueltigen Bereich
static sal_Bool IterateNumrulePiece( const SwNodeIndex& rEnd,
								SwNodeIndex& rTmpStart, SwNodeIndex& rTmpEnd )
{
	while( ( rTmpStart <= rEnd )
		   && !( rTmpStart.GetNode().IsTxtNode() ) )	// suche gueltigen Anfang
		rTmpStart++;

	rTmpEnd = rTmpStart;
	while( ( rTmpEnd <= rEnd )
		   && ( rTmpEnd.GetNode().IsTxtNode() ) )		// suche gueltiges Ende + 1
		rTmpEnd++;

	rTmpEnd--;										// gueltiges Ende

	return rTmpStart <= rTmpEnd;					// gueltig ?
}
//Modify for #119405 by chengjh, 2012-08-16
//***This function will check whether there is existing individual attribute positon for 0x0D***/
//The check will happen only once for a paragraph during loading
bool SwFltControlStack::HasSdOD()
{	
	sal_uInt16 nCnt = static_cast< sal_uInt16 >(Count());

	SwFltStackEntry* pEntry = 0;

	bool bRet = false;

	for (sal_uInt16 i=0; i < nCnt; i++)
	{
		pEntry = (*this)[ i ];
		if ( pEntry && pEntry->mnStartCP == pEntry->mnEndCP )
		{
			if ( CheckSdOD(pEntry->mnStartCP,pEntry->mnEndCP) )
			{
				bRet = true;
				break;
			}
		}
	}

	return bRet;
}
//End
void SwFltControlStack::SetAttrInDoc(const SwPosition& rTmpPos, SwFltStackEntry* pEntry)
{
    SwPaM aRegion( rTmpPos );

    switch(pEntry->pAttr->Which())
    {
    case RES_FLTR_ANCHOR:
        {
            SwFrmFmt* pFmt = ((SwFltAnchor*)pEntry->pAttr)->GetFrmFmt();
            if (pFmt != NULL)
            {
                MakePoint(pEntry, pDoc, aRegion);
                SwFmtAnchor aAnchor(pFmt->GetAnchor());
                aAnchor.SetAnchor(aRegion.GetPoint());
                pFmt->SetFmtAttr(aAnchor);
                // Damit die Frames bei Einfuegen in existierendes Doc
                //  erzeugt werden (erst nach Setzen des Ankers!):
                if(pDoc->GetCurrentViewShell()	//swmod 071108//swmod 071225
                   && (FLY_AT_PARA == pFmt->GetAnchor().GetAnchorId()))
                {
                    pFmt->MakeFrms();
                }
            }
        }
        break;
    case RES_FLTR_STYLESHEET:
        break;

    case RES_TXTATR_FIELD:
    case RES_TXTATR_ANNOTATION:
    case RES_TXTATR_INPUTFIELD:
        break;

    case RES_TXTATR_TOXMARK:
        break;

    case RES_FLTR_NUMRULE:			// Numrule 'reinsetzen
    {
        const String& rNumNm = ( (SfxStringItem*) pEntry->pAttr )->GetValue();
        SwNumRule* pNumRule = pDoc->FindNumRulePtr( rNumNm );
        if ( pNumRule )
        {
            if ( pEntry->MakeRegion( pDoc, aRegion, sal_True ) )
            {
                SwNodeIndex aTmpStart( aRegion.Start()->nNode );
                SwNodeIndex aTmpEnd( aTmpStart );
                SwNodeIndex& rRegEndNd = aRegion.End()->nNode;
                while (IterateNumrulePiece( rRegEndNd, aTmpStart, aTmpEnd ))
                {
                    SwPaM aTmpPam( aTmpStart, aTmpEnd );
                    pDoc->SetNumRule( aTmpPam, *pNumRule, false );

                    aTmpStart = aTmpEnd;    // Start fuer naechstes Teilstueck
                    aTmpStart++;
                }
            }
            else
                pDoc->DelNumRule( rNumNm );
        }
    }
        break;

	case RES_FLTR_NUMRULE_NUM:
        break;
    case RES_FLTR_BOOKMARK:
        {
            SwFltBookmark* pB = (SwFltBookmark*)pEntry->pAttr;
            const String& rName = ((SwFltBookmark*)pEntry->pAttr)->GetName();

            if (IsFlagSet(BOOK_TO_VAR_REF))
            {
                SwFieldType* pFT = pDoc->GetFldType(RES_SETEXPFLD, rName, false);
                if (!pFT)
                {
                    SwSetExpFieldType aS(pDoc, rName, nsSwGetSetExpType::GSE_STRING);
                    pFT = pDoc->InsertFldType(aS);
                }
                SwSetExpField aFld((SwSetExpFieldType*)pFT, pB->GetValSys());
                aFld.SetSubType( nsSwExtendedSubType::SUB_INVISIBLE );
                MakePoint(pEntry, pDoc, aRegion);
                pDoc->InsertPoolItem(aRegion, SwFmtFld(aFld), 0);
                MoveAttrs( *(aRegion.GetPoint()) );
            }
            if ( ( !IsFlagSet(HYPO) || IsFlagSet(BOOK_AND_REF) ) && 
                 !pEntry->bConsumedByField )
            {
                MakeBookRegionOrPoint(pEntry, pDoc, aRegion, sal_True);
                // #120879# - create a cross reference heading bookmark if appropriate.
                const IDocumentMarkAccess::MarkType eBookmarkType = 
                    ( pB->IsTOCBookmark() &&
                      IDocumentMarkAccess::IsLegalPaMForCrossRefHeadingBookmark( aRegion ) )
                    ? IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK
                    : IDocumentMarkAccess::BOOKMARK;
                pDoc->getIDocumentMarkAccess()->makeMark( aRegion, rName, eBookmarkType );
            }
        }
        break;
	case RES_FLTR_TOX:
		{
			MakePoint(pEntry, pDoc, aRegion);

			SwPosition* pPoint = aRegion.GetPoint();

			SwFltTOX* pTOXAttr = (SwFltTOX*)pEntry->pAttr;

			// test if on this node there had been a pagebreak BEFORE the
			//     tox attribut was put on the stack
			SfxItemSet aBkSet( pDoc->GetAttrPool(), RES_PAGEDESC, RES_BREAK );
			SwCntntNode* pNd = 0;
			if( !pTOXAttr->HadBreakItem() || !pTOXAttr->HadPageDescItem() )
			{
				pNd = pPoint->nNode.GetNode().GetCntntNode();
				if( pNd )
				{
					const SfxItemSet* pSet = pNd->GetpSwAttrSet();
					const SfxPoolItem* pItem;
					if( pSet )
					{
						if(    !pTOXAttr->HadBreakItem()
							&& SFX_ITEM_SET == pSet->GetItemState( RES_BREAK, sal_False, &pItem ) )
						{
							aBkSet.Put( *pItem );
							pNd->ResetAttr( RES_BREAK );
						}
						if(    !pTOXAttr->HadPageDescItem()
							&& SFX_ITEM_SET == pSet->GetItemState( RES_PAGEDESC, sal_False, &pItem ) )
						{
							aBkSet.Put( *pItem );
							pNd->ResetAttr( RES_PAGEDESC );
						}
					}
				}
			}

			delete pTOXAttr->GetBase();

			// set (aboved saved and removed) the break item at the node following the TOX
			if( aBkSet.Count() )
				pNd->SetAttr( aBkSet );
		}
		break;
	case RES_FLTR_SECTION:
		MakePoint(pEntry, pDoc, aRegion);	// bislang immer Point==Mark
        pDoc->InsertSwSection(aRegion,
                *(static_cast<SwFltSection*>(pEntry->pAttr))->GetSectionData(),
                0, 0, false);
        delete (((SwFltSection*)pEntry->pAttr)->GetSectionData());
		break;
	case RES_FLTR_REDLINE:
		{
			if (pEntry->MakeRegion(pDoc, aRegion, sal_True))
			{
			  pDoc->SetRedlineMode((RedlineMode_t)(   nsRedlineMode_t::REDLINE_ON
											  | nsRedlineMode_t::REDLINE_SHOW_INSERT
											  | nsRedlineMode_t::REDLINE_SHOW_DELETE ));
				SwFltRedline& rFltRedline = *((SwFltRedline*)pEntry->pAttr);

				if( USHRT_MAX != rFltRedline.nAutorNoPrev )
				{
					SwRedlineData aData(rFltRedline.eTypePrev,
										rFltRedline.nAutorNoPrev,
										rFltRedline.aStampPrev,
										aEmptyStr,
										0
										);
					pDoc->AppendRedline(new SwRedline(aData, aRegion), true);
				}
				SwRedlineData aData(rFltRedline.eType,
									rFltRedline.nAutorNo,
									rFltRedline.aStamp,
									aEmptyStr,
									0
									);
				pDoc->AppendRedline( new SwRedline(aData, aRegion), true );
                pDoc->SetRedlineMode((RedlineMode_t)( nsRedlineMode_t::REDLINE_NONE
												| nsRedlineMode_t::REDLINE_SHOW_INSERT
												| nsRedlineMode_t::REDLINE_SHOW_DELETE ));
			}
		}
		break;
	default:
		{
			//Modify here for #119405, by chengjh, 2012-08-16
			//Revised for more complex situations should be considered
			if ( !bSdODChecked )
			{
				bHasSdOD = HasSdOD();
				bSdODChecked = true;
			}
				sal_Int32 nStart = pEntry->GetStartCP();
				sal_Int32 nEnd = pEntry->GetEndCP();
				if (nStart != -1 && nEnd != -1 && nEnd >= nStart )					
				{
					pEntry->SetIsParaEnd( IsParaEndInCPs(nStart,nEnd,bHasSdOD) );
				}
			//End
			if (pEntry->MakeRegion(pDoc, aRegion, sal_False))
	        {
				//Modify here for #119405, by easyfan, 2012-05-24			
				//Refined 2012-08-16
				if (pEntry->IsParaEnd())
				{
					pDoc->InsertPoolItem(aRegion, *pEntry->pAttr, 0,true);
				}else
				{
					pDoc->InsertPoolItem(aRegion, *pEntry->pAttr, 0);
				}
				//End
	        }	
		}
		break;
	}
}

SfxPoolItem* SwFltControlStack::GetFmtStackAttr(sal_uInt16 nWhich, sal_uInt16 * pPos)
{
	SwFltStackEntry* pEntry;
    sal_uInt16 nSize = static_cast< sal_uInt16 >(Count());

	while (nSize)
	{
		// ist es das gesuchte Attribut ? (gueltig sind nur gelockte,
		// also akt. gesetzte Attribute!!)
		if ((pEntry = (*this)[ --nSize ])->bLocked &&
			pEntry->pAttr->Which() == nWhich)
		{
			if (pPos)
				*pPos = nSize;
			return (SfxPoolItem*)pEntry->pAttr;		// Ok, dann Ende
		}
	}
	return 0;
}

const SfxPoolItem* SwFltControlStack::GetOpenStackAttr(const SwPosition& rPos, sal_uInt16 nWhich)
{
	SwFltStackEntry* pEntry;
	sal_uInt16 nSize = static_cast< sal_uInt16 >(Count());
	SwNodeIndex aAktNode( rPos.nNode, -1 );
	sal_uInt16 nAktIdx = rPos.nContent.GetIndex();

	while (nSize)
	{
		pEntry = (*this)[ --nSize ];
		if(    pEntry->bLocked
			&& (pEntry->pAttr->Which() == nWhich)
			&& (pEntry->nMkNode  == aAktNode)
			&& (pEntry->nMkCntnt == nAktIdx ))
		{
			return (SfxPoolItem*)pEntry->pAttr;
		}
	}
	return 0;
}

const SfxPoolItem* SwFltControlStack::GetFmtAttr(const SwPosition& rPos, sal_uInt16 nWhich)
{
	SfxPoolItem* pHt = GetFmtStackAttr(nWhich);
	if (pHt)
		return (const SfxPoolItem*)pHt;

	// im Stack ist das Attribut nicht vorhanden, also befrage das Dokument
//	SwCntntNode * pNd = rPaM.GetCntntNode();
    SwCntntNode * pNd = rPos.nNode.GetNode().GetCntntNode();

	if (!pNd)			// kein ContentNode, dann das dflt. Attribut
		return &pDoc->GetAttrPool().GetDefaultItem(nWhich);
	return &pNd->GetAttr(nWhich);
}

void SwFltControlStack::Delete(const SwPaM &rPam)
{
    const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    SwNodeIndex aStartNode(pStt->nNode, -1);
    sal_uInt16 nStartIdx = pStt->nContent.GetIndex();
    SwNodeIndex aEndNode(pEnd->nNode, -1);
    sal_uInt16 nEndIdx = pEnd->nContent.GetIndex();

    //We don't support deleting content that is over one node, or removing a node.
    ASSERT(aEndNode == aStartNode, "nodes must be the same, or this method extended");
    if (aEndNode != aStartNode)
        return;

    for (sal_uInt16 nSize = static_cast< sal_uInt16 >(Count()); nSize > 0;)
    {
        SwFltStackEntry* pEntry = (*this)[--nSize];

        bool bEntryStartAfterSelStart =
            (pEntry->nMkNode == aStartNode && pEntry->nMkCntnt >= nStartIdx);

        bool bEntryStartBeforeSelEnd =
            (pEntry->nMkNode == aEndNode && pEntry->nMkCntnt <= nEndIdx);

        bool bEntryEndAfterSelStart = false;
        bool bEntryEndBeforeSelEnd = false;
        if (!pEntry->bLocked)
        {
            bEntryEndAfterSelStart =
                (pEntry->nPtNode == aStartNode && pEntry->nPtCntnt >= nStartIdx);

            bEntryEndBeforeSelEnd =
                (pEntry->nPtNode == aEndNode && pEntry->nPtCntnt <= nEndIdx);
        }

        bool bTotallyContained = false;
        if (
             bEntryStartAfterSelStart && bEntryStartBeforeSelEnd &&
             bEntryEndAfterSelStart && bEntryEndBeforeSelEnd
           )
        {
           bTotallyContained = true;
        }

        if (bTotallyContained)
        {
            //after start, before end, delete
            DeleteAndDestroy(nSize);
            continue;
        }

        xub_StrLen nCntntDiff = nEndIdx - nStartIdx;

        //to be adjusted
        if (bEntryStartAfterSelStart)
        {
            if (bEntryStartBeforeSelEnd)
            {
                //move start to new start
                pEntry->nMkNode = aStartNode;
                pEntry->nMkCntnt = nStartIdx;
            }
            else
                pEntry->nMkCntnt = pEntry->nMkCntnt - nCntntDiff;
        }

        if (bEntryEndAfterSelStart)
        {
            if (bEntryEndBeforeSelEnd)
            {
                pEntry->nPtNode = aStartNode;
                pEntry->nPtCntnt = nStartIdx;
            }
            else
                pEntry->nPtCntnt = pEntry->nPtCntnt - nCntntDiff;
        }

        //That's what locked is, end equal to start, and nPtCntnt is invalid
        if (pEntry->bLocked)
            pEntry->nPtNode = pEntry->nMkNode;
    }
}

//------ hier stehen die Methoden von SwFltAnchor -----------
SwFltAnchor::SwFltAnchor(SwFrmFmt* pFmt) :
    SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(pFmt)
{
    pClient = new SwFltAnchorClient(this);
    pFrmFmt->Add(pClient);
}

SwFltAnchor::SwFltAnchor(const SwFltAnchor& rCpy) :
	SfxPoolItem(RES_FLTR_ANCHOR), pFrmFmt(rCpy.pFrmFmt)
{
    pClient = new SwFltAnchorClient(this);
    pFrmFmt->Add(pClient);
}

SwFltAnchor::~SwFltAnchor()
{
    delete pClient;
}

void SwFltAnchor::SetFrmFmt(SwFrmFmt * _pFrmFmt)
{
    pFrmFmt = _pFrmFmt;
}

const SwFrmFmt * SwFltAnchor::GetFrmFmt() const
{
    return pFrmFmt;
}

SwFrmFmt * SwFltAnchor::GetFrmFmt()
{
    return pFrmFmt;
}

int SwFltAnchor::operator==(const SfxPoolItem& rItem) const
{
	return pFrmFmt == ((SwFltAnchor&)rItem).pFrmFmt;
}

SfxPoolItem* __EXPORT SwFltAnchor::Clone(SfxItemPool*) const
{
	return new SwFltAnchor(*this);
}

// SwFltAnchorClient

SwFltAnchorClient::SwFltAnchorClient(SwFltAnchor * pFltAnchor)
: m_pFltAnchor(pFltAnchor)
{
}

void  SwFltAnchorClient::Modify(const SfxPoolItem *, const SfxPoolItem * pNew)
{
    if (pNew->Which() == RES_FMT_CHG)
    {
        const SwFmtChg * pFmtChg = dynamic_cast<const SwFmtChg *> (pNew);

        if (pFmtChg != NULL)
        {
            SwFrmFmt * pFrmFmt = dynamic_cast<SwFrmFmt *> (pFmtChg->pChangedFmt);

            if (pFrmFmt != NULL)
                m_pFltAnchor->SetFrmFmt(pFrmFmt);
        }
    }
}

//------ hier stehen die Methoden von SwFltRedline -----------
int SwFltRedline::operator==(const SfxPoolItem& rItem) const
{
	return this == &rItem;
}

SfxPoolItem* SwFltRedline::Clone( SfxItemPool* ) const
{
	return new SwFltRedline(*this);
}

//------ hier stehen die Methoden von SwFltBookmark -----------
SwFltBookmark::SwFltBookmark( const String& rNa, const String& rVa,
                              long nHand, const bool bIsTOCBookmark )
    : SfxPoolItem( RES_FLTR_BOOKMARK )
    , mnHandle( nHand )
    , maName( rNa )
    , maVal( rVa )
    , mbIsTOCBookmark( bIsTOCBookmark )
{
    // eSrc: CHARSET_DONTKNOW fuer keine UEbersetzung bei operator <<
    // Upcase wird immer gemacht.
    // bei XXXStack.NewAttr(...) wird nie eine UEbersetzung vorgenommen.
    // ansonsten: uebergebener Src-Charset fuer aName
    // im Filter eingestellter Src-Charset fuer aVal ( Text )

    if ( IsTOCBookmark() )
    {
        maName = IDocumentMarkAccess::GetCrossRefHeadingBookmarkNamePrefix();
        maName += rNa;
    }
}

SwFltBookmark::SwFltBookmark(const SwFltBookmark& rCpy)
    : SfxPoolItem( RES_FLTR_BOOKMARK )
    , mnHandle( rCpy.mnHandle )
    , maName( rCpy.maName )
    , maVal( rCpy.maVal )
    , mbIsTOCBookmark( rCpy.mbIsTOCBookmark )
{
}

int SwFltBookmark::operator==(const SfxPoolItem& rItem) const
{
	return ( maName == ((SwFltBookmark&)rItem).maName)
			&& (mnHandle == ((SwFltBookmark&)rItem).mnHandle);
}

SfxPoolItem* SwFltBookmark::Clone(SfxItemPool*) const
{
	return new SwFltBookmark(*this);
}

//------ hier stehen die Methoden von SwFltTOX -----------

SwFltTOX::SwFltTOX(SwTOXBase* pBase, sal_uInt16 _nCols)
	: SfxPoolItem(RES_FLTR_TOX), pTOXBase(pBase), nCols( _nCols ),
	  bHadBreakItem( sal_False ), bHadPageDescItem( sal_False )
{
}

SwFltTOX::SwFltTOX(const SwFltTOX& rCpy)
	: SfxPoolItem(RES_FLTR_TOX), pTOXBase(rCpy.pTOXBase), nCols( rCpy.nCols ),
	  bHadBreakItem( rCpy.bHadBreakItem ), bHadPageDescItem( rCpy.bHadPageDescItem )
{
}

int SwFltTOX::operator==(const SfxPoolItem& rItem) const
{
	return pTOXBase == ((SwFltTOX&)rItem).pTOXBase;
}

SfxPoolItem* SwFltTOX::Clone(SfxItemPool*) const
{
	return new SwFltTOX(*this);
}

//------ hier stehen die Methoden von SwFltSwSection -----------

SwFltSection::SwFltSection(SwSectionData *const pSect)
    : SfxPoolItem(RES_FLTR_SECTION)
    , m_pSection(pSect)
{
}

SwFltSection::SwFltSection(const SwFltSection& rCpy)
    : SfxPoolItem(RES_FLTR_SECTION)
    , m_pSection(rCpy.m_pSection)
{
}

int SwFltSection::operator==(const SfxPoolItem& rItem) const
{
    return m_pSection == ((SwFltSection&)rItem).m_pSection;
}

SfxPoolItem* __EXPORT SwFltSection::Clone(SfxItemPool*) const
{
	return new SwFltSection(*this);
}

///////////////////////////////////////////////////////////////////////
//
// hier beginnt der von mdt erzeugte code. dieser ist eine shell auf
// der writer-seite nach moeglichkeit bald fuer alle filter. die ganze
// schwierigkeit, texte & formatattribute einzufuegen, die positionen
// zu verwalten, styles & kopf/fuszzeilen etc.
//

//////////////////////////////////////////////////////////// SwFltShell
SwFltShell::SwFltShell(SwDoc* pDoc, SwPaM& rPaM, const String& rBaseURL, sal_Bool bNew, sal_uLong nFieldFl) :
	pCurrentPageDesc(0),
	pSavedPos(0),
	eSubMode(None),
	nAktStyle(0),
	aStack(pDoc, nFieldFl),
	aEndStack(pDoc, nFieldFl),
	pPaM(new SwPaM(*(rPaM.GetPoint()))),
    sBaseURL(rBaseURL),
	nPageDescOffset(GetDoc().GetPageDescCnt()),
	eSrcCharSet(RTL_TEXTENCODING_MS_1252),
	bNewDoc(bNew),
	bStdPD(sal_False),
	bProtect(sal_False)
{
	memset( pColls, 0, sizeof( pColls ) );
	pOutDoc = new SwFltOutDoc( *pDoc, pPaM, aStack, aEndStack );
	pOut = pOutDoc;

	if( !bNewDoc ){		// in ein Dokument einfuegen ?
						// Da immer ganze Zeile eingelesen werden, muessen
						// evtl. Zeilen eingefuegt / aufgebrochen werden
		const SwPosition* pPos = pPaM->GetPoint();
		const SwTxtNode* pSttNd = pPos->nNode.GetNode().GetTxtNode();
		sal_uInt16 nCntPos = pPos->nContent.GetIndex();
		if( nCntPos && pSttNd->GetTxt().Len() )
											// EinfuegePos nicht in leerer Zeile
			pDoc->SplitNode( *pPos, false );		// neue Zeile erzeugen
		if( pSttNd->GetTxt().Len() ){		// EinfuegePos nicht am Ende der Zeile
			pDoc->SplitNode( *pPos, false );		// neue Zeile
			pPaM->Move( fnMoveBackward );	// gehe in leere Zeile
		}

		// verhinder das Einlesen von Tabellen in Fussnoten / Tabellen
		sal_uLong nNd = pPos->nNode.GetIndex();
		sal_Bool bReadNoTbl = 0 != pSttNd->FindTableNode() ||
			( nNd < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
			pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() < nNd );
		if( bReadNoTbl )
			pOutDoc->SetReadNoTable();
	}
	pCurrentPageDesc =  &((SwPageDesc&)const_cast<const SwDoc *>(pDoc)
                          ->GetPageDesc( 0 ));	// Standard

}

SwFltShell::~SwFltShell()
{
	sal_uInt16 i;

	if (eSubMode == Style)
		EndStyle();
	if( pOutDoc->IsInTable() )			// falls nicht ordentlich abgeschlossen
		EndTable();
	if( pOutDoc->IsInFly() )
		EndFly();

	GetDoc().SetUpdateExpFldStat(true);
	GetDoc().SetInitDBFields(sal_True);
	aStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
	aStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
	aEndStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
	aEndStack.SetAttr(*pPaM->GetPoint(), 0, sal_False);
	if( bProtect ){     // Das ganze Doc soll geschuetzt sein

		SwDoc& rDoc = GetDoc();
						// 1. SectionFmt und Section anlegen
		SwSectionFmt* pSFmt = rDoc.MakeSectionFmt( 0 );
		SwSectionData aSectionData( CONTENT_SECTION, String::CreateFromAscii(
								RTL_CONSTASCII_STRINGPARAM("PMW-Protect") ));
        aSectionData.SetProtectFlag( true );
						// 2. Start- und EndIdx suchen
		const SwNode* pEndNd = &rDoc.GetNodes().GetEndOfContent();
		SwNodeIndex aEndIdx( *pEndNd, -1L );
		const SwStartNode* pSttNd = pEndNd->StartOfSectionNode();
		SwNodeIndex aSttIdx( *pSttNd, 1L );			// +1 -> hinter StartNode
													// Section einfuegen
						// Section einfuegen
        rDoc.GetNodes().InsertTextSection(
                aSttIdx, *pSFmt, aSectionData, 0, &aEndIdx, false );

		if( !IsFlagSet(SwFltControlStack::DONT_HARD_PROTECT) ){
			SwDocShell* pDocSh = rDoc.GetDocShell();
			if( pDocSh )
				pDocSh->SetReadOnlyUI( sal_True );
		}
	}
		// Pagedescriptoren am Dokument updaten (nur so werden auch die
		// linken Seiten usw. eingestellt).

	GetDoc().ChgPageDesc( 0,
                          const_cast<const SwDoc &>(GetDoc()).
                          GetPageDesc( 0 ));	// PageDesc "Standard"
	for (i=nPageDescOffset;i<GetDoc().GetPageDescCnt();i++)
	{
		const SwPageDesc& rPD = const_cast<const SwDoc &>(GetDoc()).
            GetPageDesc(i);
		GetDoc().ChgPageDesc(i, rPD);
	}

	delete pPaM;
	for (i=0; i<sizeof(pColls)/sizeof(*pColls); i++)
		if( pColls[i] )
			delete pColls[i];
	delete pOutDoc;
}

SwFltShell& SwFltShell::operator << ( const String& rStr )
{
	ASSERT(eSubMode != Style, "char insert while in style-mode");
    GetDoc().InsertString( *pPaM, rStr );
	return *this;
}

void SwFltShell::ConvertUStr( String& rInOut )
{
	GetAppCharClass().toUpper( rInOut );
}

// QuoteString() wandelt CRs abhaengig von nFieldIniFlags in '\n' oder "\0x0d"
String SwFltShell::QuoteStr( const String& rIn )
{
	String sOut( rIn );
	sal_Bool bAllowCr = aStack.IsFlagSet( SwFltControlStack::ALLOW_FLD_CR );

	for( xub_StrLen n = 0; n < sOut.Len(); ++n )
	{
		switch( sOut.GetChar( n ) )
		{
		case 0x0a:
			sOut.Erase( n, 1 );				// 0xd 0xa wird zu \n
			break;

		case 0x0b:
		case 0x0c:
		case 0x0d:
			if( bAllowCr )
			 	sOut.SetChar( n, '\n' );
			break;
		}
	}
	return sOut;
}

SwFltShell& SwFltShell::operator << ( const sal_Unicode c )
{
	ASSERT( eSubMode != Style, "char insert while in style-mode");
    GetDoc().InsertString( *pPaM, c );
	return *this;
}

SwFltShell& SwFltShell::AddError( const sal_Char* pErr )
{
	String aName( String::CreateFromAscii(
					RTL_CONSTASCII_STRINGPARAM( "ErrorTag" )));
	SwFieldType* pFT = GetDoc().GetFldType( RES_SETEXPFLD, aName, false );
	if( pFT == 0)
	{
		SwSetExpFieldType aS(&GetDoc(), aName, nsSwGetSetExpType::GSE_STRING);
		pFT = GetDoc().InsertFldType(aS);
	}
	SwSetExpField aFld( (SwSetExpFieldType*)pFT,
						String::CreateFromAscii( pErr ));
	//, VVF_INVISIBLE
    GetDoc().InsertPoolItem(*pPaM, SwFmtFld(aFld), 0);
	return *this;
}

SwFltShell& SwFltShell::operator << (Graphic& rGraphic)
{
	// embedded Grafik !!
	GetDoc().Insert(*pPaM, aEmptyStr, aEmptyStr, &rGraphic, NULL, NULL, NULL);
	return *this;
}

void SwFltShell::NextParagraph()
{
    GetDoc().AppendTxtNode(*pPaM->GetPoint());
}

void SwFltShell::NextPage()
{
    NextParagraph();
    GetDoc().InsertPoolItem(*pPaM,
        SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE, RES_BREAK), 0);
}

SwFltShell& SwFltShell::AddGraphic( const String& rPicName )
{
	// embedded:
	GraphicFilter* pFilter = GraphicFilter::GetGraphicFilter();
	Graphic aGraphic;
	// one of: GFF_NOT GFF_BMP GFF_GIF GFF_JPG GFF_PCD GFF_PCX GFF_PNG
	// GFF_TIF GFF_XBM GFF_DXF GFF_MET GFF_PCT GFF_SGF GFF_SVM GFF_WMF
	// GFF_SGV GFF_XXX
    INetURLObject aDir(
        URIHelper::SmartRel2Abs(
            INetURLObject(GetBaseURL()), rPicName,
            URIHelper::GetMaybeFileHdl()) );
	switch (pFilter->ImportGraphic(aGraphic, aDir))
	{
		case GRFILTER_OK:
			*this << aGraphic;
			break;
		case GRFILTER_OPENERROR:
		case GRFILTER_IOERROR:
		case GRFILTER_FORMATERROR:
		case GRFILTER_VERSIONERROR:
		case GRFILTER_FILTERERROR:
		case GRFILTER_ABORT:
		case GRFILTER_TOOBIG:
		default:
			AddError( "picture import error" );
			break;
	}
	return *this;
}

SwFltShell& SwFltShell::SetStyle( sal_uInt16 nStyle )
{
	SwFltFormatCollection* p = pColls[ nStyle ];

	if (p)
	{
		if( !pOutDoc->IsInTable() && nStyle != nAktStyle )
		{
			if( pColls[nAktStyle]->IsInFly() && pOutDoc->IsInFly() )
				pOutDoc->EndFly();
			if( p->IsInFly() )
				p->BeginStyleFly( pOutDoc );
		}
		GetDoc().SetTxtFmtColl(*pPaM, p->GetColl());
		nAktStyle = nStyle;
	}
	else
	{
		ASSERT( sal_False, "Ungueltiger SwFltStyleCode" );
	}
	return *this;
}

SwFltShell& SwFltShell::operator << (SwFltBookmark& aBook)
{
	ConvertUStr( aBook.maName );
	aBook.maVal = QuoteStr(aBook.maVal);
	aEndStack.NewAttr(*pPaM->GetPoint(), aBook);
	return *this;
}

void SwFltShell::SetBookEnd(long nHandle)
{
	aEndStack.SetAttr( *pPaM->GetPoint(), RES_FLTR_BOOKMARK, sal_True, nHandle );
}

SwFltShell& SwFltShell::EndItem( sal_uInt16 nAttrId )
{
	switch( nAttrId )
	{
	case RES_FLTR_BOOKMARK:
		ASSERT( sal_False, "Falscher Aufruf fuer Bookmark-Ende" );
		break;

	case RES_FLTR_TOX:
		aEndStack.SetAttr(*pPaM->GetPoint(), nAttrId);
		break;

	default:
		aStack.SetAttr(*pPaM->GetPoint(), nAttrId);
		break;
	}
	return *this;
}

SwFltShell& SwFltShell::operator << (const SwField& rField)
{
    GetDoc().InsertPoolItem(*pPaM, SwFmtFld(rField), 0);
	return *this;
}

/*virtual*/ SwFltOutBase& SwFltOutDoc::operator << (const SfxPoolItem& rItem)
{
	rStack.NewAttr(*pPaM->GetPoint(), rItem);
	return *this;
}

/*virtual*/ SwFltOutBase& SwFltFormatCollection::operator <<
								(const SfxPoolItem& rItem)
{
    pColl->SetFmtAttr(rItem);
	return *this;
}

const SfxPoolItem& SwFltOutDoc::GetAttr(sal_uInt16 nWhich)
{
	return *rStack.GetFmtAttr(*pPaM->GetPoint(), nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetAttr(sal_uInt16 nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   // mit Parents
}

// GetNodeOrStyAttr holt Attribute fuer Toggle- und Modify-Attribute:
// Bei Formatdefinitionen aus dem altuellen Style mit Parents
// sonst aus dem Node mit Parents
// Im Stack wird nicht nachgesehen

const SfxPoolItem& SwFltOutDoc::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    SwCntntNode * pNd = pPaM->GetPoint()->nNode.GetNode().GetCntntNode();
	if (pNd)			// ContentNode: Attribut mit Parent
		return pNd->GetAttr(nWhich);
	else				// kein ContentNode, dann das dflt. Attribut
		return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

const SfxPoolItem& SwFltFormatCollection::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    return GetColl()->GetFmtAttr(nWhich);   // mit Parents
}

const SfxPoolItem& SwFltShell::GetNodeOrStyAttr(sal_uInt16 nWhich)
{
    return pOut->GetNodeOrStyAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetAttr(sal_uInt16 nWhich)
{
    return pOut->GetAttr( nWhich );
}

const SfxPoolItem& SwFltShell::GetFlyFrmAttr(sal_uInt16 nWhich)
{
    return pOut->GetFlyFrmAttr( nWhich );
}

SwFieldType* SwFltShell::GetSysFldType(sal_uInt16 eWhich)
{
    return GetDoc().GetSysFldType(eWhich);
}

sal_Bool SwFltShell::GetWeightBold()
{
	return ((SvxWeightItem&)GetNodeOrStyAttr(RES_CHRATR_WEIGHT)).GetWeight()
								!= WEIGHT_NORMAL;
}

sal_Bool SwFltShell::GetPostureItalic()
{
	return ((SvxPostureItem&)GetNodeOrStyAttr(RES_CHRATR_POSTURE)).GetPosture()
								!= ITALIC_NONE;
}

sal_Bool SwFltShell::GetCrossedOut()
{
	return ((SvxCrossedOutItem&)GetNodeOrStyAttr(RES_CHRATR_CROSSEDOUT))
									.GetStrikeout() != STRIKEOUT_NONE;
}

sal_Bool SwFltShell::GetContour()
{
	return ((SvxContourItem&)GetNodeOrStyAttr(RES_CHRATR_CONTOUR)).GetValue();
}

sal_Bool SwFltShell::GetCaseKapitaelchen()
{
	return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
									.GetCaseMap() == SVX_CASEMAP_KAPITAELCHEN;
}

sal_Bool SwFltShell::GetCaseVersalien()
{
	return ((SvxCaseMapItem&)GetNodeOrStyAttr(RES_CHRATR_CASEMAP))
									.GetCaseMap() == SVX_CASEMAP_VERSALIEN;
}

//-------------------------------------------------------------------------
// Tabellen
//-------------------------------------------------------------------------

SwFltOutBase::~SwFltOutBase()
{
}

SwFltOutBase::SwFltOutBase(SwDoc& rDocu)
    : rDoc(rDocu), eFlyAnchor(FLY_AT_PARA), bFlyAbsPos(false)
{
}

const SfxPoolItem& SwFltOutBase::GetCellAttr(sal_uInt16 nWhich)
{
	ASSERT(sal_False, "GetCellAttr ausserhalb von normalem Text");
	return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

sal_Bool SwFltOutBase::BeginTable()
{
	ASSERT(sal_False, "BeginTable ausserhalb von normalem Text");
	return sal_False;
}

void SwFltOutBase::NextTableCell()
{
	ASSERT(sal_False, "NextTableCell ausserhalb von normalem Text");
}

void SwFltOutBase::NextTableRow()
{
	ASSERT(sal_False, "NextTableRow ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableWidth(SwTwips /*nW*/)
{
	ASSERT(sal_False, "SetTableWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetTableOrient(sal_Int16 /*eOri*/)
{
	ASSERT(sal_False, "SetTableOrient ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellWidth(SwTwips /*nWidth*/, sal_uInt16 /*nCell*/)
{
	ASSERT(sal_False, "SetCellWidth ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellHeight(SwTwips /*nH*/)
{
	ASSERT(sal_False, "SetCellHeight ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellBorder(const SvxBoxItem& /*rFmtBox*/, sal_uInt16 /*nCell*/)
{
	ASSERT(sal_False, "SetCellBorder ausserhalb von normalem Text");
}

void SwFltOutBase::SetCellSpace(sal_uInt16 /*nSp*/)
{
	ASSERT(sal_False, "SetCellSpace ausserhalb von normalem Text");
}

void SwFltOutBase::DeleteCell(sal_uInt16 /*nCell*/)
{
	ASSERT(sal_False, "DeleteCell ausserhalb von normalem Text");
}

void SwFltOutBase::EndTable()
{
	ASSERT(sal_False, "EndTable ausserhalb von normalem Text");
}

/*virtual*/ sal_Bool SwFltOutDoc::IsInTable()
{
	return pTable != 0;
};

sal_Bool SwFltOutDoc::BeginTable()
{
	if(bReadNoTbl)
		return sal_False;

	if (pTable){
		ASSERT(sal_False, "BeginTable in Table");
		return sal_False;
	}
							// Alle Attribute schliessen, da sonst Attribute
							// entstehen koennen, die in Flys reinragen
	rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
	rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

// create table:
	ASSERT(pTabSavedPos == NULL, "SwFltOutDoc");
	pTabSavedPos = new SwPosition(*pPaM->GetPoint());
    pTable = GetDoc().InsertTable(
            SwInsertTableOptions( tabopts::HEADLINE_NO_BORDER, 1 ),
            *pTabSavedPos, 1, 1, text::HoriOrientation::LEFT, 0, 0, sal_False, sal_False ); // TODO MULTIHEADER
	nTableWidth = 0;
	((SwTable*)pTable)->LockModify();	// Nichts automatisch anpassen!
// set pam in 1. table cell
	usTableX =
	usTableY = 0;
	SeekCell(usTableY, usTableX, sal_True);
	return sal_True;
}

SwTableBox* SwFltOutDoc::GetBox(sal_uInt16 ny, sal_uInt16 nx /*= USHRT_MAX */)
{
	if(!pTable){
		ASSERT(pTable, "GetBox ohne Tabelle");
		return 0;
	}
	if( nx == USHRT_MAX )	// aktuelle Zelle
		nx = usTableX;

// get structs to table cells
	const SwTableLines* pTableLines = &pTable->GetTabLines();
	if(!pTableLines){
		ASSERT(sal_False, "SwFltOutDoc:GetBox:pTableLines");
		return 0;
	}
	if( ny >= pTableLines->Count() ){	// Notbremse
		ASSERT( sal_False, "SwFltOutDoc:GetBox:ny >= Count()");
		ny = pTableLines->Count() - 1;
	}
	SwTableLine* pTableLine = (*pTableLines)[ny];
	if(!pTableLine){
		ASSERT(sal_False, "SwFltOutDoc:GetBox:pTableLine");
		return 0;
	}
	SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
	if(!pTableBoxes){
		ASSERT(sal_False, "SwFltOutDoc:GetBox:pTableBoxes");
		return 0;
	}
	if( nx >= pTableBoxes->Count() ){	// Notbremse
		ASSERT(sal_False, "SwFltOutDoc:GetBox:nx >= Count()");
		nx = pTableBoxes->Count() - 1;
	}
	SwTableBox* pTableBox = (*pTableBoxes)[nx];

	ASSERT(pTableBox != 0, "SwFltOutDoc:GetBox:pTableBox");
	return pTableBox;
}

void SwFltOutDoc::NextTableCell()
{
	if(!pTable){
		ASSERT(pTable, "NextTableCell ohne Tabelle");
		return;
	}
	const SwTableLines* pTableLines = &pTable->GetTabLines();
	SwTableLine* pTableLine = (*pTableLines)[usTableY];
	SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
	SwTableBox* pTableBox = (*pTableBoxes)[usTableX];
	ASSERT(pTableBox != 0, "SwFltOutDoc:NextTableCell:pTableBox");
	if(!pTableBox)
		return;
//#pragma message(__FILE__ "(?) : Sw's const problem")
// insert cells:
	if (++usTableX >= pTableBoxes->Count())
		GetDoc().GetNodes().InsBoxen(
		 GetDoc().IsIdxInTbl(pPaM->GetPoint()->nNode),
		 pTableLine,
		 (SwTableBoxFmt*)pTableBox->GetFrmFmt(),
         GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ),
		 0,
		 pTableBoxes->Count());
	SeekCell(usTableY, usTableX, sal_True);
	pTableBox = (*pTableBoxes)[usTableX];
	ASSERT(pTableBox != 0, "SwFltOutDoc:pTableBox");
	if(pTableBox)
		(*pTableBoxes)[usTableX]->ClaimFrmFmt();
}

void SwFltOutDoc::NextTableRow()
{
	SwTableBox* pTableBox = GetBox(usTableY, 0);
	if (pTableBox)
	{
// duplicate row:
		SwSelBoxes aSelBoxes;
		aSelBoxes.Insert( pTableBox );
		GetDoc().InsertRow(aSelBoxes);
		usTableX = 0;
		SeekCell(++usTableY, usTableX, sal_True);
		GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
	}
}

void SwFltOutDoc::SetTableWidth(SwTwips nSwWidth)
{
	if(!pTable){
		ASSERT(pTable, "SetTableWidth ohne Tabelle");
		return;
	}
	ASSERT( nSwWidth > MINLAY, "Tabellenbreite <= MINLAY" );
	if( nSwWidth != nTableWidth ){
		if( nTableWidth )			// Nicht beim ersten Setzen
			SplitTable();
        pTable->GetFrmFmt()->SetFmtAttr( SwFmtFrmSize(ATT_VAR_SIZE, nSwWidth));
		nTableWidth = nSwWidth;
	}
}

void SwFltOutDoc::SetTableOrient(sal_Int16 eOri)
{
	if(!pTable){
		ASSERT(pTable, "SetTableOrient ohne Tabelle");
		return;
	}
    pTable->GetFrmFmt()->SetFmtAttr( SwFmtHoriOrient( 0, eOri ));
}

void SwFltOutDoc::SetCellWidth(SwTwips nWidth, sal_uInt16 nCell /* = USHRT_MAX */ )
{
	if(!pTable){
		ASSERT(pTable, "SetCellWidth ohne Tabelle");
		return;
	}
	ASSERT( nWidth > MINLAY, "Tabellenzellenbreite <= MINLAY" );
	if (nWidth < MINLAY)
		nWidth = MINLAY;

	SwTableBox* pTableBox = GetBox(usTableY, nCell);
	if(pTableBox && pTableBox->GetFrmFmt() ){
		SwFmtFrmSize aFmtFrmSize(ATT_FIX_SIZE);
		aFmtFrmSize.SetWidth(nWidth);
        pTableBox->GetFrmFmt()->SetFmtAttr(aFmtFrmSize);
	}
}

void SwFltOutDoc::SetCellHeight(SwTwips nHeight)
{
	if(!pTable){
		ASSERT(pTable, "SetCellHeight ohne Tabelle");
		return;
	}

	const SwTableLines* pTableLines = &pTable->GetTabLines();
	SwTableLine* pTableLine = (*pTableLines)[usTableY];
	SwFmtFrmSize aFmtFrmSize(ATT_MIN_SIZE, 0, 0);
	if (nHeight < MINLAY)
		nHeight = MINLAY;
	aFmtFrmSize.SetHeight(nHeight);
    pTableLine->GetFrmFmt()->SetFmtAttr(aFmtFrmSize);
}

const SfxPoolItem& SwFltOutDoc::GetCellAttr(sal_uInt16 nWhich)
{
	if (!pTable){
		ASSERT(pTable, "GetCellAttr ohne Table");
		return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
	}

	SwTableBox* pTableBox = GetBox(usTableY, usTableX);
	if(!pTableBox)
		return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
    return pTableBox->GetFrmFmt()->GetFmtAttr( nWhich );
}

void SwFltOutDoc::SetCellBorder(const SvxBoxItem& rFmtBox,
							   sal_uInt16 nCell /* = USHRT_MAX */ )
{
	SwTableBox* pTableBox = GetBox(usTableY, nCell);
	if(pTableBox)
        pTableBox->GetFrmFmt()->SetFmtAttr(rFmtBox);
}

// nicht aktiviert !!!
void SwFltOutDoc::SetCellSpace(sal_uInt16 nDist)
{
	if(!pTable){
		ASSERT(pTable, "SetCellSpace ohne Tabelle");
		return;
	}
	SwTableBox* pTableBox = GetBox(usTableY, usTableX);
	if(!pTableBox)
		return;

	SvxBoxItem aFmtBox( *((SvxBoxItem*)
                        &pTableBox->GetFrmFmt()->GetFmtAttr( RES_BOX )));

	// versteh ich nich, sven: if (!nDist) nDist = 18; // ca. 0.03 cm
	if (nDist > 42) // max. 0.7 mm
		nDist = 42;
	else
		if (nDist < MIN_BORDER_DIST)
			nDist = MIN_BORDER_DIST;
	aFmtBox.SetDistance(nDist);
    pTableBox->GetFrmFmt()->SetFmtAttr(aFmtBox);
}

void SwFltOutDoc::DeleteCell(sal_uInt16 nCell /* = USHRT_MAX */)
{
	SwTableBox* pTableBox = GetBox(usTableY, nCell);
	if(pTableBox){
		SwSelBoxes aSelBoxes;
		aSelBoxes.Insert( pTableBox );
		GetDoc().DeleteRowCol(aSelBoxes);
		usTableX--;
	}
}

void SwFltOutDoc::SplitTable()
{
	if(!pTable)
	{
		ASSERT(pTable, "SplitTable ohne Tabelle");
		return;
	}
	SwTableBox* pAktBox = GetBox(usTableY, usTableX);
	SwTableBox* pSplitBox = GetBox(usTableY - 1, 0);
	GetDoc().GetNodes().SplitTable(SwNodeIndex(*pSplitBox->GetSttNd()), false);
	pTable = &pAktBox->GetSttNd()->FindTableNode()->GetTable();
	usTableY = 0;
}

void SwFltOutDoc::EndTable()
{
	if (!pTable){
		ASSERT(pTable, "EndTable ohne Table");
		return;
	}
							// Alle Attribute schliessen, da sonst Attribute
							// entstehen koennen, die in Flys reinragen
	rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
	rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

	if (GetDoc().GetCurrentViewShell()){	//swmod 071108//swmod 071225
		SwTableNode* pTableNode = GetDoc().IsIdxInTbl(
			pPaM->GetPoint()->nNode);
		pTableNode->DelFrms();
		pTableNode->MakeFrms(&pPaM->GetPoint()->nNode);
	}

	*pPaM->GetPoint() = *pTabSavedPos;				// restore Cursor
	delete pTabSavedPos;
	pTabSavedPos = 0;
	((SwTable*)pTable)->UnlockModify();	// Test, nuetzt nichts gegen Assert
	pTable = 0;
	nTableWidth = 0;
}

sal_Bool SwFltOutDoc::SeekCell(short nRow, short nCol, sal_Bool bPam)
{
// get structs to table cells
	const SwTableLines* pTableLines = &pTable->GetTabLines();
	SwTableLine* pTableLine = (*pTableLines)[usTableY];
	SwTableBoxes* pTableBoxes = &pTableLine->GetTabBoxes();
	SwTableBox* pTableBox = (*pTableBoxes)[usTableX];

	if ((sal_uInt16)nRow >= pTableLines->Count())
	{
		ASSERT((sal_uInt16)nRow >= pTableLines->Count(), "SwFltOutDoc");
		return sal_False;
	}
	pTableLine = (*pTableLines)[nRow];
	pTableBoxes = &pTableLine->GetTabBoxes();
	if (nCol >= pTableBoxes->Count())
		return sal_False;
	pTableBox = (*pTableBoxes)[nCol];
	if( !pTableBox->GetSttNd() )
	{
		ASSERT(pTableBox->GetSttNd(), "SwFltOutDoc");
		return sal_False;
	}
	if(bPam)
	{
		pPaM->GetPoint()->nNode = pTableBox->GetSttIdx() + 1;
		pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
//#pragma message(__FILE__ "(?) : Sw's const problem")
#if OSL_DEBUG_LEVEL > 1
		const SwTxtFmtColl* p = GetDoc().GetDfltTxtFmtColl();
        p = GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false );
#endif
		GetDoc().SetTxtFmtColl(*pPaM,
            GetDoc().GetTxtCollFromPool(RES_POOLCOLL_STANDARD, false ));
	}
	return sal_True;
}


//-----------------------------------------------------------------------------
// Flys in SwFltOutBase
//-----------------------------------------------------------------------------

SfxItemSet* SwFltOutBase::NewFlyDefaults()
{
// Unbedingt noetige Standardwerte setzen ( falls diese Werte nicht
// spaeter explizit gesetzt werden )

	SfxItemSet* p = new SfxItemSet( GetDoc().GetAttrPool(),
									RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
	SwFmtFrmSize aSz( ATT_VAR_SIZE, MINFLY, MINFLY );
										// Default: Breite 100% ( = PMW:Auto )
	aSz.SetWidthPercent( 100 );			// Hoehe: Auto
	p->Put( aSz );
    p->Put( SwFmtHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::FRAME ));
	return p;
}

sal_Bool SwFltOutBase::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
						   sal_Bool bAbsolutePos /*= sal_False*/,
						   const SfxItemSet*
#ifdef DBG_UTIL
						   	pMoreAttrs /*= 0*/
#endif
							)
{
	ASSERT(!pMoreAttrs, "SwFltOutBase:BeginFly mit pMoreAttrs" );
	eFlyAnchor = eAnchor;
	bFlyAbsPos = bAbsolutePos;		// Bloedsinn eigentlich
	return sal_True;
}

/*virtual*/ void SwFltOutBase::SetFlyAnchor( RndStdIds eAnchor )
{
	if( !IsInFly() ){
		ASSERT( sal_False, "SetFlyAnchor() ohne Fly" );
		return;
	}
    if ( eAnchor == FLY_AS_CHAR ){
        ASSERT( sal_False, "SetFlyAnchor( FLY_AS_CHAR ) nicht implementiert" );
		return;
	}
	SwFmtAnchor& rAnchor = (SwFmtAnchor&)GetFlyFrmAttr( RES_ANCHOR );
	rAnchor.SetType( eAnchor );
}

void SwFltOutBase::EndFly()
{
	if( bFlyAbsPos ){
		// hier muessen die absoluten Positionen am Fly noch in
		// die Writer-Koordinaten umgerechnet werden.
	}
}

//-----------------------------------------------------------------------------
// Flys in SwFltDoc
//-----------------------------------------------------------------------------

/* virtual */ sal_Bool SwFltOutDoc::IsInFly()
{
	return pFly != 0;
};

SwFrmFmt* SwFltOutDoc::MakeFly( RndStdIds eAnchor, SfxItemSet* pSet )
{
	pFly = (SwFlyFrmFmt*)GetDoc().MakeFlySection( eAnchor, pPaM->GetPoint(),
													pSet );
	return pFly;
}

sal_Bool SwFltOutDoc::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
						   sal_Bool bAbsolutePos /*= sal_False*/,
						   const SfxItemSet* pMoreAttrs /*= 0*/ )

{
	SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, 0 );
	SfxItemSet* pSet = NewFlyDefaults();

// Alle Attribute schliessen, da sonst Attribute entstehen koennen,
// die in Flys reinragen
	rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
	rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

// create Fly:
	ASSERT(pFlySavedPos == NULL, "BeginFly in Fly");	// rekursiv geht noch nicht
	pFlySavedPos = new SwPosition(*pPaM->GetPoint());


	SwFmtAnchor aAnchor( eAnchor, 1 );

// Wenn Fly-Attribute im Style waren, dann jetzt als Defaults reinsetzen
	if (pMoreAttrs)
		pSet->Put(*pMoreAttrs);

//  dieses NICHT bei Seitengebundenem Fly mit Seiten-NUMMER !
	aAnchor.SetAnchor(pPaM->GetPoint());	// braucht erstaunlicherweise
											// den Stack nicht
//	aStack.NewAttr( *pPaM->GetPoint(), SwFltAnchor( pFly ) );

	pSet->Put( aAnchor );
	SwFrmFmt* pF = MakeFly( eAnchor, pSet );
	delete pSet;

// set pam in Fly
	const SwFmtCntnt& rCntnt = pF->GetCntnt();
	ASSERT( rCntnt.GetCntntIdx(), "Kein Inhalt vorbereitet." );
	pPaM->GetPoint()->nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
	SwCntntNode *pNode = pPaM->GetCntntNode();
	pPaM->GetPoint()->nContent.Assign( pNode, 0 );

	return sal_True;
}

/*virtual*/ void SwFltOutDoc::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
	if (pFly){
        pFly->SetFmtAttr( rAttr );
    }else{
		ASSERT(pFly, "SetFlyAttr ohne Doc-Fly");
        return;
    }
}

/*virtual*/ const SfxPoolItem& SwFltOutDoc::GetFlyFrmAttr(sal_uInt16 nWhich)
{
	if (pFly){
        return pFly->GetFmtAttr( nWhich );
	}else{
		ASSERT(pFly, "GetFlyAttr ohne Fly");
		return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
	}
}

void SwFltOutDoc::EndFly()
{
	if( pTable ){
		ASSERT( sal_False, "SwFltOutDoc::EndFly() in Table" );
		return;
	}
						// Alle Attribute schliessen, da sonst Attribute
						// entstehen koennen, die aus Flys rausragen
	rStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
	rEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );

	*pPaM->GetPoint() = *pFlySavedPos;				// restore Cursor
	delete pFlySavedPos;
	pFlySavedPos = 0;
	SwFltOutBase::EndFly();
	pFly = 0;
}

//-----------------------------------------------------------------------------
// Flys in SwFltFormatCollection
//-----------------------------------------------------------------------------
/*virtual*/ sal_Bool SwFltFormatCollection::IsInFly()
{
	return bHasFly;
};

/*virtual*/ void SwFltFormatCollection::SetFlyFrmAttr(const SfxPoolItem& rAttr)
{
	if (!pFlyAttrs)
		pFlyAttrs = new	SfxItemSet( GetDoc().GetAttrPool(),
							 RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
	pFlyAttrs->Put( rAttr );
}

/*virtual*/ const SfxPoolItem& SwFltFormatCollection::GetFlyFrmAttr(sal_uInt16 nWhich)
{
//	ASSERT( pFlyAttrs, "GetFlyFrmAttr ohne Coll-FlyAttrs" );
	if( pFlyAttrs )
		return pFlyAttrs->Get( nWhich, sal_False );
	else
		return GetDoc().GetAttrPool().GetDefaultItem(nWhich);
}

sal_Bool SwFltFormatCollection::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
						   sal_Bool bAbsolutePos /*= sal_False*/,
						   const SfxItemSet* pMoreAttrs /*= 0*/ )

{
	SwFltOutBase::BeginFly( eAnchor, bAbsolutePos, pMoreAttrs );
	bHasFly = sal_True;
	return sal_True;
}

void SwFltFormatCollection::EndFly()	// Wird nie aufgerufen
{
}

sal_Bool SwFltFormatCollection::BeginStyleFly( SwFltOutDoc* pOutDoc )
{
	ASSERT( pOutDoc, "BeginStyleFly ohne pOutDoc" );
	ASSERT( pOutDoc && !pOutDoc->IsInFly(), "BeginStyleFly in Fly" );
	if( pOutDoc && !pOutDoc->IsInFly() )
		return pOutDoc->BeginFly( eFlyAnchor, bFlyAbsPos, pFlyAttrs );
	else
		return sal_False;
}

//-----------------------------------------------------------------------------
// Flys in SwFltShell
//-----------------------------------------------------------------------------

sal_Bool SwFltShell::BeginFly( RndStdIds eAnchor /*= FLY_AT_PARA*/,
						   sal_Bool bAbsolutePos /*= sal_False*/ )

{
	if (pOut->IsInFly()){
		ASSERT(sal_False, "BeginFly in Fly");
		return sal_False;
	}
	if (pOutDoc->IsInTable()){
		ASSERT(sal_False, "BeginFly in Table");
		return sal_False;
	}
	pOut->BeginFly( eAnchor, bAbsolutePos, pColls[nAktStyle]->GetpFlyAttrs() );
	eSubMode = Fly;
	return sal_True;
}

void SwFltShell::SetFlyXPos( short nXPos, sal_Int16 eHRel /*= text::RelOrientation::FRAME*/,
                             sal_Int16 eHAlign /*= text::HoriOrientation::NONE*/ )
{
	SetFlyFrmAttr( SwFmtHoriOrient( nXPos, eHAlign, eHRel ) );
}

void SwFltShell::SetFlyYPos( short nYPos, sal_Int16 eVRel /*= text::RelOrientation::FRAME*/,
                             sal_Int16 eVAlign /*= text::VertOrientation::NONE*/ )
{
	SetFlyFrmAttr( SwFmtVertOrient( nYPos, eVAlign, eVRel ) );
}


void SwFltShell::EndFly()
{
	if (!pOut->IsInFly()){
		ASSERT(sal_False, "EndFly ohne Fly");
		return;
	}
	if (pOutDoc->IsInTable()){		// Table verschraenkt mit Fly macht keinen Sinn
		ASSERT(sal_False, "EndFly in Table ( verschraenkt )");
		EndTable();     // -> Table beenden
	}
	pOut->EndFly();
	eSubMode = None;
}

//-----------------------------------------------------------------------------
// Fussnoten
//-----------------------------------------------------------------------------

void SwFltShell::BeginFootnote()
{
	if( pOut->IsInFly() ){			// Passiert z.B. bei Fussnote in Fly
		ASSERT(sal_False, "Fussnote in Fly nicht erlaubt");
		return;
	}
	if( pOutDoc->IsInTable() ){
		ASSERT(sal_False, "Fussnote in Table z.Zt. nicht erlaubt");
		return;
	}

// Alle Attribute schliessen, da sonst Attribute entstehen koennen,
// die in Fussnoten reinragen
	aStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
//	aEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
//	EndStack erstmal nicht zwangs-Schliessen, damit Bookmarks ueber
//  Fussnoten im PMW uebernommen werden

	SwFmtFtn aFtn;
    GetDoc().InsertPoolItem(*pPaM, aFtn, 0);
	ASSERT(pSavedPos == NULL, "SwFltShell");
	pSavedPos = new SwPosition(*pPaM->GetPoint());
	pPaM->Move(fnMoveBackward, fnGoCntnt);
	SwTxtNode* pTxt = pPaM->GetNode()->GetTxtNode();
    SwTxtAttr *const pFN = pTxt->GetTxtAttrForCharAt(
        pPaM->GetPoint()->nContent.GetIndex(), RES_TXTATR_FTN);
	if( !pFN ){			// Passiert z.B. bei Fussnote in Fly
		ASSERT(pFN, "Probleme beim Anlegen des Fussnoten-Textes");
		return;
	}
	const SwNodeIndex* pStartIndex = ((SwTxtFtn*)pFN)->GetStartNode();
	ASSERT(pStartIndex, "Probleme beim Anlegen des Fussnoten-Textes");
	pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
	pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
	eSubMode = Footnote;
}

void SwFltShell::EndFootnote()
{
	if(!pSavedPos)
		return;
						// Alle Attribute schliessen, da sonst Attribute
						// entstehen koennen, die aus Fussnoten rausragen
	aStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
//	aEndStack.SetAttr( *pPaM->GetPoint(), 0, sal_False );
//	EndStack erstmal nicht zwangs-Schliessen, damit Bookmarks ueber
//  Fussnoten im PMW uebernommen werden

	*pPaM->GetPoint() = *pSavedPos;				// restore Cursor
	delete pSavedPos;
	pSavedPos = 0;
}

void SwFltShell::BeginHeader(SwPageDesc* /*pPD*/)
{
	SwFrmFmt* pFmt = &pCurrentPageDesc->GetMaster(
	 ); //(bUseLeft) ?  &pCurrentPageDesc->GetLeft() :
	SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtHeader(sal_True));
	pHdFtFmt = (SwFrmFmt*)pFmt->GetHeader().GetHeaderFmt();
	const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
	if (!pStartIndex)
		return;
	ASSERT(pSavedPos == NULL, "SwFltShell");
	pSavedPos = new SwPosition(*pPaM->GetPoint());
	pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
	pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
	eSubMode = Header;
}

void SwFltShell::BeginFooter(SwPageDesc* /*pPD*/)
{
	SwFrmFmt* pFmt =  &pCurrentPageDesc->GetMaster(
	 ); //(bUseLeft) ?  &pCurrentPageDesc->GetLeft() :
	SwFrmFmt* pHdFtFmt;
    pFmt->SetFmtAttr(SwFmtFooter(sal_True));
	pHdFtFmt = (SwFrmFmt*)pFmt->GetFooter().GetFooterFmt();
	const SwNodeIndex* pStartIndex = pHdFtFmt->GetCntnt().GetCntntIdx();
	if (!pStartIndex)
		return;
	ASSERT(pSavedPos == NULL, "SwFltShell");
	pSavedPos = new SwPosition(*pPaM->GetPoint());
	pPaM->GetPoint()->nNode = pStartIndex->GetIndex() + 1;
	pPaM->GetPoint()->nContent.Assign(pPaM->GetCntntNode(), 0);
	eSubMode = Footer;
}

void SwFltShell::EndHeaderFooter()
{
	*pPaM->GetPoint() = *pSavedPos;				// restore Cursor
	delete pSavedPos;
	pSavedPos = 0;
}

SwPageDesc* SwFltShell::MakePageDesc(SwPageDesc* pFirstPageDesc)
{
	if(bStdPD)						// keine Neuen PageDescs
		return pCurrentPageDesc;

	sal_Bool bFollow = (pFirstPageDesc != 0);
	SwPageDesc* pNewPD;
	sal_uInt16 nPos;
	if (bFollow && pFirstPageDesc->GetFollow() != pFirstPageDesc)
		return pFirstPageDesc;		// Fehler: hat schon Follow
// Erkennung doppelter Namen fehlt noch (Wahrscheinlichkeit
// fuer dopp. Namen ist gering)

	nPos = GetDoc().MakePageDesc( ViewShell::GetShellRes()->GetPageDescName(
						   		GetDoc().GetPageDescCnt(), sal_False, bFollow ),
								pFirstPageDesc, sal_False );

	pNewPD =  &((SwPageDesc&)const_cast<const SwDoc &>(GetDoc()).
                GetPageDesc(nPos));
	if (bFollow)
	{				// Dieser ist der folgende von pPageDesc
		pFirstPageDesc->SetFollow(pNewPD);
		pNewPD->SetFollow(pNewPD);
	}
	else
    {
        GetDoc().InsertPoolItem( *pPaM, SwFmtPageDesc( pNewPD ), 0 );
    }
	pNewPD->WriteUseOn( // alle Seiten
	 (UseOnPage)(nsUseOnPage::PD_ALL | nsUseOnPage::PD_HEADERSHARE | nsUseOnPage::PD_FOOTERSHARE));
	return pNewPD;
}

///////////////////////////////////////////////// SwFltFormatCollection
SwFltFormatCollection::SwFltFormatCollection(
    SwDoc& _rDoc, RES_POOL_COLLFMT_TYPE nType ) :
	SwFltOutBase(_rDoc),
    pColl(_rDoc.GetTxtCollFromPool( static_cast< sal_uInt16 >(nType), false )),
	pFlyAttrs( 0 ),
	bHasFly( sal_False )
{
	Reset();			// Default-Attrs loeschen und Auto-Flag
}

SwFltFormatCollection::SwFltFormatCollection(
	SwDoc& _rDoc, const String& rName ) :
	SwFltOutBase(_rDoc),
	pFlyAttrs( 0 ),
	bHasFly( sal_False )
{
	pColl = _rDoc.MakeTxtFmtColl(rName, (SwTxtFmtColl*)_rDoc.GetDfltTxtFmtColl());
	Reset();			// Default-Attrs loeschen und Auto-Flag
}

void SwFltShell::NextStyle(sal_uInt16 nWhich, sal_uInt16 nNext)
{
		ASSERT(pColls[nWhich], "Next style for noexistent style" );
		ASSERT(pColls[nNext], "Next style to noexistent style" );
		if( pColls[nWhich] && pColls[nNext] )
			pColls[nWhich]->GetColl()->SetNextTxtFmtColl(
				 *pColls[nNext]->GetColl() );
}

// UpdatePageDescs muss am Ende des Einlesevorganges aufgerufen werden, damit
// der Writer den Inhalt der Pagedescs wirklich akzeptiert
void UpdatePageDescs(SwDoc &rDoc, sal_uInt16 nInPageDescOffset)
{
	// Pagedescriptoren am Dokument updaten (nur so werden auch die
	// linken Seiten usw. eingestellt).

	// PageDesc "Standard"
	rDoc.ChgPageDesc(0, const_cast<const SwDoc &>(rDoc).GetPageDesc(0));

	// PageDescs "Konvert..."
	for (sal_uInt16 i = nInPageDescOffset; i < rDoc.GetPageDescCnt(); ++i)
		rDoc.ChgPageDesc(i, const_cast<const SwDoc &>(rDoc).GetPageDesc(i));
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
