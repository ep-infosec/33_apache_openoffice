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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <hintids.hxx>
#include <sfx2/app.hxx>
#include <svl/whiter.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/printer.hxx>
#include <svl/macitem.hxx>
#include <editeng/brshitem.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/eeitem.hxx>
#include <svx/htmlmode.hxx>
#include <svx/xdef.hxx>
#include <swmodule.hxx>
#include <wdocsh.hxx>
#include <fmtfsize.hxx>
#include <fchrfmt.hxx>
#include <svtools/htmlcfg.hxx>
#include <SwStyleNameMapper.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include "view.hxx"
#include "wrtsh.hxx"
#include "docsh.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "viewopt.hxx"
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include "swstyle.h"
#include "frmfmt.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "pagedesc.hxx"
#include "docstyle.hxx"
#include "uiitems.hxx"
#include "fmtcol.hxx"
#include "frmmgr.hxx"		//SwFrmValid
#include "swevent.hxx"
#include "edtwin.hxx"
#include "unochart.hxx"
#include "app.hrc"
#include <fmtui.hrc>
#include "swabstdlg.hxx"
#include <list.hxx>
#include <paratr.hxx>	//#outline level,add by zhaojianwei
#include <drawdoc.hxx>

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/


void  SwDocShell::StateStyleSheet(SfxItemSet& rSet, SwWrtShell* pSh)
{
	SfxWhichIter aIter(rSet);
	sal_uInt16 	nWhich 	= aIter.FirstWhich();
	sal_uInt16 nActualFamily = USHRT_MAX;

	SwWrtShell* pShell = pSh ? pSh : GetWrtShell();
    if(!pShell)
	{
		while (nWhich)
		{
			rSet.DisableItem(nWhich);
			nWhich = aIter.NextWhich();
		}
		return;
	}
	else
	{
		SfxViewFrame* pFrame = pShell->GetView().GetViewFrame();
		const ISfxTemplateCommon* pCommon = SFX_APP()->GetCurrentTemplateCommon(pFrame->GetBindings());
		if( pCommon )
            nActualFamily = static_cast< sal_uInt16 >(pCommon->GetActualFamily());
	}

	while (nWhich)
	{
		// aktuelle Vorlage zu jeder Familie ermitteln
		//
		String aName;
		switch (nWhich)
		{
			case SID_STYLE_APPLY:
			{//hier wird die Vorlage und ihre Familie an die StyleBox
			 //uebergeben, damit diese Familie angezeigt wird
				if(pShell->IsFrmSelected())
				{
					SwFrmFmt* pFmt = pShell->GetCurFrmFmt();
					if( pFmt )
						aName = pFmt->GetName();
				}
				else
				{
					SwTxtFmtColl* pColl = pShell->GetCurTxtFmtColl();
					if(pColl)
						aName = pColl->GetName();
				}
				rSet.Put(SfxTemplateItem(nWhich, aName));
			}
			break;
			case SID_STYLE_FAMILY1:
				if( !pShell->IsFrmSelected() )
				{
					SwCharFmt* pFmt = pShell->GetCurCharFmt();
					if(pFmt)
						aName = pFmt->GetName();
					else
						aName = *SwStyleNameMapper::GetTextUINameArray()[
							RES_POOLCOLL_STANDARD - RES_POOLCOLL_TEXT_BEGIN ];
                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;

			case SID_STYLE_FAMILY2:
				if(!pShell->IsFrmSelected())
				{
					SwTxtFmtColl* pColl = pShell->GetCurTxtFmtColl();
					if(pColl)
						aName = pColl->GetName();

					SfxTemplateItem aItem(nWhich, aName);

					sal_uInt16 nMask = 0;
					if( mpDoc->get(IDocumentSettingAccess::HTML_MODE) )
						nMask = SWSTYLEBIT_HTML;
					else
					{
						const int nSelection = pShell->GetFrmType(0,sal_True);
						if(pShell->GetCurTOX())
							nMask = SWSTYLEBIT_IDX	;
						else if(nSelection & FRMTYPE_HEADER 	||
								nSelection & FRMTYPE_FOOTER 	||
								nSelection & FRMTYPE_TABLE 		||
								nSelection & FRMTYPE_FLY_ANY 	||
								nSelection & FRMTYPE_FOOTNOTE 	||
								nSelection & FRMTYPE_FTNPAGE)
							nMask = SWSTYLEBIT_EXTRA;
						else
							nMask = SWSTYLEBIT_TEXT;
					}

					aItem.SetValue(nMask);
					rSet.Put(aItem);
				}

				break;

			case SID_STYLE_FAMILY3:

				if( mpDoc->get(IDocumentSettingAccess::HTML_MODE) )
					rSet.DisableItem( nWhich );
				else
				{
					SwFrmFmt* pFmt = pShell->GetCurFrmFmt();
					if(pFmt && pShell->IsFrmSelected())
                    {
						aName = pFmt->GetName();
                        rSet.Put(SfxTemplateItem(nWhich, aName));
                    }
                }
				break;

			case SID_STYLE_FAMILY4:
			{
				SvxHtmlOptions* pHtmlOpt = SvxHtmlOptions::Get();
				if( mpDoc->get(IDocumentSettingAccess::HTML_MODE) && !pHtmlOpt->IsPrintLayoutExtension())
					rSet.DisableItem( nWhich );
				else
				{
					sal_uInt16 n = pShell->GetCurPageDesc( sal_False );
					if( n < pShell->GetPageDescCnt() )
						aName = pShell->GetPageDesc( n ).GetName();

					rSet.Put(SfxTemplateItem(nWhich, aName));
				}
			}
			break;
			case SID_STYLE_FAMILY5:
				{
					const SwNumRule* pRule = pShell->GetNumRuleAtCurrCrsrPos();
					if( pRule )
						aName = pRule->GetName();

					rSet.Put(SfxTemplateItem(nWhich, aName));
				}
				break;

			case SID_STYLE_WATERCAN:
			{
                SwEditWin& rEdtWin = pShell->GetView().GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->eType != 0));
			}
            break;
			case SID_STYLE_UPDATE_BY_EXAMPLE:
				if( pShell->IsFrmSelected()
						? SFX_STYLE_FAMILY_FRAME != nActualFamily
						: ( SFX_STYLE_FAMILY_FRAME == nActualFamily ||
							SFX_STYLE_FAMILY_PAGE == nActualFamily ||
							(SFX_STYLE_FAMILY_PSEUDO == nActualFamily && !pShell->GetNumRuleAtCurrCrsrPos())) )
				{
					rSet.DisableItem( nWhich );
				}
				break;

			case SID_STYLE_NEW_BY_EXAMPLE:
				if( (pShell->IsFrmSelected()
						? SFX_STYLE_FAMILY_FRAME != nActualFamily
						: SFX_STYLE_FAMILY_FRAME == nActualFamily) ||
					(SFX_STYLE_FAMILY_PSEUDO == nActualFamily && !pShell->GetNumRuleAtCurrCrsrPos()) )
				{
					rSet.DisableItem( nWhich );
				}
				break;

			default:
				DBG_ERROR( "Invalid SlotId");
		}
		nWhich = aIter.NextWhich();
	}
}


