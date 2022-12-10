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


#include <string> // HACK: prevent conflict between STLPORT and Workshop headers
#include <hintids.hxx>
#include <com/sun/star/util/SearchOptions.hpp>
#include <svl/cjkoptions.hxx>
#include <svl/ctloptions.hxx>
#include <svx/pageitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svtools/txtcmp.hxx>
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/request.hxx>
#include <svx/srchdlg.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/wrkwin.hxx>
#include "editeng/unolingu.hxx"
#include <swmodule.hxx>
#include <swwait.hxx>
#include <workctrl.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <swundo.hxx>               	// fuer Undo-Ids
#include <uitool.hxx>
#include <cmdid.h>
#include <docsh.hxx>

#include <view.hrc>
#include <SwRewriter.hxx>
#include <comcore.hrc>

#include "PostItMgr.hxx"

using namespace com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::i18n;

#define SRCH_ATTR_OFF	0
#define SRCH_ATTR_ON    1
#define SRCH_ATTR_SET   2

/*--------------------------------------------------------------------
	Beschreibung:	Search Parameter
 --------------------------------------------------------------------*/

struct SwSearchOptions
{
	SwDocPositions eStart, eEnd;
	sal_Bool bDontWrap;

	SwSearchOptions( SwWrtShell* pSh, sal_Bool bBackward );
};


inline Window* GetParentWindow( SvxSearchDialog* pSrchDlg )
{
	Window* pWin;
	if( pSrchDlg && pSrchDlg->IsVisible() )
		pWin = LAYOUT_THIS_WINDOW (pSrchDlg);
	else
		pWin = 0;
	return pWin;
}


/*-----------------12.04.97 13:04-------------------

--------------------------------------------------*/


