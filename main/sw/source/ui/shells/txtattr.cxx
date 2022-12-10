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

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <svl/whiter.hxx>
#include <svl/stritem.hxx>
#include <svl/itemiter.hxx>
#include <svl/ctloptions.hxx>
#include <swmodule.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/escpitem.hxx>
#include <svx/htmlmode.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include "paratr.hxx"

#include <fmtinfmt.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <viewopt.hxx>
#include <uitool.hxx>
#ifndef _TEXTSH_HXX
#include <textsh.hxx>
#endif
#include <num.hxx>
#include <swundo.hxx>
#include <fmtcol.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#include <globals.h>
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#include <SwStyleNameMapper.hxx>
#include "swabstdlg.hxx"
#include "chrdlg.hrc"
const SwTwips lFontInc = 2 * 20;		   // ==> PointToTwips(2)
const SwTwips lFontMaxSz = 72 * 20; 	   // ==> PointToTwips(72)




void SwTextShell::ExecCharAttr(SfxRequest &rReq)
{
	SwWrtShell &rSh = GetShell();
	const SfxItemSet  *pArgs   = rReq.GetArgs();
		  int          eState = STATE_TOGGLE;
	sal_uInt16 nWhich = rReq.GetSlot();

	if(pArgs )
	{
		const SfxPoolItem* pItem;
		pArgs->GetItemState(nWhich, sal_False, &pItem);
		eState =  ((const SfxBoolItem &) pArgs->
								Get( nWhich )).GetValue() ? STATE_ON : STATE_OFF;
	}


	SfxItemSet aSet( GetPool(), RES_CHRATR_BEGIN, RES_CHRATR_END-1 );
	if (STATE_TOGGLE == eState)
        rSh.GetCurAttr( aSet );

	switch ( nWhich )
	{
		case FN_SET_SUB_SCRIPT:
		case FN_SET_SUPER_SCRIPT:
		{
            SvxEscapement eEscape = SVX_ESCAPEMENT_SUBSCRIPT;
			switch (eState)
			{
			case STATE_TOGGLE:
			{
				short nTmpEsc = ((const SvxEscapementItem&)
							aSet.Get( RES_CHRATR_ESCAPEMENT )).GetEsc();
				eEscape = nWhich == FN_SET_SUPER_SCRIPT ?
								SVX_ESCAPEMENT_SUPERSCRIPT:
								SVX_ESCAPEMENT_SUBSCRIPT;
				if( (nWhich == FN_SET_SUB_SCRIPT && nTmpEsc < 0) ||
							(nWhich == FN_SET_SUPER_SCRIPT && nTmpEsc > 0) )
					eEscape = SVX_ESCAPEMENT_OFF;

				SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
				if( nWhich == FN_SET_SUB_SCRIPT )
					rBind.SetState( SfxBoolItem( FN_SET_SUPER_SCRIPT,
																	sal_False ) );
				else
					rBind.SetState( SfxBoolItem( FN_SET_SUB_SCRIPT,
																	sal_False ) );

			}
			break;
			case STATE_ON:
				eEscape = nWhich == FN_SET_SUPER_SCRIPT ?
								SVX_ESCAPEMENT_SUPERSCRIPT:
								SVX_ESCAPEMENT_SUBSCRIPT;
				break;
			case STATE_OFF:
				eEscape = SVX_ESCAPEMENT_OFF;
				break;
			}
            SvxEscapementItem aEscape( eEscape, RES_CHRATR_ESCAPEMENT );
			if(eEscape == SVX_ESCAPEMENT_SUPERSCRIPT)
				aEscape.GetEsc() = DFLT_ESC_AUTO_SUPER;
			else if(eEscape == SVX_ESCAPEMENT_SUBSCRIPT)
				aEscape.GetEsc() = DFLT_ESC_AUTO_SUB;
			if(eState != STATE_OFF )
			{
				if(eEscape == FN_SET_SUPER_SCRIPT)
					aEscape.GetEsc() *= -1;
			}
			rSh.SetAttrItem( aEscape );
            rReq.AppendItem( aEscape );
            rReq.Done();
		}
		break;

		case FN_UPDATE_STYLE_BY_EXAMPLE:
			rSh.QuickUpdateStyle();
            rReq.Done();
			break;
		case FN_UNDERLINE_DOUBLE:
		{
			FontUnderline eUnderline =  ((const SvxUnderlineItem&)
							aSet.Get(RES_CHRATR_UNDERLINE)).GetLineStyle();
			switch( eState )
			{
				case STATE_TOGGLE:
					eUnderline = eUnderline == UNDERLINE_DOUBLE ?
						UNDERLINE_NONE :
							UNDERLINE_DOUBLE;
				break;
				case STATE_ON:
					eUnderline = UNDERLINE_DOUBLE;
				break;
				case STATE_OFF:
					eUnderline = UNDERLINE_NONE;
				break;
			}
            SvxUnderlineItem aUnderline(eUnderline, RES_CHRATR_UNDERLINE );
            rSh.SetAttrItem( aUnderline );
            rReq.AppendItem( aUnderline );
            rReq.Done();
		}
		break;
        case FN_REMOVE_DIRECT_CHAR_FORMATS:
            if( !rSh.HasReadonlySel() && rSh.IsEndPara())
                rSh.DontExpandFmt();
        break;
		default:
			ASSERT(sal_False, falscher Dispatcher);
			return;
	}
}