/*--------------------------------------------------------------------
	Beschreibung:	StyleSheet-Requeste auswerten
 --------------------------------------------------------------------*/


void SwDocShell::ExecStyleSheet( SfxRequest& rReq )
{
	sal_uInt16 	nSlot 	= rReq.GetSlot();
	sal_uInt16 	nRet 	= 0xffff;

	const SfxItemSet* pArgs = rReq.GetArgs();
	const SfxPoolItem* pItem;
	SwWrtShell* pActShell = 0;
	sal_Bool bSetReturn = sal_True;
	switch (nSlot)
	{
	case SID_STYLE_NEW:
		if( pArgs && SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_FAMILY,
			sal_False, &pItem ))
		{
			const sal_uInt16 nFamily = ((const SfxUInt16Item*)pItem)->GetValue();

			String sName;
			sal_uInt16 nMask = 0;
			if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_NEW,
				sal_False, &pItem ))
				sName = ((const SfxStringItem*)pItem)->GetValue();
			if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_MASK,
				sal_False, &pItem ))
				nMask = ((const SfxUInt16Item*)pItem)->GetValue();
			String sParent;
			if( SFX_ITEM_SET == pArgs->GetItemState( SID_STYLE_REFERENCE,
				sal_False, &pItem ))
				sParent = ((const SfxStringItem*)pItem)->GetValue();

			nRet = Edit( sName, sParent, nFamily, nMask, sal_True, 0, 0, rReq.IsAPI() );
		}
		break;

		case SID_STYLE_APPLY:
			if( !pArgs )
			{
				GetView()->GetViewFrame()->GetDispatcher()->Execute(SID_STYLE_DESIGNER, sal_False);
				break;
            }
            else
            {
                // convert internal StyleName to DisplayName (slot implementation uses the latter)
                SFX_REQUEST_ARG( rReq, pNameItem, SfxStringItem, SID_APPLY_STYLE, sal_False );
                SFX_REQUEST_ARG( rReq, pFamilyItem, SfxStringItem, SID_STYLE_FAMILYNAME, sal_False );
                if ( pFamilyItem && pNameItem )
                {
                    uno::Reference< style::XStyleFamiliesSupplier > xModel(GetModel(), uno::UNO_QUERY);
                    try
                    {
                        uno::Reference< container::XNameAccess > xStyles;
                        uno::Reference< container::XNameAccess > xCont = xModel->getStyleFamilies();
                        xCont->getByName(pFamilyItem->GetValue()) >>= xStyles;
                        uno::Reference< beans::XPropertySet > xInfo;
                        xStyles->getByName( pNameItem->GetValue() ) >>= xInfo;
                        ::rtl::OUString aUIName;
                        xInfo->getPropertyValue( ::rtl::OUString::createFromAscii("DisplayName") ) >>= aUIName;
                        if ( aUIName.getLength() )
                            rReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aUIName ) );
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
            }

            // intentionally no break

		case SID_STYLE_EDIT:
		case SID_STYLE_DELETE:
		case SID_STYLE_WATERCAN:
		case SID_STYLE_FAMILY:
		case SID_STYLE_UPDATE_BY_EXAMPLE:
		case SID_STYLE_NEW_BY_EXAMPLE:
		{
			String aParam;
            sal_uInt16 nFamily = SFX_STYLE_FAMILY_PARA;
            sal_uInt16 nMask = 0;

			if( !pArgs )
			{
				nFamily = SFX_STYLE_FAMILY_PARA;

				switch (nSlot)
				{
					case SID_STYLE_NEW_BY_EXAMPLE:
					{
						SfxNewStyleDlg *pDlg = new SfxNewStyleDlg( 0,
													*GetStyleSheetPool());
						if(RET_OK == pDlg->Execute())
                        {
							aParam = pDlg->GetName();
							rReq.AppendItem(SfxStringItem(nSlot, aParam));
                        }

						delete pDlg;
					}
					break;

					case SID_STYLE_UPDATE_BY_EXAMPLE:
					case SID_STYLE_EDIT:
					{
						SwTxtFmtColl* pColl = GetWrtShell()->GetCurTxtFmtColl();
						if(pColl)
						{
							aParam = pColl->GetName();
							rReq.AppendItem(SfxStringItem(nSlot, aParam));
						}
					}
					break;
				}
			}
			else
			{
				ASSERT( pArgs->Count(), "SfxBug ItemSet ist leer");

				SwWrtShell* pShell = GetWrtShell();
				if( SFX_ITEM_SET == pArgs->GetItemState(nSlot, sal_False, &pItem ))
					aParam = ((const SfxStringItem*)pItem)->GetValue();

				if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_FAMILY,
					sal_False, &pItem ))
					nFamily = ((const SfxUInt16Item*)pItem)->GetValue();

				if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_FAMILYNAME, sal_False, &pItem ))
                {
					String aFamily = ((const SfxStringItem*)pItem)->GetValue();
                    if(aFamily.CompareToAscii("CharacterStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_CHAR;
                    else
                    if(aFamily.CompareToAscii("ParagraphStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_PARA;
                    else
                    if(aFamily.CompareToAscii("PageStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_PAGE;
                    else
                    if(aFamily.CompareToAscii("FrameStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_FRAME;
                    else
                    if(aFamily.CompareToAscii("NumberingStyles") == COMPARE_EQUAL)
                        nFamily = SFX_STYLE_FAMILY_PSEUDO;
                }

				if( SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_MASK,
					sal_False, &pItem ))
					nMask = ((const SfxUInt16Item*)pItem)->GetValue();
				if( SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_WRTSHELL,
					sal_False, &pItem ))
					pActShell = pShell = (SwWrtShell*)((SwPtrItem*)pItem)->GetValue();

				if( nSlot == SID_STYLE_UPDATE_BY_EXAMPLE )
				{
					switch( nFamily )
					{
						case SFX_STYLE_FAMILY_PARA:
						{
							SwTxtFmtColl* pColl = pShell->GetCurTxtFmtColl();
							if(pColl)
								aParam = pColl->GetName();
						}
						break;
						case SFX_STYLE_FAMILY_FRAME:
						{
							SwFrmFmt* pFrm = mpWrtShell->GetCurFrmFmt();
							if( pFrm )
								aParam = pFrm->GetName();
						}
						break;
						case SFX_STYLE_FAMILY_CHAR:
						{
							SwCharFmt* pChar = mpWrtShell->GetCurCharFmt();
							if( pChar )
								aParam = pChar->GetName();
						}
						break;
						case SFX_STYLE_FAMILY_PSEUDO:
						if(SFX_ITEM_SET == pArgs->GetItemState(SID_STYLE_UPD_BY_EX_NAME, sal_False, &pItem))
						{
							aParam = ((const SfxStringItem*)pItem)->GetValue();
						}
						break;
					}
					rReq.AppendItem(SfxStringItem(nSlot, aParam));
				}
			}
			if (aParam.Len() || nSlot == SID_STYLE_WATERCAN )
			{
				switch(nSlot)
				{
					case SID_STYLE_EDIT:
						nRet = Edit(aParam, aEmptyStr, nFamily, nMask, sal_False, 0, pActShell );
						break;
					case SID_STYLE_DELETE:
						nRet = Delete(aParam, nFamily);
						break;
					case SID_STYLE_APPLY:
						// Shellwechsel in ApplyStyles
                        nRet = ApplyStyles(aParam, nFamily, pActShell, rReq.GetModifier() );
						break;
					case SID_STYLE_WATERCAN:
						nRet = DoWaterCan(aParam, nFamily);
						break;
					case SID_STYLE_UPDATE_BY_EXAMPLE:
						nRet = UpdateStyle(aParam, nFamily, pActShell);
						break;
					case SID_STYLE_NEW_BY_EXAMPLE:
					{
						nRet = MakeByExample(aParam, nFamily, nMask, pActShell );
						SfxTemplateDialog* pDlg = SFX_APP()->GetTemplateDialog();

						if(pDlg && pDlg->IsVisible())
							pDlg->Update();
					}
					break;

					default:
						DBG_ERROR( "Falsche Slot-Id");
				}

                rReq.Done();
			}

			break;
		}
	}

	if(bSetReturn)
	{
		if(rReq.IsAPI()) // Basic bekommt nur sal_True oder sal_False
			rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet !=0));
		else
			rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet));
	}

}