void SwView::ExecSearch(SfxRequest& rReq, sal_Bool bNoMessage)
{
	const SfxItemSet* pArgs = rReq.GetArgs();
	const SfxPoolItem* pItem = 0;
	const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
	SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
    sal_Bool bQuiet = sal_False;
    if(pArgs && SFX_ITEM_SET == pArgs->GetItemState(SID_SEARCH_QUIET, sal_False, &pItem))
        bQuiet = ((const SfxBoolItem*) pItem)->GetValue();

    sal_Bool bApi = bQuiet | bNoMessage;

	sal_uInt16 nSlot = rReq.GetSlot();
	if (nSlot == FN_REPEAT_SEARCH && !pSrchItem)
	{
		if(bApi)
		{
			rReq.SetReturnValue(SfxBoolItem(nSlot, sal_False));
			nSlot = 0;
		}
	}
    if( pWrtShell->IsBlockMode() )
        pWrtShell->LeaveBlockMode();
	switch (nSlot)
	{
	// erstmal Nichts tun
	case SID_SEARCH_ITEM:
	{
		delete pSrchItem;
		pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();
	}
	break;

	case FID_SEARCH_ON:
		bJustOpened = sal_True;
		GetViewFrame()->GetBindings().Invalidate(SID_SEARCH_ITEM);
		break;

	case FID_SEARCH_OFF:
		if(pArgs)
		{
			// Dialog abmelden
			delete pSrchItem;
			pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();

			DELETEZ( pSrchList );
			DELETEZ( pReplList );

			if ( pWrp )
			{
                pSrchDlg = static_cast <SvxSearchDialog*> (pWrp->getDialog ());
				// die Search / Replace -Items merken wir uns
				const SearchAttrItemList* pList = pSrchDlg->GetSearchItemList();
				if( pList && pList->Count() )
					pSrchList = new SearchAttrItemList( *pList );

				if( 0 != (pList = pSrchDlg->GetReplaceItemList() ) &&
					pList->Count() )
					pReplList = new SearchAttrItemList( *pList );
			}
		}
		break;

		case FN_REPEAT_SEARCH:
		case FID_SEARCH_NOW:
        {
			{
				if(FID_SEARCH_NOW == nSlot && !rReq.IsAPI())
					SwView::SetMoveType(NID_SRCH_REP);
				if ( pWrp )
				{
                    pSrchDlg = static_cast <SvxSearchDialog*> (pWrp->getDialog ());
				}
				else
					pSrchDlg = 0;
			}

			if (pSrchDlg)
			{
				DELETEZ( pSrchList );
				DELETEZ( pReplList );

				const SearchAttrItemList* pList = pSrchDlg->GetSearchItemList();
				if( pList && pList->Count() )
					pSrchList = new SearchAttrItemList( *pList );

				if( 0 != (pList = pSrchDlg->GetReplaceItemList() ) &&
					pList->Count() )
					pReplList = new SearchAttrItemList( *pList );
			}

			if (nSlot == FN_REPEAT_SEARCH)
			{
				ASSERT(pSrchItem, "Search-Item fehlt");
				if( !pSrchItem )
                    pSrchItem = new SvxSearchItem(SID_SEARCH_ITEM);
			}
			else
			{
				// SearchItem aus Request besorgen
				ASSERT(pArgs, "Args fehlen");
                if ( pArgs )
                {
				    delete pSrchItem;
				    pSrchItem = (SvxSearchItem*) pArgs->Get(SID_SEARCH_ITEM).Clone();
                }
			}
			switch (pSrchItem->GetCommand())
			{
			case SVX_SEARCHCMD_FIND:
			{
				sal_Bool bRet = SearchAndWrap(bApi);
				if( bRet )
					Scroll(pWrtShell->GetCharRect().SVRect());
				rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
				if ( Application::IsAccessibilityEnabled() )
				{
					const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
					SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
					if ( pWrp )
					{					
						pSrchDlg = (SvxSearchDialog*)(pWrp->GetWindow());
						pSrchDlg->SetDocWin( (Window*)pEditWin );
						pSrchDlg->SetSrchFlag();
					}
				}
			}
			break;
			case SVX_SEARCHCMD_FIND_ALL:
			{
				sal_Bool bRet = SearchAll();
				if( !bRet )
				{
					if( !bApi )
					{
						Window* pParentWindow = GetParentWindow( pSrchDlg );
						InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
					}
					bFound = sal_False;
				}
				rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
				if ( Application::IsAccessibilityEnabled() )
				{
					const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
					SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);

					if ( pWrp )
					{					
						pSrchDlg = (SvxSearchDialog*)(pWrp->GetWindow());
						pSrchDlg->SetDocWin( (Window*)pEditWin );
						pSrchDlg->SetSrchFlag();				
					}
				}
			}
			break;
			case SVX_SEARCHCMD_REPLACE:
				{

					// 1) Selektion ersetzen (nicht. wenn nur Attribute ersetzt
					//    werden sollen)
//JP 27.04.95: warum ?
// 		was ist, wenn man das gefundene nur attributieren will??

					sal_uInt16 nCmd = SVX_SEARCHCMD_FIND;
					if( pSrchItem->GetReplaceString().Len() ||
						!pReplList )
					{
						// Verhindern, dass - falls der Suchstring im
						// Ersetzungsstring enthalten ist - der ersetzte String
						// noch einmal gefunden wird.

						sal_Bool bBack = pSrchItem->GetBackward();
						if (bBack)
							pWrtShell->Push();
                        String aReplace( pSrchItem->GetReplaceString() );
                        SearchOptions aTmp( pSrchItem->GetSearchOptions() );
                        String *pBackRef = ReplaceBackReferences( aTmp, pWrtShell->GetCrsr() );
                        if( pBackRef )
                            pSrchItem->SetReplaceString( *pBackRef );
						Replace();
                        if( pBackRef )
                        {
                            pSrchItem->SetReplaceString( aReplace );
                            delete pBackRef;
                        }
						if (bBack)
						{
							pWrtShell->Pop();
							pWrtShell->SwapPam();
						}
					}
					else if( pReplList )
						nCmd = SVX_SEARCHCMD_REPLACE;

					// 2) Weiter suchen (ohne zu ersetzen!)

					sal_uInt16 nOldCmd = pSrchItem->GetCommand();
					pSrchItem->SetCommand( nCmd );
					sal_Bool bRet = SearchAndWrap(bApi);
					if( bRet )
						Scroll( pWrtShell->GetCharRect().SVRect());
					pSrchItem->SetCommand( nOldCmd );
					rReq.SetReturnValue(SfxBoolItem(nSlot, bRet));
				}
				{					
					const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
					SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
				
					if ( pWrp )
					{					
						pSrchDlg = (SvxSearchDialog*)(pWrp->GetWindow());
						pSrchDlg->SetDocWin( (Window*)pEditWin );
						pSrchDlg->SetSrchFlag();			
					}
				}
				break;

			case SVX_SEARCHCMD_REPLACE_ALL:
				{
					SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );


					if( !pSrchItem->GetSelection() )
					{
						// bestehende Selektionen aufheben,
						// wenn nicht in selektierten Bereichen gesucht werden soll
						(pWrtShell->*pWrtShell->fnKillSel)(0, sal_False);
						if( DOCPOS_START == aOpts.eEnd )
							pWrtShell->EndDoc();
						else
							pWrtShell->SttDoc();
					}

					bExtra = sal_False;
					sal_uLong nFound;

					{	//Scope for SwWait-Object
						SwWait aWait( *GetDocShell(), true );
						pWrtShell->StartAllAction();
						nFound = FUNC_Search( aOpts );
						pWrtShell->EndAllAction();
					}
                    rReq.SetReturnValue(SfxBoolItem(nSlot, nFound != 0 && ULONG_MAX != nFound));
					if( !nFound )
					{
						if( !bApi )
						{
							Window* pParentWindow = GetParentWindow( pSrchDlg );
							InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
						}
						bFound = sal_False;
						return;
					}

                    if( !bApi && ULONG_MAX != nFound)
					{
						String aText( SW_RES( STR_NB_REPLACED ) );
						const xub_StrLen nPos = aText.Search( String::CreateFromAscii("XX") );
						aText.Erase( nPos, 2 );
						aText.Insert( String::CreateFromInt32( nFound ), nPos );
						Window* pParentWindow = GetParentWindow( pSrchDlg );
						InfoBox( pParentWindow, aText ).Execute();
					}
				}
				const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
				SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
			
				if ( pWrp )
				{					
					pSrchDlg = (SvxSearchDialog*)(pWrp->GetWindow());
					pSrchDlg->SetDocWin( (Window*)pEditWin );
					pSrchDlg->SetSrchFlag();				
				}
				break;
			}

            uno::Reference< frame::XDispatchRecorder > xRecorder =
                    GetViewFrame()->GetBindings().GetRecorder();
            //prevent additional dialogs in recorded macros
            if ( xRecorder.is() )
                rReq.AppendItem(SfxBoolItem(SID_SEARCH_QUIET, sal_True));

			rReq.Done();
        }
        break;
		case FID_SEARCH_SEARCHSET:
		case FID_SEARCH_REPLACESET:
		{
			static const sal_uInt16 aNormalAttr[] =
			{
/* 0 */			RES_CHRATR_CASEMAP,		RES_CHRATR_CASEMAP,
/* 2 */			RES_CHRATR_COLOR, 		RES_CHRATR_POSTURE,
/* 4 */			RES_CHRATR_SHADOWED, 	RES_CHRATR_WORDLINEMODE,
/* 6 */			RES_CHRATR_BLINK,		RES_CHRATR_BLINK,
/* 8 */			RES_CHRATR_BACKGROUND,	RES_CHRATR_BACKGROUND,
/*10 */			RES_CHRATR_ROTATE,		RES_CHRATR_ROTATE,
/*12 */			RES_CHRATR_SCALEW,		RES_CHRATR_RELIEF,
// insert position for CJK/CTL attributes!
/*14 */			RES_PARATR_LINESPACING, RES_PARATR_HYPHENZONE,
/*16 */			RES_PARATR_REGISTER, 	RES_PARATR_REGISTER,
/*18 */			RES_PARATR_VERTALIGN, 	RES_PARATR_VERTALIGN,
/*20 */			RES_LR_SPACE, 			RES_UL_SPACE,
/*22 */			SID_ATTR_PARA_MODEL, 	SID_ATTR_PARA_KEEP,
/*24 */ 		0
			};

			static const sal_uInt16 aCJKAttr[] =
			{
				RES_CHRATR_CJK_FONT,	RES_CHRATR_CJK_WEIGHT,
				RES_CHRATR_EMPHASIS_MARK, RES_CHRATR_TWO_LINES,
				RES_PARATR_SCRIPTSPACE, RES_PARATR_FORBIDDEN_RULES
			};
			static const sal_uInt16 aCTLAttr[] =
			{
				RES_CHRATR_CTL_FONT,	RES_CHRATR_CTL_WEIGHT
			};

			SvUShorts aArr( 0, 16 );
			aArr.Insert(	aNormalAttr,
							sizeof( aNormalAttr ) / sizeof( aNormalAttr[0] ),
							0 );
            if( SW_MOD()->GetCTLOptions().IsCTLFontEnabled() )
				aArr.Insert(	aCTLAttr,
								sizeof( aCTLAttr ) / sizeof( aCTLAttr[0] ),
								14 );
			SvtCJKOptions aCJKOpt;
			if( aCJKOpt.IsAnyEnabled() )
				aArr.Insert( 	aCJKAttr,
								sizeof( aCJKAttr ) / sizeof( aCJKAttr[0] ),
								14 );

			SfxItemSet aSet( pWrtShell->GetAttrPool(), aArr.GetData() );
			sal_uInt16 nWhich = SID_SEARCH_SEARCHSET;

			if ( FID_SEARCH_REPLACESET == nSlot )
			{
				nWhich = SID_SEARCH_REPLACESET;

				if ( pReplList )
				{
					pReplList->Get( aSet );
					DELETEZ( pReplList );
				}
			}
			else if ( pSrchList )
			{
				pSrchList->Get( aSet );
				DELETEZ( pSrchList );
			}
			rReq.SetReturnValue( SvxSetItem( nWhich, aSet ) );
		}
		break;
		default:
#ifdef DBG_UTIL
			if(nSlot)
			{
				ByteString sStr( "nSlot: " );
				sStr += ByteString::CreateFromInt32( nSlot );
				sStr += " falscher Dispatcher (viewsrch.cxx)";
				DBG_ERROR( sStr.GetBuffer() );
			}
#endif
			return;
	}
}


sal_Bool SwView::SearchAndWrap(sal_Bool bApi)
{
	SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );

		// Startposition der Suche fuer WrapAround merken
		// Start- / EndAction wegen vielleicht bestehender Selektionen
		// aus 'Suche alle'
	pWrtShell->StartAllAction();
	pWrtShell->Push();
		// falls in selektierten Bereichen gesucht werden soll, duerfen sie
		// nicht aufgehoben werden
	if (!pSrchItem->GetSelection())
		(pWrtShell->*pWrtShell->fnKillSel)(0, sal_False);

	SwWait *pWait = new SwWait( *GetDocShell(), true );
	if( FUNC_Search( aOpts ) )
	{
		bFound = sal_True;
		if(pWrtShell->IsSelFrmMode())
		{
			pWrtShell->UnSelectFrm();
			pWrtShell->LeaveSelFrmMode();
		}
		pWrtShell->Pop();
		pWrtShell->EndAllAction();
		delete pWait;
		return sal_True;
	}
	delete pWait, pWait = 0;

		// Suchen in den Sonderbereichen, wenn keine
		// Suche in Selektionen vorliegt. Bei Suche in Selektionen
		// wird ohnehin in diesen Sonderbereichen gesucht
	sal_Bool bHasSrchInOther = bExtra;
	if (!pSrchItem->GetSelection() && !bExtra )
	{
		bExtra = sal_True;
		if( FUNC_Search( aOpts ) )
		{
			bFound = sal_True;
			pWrtShell->Pop();
			pWrtShell->EndAllAction();
			return sal_True;
		}
		bExtra = sal_False;
	}
	else
		bExtra = !bExtra;

	const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
	SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)GetViewFrame()->GetChildWindow(nId);
	pSrchDlg = pWrp ? static_cast <SvxSearchDialog*> (pWrp->getDialog ()) : 0;

		// falls Startposition am Dokumentende / -anfang
	if (aOpts.bDontWrap)
	{
		pWrtShell->EndAllAction();
		if( !bApi )
		{
			Window* pParentWindow = GetParentWindow( pSrchDlg );
			InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
		}
		bFound = sal_False;
		pWrtShell->Pop();
		return sal_False;
	}
	pWrtShell->EndAllAction();
		// noch mal mit WrapAround versuchen?

	if( bApi || RET_NO == QueryBox( GetParentWindow( pSrchDlg ),
										SW_RES( DOCPOS_START == aOpts.eEnd
											? MSG_SEARCH_START
											: MSG_SEARCH_END )
									).Execute() )
	{
		bFound = sal_False;
		pWrtShell->Pop();
		return sal_False;
	}
	pWrtShell->StartAllAction();
	pWrtShell->Pop(sal_False);
	pWait = new SwWait( *GetDocShell(), true );

	sal_Bool bSrchBkwrd = DOCPOS_START == aOpts.eEnd;

	aOpts.eEnd =  bSrchBkwrd ? DOCPOS_START : DOCPOS_END;
	aOpts.eStart = bSrchBkwrd ? DOCPOS_END : DOCPOS_START;

	if (bHasSrchInOther)
	{
		pWrtShell->ClearMark();
		if (bSrchBkwrd)
			pWrtShell->EndDoc();
		else
			pWrtShell->SttDoc();
	}

	bFound = 0 != FUNC_Search( aOpts );
	pWrtShell->EndAllAction();
	delete pWait;
	if ( bFound )
		return bFound;
	if(!bApi)
	{
		Window* pParentWindow = GetParentWindow( pSrchDlg );
		InfoBox( pParentWindow, SW_RES(MSG_NOT_FOUND)).Execute();
	}
	return bFound = sal_False;
}