void SwTextShell::ExecCharAttrArgs(SfxRequest &rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_Bool bArgs = pArgs != 0 && pArgs->Count() > 0;
    int bGrow = sal_False;
    SwWrtShell& rWrtSh = GetShell();
    SwTxtFmtColl* pColl = 0;

    // nur gesetzt, wenn gesamter Absatz selektiert ist und AutoUpdateFmt gesetzt ist
    if ( rWrtSh.HasSelection() && rWrtSh.IsSelFullPara() )
    {
        pColl = rWrtSh.GetCurTxtFmtColl();
        if ( pColl && !pColl->IsAutoUpdateFmt() )
            pColl = 0;
    }
    SfxItemPool& rPool = GetPool();
    sal_uInt16 nWhich = rPool.GetWhich( nSlot );
    switch (nSlot)
    {
    case FN_TXTATR_INET:
        // Sonderbehandlung der PoolId des SwFmtInetFmt
        if ( bArgs )
        {
            const SfxPoolItem& rItem = pArgs->Get( nWhich );

            SwFmtINetFmt aINetFmt( (const SwFmtINetFmt&) rItem );
            if ( USHRT_MAX == aINetFmt.GetVisitedFmtId() )
            {
                ASSERT( false, "<SwTextShell::ExecCharAttrArgs(..)> - unexpected visited character format ID at hyperlink attribute" );
                aINetFmt.SetVisitedFmtAndId(
                        aINetFmt.GetVisitedFmt(),
                        SwStyleNameMapper::GetPoolIdFromUIName( aINetFmt.GetVisitedFmt(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT ) );
            }
            if ( USHRT_MAX == aINetFmt.GetINetFmtId() )
            {
                ASSERT( false, "<SwTextShell::ExecCharAttrArgs(..)> - unexpected unvisited character format ID at hyperlink attribute" );
                aINetFmt.SetINetFmtAndId(
                        aINetFmt.GetINetFmt(),
                        SwStyleNameMapper::GetPoolIdFromUIName( aINetFmt.GetINetFmt(), nsSwGetPoolIdFromName::GET_POOLID_CHRFMT ) );
            }

            if ( pColl )
                pColl->SetFmtAttr( aINetFmt );
            else
                rWrtSh.SetAttrItem( aINetFmt );
            rReq.Done();
        }
        break;

		case FN_GROW_FONT_SIZE:
			bGrow = sal_True;
			// kein break !!
		case FN_SHRINK_FONT_SIZE:
		{
			SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT, rPool );
            rWrtSh.GetCurAttr( aSetItem.GetItemSet() );
			SfxItemSet aAttrSet( rPool, aSetItem.GetItemSet().GetRanges() );

			const SfxPoolItem* pI;
			static const sal_uInt16 aScrTypes[] = {
				SCRIPTTYPE_LATIN, SCRIPTTYPE_ASIAN, SCRIPTTYPE_COMPLEX, 0 };
			sal_uInt16 nScriptType = rWrtSh.GetScriptType();
			for( const sal_uInt16* pScrpTyp = aScrTypes; *pScrpTyp; ++pScrpTyp )
				if( ( nScriptType & *pScrpTyp ) &&
					0 != ( pI = aSetItem.GetItemOfScript( *pScrpTyp )))
				{
					SvxFontHeightItem aSize( *(const SvxFontHeightItem*)pI );
					SwTwips lSize = (SwTwips) aSize.GetHeight();

					if (bGrow)
					{
						if( lSize == lFontMaxSz )
							break;		// das wars, hoeher gehts nicht
						if( ( lSize += lFontInc ) > lFontMaxSz )
							lSize = lFontMaxSz;
					}
					else
					{
						if( 4 == lSize )
							break;
						if( ( lSize -= lFontInc ) < 4 )
							lSize = 4;
					}
					aSize.SetHeight( lSize );
					aAttrSet.Put( aSize );
				}
				if( aAttrSet.Count() )
				{
					if( pColl )
                        pColl->SetFmtAttr( aAttrSet );
					else
						rWrtSh.SetAttrSet( aAttrSet );
				}
            rReq.Done();
		}
		break;

		default:
			ASSERT(sal_False, falscher Dispatcher);
			return;
	}
}