/*--------------------------------------------------------------------
	Beschreibung:	Edit
 --------------------------------------------------------------------*/

sal_uInt16 SwDocShell::Edit(
    const String &rName,
    const String &rParent,
    const sal_uInt16 nFamily,
    sal_uInt16 nMask,
    const sal_Bool bNew,
    const sal_uInt16 nSlot,
    SwWrtShell* pActShell,
    const sal_Bool bBasic )
{
	ASSERT(GetWrtShell(), "Keine Shell, keine Styles");
	SfxStyleSheetBase *pStyle = 0;

	sal_uInt16 nRet = nMask;
	sal_Bool bModified = mpDoc->IsModified();

	if( bNew )
	{
		if( SFXSTYLEBIT_ALL != nMask && SFXSTYLEBIT_USED != nMask )
			nMask |= SFXSTYLEBIT_USERDEF;
		else
			nMask = SFXSTYLEBIT_USERDEF;

		pStyle = &mxBasePool->Make( rName, (SfxStyleFamily)nFamily, nMask );

		// die aktuellen als Parent setzen
		SwDocStyleSheet* pDStyle = (SwDocStyleSheet*)pStyle;
		switch( nFamily )
		{
			case SFX_STYLE_FAMILY_PARA:
			{
				if(rParent.Len())
				{
					SwTxtFmtColl* pColl = mpWrtShell->FindTxtFmtCollByName( rParent );
                    if(!pColl)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
                        if(USHRT_MAX != nId)
                            pColl =  mpWrtShell->GetTxtCollFromPool( nId );
                    }
					pDStyle->GetCollection()->SetDerivedFrom( pColl );
					pDStyle->PresetParent( rParent );

					 /*When a new paragraph style is created based on a "to outline style
						assigned" paragraph style, the outline level attribute and the list
						style attribute of the new paragraph style have to be set to 0
						respectively "".*/
					if( pColl->IsAssignedToListLevelOfOutlineStyle())
					{
						SwNumRuleItem aItem(aEmptyStr);
						pDStyle->GetCollection()->SetFmtAttr( aItem );
						pDStyle->GetCollection()->SetAttrOutlineLevel( 0 );
					}

				}
				else
				{
					SwTxtFmtColl* pColl = mpWrtShell->GetCurTxtFmtColl();
					pDStyle->GetCollection()->SetDerivedFrom( pColl );
					if( pColl )
						pDStyle->PresetParent( pColl->GetName() );
				}
			}
			break;
			case SFX_STYLE_FAMILY_CHAR:
			{
				if(rParent.Len())
				{
					SwCharFmt* pCFmt = mpWrtShell->FindCharFmtByName( rParent );
                    if(!pCFmt)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
                        if(USHRT_MAX != nId)
                            pCFmt =  mpWrtShell->GetCharFmtFromPool( nId );
                    }

					pDStyle->GetCharFmt()->SetDerivedFrom( pCFmt );
					pDStyle->PresetParent( rParent );
				}
				else
				{
					SwCharFmt* pCFmt = mpWrtShell->GetCurCharFmt();
					pDStyle->GetCharFmt()->SetDerivedFrom( pCFmt );
						if( pCFmt )
							pDStyle->PresetParent( pCFmt->GetName() );
				}
			}
			break;
			case SFX_STYLE_FAMILY_FRAME :
			{
				if(rParent.Len())
				{
					SwFrmFmt* pFFmt = mpWrtShell->GetDoc()->FindFrmFmtByName( rParent );
                    if(!pFFmt)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT);
                        if(USHRT_MAX != nId)
                            pFFmt =  mpWrtShell->GetFrmFmtFromPool( nId );
                    }
                    pDStyle->GetFrmFmt()->SetDerivedFrom( pFFmt );
					pDStyle->PresetParent( rParent );
				}
			}
			break;
		}
	}
	else
	{
		pStyle = mxBasePool->Find( rName, (SfxStyleFamily)nFamily );
		ASSERT(pStyle, "Vorlage nicht gefunden");
	}

	if(!pStyle)
		return sal_False;

	// Dialoge zusammenstoepseln
	//
	rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *(SwDocStyleSheet*)pStyle ) );
	if( SFX_STYLE_FAMILY_PARA == nFamily )
	{
		SfxItemSet& rSet = xTmp->GetItemSet();
		::SwToSfxPageDescAttr( rSet );
		// erstmal nur eine Null
		rSet.Put(SwBackgroundDestinationItem(SID_PARA_BACKGRND_DESTINATION, 0));
        // --> OD 2008-02-13 #newlistlevelattrs#
        // merge list level indent attributes into the item set if needed
        xTmp->MergeIndentAttrsOfListStyle( rSet );
        // <--
	}