sal_Bool SwView::SearchAll(sal_uInt16* pFound)
{
	SwWait aWait( *GetDocShell(), true );
	pWrtShell->StartAllAction();

	SwSearchOptions aOpts( pWrtShell, pSrchItem->GetBackward() );

	if (!pSrchItem->GetSelection())
	{
		// bestehende Selektionen aufheben,
		// wenn nicht in selektierten Bereichen gesucht werden soll
		(pWrtShell->*pWrtShell->fnKillSel)(0, sal_False);

		if( DOCPOS_START == aOpts.eEnd )
			pWrtShell->EndDoc();
		else
			pWrtShell->SttDoc();
	}
	bExtra = sal_False;
	sal_uInt16 nFound = (sal_uInt16)FUNC_Search( aOpts );
	if(pFound)
		*pFound = nFound;
	bFound = 0 != nFound;

	pWrtShell->EndAllAction();
	return bFound;
}


void SwView::Replace()
{
	SwWait aWait( *GetDocShell(), true );

	pWrtShell->StartAllAction();

	if( pSrchItem->GetPattern() ) // Vorlagen?
    {
        SwRewriter aRewriter;
        aRewriter.AddRule(UNDO_ARG1, pSrchItem->GetSearchString());
        aRewriter.AddRule(UNDO_ARG2, SW_RES(STR_YIELDS));
        aRewriter.AddRule(UNDO_ARG3, pSrchItem->GetReplaceString());

        pWrtShell->StartUndo(UNDO_UI_REPLACE_STYLE, &aRewriter); // #111827#

		pWrtShell->SetTxtFmtColl( pWrtShell->GetParaStyle(
							pSrchItem->GetReplaceString(),
							SwWrtShell::GETSTYLE_CREATESOME ));

        pWrtShell->EndUndo(); // #111827#
    }
	else
	{
        if (GetPostItMgr()->HasActiveSidebarWin())
			GetPostItMgr()->Replace(pSrchItem);
		sal_Bool bReplaced = pWrtShell->SwEditShell::Replace( pSrchItem->GetReplaceString(),
											pSrchItem->GetRegExp());

		if( bReplaced && pReplList && pReplList->Count() && pWrtShell->HasSelection() )
		{
			SfxItemSet aReplSet( pWrtShell->GetAttrPool(),
									aTxtFmtCollSetRange );
			if( pReplList->Get( aReplSet ).Count() )
			{
				::SfxToSwPageDescAttr( *pWrtShell, aReplSet );
				pWrtShell->SwEditShell::SetAttrSet( aReplSet );
			}
		}
	}

	pWrtShell->EndAllAction();
}