#ifdef CFRONT

void lcl_SetAdjust(SvxAdjust eAdjst, SfxItemSet& rSet)
{
	rSet.Put(SvxAdjustItem(eAdjst,RES_PARATR_ADJUST ));
}



void lcl_SetLineSpace(sal_uInt8 ePropL,SfxItemSet& rSet)
{
	SvxLineSpacingItem aLineSpacing(ePropL, RES_PARATR_LINESPACING );
	aLineSpacing.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
	if( 100 == ePropL )
		aLineSpacing.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
	else
		aLineSpacing.SetPropLineSpace(ePropL);
	rSet.Put( aLineSpacing );
}



void SwTextShell::ExecParaAttr(SfxRequest &rReq)
{
	// gleiche beide Attribute holen, ist nicht teuerer !!
	SfxItemSet aSet( GetPool(), RES_PARATR_LINESPACING, RES_PARATR_ADJUST );

	switch (rReq.GetSlot())
	{
	case FN_SET_LEFT_PARA:			lcl_SetAdjust(ADJLEFT,aSet); 	break;
	case FN_SET_RIGHT_PARA:			lcl_SetAdjust(ADJRIGHT,aSet); 	break;
	case FN_SET_CENTER_PARA:        lcl_SetAdjust(ADJCENTER,aSet); 	break;
	case SID_ATTR_PARA_ADJUST_BLOCK:lcl_SetAdjust(ADJBLOCK,aSet); 	break;

	case FN_SET_LINE_SPACE_1:	lcl_SetLineSpace(100,aSet); 	break;
	case FN_SET_LINE_SPACE_15:  lcl_SetLineSpace(150,aSet); 	break;
	case FN_SET_LINE_SPACE_2:	lcl_SetLineSpace(200,aSet); 	break;

	default:
		DBG_ERROR("SwTextShell::ExecParaAttr falscher Dispatcher");
		return;
	}
	SwWrtShell& rWrtSh = GetShell();
	SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
	if(pColl && pColl->IsAutoUpdateFmt())
	{
		rWrtSh.AutoUpdatePara(pColl, *pSet);
	}
	else
    {
		rWrtSh.SetAttrSet( aSet );
        rReq.Done( aSet );
    }
}

#else