/*	else if( SFX_STYLE_FAMILY_FRAME == nFamily )
	{
		// Auskommentiert wegen Bug #45776 (per default keine Breite&Groesse in Rahmenvorlagen)
		SfxItemSet& rSet = aTmp.GetItemSet();
		if( SFX_ITEM_SET != rSet.GetItemState( RES_FRM_SIZE ))
		{
			// dann sollten wir spaetesten hier eines anlegen
			SwFrmValid aFrmDefValues;
			rSet.Put( SwFmtFrmSize( ATT_VAR_SIZE, aFrmDefValues.nWidth,
									aFrmDefValues.nHeight ));
		}
	}*/
	else if( SFX_STYLE_FAMILY_CHAR == nFamily )
	{
		SfxItemSet& rSet = xTmp->GetItemSet();
		const SfxPoolItem *pTmpBrush;
		if( SFX_ITEM_SET == rSet.GetItemState( RES_CHRATR_BACKGROUND,
			sal_True, &pTmpBrush ) )
		{
			SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
			aTmpBrush.SetWhich( RES_BACKGROUND );
			rSet.Put( aTmpBrush );
		}
	}

    if(SFX_STYLE_FAMILY_PAGE == nFamily || SFX_STYLE_FAMILY_PARA == nFamily)
    {
        //UUUU create needed items for XPropertyList entries from the DrawModel so that
        // the Area TabPage can access them
        SfxItemSet& rSet = xTmp->GetItemSet();
        const SwDrawModel* pDrawModel = GetDoc()->GetDrawModel();

        rSet.Put(SvxColorTableItem(pDrawModel->GetColorTableFromSdrModel(), SID_COLOR_TABLE));
        rSet.Put(SvxGradientListItem(pDrawModel->GetGradientListFromSdrModel(), SID_GRADIENT_LIST));
        rSet.Put(SvxHatchListItem(pDrawModel->GetHatchListFromSdrModel(), SID_HATCH_LIST));
        rSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapListFromSdrModel(), SID_BITMAP_LIST));
    }

    if (!bBasic)
    {
        // vor dem Dialog wird der HtmlMode an der DocShell versenkt
        sal_uInt16 nHtmlMode = ::GetHtmlMode(this);

        // In HTML mode, we do not always have a printer. In order to show
        // the correct page size in the Format - Page dialog, we have to
        // get one here.
        SwWrtShell* pCurrShell = ( pActShell ? pActShell : mpWrtShell );
        if( ( HTMLMODE_ON & nHtmlMode ) &&
            !pCurrShell->getIDocumentDeviceAccess()->getPrinter( false ) )
            pCurrShell->InitPrt( pCurrShell->getIDocumentDeviceAccess()->getPrinter( true ) );

        PutItem(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
        FieldUnit eMetric = ::GetDfltMetric(0 != (HTMLMODE_ON&nHtmlMode));
        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "Dialogdiet fail!");
        SfxAbstractTabDialog* pDlg = pFact->CreateTemplateDialog( DLG_TEMPLATE_BASE,
                                                    0, *(xTmp.get()), nFamily, nSlot,
                                                    pActShell ? pActShell : mpWrtShell, bNew);
        DBG_ASSERT(pDlg, "Dialogdiet fail!");
		if(RET_OK == pDlg->Execute())
		{
			GetWrtShell()->StartAllAction();

			// nur bei Absatz-Vorlagen die Maske neu setzen
			if( bNew )
			{
				nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
						? xTmp->GetMask()
						: SFXSTYLEBIT_USERDEF;
			}
			else if( pStyle->GetMask() != xTmp->GetMask() )
				nRet = xTmp->GetMask();

			if( SFX_STYLE_FAMILY_PARA == nFamily )
			{
				SfxItemSet aSet( *pDlg->GetOutputItemSet() );
				::SfxToSwPageDescAttr( *GetWrtShell(), aSet  );
                // --> OD 2008-02-12 #newlistlevelattrs#
                // reset indent attributes at paragraph style, if a list style
                // will be applied and no indent attributes will be applied.
				xTmp->SetItemSet( aSet, true );
                // <--
			}
			else
			{
                if(SFX_STYLE_FAMILY_PAGE == nFamily)
                {
                    static const sal_uInt16 aInval[] = {
                        SID_IMAGE_ORIENTATION,
						SID_ATTR_CHAR_FONT,
						FN_INSERT_CTRL, FN_INSERT_OBJ_CTRL, 0};
                    mpView->GetViewFrame()->GetBindings().Invalidate(aInval);
                }
                SfxItemSet aTmpSet( *pDlg->GetOutputItemSet() );
				if( SFX_STYLE_FAMILY_CHAR == nFamily )
				{
					const SfxPoolItem *pTmpBrush;
					if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND,
						sal_False, &pTmpBrush ) )
					{
						SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
						aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
						aTmpSet.Put( aTmpBrush );
					}
					aTmpSet.ClearItem( RES_BACKGROUND );
				}

				xTmp->SetItemSet( aTmpSet );

                if( SFX_STYLE_FAMILY_PAGE == nFamily && SvtLanguageOptions().IsCTLFontEnabled() )
                {
                    const SfxPoolItem *pItem = NULL;
                    if( aTmpSet.GetItemState( GetPool().GetTrueWhich( SID_ATTR_FRAMEDIRECTION, sal_False ) , sal_True, &pItem ) == SFX_ITEM_SET )
                        SwChartHelper::DoUpdateAllCharts( mpDoc );
                }
			}

            //UUUU
            if(bNew)
            {
                if(SFX_STYLE_FAMILY_FRAME == nFamily || SFX_STYLE_FAMILY_PARA == nFamily)
                {
                    // clear FillStyle so that it works as a derived attribute
                    SfxItemSet aTmpSet(*pDlg->GetOutputItemSet());

                    aTmpSet.ClearItem(XATTR_FILLSTYLE);
                    xTmp->SetItemSet(aTmpSet);
                }
            }

			if(SFX_STYLE_FAMILY_PAGE == nFamily)
				mpView->InvalidateRulerPos();

			if( bNew )
				mxBasePool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *xTmp.get() ) );

			// JP 19.09.97:
			// Dialog vorm EndAction zerstoeren - bei Seitenvorlagen kann
			// muss der ItemSet zerstoert werden, damit die Cursor aus den
			// Kopf-/Fusszeilen entfernt werden. Sonst kommts zu GPFs!!!
			delete pDlg;

			mpDoc->SetModified();
			if( !bModified )	// Bug 57028
            {
                mpDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
            }

			GetWrtShell()->EndAllAction();
		}
		else
		{
			if( bNew )
            {
                // #116530#
				//pBasePool->Erase( &aTmp );
                GetWrtShell()->Undo(1);
                mpDoc->GetIDocumentUndoRedo().ClearRedo();
            }

			if( !bModified )
				mpDoc->ResetModified();
			delete pDlg;
		}
	}
	else
	{
		// vor dem Dialog wird der HtmlMode an der DocShell versenkt
		PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));

		GetWrtShell()->StartAllAction();

		// nur bei Absatz-Vorlagen die Maske neu setzen
		if( bNew )
		{
			nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
					? xTmp->GetMask()
					: SFXSTYLEBIT_USERDEF;
		}
		else if( pStyle->GetMask() != xTmp->GetMask() )
			nRet = xTmp->GetMask();

		if( SFX_STYLE_FAMILY_PARA == nFamily )
			::SfxToSwPageDescAttr( *GetWrtShell(), xTmp->GetItemSet() );
		else
		{
			SfxItemSet aTmpSet( xTmp->GetItemSet() );
			if( SFX_STYLE_FAMILY_CHAR == nFamily )
			{
				const SfxPoolItem *pTmpBrush;
				if( SFX_ITEM_SET == aTmpSet.GetItemState( RES_BACKGROUND,
					sal_False, &pTmpBrush ) )
				{
					SvxBrushItem aTmpBrush( *((SvxBrushItem*)pTmpBrush) );
					aTmpBrush.SetWhich( RES_CHRATR_BACKGROUND );
					aTmpSet.Put( aTmpBrush );
				}
				aTmpSet.ClearItem( RES_BACKGROUND );
			}
			xTmp->SetItemSet( aTmpSet );
		}
		if(SFX_STYLE_FAMILY_PAGE == nFamily)
			mpView->InvalidateRulerPos();

		if( bNew )
			mxBasePool->Broadcast( SfxStyleSheetHint( SFX_STYLESHEET_CREATED, *xTmp.get() ) );

		mpDoc->SetModified();
		if( !bModified )		// Bug 57028
        {
            mpDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
		GetWrtShell()->EndAllAction();
	}

	return nRet;
}