SwSearchOptions::SwSearchOptions( SwWrtShell* pSh, sal_Bool bBackward )
{
	eStart = DOCPOS_CURR;
	if( bBackward )
	{
		eEnd = DOCPOS_START;
		bDontWrap = pSh->IsEndOfDoc();
	}
	else
	{
		eEnd = DOCPOS_END;
		bDontWrap = pSh->IsStartOfDoc();
	}
}

sal_uLong SwView::FUNC_Search( const SwSearchOptions& rOptions )
{
	sal_Bool bDoReplace = pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE ||
					  pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL;

	int eRanges = pSrchItem->GetSelection() ?
		FND_IN_SEL : bExtra ? FND_IN_OTHER : FND_IN_BODY;
	if (pSrchItem->GetCommand() == SVX_SEARCHCMD_FIND_ALL    ||
		pSrchItem->GetCommand() == SVX_SEARCHCMD_REPLACE_ALL)
		eRanges |= FND_IN_SELALL;

	pWrtShell->SttSelect();

	static sal_uInt16 __READONLY_DATA aSearchAttrRange[] = {
		RES_FRMATR_BEGIN, RES_FRMATR_END-1,
		RES_CHRATR_BEGIN, RES_CHRATR_END-1,
		RES_PARATR_BEGIN, RES_PARATR_END-1,
		SID_ATTR_PARA_MODEL, SID_ATTR_PARA_KEEP,
		0 };

	SfxItemSet aSrchSet( pWrtShell->GetAttrPool(), aSearchAttrRange);
	if( pSrchList && pSrchList->Count() )
	{
		pSrchList->Get( aSrchSet );

		/*  -- Seitenumbruch mit Seitenvorlage */
		::SfxToSwPageDescAttr( *pWrtShell, aSrchSet );
	}

	SfxItemSet* pReplSet = 0;
	if( bDoReplace && pReplList && pReplList->Count() )
	{
		pReplSet = new SfxItemSet( pWrtShell->GetAttrPool(),
										aSearchAttrRange );
		pReplList->Get( *pReplSet );

		/*  -- Seitenumbruch mit Seitenvorlage */
		::SfxToSwPageDescAttr( *pWrtShell, *pReplSet );

		if( !pReplSet->Count() )		// schade, die Attribute
			DELETEZ( pReplSet );		// kennen wir nicht
	}

	//
	// build SearchOptions to be used
	//
	SearchOptions aSearchOpt( pSrchItem->GetSearchOptions() );
	aSearchOpt.Locale = SvxCreateLocale( (sal_uInt16)GetAppLanguage() );
	if( !bDoReplace )
		aSearchOpt.replaceString = aEmptyStr;

	sal_uLong nFound;
	if( aSrchSet.Count() || ( pReplSet && pReplSet->Count() ))
	{
		nFound = pWrtShell->SearchAttr(
			aSrchSet,
			!pSrchItem->GetPattern(),
			rOptions.eStart,
			rOptions.eEnd,
			FindRanges(eRanges),
			pSrchItem->GetSearchString().Len() ? &aSearchOpt : 0,
			pReplSet );
	}
	else if( pSrchItem->GetPattern() )
	{
		// Suchen (und ersetzen) von Vorlagen
		const String sRplStr( pSrchItem->GetReplaceString() );
		nFound = pWrtShell->SearchTempl( pSrchItem->GetSearchString(),
			rOptions.eStart,
			rOptions.eEnd,
			FindRanges(eRanges),
			bDoReplace ? &sRplStr : 0 );
	}
	else
	{
		// Normale Suche
		nFound = pWrtShell->SearchPattern(aSearchOpt, pSrchItem->GetNotes(),
										  rOptions.eStart,
										  rOptions.eEnd,
										  FindRanges(eRanges),
										  bDoReplace );
	}
	pWrtShell->EndSelect();
	return nFound;
}