void SwTextShell::ExecParaAttr(SfxRequest &rReq)
{
	SvxAdjust eAdjst;
	sal_uInt8 ePropL;
	const SfxItemSet* pArgs = rReq.GetArgs();

	// gleich beide Attribute holen, ist nicht teuerer !!
    SfxItemSet aSet( GetPool(),
        RES_PARATR_LINESPACING, RES_PARATR_ADJUST,
        RES_FRAMEDIR, RES_FRAMEDIR,
        0 );

	sal_uInt16 nSlot = rReq.GetSlot();
	switch (nSlot)
	{
		case SID_ATTR_PARA_ADJUST:
		{
			if( pArgs && SFX_ITEM_SET == pArgs->GetItemState(RES_PARATR_ADJUST) )
			{
				const SvxAdjustItem& rAdj = (const SvxAdjustItem&) pArgs->Get(RES_PARATR_ADJUST);
				SvxAdjustItem aAdj( rAdj.GetAdjust(), RES_PARATR_ADJUST );
				if ( rAdj.GetAdjust() == SVX_ADJUST_BLOCK )
				{
					aAdj.SetLastBlock( rAdj.GetLastBlock() );
					aAdj.SetOneWord( rAdj.GetOneWord() );
				}

				aSet.Put(aAdj);
			}
		}
		break;
		case SID_ATTR_PARA_ADJUST_LEFT:		eAdjst =  SVX_ADJUST_LEFT;		goto SET_ADJUST;
		case SID_ATTR_PARA_ADJUST_RIGHT:	eAdjst =  SVX_ADJUST_RIGHT;		goto SET_ADJUST;
		case SID_ATTR_PARA_ADJUST_CENTER:   eAdjst =  SVX_ADJUST_CENTER;	goto SET_ADJUST;
		case SID_ATTR_PARA_ADJUST_BLOCK:	eAdjst =  SVX_ADJUST_BLOCK;		goto SET_ADJUST;
SET_ADJUST:
		{
			aSet.Put(SvxAdjustItem(eAdjst,RES_PARATR_ADJUST));
            rReq.AppendItem( SfxBoolItem( GetPool().GetWhich(nSlot), sal_True ) );
		}
		break;

		case SID_ATTR_PARA_LINESPACE:
			if(pArgs && SFX_ITEM_SET == pArgs->GetItemState( GetPool().GetWhich(nSlot) ))
			{
				SvxLineSpacingItem aLineSpace = (const SvxLineSpacingItem&)pArgs->Get(
															GetPool().GetWhich(nSlot));
				aSet.Put( aLineSpace );
			}
		break;
		case SID_ATTR_PARA_LINESPACE_10:	ePropL = 100;	goto SET_LINESPACE;
		case SID_ATTR_PARA_LINESPACE_15:	ePropL = 150;	goto SET_LINESPACE;
		case SID_ATTR_PARA_LINESPACE_20:	ePropL = 200;	goto SET_LINESPACE;

SET_LINESPACE:
		{

			SvxLineSpacingItem aLineSpacing(ePropL, RES_PARATR_LINESPACING );
			aLineSpacing.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
			if( 100 == ePropL )
				aLineSpacing.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
			else
				aLineSpacing.SetPropLineSpace(ePropL);
			aSet.Put( aLineSpacing );
		}
		break;

        case SID_ATTR_PARA_LEFT_TO_RIGHT :
        case SID_ATTR_PARA_RIGHT_TO_LEFT :
        {
            sal_Bool bSet = sal_True;
            int eState = pArgs ? pArgs->GetItemState(nSlot) : SFX_ITEM_DISABLED;
            if (pArgs && SFX_ITEM_SET == eState)
                bSet = ((const SfxBoolItem&)pArgs->Get(nSlot)).GetValue();
/*
// toggling of the slots not used anymore

           if(!bSet)
				nSlot = SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot ?
					SID_ATTR_PARA_RIGHT_TO_LEFT :
					SID_ATTR_PARA_LEFT_TO_RIGHT;
*/
            SfxItemSet aAdjustSet( GetPool(),
                    RES_PARATR_ADJUST, RES_PARATR_ADJUST );
            GetShell().GetCurAttr(aAdjustSet);
            sal_Bool bChgAdjust = sal_False;
            SfxItemState eAdjustState = aAdjustSet.GetItemState(RES_PARATR_ADJUST, sal_False);
            if(eAdjustState  >= SFX_ITEM_DEFAULT)
            {
                int eAdjust = (int)(( const SvxAdjustItem& )
						aAdjustSet.Get(RES_PARATR_ADJUST)).GetAdjust();
//                bChgAdjust = SVX_ADJUST_CENTER  != eAdjust  &&  SVX_ADJUST_BLOCK != eAdjust;
                bChgAdjust = (SVX_ADJUST_LEFT  == eAdjust  &&  SID_ATTR_PARA_RIGHT_TO_LEFT == nSlot) ||
                             (SVX_ADJUST_RIGHT == eAdjust  &&  SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot);
            }
            else
                bChgAdjust = sal_True;

            SvxFrameDirection eFrmDirection =
                    (SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot) ?
                        FRMDIR_HORI_LEFT_TOP : FRMDIR_HORI_RIGHT_TOP;
            aSet.Put( SvxFrameDirectionItem( eFrmDirection, RES_FRAMEDIR ) );

            if (bChgAdjust)
            {
                SvxAdjust eAdjust = (SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot) ?
                        SVX_ADJUST_LEFT : SVX_ADJUST_RIGHT;
                SvxAdjustItem aAdjust( eAdjust, RES_PARATR_ADJUST );
                aSet.Put( aAdjust );
                aAdjust.SetWhich(SID_ATTR_PARA_ADJUST);
                GetView().GetViewFrame()->GetBindings().SetState( aAdjust );
                // Toggle numbering alignment
                const SwNumRule* pCurRule = GetShell().GetNumRuleAtCurrCrsrPos();
                if( pCurRule )
                {
                    SvxNumRule aRule = pCurRule->MakeSvxNumRule();

                    for(sal_uInt16 i = 0; i < aRule.GetLevelCount(); i++)
                    {
                        SvxNumberFormat aFmt(aRule.GetLevel(i));
                        if(SVX_ADJUST_LEFT == aFmt.GetNumAdjust())
                            aFmt.SetNumAdjust( SVX_ADJUST_RIGHT );

                        else if(SVX_ADJUST_RIGHT == aFmt.GetNumAdjust())
                            aFmt.SetNumAdjust( SVX_ADJUST_LEFT );

                        aRule.SetLevel(i, aFmt, aRule.Get(i) != 0);
                    }
                    // --> OD 2008-02-11 #newlistlevelattrs#
                    SwNumRule aSetRule( pCurRule->GetName(),
                                        pCurRule->Get( 0 ).GetPositionAndSpaceMode() );
                    // <--
                    aSetRule.SetSvxRule( aRule, GetShell().GetDoc());
                    aSetRule.SetAutoRule( sal_True );
                    // --> OD 2008-03-17 #refactorlists#
                    // no start or continuation of a list - list style is only changed
                    GetShell().SetCurNumRule( aSetRule, false );
                    // <--
                }
            }
        }
        break;

		default:
			ASSERT(sal_False, falscher Dispatcher);
			return;
	}
	SwWrtShell& rWrtSh = GetShell();
	SwTxtFmtColl* pColl = rWrtSh.GetCurTxtFmtColl();
	if(pColl && pColl->IsAutoUpdateFmt())
	{
		rWrtSh.AutoUpdatePara(pColl, aSet);
	}
	else
		rWrtSh.SetAttrSet( aSet );
    rReq.Done();
}