/*--------------------------------------------------------------------
	Beschreibung:	Delete
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::Delete(const String &rName, sal_uInt16 nFamily)
{
	SfxStyleSheetBase *pStyle = mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

	if(pStyle)
	{
		ASSERT(GetWrtShell(), "Keine Shell, keine Styles");

		GetWrtShell()->StartAllAction();
		mxBasePool->Remove(pStyle);
		GetWrtShell()->EndAllAction();

		return sal_True;
	}
	return sal_False;
}

/*--------------------------------------------------------------------
	Beschreibung:	Vorlage anwenden
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::ApplyStyles(
    const String &rName,
    const sal_uInt16 nFamily,
    SwWrtShell* pShell,
    const sal_uInt16 nMode )
{
    SwDocStyleSheet* pStyle = (SwDocStyleSheet*) mxBasePool->Find( rName, (SfxStyleFamily) nFamily );

    ASSERT( pStyle, "Wo ist der StyleSheet" );
    if ( !pStyle )
        return sal_False;

    SwWrtShell *pSh = pShell ? pShell : GetWrtShell();

    ASSERT( pSh, "Keine Shell, keine Styles" );

    pSh->StartAllAction();

    switch (nFamily)
    {
    case SFX_STYLE_FAMILY_CHAR:
    {
        SwFmtCharFmt aFmt( pStyle->GetCharFmt() );
        pSh->SetAttrItem( aFmt, ( nMode & KEY_SHIFT ) ? nsSetAttrMode::SETATTR_DONTREPLACE : nsSetAttrMode::SETATTR_DEFAULT );
        break;
    }
    case SFX_STYLE_FAMILY_PARA:
    {
        // clear also list attributes at affected text nodes, if paragraph style has the list style attribute set.
        pSh->SetTxtFmtColl( pStyle->GetCollection(), true );
        break;
    }
    case SFX_STYLE_FAMILY_FRAME:
    {
        if ( pSh->IsFrmSelected() )
            pSh->SetFrmFmt( pStyle->GetFrmFmt() );
        break;
    }
    case SFX_STYLE_FAMILY_PAGE:
    {
        pSh->SetPageStyle( pStyle->GetPageDesc()->GetName() );
        break;
    }

    case SFX_STYLE_FAMILY_PSEUDO:
    {
        const SwNumRule* pNumRule = pStyle->GetNumRule();
        const String sListIdForStyle = pNumRule->GetDefaultListId();
        pSh->SetCurNumRule( *pNumRule, false, sListIdForStyle, true );
        break;
    }

    default:
        DBG_ERROR( "Unbekannte Familie" );
    }
    pSh->EndAllAction();

    return nFamily;
}

/*--------------------------------------------------------------------
	Beschreibung:	Giesskanne starten
 --------------------------------------------------------------------*/