LAYOUT_NS Dialog* SwView::GetSearchDialog()
{
	const sal_uInt16 nId = SvxSearchDialogWrapper::GetChildWindowId();
	SvxSearchDialogWrapper *pWrp = (SvxSearchDialogWrapper*)SfxViewFrame::Current()->GetChildWindow(nId);
	if ( pWrp )
		pSrchDlg = pWrp->getDialog ();
	else
		pSrchDlg = 0;
	return pSrchDlg;
}

void SwView::StateSearch(SfxItemSet &rSet)
{
	SfxWhichIter aIter(rSet);
	sal_uInt16 nWhich = aIter.FirstWhich();

	while(nWhich)
	{
		switch(nWhich)
		{
			case SID_SEARCH_OPTIONS:
			{
				sal_uInt16 nOpt = 0xFFFF;
				if( GetDocShell()->IsReadOnly() )
					nOpt &= ~( SEARCH_OPTIONS_REPLACE |
							   SEARCH_OPTIONS_REPLACE_ALL );
				rSet.Put( SfxUInt16Item( SID_SEARCH_OPTIONS, nOpt));
			}
			break;
			case SID_SEARCH_ITEM:
			{
				if ( !pSrchItem )
				{
					pSrchItem = new SvxSearchItem( SID_SEARCH_ITEM );
					pSrchItem->SetFamily(SFX_STYLE_FAMILY_PARA);
					pSrchItem->SetSearchString( pWrtShell->GetSelTxt() );
				}

				if( bJustOpened && pWrtShell->IsSelection() )
				{
					String aTxt;
					if( 1 == pWrtShell->GetCrsrCnt() &&
						( aTxt = pWrtShell->SwCrsrShell::GetSelTxt() ).Len() )
					{
						pSrchItem->SetSearchString( aTxt );
						pSrchItem->SetSelection( sal_False );
					}
					else
						pSrchItem->SetSelection( sal_True );
				}

				bJustOpened = sal_False;
				rSet.Put( *pSrchItem );
			}
			break;

/*			case SID_SEARCH_REPLACESET:
			case SID_SEARCH_SEARCHSET:
			{
				static sal_uInt16 __READONLY_DATA aSearchAttrRange[] =
				{
						RES_CHRATR_CASEMAP,		RES_CHRATR_POSTURE,
						RES_CHRATR_SHADOWED, 	RES_CHRATR_WORDLINEMODE,
						RES_PARATR_LINESPACING, RES_PARATR_HYPHENZONE,
						RES_LR_SPACE, 			RES_UL_SPACE,
						SID_ATTR_PARA_MODEL, 	SID_ATTR_PARA_KEEP,
						0
				};

				SfxItemSet aSet(pWrtShell->GetAttrPool(), aSearchAttrRange );
				if( SID_SEARCH_REPLACESET==nWhich )
				{
					if( pReplList )
					{
						pReplList->Get( aSet );
						DELETEZ( pReplList );
					}
				}
				else if( pSrchList )
				{
					pSrchList->Get( aSet );
					DELETEZ( pSrchList );
				}
				rSet.Put( SvxSetItem( nWhich, aSet ));
			}
			break;
*/
		}
		nWhich = aIter.NextWhich();
	}
}



