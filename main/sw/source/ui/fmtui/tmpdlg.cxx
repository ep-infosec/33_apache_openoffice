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

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <hintids.hxx>

#ifndef _SVSTDARR_STRINGSSORTDTOR
#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
#endif
#include <vcl/msgbox.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/hdft.hxx>
#include <editeng/flstitem.hxx>
#include <svx/htmlmode.hxx>
#include <svtools/htmlcfg.hxx>
#include <svl/cjkoptions.hxx>
#include <numpara.hxx>
#include <fmtclds.hxx>
#include <swtypes.hxx>
#include <swmodule.hxx>
#include <errhdl.hxx>		// fuer Create-Methoden
#include <wrtsh.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <wdocsh.hxx>
#include <viewopt.hxx>
#include <pgfnote.hxx>
#ifndef _PGFGRID_HXX
#include <pggrid.hxx>
#endif
#ifndef _TMPDLG_HXX
#include <tmpdlg.hxx>		// der Dialog
#endif
#include <column.hxx>		// Spalten
#include <drpcps.hxx>		// Initialen
#include <frmpage.hxx>		// Rahmen
#include <wrap.hxx>			// Rahmen
#include <frmmgr.hxx>		// Rahmen
#include <ccoll.hxx>		// CondColl
#include <swuiccoll.hxx>
#include <docstyle.hxx>		//
#include <fmtcol.hxx>		//
#include <macassgn.hxx>		//
#include <poolfmt.hxx>
#include <uitool.hxx>
#include <shellres.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _FMTUI_HRC
#include <fmtui.hrc>
#endif
#include <SwStyleNameMapper.hxx>
#include <svx/svxids.hrc>
#include <svl/stritem.hxx>
#include <svl/aeitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include <svx/flagsdef.hxx>

extern SW_DLLPUBLIC SwWrtShell* GetActiveWrtShell();

/*--------------------------------------------------------------------
	Beschreibung:	Der Traeger des Dialoges
 --------------------------------------------------------------------*/