sal_uInt16 SwDocShell::DoWaterCan(const String &rName, sal_uInt16 nFamily)
{
	ASSERT(GetWrtShell(), "Keine Shell, keine Styles");

	SwEditWin& rEdtWin = mpView->GetEditWin();
	SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
	sal_Bool bWaterCan = !(pApply && pApply->eType != 0);
	if( !rName.Len() )
		bWaterCan = sal_False;
	SwApplyTemplate aTemplate;
	aTemplate.eType = nFamily;

	if(bWaterCan)
	{
		SwDocStyleSheet* pStyle =
			(SwDocStyleSheet*)mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

		ASSERT(pStyle, "Wo ist der StyleSheet");
		if(!pStyle) return nFamily;

		switch(nFamily)
		{
			case SFX_STYLE_FAMILY_CHAR:
				aTemplate.aColl.pCharFmt = pStyle->GetCharFmt();
				break;
			case SFX_STYLE_FAMILY_PARA:
				aTemplate.aColl.pTxtColl = pStyle->GetCollection();
				break;
			case SFX_STYLE_FAMILY_FRAME:
				aTemplate.aColl.pFrmFmt = pStyle->GetFrmFmt();
				break;
			case SFX_STYLE_FAMILY_PAGE:
				aTemplate.aColl.pPageDesc = (SwPageDesc*)pStyle->GetPageDesc();
				break;
			case SFX_STYLE_FAMILY_PSEUDO:
				aTemplate.aColl.pNumRule = (SwNumRule*)pStyle->GetNumRule();
				break;

			default:
				DBG_ERROR( "Unbekannte Familie");
		}
	}
	else
		aTemplate.eType = 0;

	// Template anwenden
	mpView->GetEditWin().SetApplyTemplate(aTemplate);

	return nFamily;
}