#endif



void SwTextShell::ExecParaAttrArgs(SfxRequest &rReq)
{
	SwWrtShell &rSh = GetShell();
	const SfxItemSet *pArgs = rReq.GetArgs();
	const SfxPoolItem *pItem = 0;

	sal_uInt16 nSlot = rReq.GetSlot();
	if(pArgs)
		pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);
	switch ( nSlot )
	{
        case FN_DROP_CHAR_STYLE_NAME:
            if( pItem )
            {
                String sCharStyleName = ((const SfxStringItem*)pItem)->GetValue();
                SfxItemSet aSet(GetPool(), RES_PARATR_DROP, RES_PARATR_DROP, 0L);
                rSh.GetCurAttr(aSet);
                SwFmtDrop aDropItem((const SwFmtDrop&)aSet.Get(RES_PARATR_DROP));
                SwCharFmt* pFmt = 0;
                if(sCharStyleName.Len())
                    pFmt = rSh.FindCharFmtByName( sCharStyleName );
                aDropItem.SetCharFmt( pFmt );
                aSet.Put(aDropItem);
                rSh.SetAttrSet(aSet);
            }
        break;
        case FN_FORMAT_DROPCAPS:
        {
			if(pItem)
            {
				rSh.SetAttrItem(*pItem);
                rReq.Done();
            }
			else
			{
				SfxItemSet aSet(GetPool(), RES_PARATR_DROP, RES_PARATR_DROP,
										   HINT_END, HINT_END, 0);
                rSh.GetCurAttr(aSet);
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                DBG_ASSERT(pFact, "SwAbstractDialogFactory fail!");

                SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( GetView().GetWindow(), aSet, 
					rSh.GetView().GetViewFrame()->GetFrame().GetFrameInterface(), DLG_SWDROPCAPS );
                DBG_ASSERT(pDlg, "Dialogdiet fail!");
				if (pDlg->Execute() == RET_OK)
				{
					rSh.StartAction();
					rSh.StartUndo( UNDO_START );
					if ( SFX_ITEM_SET == aSet.GetItemState(HINT_END,sal_False,&pItem) )
					{
						if ( ((SfxStringItem*)pItem)->GetValue().Len() )
							rSh.ReplaceDropTxt(((SfxStringItem*)pItem)->GetValue());
					}
					rSh.SetAttrSet(*pDlg->GetOutputItemSet());
					rSh.StartUndo( UNDO_END );
					rSh.EndAction();
                    rReq.Done(*pDlg->GetOutputItemSet());
				}
				delete pDlg;
			}
		}
 		break;
		case SID_ATTR_PARA_PAGEBREAK:
			if(pItem)
			{
				rSh.SetAttrItem( *pItem );
                rReq.Done();
			}
		break;
		case SID_ATTR_PARA_MODEL:
		{
			if(pItem)
			{
				SfxItemSet aCoreSet( GetPool(),
					RES_PAGEDESC,	RES_PAGEDESC,
					SID_ATTR_PARA_MODEL, SID_ATTR_PARA_MODEL, 0);
				aCoreSet.Put(*pItem);
				SfxToSwPageDescAttr( rSh, aCoreSet);
				rSh.SetAttrSet(aCoreSet);
                rReq.Done();
			}
		}
		break;

        default:
			ASSERT(sal_False, falscher Dispatcher);
			return;
	}
}