SwTemplateDlg::SwTemplateDlg(Window*			pParent,
							 SfxStyleSheetBase& rBase,
							 sal_uInt16 			nRegion,
							 const sal_uInt16 nSlot,
							 SwWrtShell* 		pActShell,
							 sal_Bool 				bNew ) :
	SfxStyleDialog(	pParent,
					SW_RES(DLG_TEMPLATE_BASE + nRegion),
					rBase,
					sal_False,
					0 ),
	nType( nRegion ),
	pWrtShell(pActShell),
	bNewStyle(bNew)
{
	FreeResource();

	nHtmlMode = ::GetHtmlMode(pWrtShell->GetView().GetDocShell());
	SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    DBG_ASSERT(pFact, "Dialogdiet fail!");
	// TabPages zusammenfieseln
	switch( nRegion )
	{
		// Zeichenvorlagen
		case SFX_STYLE_FAMILY_CHAR:
		{
            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) , "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON || !aCJKOptions.IsDoubleLinesEnabled())
                RemoveTabPage(TP_CHAR_TWOLN);
        }
        break;
		// Absatzvorlagen
		case SFX_STYLE_FAMILY_PARA:
		{
                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH), "GetTabPageRangesFunc fail!");
				AddTabPage( TP_PARA_STD,	pFact->GetTabPageCreatorFunc(RID_SVXPAGE_STD_PARAGRAPH),		pFact->GetTabPageRangesFunc(RID_SVXPAGE_STD_PARAGRAPH) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH), "GetTabPageRangesFunc fail!");
				AddTabPage( TP_PARA_ALIGN,	pFact->GetTabPageCreatorFunc(RID_SVXPAGE_ALIGN_PARAGRAPH),		pFact->GetTabPageRangesFunc(RID_SVXPAGE_ALIGN_PARAGRAPH) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH), "GetTabPageRangesFunc fail!");
				AddTabPage( TP_PARA_EXT,	pFact->GetTabPageCreatorFunc(RID_SVXPAGE_EXT_PARAGRAPH),		pFact->GetTabPageRangesFunc(RID_SVXPAGE_EXT_PARAGRAPH) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN), "GetTabPageRangesFunc fail!");
				AddTabPage( TP_PARA_ASIAN,	pFact->GetTabPageCreatorFunc(RID_SVXPAGE_PARA_ASIAN),		pFact->GetTabPageRangesFunc(RID_SVXPAGE_PARA_ASIAN) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ), "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_STD, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_NAME ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_NAME ) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ), "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_EXT, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_EFFECTS ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_EFFECTS ) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) , "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_POS, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_POSITION ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_POSITION ) );

                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) , "GetTabPageRangesFunc fail!");
                AddTabPage(TP_CHAR_TWOLN, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_CHAR_TWOLINES ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_CHAR_TWOLINES ) );


            DBG_ASSERT(pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR), "GetTabPageRangesFunc fail!");
			AddTabPage( TP_TABULATOR,	pFact->GetTabPageCreatorFunc(RID_SVXPAGE_TABULATOR),		pFact->GetTabPageRangesFunc(RID_SVXPAGE_TABULATOR) );


			AddTabPage(TP_NUMPARA,   SwParagraphNumTabPage::Create,
									SwParagraphNumTabPage::GetRanges);
			AddTabPage(TP_DROPCAPS, 	SwDropCapsPage::Create,
										SwDropCapsPage::GetRanges );
            //UUUU remove?
            //DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            //DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            //AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            //UUUU add Area and Transparence TabPages
            AddTabPage(RID_SVXPAGE_AREA);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE);

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

			AddTabPage( TP_CONDCOLL,    SwCondCollPage::Create,
										SwCondCollPage::GetRanges );
			if( (!bNewStyle && RES_CONDTXTFMTCOLL != ((SwDocStyleSheet&)rBase).GetCollection()->Which())
			|| nHtmlMode & HTMLMODE_ON )
				RemoveTabPage(TP_CONDCOLL);

            SvtCJKOptions aCJKOptions;
            if(nHtmlMode & HTMLMODE_ON)
			{
				SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
				if (!pHtmlOpt->IsPrintLayoutExtension())
					RemoveTabPage(TP_PARA_EXT);
				RemoveTabPage(TP_PARA_ASIAN);
				RemoveTabPage(TP_TABULATOR);
				RemoveTabPage(TP_NUMPARA);
				RemoveTabPage(TP_CHAR_TWOLN);
				if(!(nHtmlMode & HTMLMODE_FULL_STYLES))
				{
					RemoveTabPage(TP_BACKGROUND);
					RemoveTabPage(TP_DROPCAPS);
				}
				if(!(nHtmlMode & HTMLMODE_PARA_BORDER))
					RemoveTabPage(TP_BORDER);
			}
            else
            {
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage(TP_PARA_ASIAN);
                if(!aCJKOptions.IsDoubleLinesEnabled())
                    RemoveTabPage(TP_CHAR_TWOLN);
            }
        }
		break;
		// Rahmenvorlagen
		case SFX_STYLE_FAMILY_FRAME:
		{
			AddTabPage(TP_FRM_STD, 		SwFrmPage::Create,
										SwFrmPage::GetRanges );
			AddTabPage(TP_FRM_ADD,		SwFrmAddPage::Create,
										SwFrmAddPage::GetRanges );
			AddTabPage(TP_FRM_WRAP, 	SwWrapTabPage::Create,
										SwWrapTabPage::GetRanges );

            //UUUU remove?
            //DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            //DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");

            //UUUU remove?
            //AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            //UUUU add Area and Transparence TabPages
            AddTabPage(RID_SVXPAGE_AREA);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE);

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_BORDER, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );

			AddTabPage(TP_COLUMN, 		SwColumnPage::Create,
										SwColumnPage::GetRanges );

			AddTabPage( TP_MACRO_ASSIGN, pFact->GetTabPageCreatorFunc(RID_SVXPAGE_MACROASSIGN), 0);

		break;
		}
		// Seitenvorlagen
		case SFX_STYLE_FAMILY_PAGE:
		{
            //UUUU remove?
            //DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageCreatorFunc fail!");
            //DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ), "GetTabPageRangesFunc fail!");
            //AddTabPage(TP_BACKGROUND, pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BACKGROUND ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BACKGROUND ) );

            //UUUU add Area and Transparence TabPages
            AddTabPage(RID_SVXPAGE_AREA);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE);

            AddTabPage(TP_HEADER_PAGE,		String(SW_RES(STR_PAGE_HEADER)),
                                            SvxHeaderPage::Create,
                                            SvxHeaderPage::GetRanges );
            AddTabPage(TP_FOOTER_PAGE,		String(SW_RES(STR_PAGE_FOOTER)),
                                            SvxFooterPage::Create,
                                            SvxFooterPage::GetRanges );
            if ( nSlot == FN_FORMAT_PAGE_COLUMN_DLG )
            {
                SetCurPageId(TP_COLUMN);
            }
            else if ( nSlot == FN_FORMAT_PAGE_SETTING_DLG )
            {
                SetCurPageId(TP_PAGE_STD);
            }

            DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ), "GetTabPageCreatorFunc fail!");
            DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ), "GetTabPageRangesFunc fail!");
            AddTabPage(TP_PAGE_STD,			String(SW_RES(STR_PAGE_STD)),
                                            pFact->GetTabPageCreatorFunc( RID_SVXPAGE_PAGE ),
                                            pFact->GetTabPageRangesFunc( RID_SVXPAGE_PAGE ),
                                            sal_False,
                                            1 ); // nach der Verwalten-Page
            if(!pActShell || 0 == ::GetHtmlMode(pWrtShell->GetView().GetDocShell()))
            {
                DBG_ASSERT(pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), "GetTabPageCreatorFunc fail!");
                DBG_ASSERT(pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ), "GetTabPageRangesFunc fail!");
                AddTabPage(TP_BORDER, 			String(SW_RES(STR_PAGE_BORDER)),
                                pFact->GetTabPageCreatorFunc( RID_SVXPAGE_BORDER ), pFact->GetTabPageRangesFunc( RID_SVXPAGE_BORDER ) );
                AddTabPage(TP_COLUMN,			String(SW_RES(STR_PAGE_COLUMN)),
                                                SwColumnPage::Create,
                                                SwColumnPage::GetRanges );
                AddTabPage(TP_FOOTNOTE_PAGE,	String(SW_RES(STR_PAGE_FOOTNOTE)),
                                                SwFootNotePage::Create,
                                                SwFootNotePage::GetRanges );
                AddTabPage(TP_TEXTGRID_PAGE,    String(SW_RES(STR_PAGE_TEXTGRID)),
                                                SwTextGridPage::Create,
                                                SwTextGridPage::GetRanges );
                SvtCJKOptions aCJKOptions;
                if(!aCJKOptions.IsAsianTypographyEnabled())
                    RemoveTabPage(TP_TEXTGRID_PAGE);
            }


        }
        break;
        // Numerierungsvorlagen
		case SFX_STYLE_FAMILY_PSEUDO:
		{
            AddTabPage( RID_SVXPAGE_PICK_SINGLE_NUM );
            AddTabPage( RID_SVXPAGE_PICK_BULLET );
            AddTabPage( RID_SVXPAGE_PICK_NUM );
            AddTabPage( RID_SVXPAGE_PICK_BMP);
            AddTabPage( RID_SVXPAGE_NUM_OPTIONS );
            AddTabPage( RID_SVXPAGE_NUM_POSITION );


/*			const SfxPoolItem* pItem;
			if(SFX_ITEM_SET == rBase.GetItemSet().GetItemState(
				FN_PARAM_ACT_NUMBER, sal_False, &pItem ))
			{
//				pActNum = (ActNumberFormat*)((const SwPtrItem*)pItem)->GetValue();
			}
*/
		}
		break;

		default:
			ASSERT(sal_False, "Wrong family");

	}
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