/*--------------------------------------------------------------------
	Beschreibung:	Vorlage Updaten
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::UpdateStyle(const String &rName, sal_uInt16 nFamily, SwWrtShell* pShell)
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
	ASSERT(GetWrtShell(), "Keine Shell, keine Styles");

	SwDocStyleSheet* pStyle =
		(SwDocStyleSheet*)mxBasePool->Find(rName, (SfxStyleFamily)nFamily);

	if(!pStyle)
		return nFamily;

	switch(nFamily)
	{
		case SFX_STYLE_FAMILY_PARA:
		{
			SwTxtFmtColl* pColl = pStyle->GetCollection();
			if(pColl && !pColl->IsDefault())
			{
				GetWrtShell()->StartAllAction();

                SwRewriter aRewriter;
                aRewriter.AddRule(UNDO_ARG1, pColl->GetName());

                GetWrtShell()->StartUndo(UNDO_INSFMTATTR, &aRewriter);
				GetWrtShell()->FillByEx(pColl);
					// Vorlage auch anwenden, um harte Attributierung
					// zu entfernen
				GetWrtShell()->SetTxtFmtColl( pColl );
                GetWrtShell()->EndUndo();
				GetWrtShell()->EndAllAction();
			}
			break;
		}
		case SFX_STYLE_FAMILY_FRAME:
		{
			SwFrmFmt* pFrm = pStyle->GetFrmFmt();
            if( pCurrWrtShell->IsFrmSelected() && pFrm && !pFrm->IsDefault() )
			{
				SfxItemSet aSet( GetPool(), aFrmFmtSetRange );
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->GetFlyFrmAttr( aSet );

                // --> OD 2009-12-28 #i105535#
                // no update of anchor attribute
                aSet.ClearItem( RES_ANCHOR );
                // <--

                pFrm->SetFmtAttr( aSet );

					// Vorlage auch anwenden, um harte Attributierung
					// zu entfernen
                pCurrWrtShell->SetFrmFmt( pFrm, sal_True );
                pCurrWrtShell->EndAllAction();
			}
		}
		break;
		case SFX_STYLE_FAMILY_CHAR:
		{
			SwCharFmt* pChar = pStyle->GetCharFmt();
			if( pChar && !pChar->IsDefault() )
			{
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pChar);
					// Vorlage auch anwenden, um harte Attributierung
					// zu entfernen
                pCurrWrtShell->EndAllAction();
			}

		}
		break;
		case SFX_STYLE_FAMILY_PSEUDO:
		{
			const SwNumRule* pCurRule;
			if( pStyle->GetNumRule() &&
                0 != ( pCurRule = pCurrWrtShell->GetNumRuleAtCurrCrsrPos() ))
			{
				SwNumRule aRule( *pCurRule );
                // --> OD 2008-07-08 #i91400#
                aRule.SetName( pStyle->GetNumRule()->GetName(),
                               *(pCurrWrtShell->GetDoc()) );
                // <--
                pCurrWrtShell->ChgNumRuleFmts( aRule );
			}
		}
		break;
	}
	return nFamily;
}

/*--------------------------------------------------------------------
	Beschreibung:	NewByExample
 --------------------------------------------------------------------*/


sal_uInt16 SwDocShell::MakeByExample( const String &rName, sal_uInt16 nFamily,
									sal_uInt16 nMask, SwWrtShell* pShell )
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
	SwDocStyleSheet* pStyle = (SwDocStyleSheet*)mxBasePool->Find(
											rName, (SfxStyleFamily)nFamily );
	if(!pStyle)
	{
		// JP 07.07.95: behalte die akt. Maske vom PI bei, dadurch werden
		//				neue sofort in den sichtbaren Bereich einsortiert
		if( SFXSTYLEBIT_ALL == nMask || SFXSTYLEBIT_USED == nMask )
			nMask = SFXSTYLEBIT_USERDEF;
		else
			nMask |= SFXSTYLEBIT_USERDEF;

		pStyle = (SwDocStyleSheet*)&mxBasePool->Make(rName,
								(SfxStyleFamily)nFamily, nMask );
	}

	switch(nFamily)
	{
		case  SFX_STYLE_FAMILY_PARA:
		{
			SwTxtFmtColl* pColl = pStyle->GetCollection();
			if(pColl && !pColl->IsDefault())
			{
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pColl);
					// Vorlage auch anwenden, um harte Attributierung
					// zu entfernen
                pColl->SetDerivedFrom(pCurrWrtShell->GetCurTxtFmtColl());

					// setze die Maske noch an der Collection:
				sal_uInt16 nId = pColl->GetPoolFmtId() & 0x87ff;
				switch( nMask & 0x0fff )
				{
				case SWSTYLEBIT_TEXT:
					nId |= COLL_TEXT_BITS;
					break;
				case SWSTYLEBIT_CHAPTER:
					nId |= COLL_DOC_BITS;
					break;
				case SWSTYLEBIT_LIST:
					nId |= COLL_LISTS_BITS;
					break;
				case SWSTYLEBIT_IDX:
					nId |= COLL_REGISTER_BITS;
					break;
				case SWSTYLEBIT_EXTRA:
					nId |= COLL_EXTRA_BITS;
					break;
				case SWSTYLEBIT_HTML:
					nId |= COLL_HTML_BITS;
					break;
				}
				pColl->SetPoolFmtId(nId);

                pCurrWrtShell->SetTxtFmtColl(pColl);
                pCurrWrtShell->EndAllAction();
			}
		}
		break;
		case SFX_STYLE_FAMILY_FRAME:
		{
			SwFrmFmt* pFrm = pStyle->GetFrmFmt();
            if(pCurrWrtShell->IsFrmSelected() && pFrm && !pFrm->IsDefault())
			{
                pCurrWrtShell->StartAllAction();

				SfxItemSet aSet(GetPool(), aFrmFmtSetRange );
                pCurrWrtShell->GetFlyFrmAttr( aSet );

                SwFrmFmt* pFFmt = pCurrWrtShell->GetCurFrmFmt();
                pFrm->SetDerivedFrom( pFFmt );

				// JP 10.06.98: nur automatische Orientierungen uebernehmen
/*				#61359# jetzt auch wieder alle Orientierungen
				const SfxPoolItem* pItem;
				if( SFX_ITEM_SET == aSet.GetItemState( RES_VERT_ORIENT,
					sal_False, &pItem ) &&
                    text::VertOrientation::NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient())
					aSet.ClearItem( RES_VERT_ORIENT );

				if( SFX_ITEM_SET == aSet.GetItemState( RES_HORI_ORIENT,
					sal_False, &pItem ) &&
                    text::HoriOrientation::NONE == ((SwFmtHoriOrient*)pItem)->GetHoriOrient())
					aSet.ClearItem( RES_HORI_ORIENT );
 */

                pFrm->SetFmtAttr( aSet );
					// Vorlage auch anwenden, um harte Attributierung
					// zu entfernen
                pCurrWrtShell->SetFrmFmt( pFrm );
                pCurrWrtShell->EndAllAction();
			}
		}
		break;
		case SFX_STYLE_FAMILY_CHAR:
		{
			SwCharFmt* pChar = pStyle->GetCharFmt();
			if(pChar && !pChar->IsDefault())
			{
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx( pChar );
                pChar->SetDerivedFrom( pCurrWrtShell->GetCurCharFmt() );
				SwFmtCharFmt aFmt( pChar );
                pCurrWrtShell->SetAttrItem( aFmt );
                pCurrWrtShell->EndAllAction();
			}
		}
		break;

		case SFX_STYLE_FAMILY_PAGE:
		{
            pCurrWrtShell->StartAllAction();
            sal_uInt16 nPgDsc = pCurrWrtShell->GetCurPageDesc();
            SwPageDesc& rSrc = (SwPageDesc&)pCurrWrtShell->GetPageDesc( nPgDsc );
			SwPageDesc& rDest = *(SwPageDesc*)pStyle->GetPageDesc();

			sal_uInt16 nPoolId = rDest.GetPoolFmtId();
			sal_uInt16 nHId = rDest.GetPoolHelpId();
			sal_uInt8 nHFId = rDest.GetPoolHlpFileId();

            pCurrWrtShell->GetDoc()->CopyPageDesc( rSrc, rDest );

			// PoolId darf NIE kopiert werden!
			rDest.SetPoolFmtId( nPoolId );
			rDest.SetPoolHelpId( nHId );
			rDest.SetPoolHlpFileId( nHFId );

			// werden Kopf-/Fusszeilen angelegt, so gibt es kein Undo mehr!
            pCurrWrtShell->GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();

            pCurrWrtShell->EndAllAction();
		}
		break;

		case SFX_STYLE_FAMILY_PSEUDO:
		{
            pCurrWrtShell->StartAllAction();

            SwNumRule aRule( *pCurrWrtShell->GetNumRuleAtCurrCrsrPos() );
			String sOrigRule( aRule.GetName() );
            // --> OD 2008-07-08 #i91400#
            aRule.SetName( pStyle->GetNumRule()->GetName(),
                           *(pCurrWrtShell->GetDoc()) );
            // <--
            pCurrWrtShell->ChgNumRuleFmts( aRule );

            pCurrWrtShell->ReplaceNumRule( sOrigRule, aRule.GetName() );


            pCurrWrtShell->EndAllAction();
		}
		break;
	}
	return nFamily;
}