void SwTextShell::GetAttrState(SfxItemSet &rSet)
{
	SwWrtShell &rSh = GetShell();
	SfxItemPool& rPool = GetPool();
	SfxItemSet aCoreSet(rPool, aTxtFmtCollSetRange);
    rSh.GetCurAttr(aCoreSet); // *alle* Textattribute von der Core erfragen

	SfxWhichIter aIter(rSet);
	sal_uInt16 nSlot = aIter.FirstWhich();
    sal_Bool bFlag = sal_False;
	SfxBoolItem aFlagItem;
	const SfxPoolItem* pItem = 0;
	int eAdjust = -1;	// Illegaler Wert, um DONTCARE zu erkennen
	SfxItemState eState = aCoreSet.GetItemState(RES_PARATR_ADJUST, sal_False, &pItem);

	if( SFX_ITEM_DEFAULT == eState )
		pItem = &rPool.GetDefaultItem(RES_PARATR_ADJUST);
	if( SFX_ITEM_DEFAULT <= eState )
		eAdjust = (int)(( SvxAdjustItem* ) pItem)->GetAdjust();

	short nEsc = 0;
	eState =  aCoreSet.GetItemState(RES_CHRATR_ESCAPEMENT, sal_False, &pItem);
	if( SFX_ITEM_DEFAULT == eState )
		pItem = &rPool.GetDefaultItem(RES_CHRATR_ESCAPEMENT);
	if( eState >= SFX_ITEM_DEFAULT )
		nEsc = ((SvxEscapementItem* )pItem)->GetEsc();

	sal_uInt16 nLineSpace = 0;
	eState =  aCoreSet.GetItemState(RES_PARATR_LINESPACING, sal_False, &pItem);
	if( SFX_ITEM_DEFAULT == eState )
		pItem = &rPool.GetDefaultItem(RES_PARATR_LINESPACING);
	if( SFX_ITEM_DEFAULT <= eState &&
			((SvxLineSpacingItem* )pItem)->GetLineSpaceRule() == SVX_LINE_SPACE_AUTO )
	{
		if(SVX_INTER_LINE_SPACE_OFF ==
					((SvxLineSpacingItem* )pItem)->GetInterLineSpaceRule())
			nLineSpace = 100;
		else
			nLineSpace = ((SvxLineSpacingItem* )pItem)->GetPropLineSpace();
	}

	while (nSlot)
	{
		switch(nSlot)
		{
			case FN_SET_SUPER_SCRIPT:
					bFlag = 0 < nEsc;
				break;
			case FN_SET_SUB_SCRIPT:
					bFlag = 0 > nEsc;
				break;
			case SID_ATTR_PARA_ADJUST_LEFT:
				if (eAdjust == -1)
				{
					rSet.InvalidateItem( nSlot );
					nSlot = 0;
				}
				else
					bFlag = SVX_ADJUST_LEFT == eAdjust;
				break;
			case SID_ATTR_PARA_ADJUST_RIGHT:
				if (eAdjust == -1)
				{
					rSet.InvalidateItem( nSlot );
					nSlot = 0;
				}
				else
					bFlag = SVX_ADJUST_RIGHT == eAdjust;
				break;
			case SID_ATTR_PARA_ADJUST_CENTER:
				if (eAdjust == -1)
				{
					rSet.InvalidateItem( nSlot );
					nSlot = 0;
				}
				else
					bFlag = SVX_ADJUST_CENTER == eAdjust;
				break;
			case SID_ATTR_PARA_ADJUST_BLOCK:
			{
				if (eAdjust == -1)
				{
					rSet.InvalidateItem( nSlot );
					nSlot = 0;
				}
				else
				{
					bFlag = SVX_ADJUST_BLOCK == eAdjust;
					sal_uInt16 nHtmlMode = GetHtmlMode(rSh.GetView().GetDocShell());
					if((nHtmlMode & HTMLMODE_ON) && !(nHtmlMode & (HTMLMODE_FULL_STYLES|HTMLMODE_FIRSTLINE) ))
					{
						rSet.DisableItem( nSlot );
						nSlot = 0;
					}
				}
			}
			break;
			case SID_ATTR_PARA_LINESPACE_10:
				bFlag = nLineSpace == 100;
			break;
			case SID_ATTR_PARA_LINESPACE_15:
				bFlag = nLineSpace == 150;
			break;
			case SID_ATTR_PARA_LINESPACE_20:
				bFlag = nLineSpace == 200;
			break;
			case FN_GROW_FONT_SIZE:
			case FN_SHRINK_FONT_SIZE:
			{
				SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONTHEIGHT,
											*rSet.GetPool() );
				aSetItem.GetItemSet().Put( aCoreSet, sal_False );
				if( !aSetItem.GetItemOfScript( rSh.GetScriptType() ))
					rSet.DisableItem( nSlot );
				nSlot = 0;
			}
			break;
			case FN_UNDERLINE_DOUBLE:
			{
				eState = aCoreSet.GetItemState(RES_CHRATR_UNDERLINE);
				if( eState >= SFX_ITEM_DEFAULT )
				{
					FontUnderline eUnderline =  ((const SvxUnderlineItem&)
							aCoreSet.Get(RES_CHRATR_UNDERLINE)).GetLineStyle();
					rSet.Put(SfxBoolItem(nSlot, eUnderline == UNDERLINE_DOUBLE));
				}
				else
					rSet.InvalidateItem(nSlot);
				nSlot = 0;
			}
			break;
			case SID_ATTR_PARA_ADJUST:
				if (eAdjust == -1)
					rSet.InvalidateItem( nSlot );
				else
					rSet.Put(SvxAdjustItem((SvxAdjust)eAdjust, SID_ATTR_PARA_ADJUST ));
				nSlot = 0;
			break;
			case SID_ATTR_PARA_LRSPACE:
			{
				eState = aCoreSet.GetItemState(RES_LR_SPACE);
				if( eState >= SFX_ITEM_DEFAULT )
				{
					SvxLRSpaceItem aLR = ( (const SvxLRSpaceItem&) aCoreSet.Get( RES_LR_SPACE ) );
					aLR.SetWhich(SID_ATTR_PARA_LRSPACE);
					rSet.Put(aLR);					
				}
				else
					rSet.InvalidateItem(nSlot);
				nSlot = 0;
			}

            case SID_ATTR_PARA_LEFT_TO_RIGHT :
            case SID_ATTR_PARA_RIGHT_TO_LEFT :
            {
                if ( !SW_MOD()->GetCTLOptions().IsCTLFontEnabled() )
				{
					rSet.DisableItem( nSlot );
					nSlot = 0;
				}
				else
				{
                    // is the item set?
                    sal_uInt16 nHtmlMode = GetHtmlMode(rSh.GetView().GetDocShell());
                    if((!(nHtmlMode & HTMLMODE_ON) || (0 != (nHtmlMode & HTMLMODE_SOME_STYLES))) &&
                    aCoreSet.GetItemState( RES_FRAMEDIR, sal_False ) >= SFX_ITEM_DEFAULT)
					{
						SvxFrameDirection eFrmDir = (SvxFrameDirection)
								((const SvxFrameDirectionItem& )aCoreSet.Get(RES_FRAMEDIR)).GetValue();
						if (FRMDIR_ENVIRONMENT == eFrmDir)
						{
							eFrmDir = rSh.IsInRightToLeftText() ?
									FRMDIR_HORI_RIGHT_TOP : FRMDIR_HORI_LEFT_TOP;
						}
						bFlag = (SID_ATTR_PARA_LEFT_TO_RIGHT == nSlot &&
											FRMDIR_HORI_LEFT_TOP == eFrmDir) ||
								(SID_ATTR_PARA_RIGHT_TO_LEFT == nSlot &&
											FRMDIR_HORI_RIGHT_TOP == eFrmDir);
					}
					else
					{
						rSet.InvalidateItem(nSlot);
						nSlot = 0;
					}
				}
            }
            break;

			case SID_ATTR_CHAR_LANGUAGE:
			case SID_ATTR_CHAR_KERNING:
			case RES_PARATR_DROP:
			{
#if OSL_DEBUG_LEVEL > 1
				const SfxPoolItem& rItem = aCoreSet.Get(GetPool().GetWhich(nSlot), sal_True);
				rSet.Put(rItem);
#else
				rSet.Put(aCoreSet.Get( GetPool().GetWhich(nSlot), sal_True));
#endif
				nSlot = 0;
			}
			break;
			case SID_ATTR_PARA_MODEL:
			{
				SfxItemSet aTemp(GetPool(),
						RES_PAGEDESC,RES_PAGEDESC,
						SID_ATTR_PARA_MODEL,SID_ATTR_PARA_MODEL,
						0L);
				aTemp.Put(aCoreSet);
				::SwToSfxPageDescAttr(aTemp);
				rSet.Put(aTemp.Get(SID_ATTR_PARA_MODEL));
				nSlot = 0;
			}
			break;
			case RES_TXTATR_INETFMT:
			{
				SfxItemSet aSet(GetPool(), RES_TXTATR_INETFMT, RES_TXTATR_INETFMT);
                rSh.GetCurAttr(aSet);
#if OSL_DEBUG_LEVEL > 1
				const SfxPoolItem& rItem = aSet.Get(RES_TXTATR_INETFMT, sal_True);
				rSet.Put(rItem);
#else
				rSet.Put(aSet.Get( RES_TXTATR_INETFMT, sal_True));
#endif
				nSlot = 0;
			}
			break;

			default:
			// Nichts tun
			nSlot = 0;
			break;

		}
		if( nSlot )
		{
			aFlagItem.SetWhich( nSlot );
			aFlagItem.SetValue( bFlag );
			rSet.Put( aFlagItem );
		}
		nSlot = aIter.NextWhich();
	}

	rSet.Put(aCoreSet,sal_False);
}