SwTemplateDlg::~SwTemplateDlg()
{
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

short SwTemplateDlg::Ok()
{
	short nRet = SfxTabDialog::Ok();
	if( RET_OK == nRet )
	{
		const SfxPoolItem *pOutItem, *pExItem;
		if( SFX_ITEM_SET == pExampleSet->GetItemState(
			SID_ATTR_NUMBERING_RULE, sal_False, &pExItem ) &&
			( !GetOutputItemSet() ||
			SFX_ITEM_SET != GetOutputItemSet()->GetItemState(
			SID_ATTR_NUMBERING_RULE, sal_False, &pOutItem ) ||
			*pExItem != *pOutItem ))
		{
			if( GetOutputItemSet() )
				((SfxItemSet*)GetOutputItemSet())->Put( *pExItem );
			else
				nRet = RET_CANCEL;
		}
	}
	else
		//JP 09.01.98 Bug #46446#:
		// das ist der Ok - Handler also muss auf OK defaultet werden!
		nRet = RET_OK;
	return nRet;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

const SfxItemSet* SwTemplateDlg::GetRefreshedSet()
{
	SfxItemSet* pInSet = GetInputSetImpl();
	pInSet->ClearItem();
	pInSet->SetParent( &GetStyleSheet().GetItemSet() );
	return pInSet;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwTemplateDlg::PageCreated(sal_uInt16 nId,SfxTabPage &rPage)
{
    //Namen der Vorlagen und Metric setzen
    String sNumCharFmt,sBulletCharFmt;
    SwStyleNameMapper::FillUIName(RES_POOLCHR_NUM_LEVEL,sNumCharFmt);
    SwStyleNameMapper::FillUIName(RES_POOLCHR_BUL_LEVEL,sBulletCharFmt);
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));

    switch(nId)
    {
        case TP_CHAR_STD:
        {
            ASSERT(::GetActiveView(),"keine View aktiv");

            SvxFontListItem aFontListItem(*((SvxFontListItem*)::GetActiveView()->
                GetDocShell()->GetItem(SID_ATTR_CHAR_FONTLIST)));

            aSet.Put(SvxFontListItem(aFontListItem.GetFontList(),SID_ATTR_CHAR_FONTLIST));
            sal_uInt32 nFlags = 0;
            if(rPage.GetItemSet().GetParent() && 0 == (nHtmlMode & HTMLMODE_ON))
                nFlags = SVX_RELATIVE_MODE;
            if(SFX_STYLE_FAMILY_CHAR == nType)
                nFlags = nFlags | SVX_PREVIEW_CHARACTER;
            aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,nFlags));
            rPage.PageCreated(aSet);
            break;
        }

        case TP_CHAR_EXT:
        {
            sal_uInt32 nFlags = SVX_ENABLE_FLASH;
            if(SFX_STYLE_FAMILY_CHAR == nType)
                nFlags = nFlags | SVX_PREVIEW_CHARACTER;
            aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,nFlags));
            rPage.PageCreated(aSet);
            break;
        }

        case TP_CHAR_POS:
        {
            if(SFX_STYLE_FAMILY_CHAR == nType)
            {
                aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
                rPage.PageCreated(aSet);
            }
            break;
        }

        case TP_CHAR_TWOLN:
        {
            if(SFX_STYLE_FAMILY_CHAR == nType)
            {
                aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,SVX_PREVIEW_CHARACTER));
                rPage.PageCreated(aSet);
            }
            break;
        }

        case TP_PARA_STD:
        {
            if(rPage.GetItemSet().GetParent())
            {
                aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_ABSLINEDIST,MM50 / 10));
                aSet.Put(SfxUInt32Item(SID_SVXSTDPARAGRAPHTABPAGE_FLAGSET,0x000F));
                rPage.PageCreated(aSet);
            }

            break;
        }

        case TP_NUMPARA:
        {
            //-->#outlinelevel added by zhaojianwei
            //  handle if the current paragraph style is assigned to a list level of outline style,
            SwTxtFmtColl* pTmpColl = pWrtShell->FindTxtFmtCollByName(GetStyleSheet().GetName());
            if(pTmpColl && pTmpColl->IsAssignedToListLevelOfOutlineStyle())
            {
                ((SwParagraphNumTabPage&)rPage).DisableOutline();
                ((SwParagraphNumTabPage&)rPage).DisableNumbering();
            }//<-end
            ListBox & rBox = ((SwParagraphNumTabPage&)rPage).GetStyleBox();
            SfxStyleSheetBasePool* pPool = pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
            pPool->SetSearchMask(SFX_STYLE_FAMILY_PSEUDO,SFXSTYLEBIT_ALL);
            const SfxStyleSheetBase* pBase = pPool->First();
            SvStringsSortDtor aNames;
            while(pBase)
            {
                aNames.Insert(new String(pBase->GetName()));
                pBase = pPool->Next();
            }
            for(sal_uInt16 i = 0; i < aNames.Count(); i++)
                rBox.InsertEntry(*aNames.GetObject(i));
            break;
        }

        case TP_PARA_ALIGN:
        {
            aSet.Put(SfxBoolItem(SID_SVXPARAALIGNTABPAGE_ENABLEJUSTIFYEXT,sal_True));
            rPage.PageCreated(aSet);
            break;
        }

        case TP_FRM_STD:
        {
            ((SwFrmPage&)rPage).SetNewFrame(sal_True);
            ((SwFrmPage&)rPage).SetFormatUsed(sal_True);
            break;
        }

        case TP_FRM_ADD:
        {
            ((SwFrmAddPage&)rPage).SetFormatUsed(sal_True);
            ((SwFrmAddPage&)rPage).SetNewFrame(sal_True);
            break;
        }

        case TP_FRM_WRAP:
        {
            ((SwWrapTabPage&)rPage).SetFormatUsed(sal_True,sal_False);
            break;
        }

        case TP_COLUMN:
        {
            if(nType == SFX_STYLE_FAMILY_FRAME)
                ((SwColumnPage&)rPage).SetFrmMode(sal_True);
            ((SwColumnPage&)rPage).SetFormatUsed(sal_True);
            break;
        }

        //UUUU do not remove; many other style dialog combinations still use the SfxTabPage
        // for the SvxBrushItem (see RID_SVXPAGE_BACKGROUND)
        case TP_BACKGROUND:
        {
            sal_Int32 nFlagType = 0;
            if(SFX_STYLE_FAMILY_PARA == nType)
                nFlagType |= SVX_SHOW_PARACTL;
            if(SFX_STYLE_FAMILY_CHAR != nType)
                nFlagType |= SVX_SHOW_SELECTOR;
            if(SFX_STYLE_FAMILY_FRAME == nType)
                nFlagType |= SVX_ENABLE_TRANSPARENCY;
            aSet.Put(SfxUInt32Item(SID_FLAG_TYPE,nFlagType));
            rPage.PageCreated(aSet);
            break;
        }

        case TP_CONDCOLL:
        {
            ((SwCondCollPage&)rPage).SetCollection(
                ((SwDocStyleSheet&)GetStyleSheet()).GetCollection(),bNewStyle);
            break;
        }

        case TP_PAGE_STD:
        {
            if(0 == (nHtmlMode & HTMLMODE_ON))
            {
                List aList;
                String* pNew = new String;
                SwStyleNameMapper::FillUIName(RES_POOLCOLL_TEXT,*pNew);
                aList.Insert(pNew,(sal_uLong)0);

                if(pWrtShell)
                {
                    SfxStyleSheetBasePool* pStyleSheetPool = pWrtShell->GetView().GetDocShell()->GetStyleSheetPool();
                    pStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_PARA);
                    SfxStyleSheetBase *pFirstStyle = pStyleSheetPool->First();

                    while(pFirstStyle)
                    {
                        aList.Insert(new String(pFirstStyle->GetName()), aList.Count());
                        pFirstStyle = pStyleSheetPool->Next();
                    }
                }

                //UUUU set DrawingLayer FillStyles active
                aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
                aSet.Put(SfxStringListItem(SID_COLLECT_LIST, &aList));
                rPage.PageCreated(aSet);

                for(sal_uInt16 i = (sal_uInt16)aList.Count(); i; --i)
                {
                    delete (String*)aList.Remove(i);
                }
            }

            break;
        }

        case TP_MACRO_ASSIGN:
        {
            SfxAllItemSet aNewSet(*aSet.GetPool());
            aNewSet.Put(SwMacroAssignDlg::AddEvents(MACASSGN_ALLFRM));
            if(pWrtShell)
                rPage.SetFrame(pWrtShell->GetView().GetViewFrame()->GetFrame().GetFrameInterface());
            rPage.PageCreated(aNewSet);
            break;
        }

        case RID_SVXPAGE_PICK_NUM:
        {
                aSet.Put (SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
                aSet.Put (SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
                rPage.PageCreated(aSet);
            break;
        }

        case RID_SVXPAGE_NUM_OPTIONS:
        {
            aSet.Put(SfxStringItem(SID_NUM_CHAR_FMT,sNumCharFmt));
            aSet.Put(SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
            // Zeichenvorlagen sammeln
            ListBox rCharFmtLB(this);
            rCharFmtLB.Clear();
            rCharFmtLB.InsertEntry(ViewShell::GetShellRes()->aStrNone);
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            ::FillCharStyleListBox(rCharFmtLB,pDocShell);
            List aList;
            for(sal_uInt16 j = 0; j < rCharFmtLB.GetEntryCount(); j++)
            {

                aList.Insert(new XubString(rCharFmtLB.GetEntry(j)),LIST_APPEND);
            }
            aSet.Put(SfxStringListItem(SID_CHAR_FMT_LIST_BOX,&aList));
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell,pDocShell));
            aSet.Put(SfxAllEnumItem(SID_METRIC_ITEM,static_cast<sal_uInt16>(eMetric)));
            rPage.PageCreated(aSet);
            for(sal_uInt16 i = (sal_uInt16)aList.Count(); i; --i)
                delete (XubString*)aList.Remove(i);
            aList.Clear();
            break;
        }

        case RID_SVXPAGE_NUM_POSITION:
        {
            SwDocShell* pDocShell = ::GetActiveWrtShell()->GetView().GetDocShell();
            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebDocShell,pDocShell));

            aSet.Put(SfxAllEnumItem(SID_METRIC_ITEM,static_cast<sal_uInt16>(eMetric)));
            rPage.PageCreated(aSet);
            break;
        }

        case  RID_SVXPAGE_PICK_BULLET:
        {
            aSet.Put(SfxStringItem(SID_BULLET_CHAR_FMT,sBulletCharFmt));
            rPage.PageCreated(aSet);
            break;
        }

        case  TP_HEADER_PAGE:
        {
            if(0 == (nHtmlMode & HTMLMODE_ON))
            {
                static_cast< SvxHeaderPage& >(rPage).EnableDynamicSpacing();
            }

            //UUUU set DrawingLayer FillStyles active
            aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
            rPage.PageCreated(aSet);

            break;
        }

        case  TP_FOOTER_PAGE:
        {
            if(0 == (nHtmlMode & HTMLMODE_ON))
            {
                static_cast< SvxFooterPage& >(rPage).EnableDynamicSpacing();
            }

            //UUUU set DrawingLayer FillStyles active
            aSet.Put(SfxBoolItem(SID_DRAWINGLAYER_FILLSTYLES, true));
            rPage.PageCreated(aSet);

            break;
        }

        case TP_BORDER:
        {
            if(SFX_STYLE_FAMILY_PARA == nType)
            {
                aSet.Put(SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_PARA));
            }
            else if(SFX_STYLE_FAMILY_FRAME == nType)
            {
                aSet.Put(SfxUInt16Item(SID_SWMODE_TYPE,SW_BORDER_MODE_FRAME));
            }
            rPage.PageCreated(aSet);
            break;
        }

        //UUUU inits for Area and Transparency TabPages
        // The selection attribute lists (XPropertyList derivates, e.g. XColorList for
        // the color table) need to be added as items (e.g. SvxColorTableItem) to make
        // these pages find the needed attributes for fill style suggestions.
        // These are added in SwDocStyleSheet::GetItemSet() for the SFX_STYLE_FAMILY_PARA on 
        // demand, but could also be directly added from the DrawModel.
        case RID_SVXPAGE_AREA:
        {
            aSet.Put(GetStyleSheet().GetItemSet());

            // add flag for direct graphic content selection
            aSet.Put(SfxBoolItem(SID_OFFER_IMPORT, true));

            rPage.PageCreated(aSet);
            break;
        }

        case RID_SVXPAGE_TRANSPARENCE:
        {
            rPage.PageCreated(GetStyleSheet().GetItemSet());
            break;
        }
	}
}

// eof