void  SwDocShell::LoadStyles( SfxObjectShell& rSource )
{
    _LoadStyles(rSource, sal_False);
}
/* -----------------16.05.2003 15:45-----------------
    bPreserveCurrentDocument determines whether SetFixFields() is called
    This call modifies the source document. This mustn't happen when the source
    is a document the user is working on.
    Calls of ::LoadStyles() normally use files especially loaded for the purpose
    of importing styles.
 --------------------------------------------------*/
void SwDocShell::_LoadStyles( SfxObjectShell& rSource, sal_Bool bPreserveCurrentDocument )
{
/*	[Beschreibung]

	Diese Methode wird vom SFx gerufen, wenn aus einer Dokument-Vorlage
	Styles nachgeladen werden sollen. Bestehende Styles soll dabei
	"uberschrieben werden. Das Dokument mu"s daher neu formatiert werden.
	Daher werden die Applikationen in der Regel diese Methode "uberladen
	und in ihrer Implementierung die Implementierung der Basisklasse
	rufen.
*/
	// ist die Source unser Document, dann uebernehmen wir das
	// abpruefen selbst (wesentlich schneller und laeuft nicht ueber
	// die Kruecke SfxStylePool
	if( rSource.ISA( SwDocShell ))
	{
		//JP 28.05.99: damit die Kopf-/Fusszeilen nicht den fixen Inhalt
		// 				der Vorlage erhalten, einmal alle FixFelder der
		//				Source aktualisieren
        if(!bPreserveCurrentDocument)
            ((SwDocShell&)rSource).mpDoc->SetFixFields(false, NULL);
		if( mpWrtShell )
		{
			mpWrtShell->StartAllAction();
			mpDoc->ReplaceStyles( *((SwDocShell&)rSource).mpDoc );
			mpWrtShell->EndAllAction();
		}
		else
		{
			sal_Bool bModified = mpDoc->IsModified();
			mpDoc->ReplaceStyles( *((SwDocShell&)rSource).mpDoc );
			if( !bModified && mpDoc->IsModified() && !mpView )
			{
				// die View wird spaeter angelegt, ueberschreibt aber das
				// Modify-Flag. Per Undo ist sowieso nichts mehr zu machen
                mpDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
            }
        }
    }
	else
		SfxObjectShell::LoadStyles( rSource );
}


void SwDocShell::FormatPage(
    const String& rPage, 
    const sal_uInt16 nSlot, 
    SwWrtShell& rActShell )
{
    Edit( rPage, aEmptyStr, SFX_STYLE_FAMILY_PAGE, 0, sal_False, nSlot, &rActShell);
}

Bitmap SwDocShell::GetStyleFamilyBitmap( SfxStyleFamily eFamily, BmpColorMode eColorMode )
{
	if( SFX_STYLE_FAMILY_PSEUDO == eFamily )
	{
		if ( eColorMode == BMP_COLOR_NORMAL )
			return Bitmap( SW_RES( BMP_STYLES_FAMILY_NUM ));
		else
			return Bitmap( SW_RES( BMP_STYLES_FAMILY_NUM_HC ));
	}

	return SfxObjectShell::GetStyleFamilyBitmap( eFamily, eColorMode );
}



