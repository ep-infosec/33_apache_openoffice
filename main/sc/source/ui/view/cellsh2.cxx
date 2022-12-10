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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <svl/aeitem.hxx>
#include <basic/sbxcore.hxx>
#include <svl/whiter.hxx>
#include <svl/zforlist.hxx>
#include <vcl/msgbox.hxx>
#include <svl/stritem.hxx>
#include <svl/visitem.hxx>
#include <unotools/moduleoptions.hxx>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>

#include "cellsh.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "global.hxx"
#include "scmod.hxx"
#include "docsh.hxx"
#include "document.hxx"
#include "uiitems.hxx"
#include "dbfunc.hxx"
#include "dbdocfun.hxx"
//CHINA001 #include "lbseldlg.hxx"
//CHINA001 #include "sortdlg.hxx"
#include "filtdlg.hxx"
#include "dbnamdlg.hxx"
//CHINA001 #include "subtdlg.hxx"
#include "reffact.hxx"
#include "pvlaydlg.hxx"
#include "validat.hxx"
#include "scresid.hxx"
//CHINA001 #include "validate.hxx"
#include "pivot.hxx"
#include "dpobject.hxx"
//CHINA001 #include "dapitype.hxx"
//CHINA001 #include "dapidata.hxx"
#include "dpsdbtab.hxx"		// ScImportSourceDesc
#include "dpshttab.hxx"		// ScSheetSourceDesc

#include "validate.hrc" //CHINA001 add for ScValidationDlg
#include "scui_def.hxx" //CHINA001
#include "scabstdlg.hxx" //CHINA001
#include "impex.hxx"
#include "asciiopt.hxx"
using namespace com::sun::star;

//#include "strindlg.hxx"		//! Test !!!!!

//static ScArea aPivotSource; 		//! wohin? (ueber den Dialog retten)


#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), sal_True, ppItem ) == SFX_ITEM_SET)

//------------------------------------------------------------------

bool lcl_GetTextToColumnsRange( const ScViewData* pData, ScRange& rRange )
{
    DBG_ASSERT( pData, "lcl_GetTextToColumnsRange: pData is null!" );

    bool bRet = false;
    const ScMarkData& rMark = pData->GetMarkData();

    if ( rMark.IsMarked() )
    {
        if ( !rMark.IsMultiMarked() )
        {
            rMark.GetMarkArea( rRange );
            if ( rRange.aStart.Col() == rRange.aEnd.Col() )
            {
                bRet = true;
            }
        }
    }
    else
    {
        const SCCOL nCol = pData->GetCurX();
        const SCROW nRow = pData->GetCurY();
        const SCTAB nTab = pData->GetTabNo();
        rRange = ScRange( nCol, nRow, nTab, nCol, nRow, nTab );
        bRet = true;
    }

    const ScDocument* pDoc = pData->GetDocument();
    DBG_ASSERT( pDoc, "lcl_GetTextToColumnsRange: pDoc is null!" );

    if ( bRet && pDoc->IsBlockEmpty( rRange.aStart.Tab(), rRange.aStart.Col(),
                                     rRange.aStart.Row(), rRange.aEnd.Col(),
                                     rRange.aEnd.Row() ) )
    {
        bRet = false;
    }

    return bRet;
}

sal_Bool lcl_GetSortParam( const ScViewData* pData, ScSortParam& rSortParam )
{
    ScTabViewShell* pTabViewShell   = pData->GetViewShell();
    ScDBData*	pDBData             = pTabViewShell->GetDBData();
    ScDocument* pDoc                = pData->GetDocument();
    SCTAB nTab                      = pData->GetTabNo();
    ScDirection eFillDir            = DIR_TOP;
    sal_Bool  bSort                     = sal_True;
    ScRange aExternalRange;

    if( rSortParam.nCol1 != rSortParam.nCol2 )
        eFillDir = DIR_LEFT;
    if( rSortParam.nRow1 != rSortParam.nRow2 )
        eFillDir = DIR_TOP;

    SCSIZE nCount = pDoc->GetEmptyLinesInBlock( rSortParam.nCol1, rSortParam.nRow1, nTab, rSortParam.nCol2, rSortParam.nRow2, nTab, eFillDir );

    if( rSortParam.nRow2 == MAXROW )
        aExternalRange = ScRange( rSortParam.nCol1,sal::static_int_cast<SCROW>( nCount ), nTab );
    else
        aExternalRange = ScRange( pData->GetCurX(), pData->GetCurY(), nTab );

    SCROW nStartRow = aExternalRange.aStart.Row();
    SCCOL nStartCol = aExternalRange.aStart.Col();
    SCROW nEndRow   = aExternalRange.aEnd.Row();
    SCCOL nEndCol   = aExternalRange.aEnd.Col();
    pDoc->GetDataArea( aExternalRange.aStart.Tab(), nStartCol, nStartRow, nEndCol, nEndRow, sal_False, false );
    aExternalRange.aStart.SetRow( nStartRow );
    aExternalRange.aStart.SetCol( nStartCol );
    aExternalRange.aEnd.SetRow( nEndRow );
    aExternalRange.aEnd.SetCol( nEndCol );

    if(( rSortParam.nCol1 == rSortParam.nCol2 && aExternalRange.aStart.Col() != aExternalRange.aEnd.Col() ) ||
        ( rSortParam.nRow1 == rSortParam.nRow2 && aExternalRange.aStart.Row() != aExternalRange.aEnd.Row() ) )
    {
        sal_uInt16 nFmt = SCA_VALID;
        String aExtendStr,aCurrentStr;

        pTabViewShell->AddHighlightRange( aExternalRange,Color( COL_LIGHTBLUE ) );
        ScRange rExtendRange( aExternalRange.aStart.Col(), aExternalRange.aStart.Row(), nTab, aExternalRange.aEnd.Col(), aExternalRange.aEnd.Row(), nTab );
        rExtendRange.Format( aExtendStr, nFmt, pDoc );

        ScRange rCurrentRange( rSortParam.nCol1, rSortParam.nRow1, nTab, rSortParam.nCol2, rSortParam.nRow2, nTab );
        rCurrentRange.Format( aCurrentStr, nFmt, pDoc );

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

        VclAbstractDialog* pWarningDlg = pFact->CreateScSortWarningDlg( pTabViewShell->GetDialogParent(),aExtendStr,aCurrentStr,RID_SCDLG_SORT_WARNING );
        DBG_ASSERT(pWarningDlg, "Dialog create fail!");//CHINA001
        short bResult = pWarningDlg->Execute();
        if( bResult == BTN_EXTEND_RANGE || bResult == BTN_CURRENT_SELECTION )
        {
            if( bResult == BTN_EXTEND_RANGE )
            {
                pTabViewShell->MarkRange( aExternalRange, sal_False );
                pDBData->SetArea( nTab, aExternalRange.aStart.Col(), aExternalRange.aStart.Row(), aExternalRange.aEnd.Col(), aExternalRange.aEnd.Row() );
            }
        }
        else
        {
            bSort = sal_False;
            pData->GetDocShell()->CancelAutoDBRange();
        }

        delete pWarningDlg;
        pTabViewShell->ClearHighlightRanges();
    }
    return bSort;
}

//<!-- Added by PengYunQuan for Validity Cell Range Picker
//after end execute from !IsModalInputMode, it is safer to delay deleting
namespace
{
	long DelayDeleteAbstractDialog( void *pAbstractDialog, void * /*pArg*/ )
	{
		delete reinterpret_cast<VclAbstractDialog*>( pAbstractDialog );
		return 0;
	} 
}
//--> Added by PengYunQuan for Validity Cell Range Picker

void ScCellShell::ExecuteDB( SfxRequest& rReq )
{
	ScTabViewShell*	pTabViewShell  	= GetViewData()->GetViewShell();
	sal_uInt16 nSlotId = rReq.GetSlot();
	const SfxItemSet*	pReqArgs	= rReq.GetArgs();
	ScModule*			pScMod		= SC_MOD();

	pTabViewShell->HideListBox();					// Autofilter-DropDown-Listbox

	if ( GetViewData()->HasEditView( GetViewData()->GetActivePart() ) )
	{
		pScMod->InputEnterHandler();
		pTabViewShell->UpdateInputHandler();
	}

	switch ( nSlotId )
	{
		case SID_VIEW_DATA_SOURCE_BROWSER:
			{
				//	check if database beamer is open

				SfxViewFrame* pViewFrame = pTabViewShell->GetViewFrame();
				sal_Bool bWasOpen = sal_False;
				{
					uno::Reference<frame::XFrame> xFrame = pViewFrame->GetFrame().GetFrameInterface();
					uno::Reference<frame::XFrame> xBeamerFrame = xFrame->findFrame(
														rtl::OUString::createFromAscii("_beamer"),
														frame::FrameSearchFlag::CHILDREN);
					if ( xBeamerFrame.is() )
						bWasOpen = sal_True;
				}

				if ( bWasOpen )
				{
					//	close database beamer: just forward to SfxViewFrame

					pViewFrame->ExecuteSlot( rReq );
				}
				else
				{
					//	show database beamer: SfxViewFrame call must be synchronous

					pViewFrame->ExecuteSlot( rReq, (sal_Bool) sal_False );		// sal_False = synchronous

					//	select current database in database beamer

					ScImportParam aImportParam;
					ScDBData* pDBData = pTabViewShell->GetDBData(sal_True,SC_DB_OLD);		// don't create if none found
					if (pDBData)
						pDBData->GetImportParam( aImportParam );

					ScDBDocFunc::ShowInBeamer( aImportParam, pTabViewShell->GetViewFrame() );
				}
				rReq.Done();		// needed because it's a toggle slot
			}
			break;

		case SID_REIMPORT_DATA:
			{
				sal_Bool bOk = sal_False;
				ScDBData* pDBData = pTabViewShell->GetDBData(sal_True,SC_DB_OLD);
				if (pDBData)
				{
					ScImportParam aImportParam;
					pDBData->GetImportParam( aImportParam );
					if (aImportParam.bImport && !pDBData->HasImportSelection())
					{
						pTabViewShell->ImportData( aImportParam );
						pDBData->SetImportParam( aImportParam );	//! Undo ??
						bOk = sal_True;
					}
				}

				if (!bOk && ! rReq.IsAPI() )
					pTabViewShell->ErrorMessage(STR_REIMPORT_EMPTY);

				if( bOk )
					rReq.Done();
			}
			break;

		case SID_REFRESH_DBAREA:
			{
				ScDBData* pDBData = pTabViewShell->GetDBData(sal_True,SC_DB_OLD);
				if (pDBData)
				{
					//	Import wiederholen wie SID_REIMPORT_DATA

					sal_Bool bContinue = sal_True;
					ScImportParam aImportParam;
					pDBData->GetImportParam( aImportParam );
					if (aImportParam.bImport && !pDBData->HasImportSelection())
					{
						bContinue = pTabViewShell->ImportData( aImportParam );
						pDBData->SetImportParam( aImportParam );	//! Undo ??

						//	markieren (Groesse kann sich geaendert haben)
						ScRange aNewRange;
						pDBData->GetArea(aNewRange);
						pTabViewShell->MarkRange(aNewRange);
					}

					if ( bContinue )		// #41905# Fehler beim Import -> Abbruch
					{
						//	interne Operationen, wenn welche gespeichert

						if ( pDBData->HasQueryParam() || pDBData->HasSortParam() ||
														  pDBData->HasSubTotalParam() )
							pTabViewShell->RepeatDB();

						//	Pivottabellen die den Bereich als Quelldaten haben

						ScRange aRange;
						pDBData->GetArea(aRange);
						GetViewData()->GetDocShell()->RefreshPivotTables(aRange);
					}
				}
				rReq.Done();
			}
			break;

		case SID_SBA_BRW_INSERT:
			{
				DBG_ERROR( "Deprecated Slot" );
			}
			break;

		case SID_SUBTOTALS:
			{
				const SfxItemSet* pArgs = rReq.GetArgs();
				if ( pArgs )
				{
					pTabViewShell->DoSubTotals( ((const ScSubTotalItem&) pArgs->Get( SCITEM_SUBTDATA )).
									GetSubTotalData() );
					rReq.Done();
				}
				else
				{
					//CHINA001 ScSubTotalDlg*	pDlg = NULL;
					SfxAbstractTabDialog * pDlg = NULL;
					ScSubTotalParam aSubTotalParam;
					SfxItemSet		aArgSet( GetPool(), SCITEM_SUBTDATA, SCITEM_SUBTDATA );

                    ScDBData* pDBData = pTabViewShell->GetDBData();
					pDBData->GetSubTotalParam( aSubTotalParam );
					aSubTotalParam.bRemoveOnly = sal_False;

					aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA, GetViewData(), &aSubTotalParam ) );
					//CHINA001 pDlg = new ScSubTotalDlg( pTabViewShell->GetDialogParent(), &aArgSet );
					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

					pDlg = pFact->CreateScSubTotalDlg( pTabViewShell->GetDialogParent(), &aArgSet, RID_SCDLG_SUBTOTALS );
					DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
					pDlg->SetCurPageId(1);

					short bResult = pDlg->Execute();

					if ( (bResult == RET_OK) || (bResult == SCRET_REMOVE) )
					{
						const SfxItemSet* pOutSet = NULL;

						if ( bResult == RET_OK )
						{
							pOutSet = pDlg->GetOutputItemSet();
							aSubTotalParam =
								((const ScSubTotalItem&)
									pOutSet->Get( SCITEM_SUBTDATA )).
										GetSubTotalData();
						}
						else // if (bResult == SCRET_REMOVE)
						{
							pOutSet = &aArgSet;
							aSubTotalParam.bRemoveOnly = sal_True;
							aSubTotalParam.bReplace    = sal_True;
							aArgSet.Put( ScSubTotalItem( SCITEM_SUBTDATA,
														 GetViewData(),
														 &aSubTotalParam ) );
						}

						pTabViewShell->DoSubTotals( aSubTotalParam );
						rReq.Done( *pOutSet );
					}
                    else
                        GetViewData()->GetDocShell()->CancelAutoDBRange();

					delete pDlg;
				}
			}
			break;

        case SID_SORT_DESCENDING:
        case SID_SORT_ASCENDING:
            {
                //#i60401 ux-ctest: Calc does not support all users' strategies regarding sorting data
                //the patch comes from maoyg
                ScSortParam aSortParam;
                ScDBData*	pDBData = pTabViewShell->GetDBData();
                ScViewData* pData   = GetViewData();

                pDBData->GetSortParam( aSortParam );

                if( lcl_GetSortParam( pData, aSortParam ) )
                {
                    SfxItemSet  aArgSet( GetPool(), SCITEM_SORTDATA, SCITEM_SORTDATA );
                    SCCOL nCol  = GetViewData()->GetCurX();
                    SCCOL nTab  = GetViewData()->GetTabNo();
                    ScDocument* pDoc    = GetViewData()->GetDocument();

                    pDBData->GetSortParam( aSortParam );
                    sal_Bool bHasHeader = pDoc->HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, nTab );

                    if( nCol < aSortParam.nCol1 )
                        nCol = aSortParam.nCol1;
                    else if( nCol > aSortParam.nCol2 )
                        nCol = aSortParam.nCol2;

                    aSortParam.bHasHeader		= bHasHeader;
                    aSortParam.bByRow			= sal_True;
                    aSortParam.bCaseSens		= sal_False;
                    aSortParam.bIncludePattern	= sal_True;
                    aSortParam.bInplace 		= sal_True;
                    aSortParam.bDoSort[0]		= sal_True;
                    aSortParam.nField[0]		= nCol;
                    aSortParam.bAscending[0]	= (nSlotId == SID_SORT_ASCENDING);

                    for ( sal_uInt16 i=1; i<MAXSORT; i++ )
                        aSortParam.bDoSort[i] = sal_False;

                    aArgSet.Put( ScSortItem( SCITEM_SORTDATA, GetViewData(), &aSortParam ) );

                    pTabViewShell->UISort( aSortParam );		// Teilergebnisse bei Bedarf neu

                    rReq.Done();
                }
            }
            break;

        case SID_SORT:
            {
                const SfxItemSet* pArgs = rReq.GetArgs();

                //#i60401 ux-ctest: Calc does not support all users' strategies regarding sorting data
                //the patch comes from maoyg

                if ( pArgs )		// Basic
                {
                    ScSortParam aSortParam;
                    ScDBData*	pDBData = pTabViewShell->GetDBData();
                    ScViewData* pData   = GetViewData();

                    pDBData->GetSortParam( aSortParam );

                    if( lcl_GetSortParam( pData, aSortParam ) )
                    {
                        ScDocument* pDoc = GetViewData()->GetDocument();

                        pDBData->GetSortParam( aSortParam );
                        sal_Bool bHasHeader = pDoc->HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, pData->GetTabNo() );
                        if( bHasHeader )
                            aSortParam.bHasHeader = bHasHeader;

                        aSortParam.bInplace = sal_True;				// von Basic immer

                        const SfxPoolItem* pItem;
                        if ( pArgs->GetItemState( SID_SORT_BYROW, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bByRow = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_HASHEADER, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bHasHeader = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_CASESENS, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bCaseSens = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_ATTRIBS, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bIncludePattern = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pArgs->GetItemState( SID_SORT_USERDEF, sal_True, &pItem ) == SFX_ITEM_SET )
                        {
                            sal_uInt16 nUserIndex = ((const SfxUInt16Item*)pItem)->GetValue();
                            aSortParam.bUserDef = ( nUserIndex != 0 );
                            if ( nUserIndex )
                                aSortParam.nUserIndex = nUserIndex - 1;		// Basic: 1-basiert
                        }

                        SCCOLROW nField0 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_1, sal_True, &pItem ) == SFX_ITEM_SET )
                            nField0 = ((const SfxInt32Item*)pItem)->GetValue();
                        aSortParam.bDoSort[0] = ( nField0 != 0 );
                        aSortParam.nField[0] = nField0 > 0 ? (nField0-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_2, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bAscending[0] = ((const SfxBoolItem*)pItem)->GetValue();
                        SCCOLROW nField1 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_3, sal_True, &pItem ) == SFX_ITEM_SET )
                            nField1 = ((const SfxInt32Item*)pItem)->GetValue();
                        aSortParam.bDoSort[1] = ( nField1 != 0 );
                        aSortParam.nField[1] = nField1 > 0 ? (nField1-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_4, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bAscending[1] = ((const SfxBoolItem*)pItem)->GetValue();
                        SCCOLROW nField2 = 0;
                        if ( pArgs->GetItemState( FN_PARAM_5, sal_True, &pItem ) == SFX_ITEM_SET )
                            nField2 = ((const SfxInt32Item*)pItem)->GetValue();
                        aSortParam.bDoSort[2] = ( nField2 != 0 );
                        aSortParam.nField[2] = nField2 > 0 ? (nField2-1) : 0;
                        if ( pArgs->GetItemState( FN_PARAM_6, sal_True, &pItem ) == SFX_ITEM_SET )
                            aSortParam.bAscending[2] = ((const SfxBoolItem*)pItem)->GetValue();

                        // Teilergebnisse bei Bedarf neu
                        pTabViewShell->UISort( aSortParam );
                        rReq.Done();
                    }
                }
                else
                {
                    ScSortParam aSortParam;
                    ScDBData*	pDBData = pTabViewShell->GetDBData();
                    ScViewData* pData   = GetViewData();

                    pDBData->GetSortParam( aSortParam );

                    if( lcl_GetSortParam( pData, aSortParam ) )
                    {
                        SfxAbstractTabDialog* pDlg = NULL;
                        ScDocument* pDoc = GetViewData()->GetDocument();
                        SfxItemSet	aArgSet( GetPool(), SCITEM_SORTDATA, SCITEM_SORTDATA );

                        pDBData->GetSortParam( aSortParam );
                        sal_Bool bHasHeader = pDoc->HasColHeader( aSortParam.nCol1, aSortParam.nRow1, aSortParam.nCol2, aSortParam.nRow2, pData->GetTabNo() );
                        if( bHasHeader )
                            aSortParam.bHasHeader = bHasHeader;

                        aArgSet.Put( ScSortItem( SCITEM_SORTDATA, GetViewData(), &aSortParam ) );

                        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                        DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

                        pDlg = pFact->CreateScSortDlg( pTabViewShell->GetDialogParent(),  &aArgSet, RID_SCDLG_SORT );
                        DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
                        pDlg->SetCurPageId(1);

                        if ( pDlg->Execute() == RET_OK )
                        {
                            const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                            const ScSortParam& rOutParam = ((const ScSortItem&)
                                pOutSet->Get( SCITEM_SORTDATA )).GetSortData();

                            // Teilergebnisse bei Bedarf neu
                            pTabViewShell->UISort( rOutParam );

                            if ( rOutParam.bInplace )
                            {
                                rReq.AppendItem( SfxBoolItem( SID_SORT_BYROW,
                                    rOutParam.bByRow ) );
                                rReq.AppendItem( SfxBoolItem( SID_SORT_HASHEADER,
                                    rOutParam.bHasHeader ) );
                                rReq.AppendItem( SfxBoolItem( SID_SORT_CASESENS,
                                    rOutParam.bCaseSens ) );
                                rReq.AppendItem( SfxBoolItem( SID_SORT_ATTRIBS,
                                    rOutParam.bIncludePattern ) );
                                sal_uInt16 nUser = rOutParam.bUserDef ? ( rOutParam.nUserIndex + 1 ) : 0;
                                rReq.AppendItem( SfxUInt16Item( SID_SORT_USERDEF, nUser ) );
                                if ( rOutParam.bDoSort[0] )
                                {
                                    rReq.AppendItem( SfxInt32Item( FN_PARAM_1,
                                        rOutParam.nField[0] + 1 ) );
                                    rReq.AppendItem( SfxBoolItem( FN_PARAM_2,
                                        rOutParam.bAscending[0] ) );
                                }
                                if ( rOutParam.bDoSort[1] )
                                {
                                    rReq.AppendItem( SfxInt32Item( FN_PARAM_3,
                                        rOutParam.nField[1] + 1 ) );
                                    rReq.AppendItem( SfxBoolItem( FN_PARAM_4,
                                        rOutParam.bAscending[1] ) );
                                }
                                if ( rOutParam.bDoSort[2] )
                                {
                                    rReq.AppendItem( SfxInt32Item( FN_PARAM_5,
                                        rOutParam.nField[2] + 1 ) );
                                    rReq.AppendItem( SfxBoolItem( FN_PARAM_6,
                                        rOutParam.bAscending[2] ) );
                                }
                            }

                            rReq.Done();
                        }
                        else
                            GetViewData()->GetDocShell()->CancelAutoDBRange();

                        delete pDlg;
                    }
                }
            }
            break;

		case SID_FILTER:
			{
				const SfxItemSet* pArgs = rReq.GetArgs();
				if ( pArgs )
				{
					DBG_ERROR("SID_FILTER with arguments?");
					pTabViewShell->Query( ((const ScQueryItem&)
							pArgs->Get( SCITEM_QUERYDATA )).GetQueryData(), NULL, sal_True );
					rReq.Done();
				}
				else
				{
					sal_uInt16			nId  = ScFilterDlgWrapper::GetChildWindowId();
					SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
					SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

					pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
				}
			}
			break;

		case SID_SPECIAL_FILTER:
			{
				const SfxItemSet* pArgs = rReq.GetArgs();
				if ( pArgs )
				{
					DBG_ERROR("SID_SPECIAL_FILTER with arguments?");
					pTabViewShell->Query( ((const ScQueryItem&)
							pArgs->Get( SCITEM_QUERYDATA )).GetQueryData(), NULL, sal_True );
					rReq.Done();
				}
				else
				{
					sal_uInt16			nId  = ScSpecialFilterDlgWrapper::GetChildWindowId();
					SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
					SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

					pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
				}
			}
			break;

		case FID_FILTER_OK:
			{
                const SfxPoolItem* pItem;
                if ( pReqArgs && SFX_ITEM_SET ==
                        pReqArgs->GetItemState( SCITEM_QUERYDATA, sal_True, &pItem ) )
                {
                    const ScQueryItem& rQueryItem = static_cast<const ScQueryItem&>(*pItem);

                    SCTAB nCurTab = GetViewData()->GetTabNo();
                    SCTAB nRefTab = GetViewData()->GetRefTabNo();

                    // If RefInput switched to a different sheet from the data sheet,
                    // switch back:

                    if ( nCurTab != nRefTab )
                    {
                        pTabViewShell->SetTabNo( nRefTab );
                        pTabViewShell->PaintExtras();
                    }

                    ScRange aAdvSource;
                    if (rQueryItem.GetAdvancedQuerySource(aAdvSource))
                        pTabViewShell->Query( rQueryItem.GetQueryData(), &aAdvSource, sal_True );
                    else
                        pTabViewShell->Query( rQueryItem.GetQueryData(), NULL, sal_True );
                    rReq.Done( *pReqArgs );
                }
			}
			break;

		case SID_UNFILTER:
			{
				ScQueryParam aParam;
				ScDBData*	 pDBData = pTabViewShell->GetDBData();

				pDBData->GetQueryParam( aParam );
				SCSIZE nEC = aParam.GetEntryCount();
				for (SCSIZE i=0; i<nEC; i++)
					aParam.GetEntry(i).bDoQuery = sal_False;
				aParam.bDuplicate = sal_True;
				pTabViewShell->Query( aParam, NULL, sal_True );
				rReq.Done();
			}
			break;

		case SID_AUTO_FILTER:
			pTabViewShell->ToggleAutoFilter();
			rReq.Done();
			break;

		case SID_AUTOFILTER_HIDE:
			pTabViewShell->HideAutoFilter();
			rReq.Done();
			break;

		case SID_PIVOT_TABLE:
			{
				const SfxPoolItem* pItem;
				if ( pReqArgs && SFX_ITEM_SET ==
						pReqArgs->GetItemState( SCITEM_PIVOTDATA, sal_True, &pItem ) )
				{
					SCTAB nCurTab = GetViewData()->GetTabNo();
					SCTAB nRefTab = GetViewData()->GetRefTabNo();

					// Wenn RefInput auf andere Tabelle als Datentabelle umgeschaltet
					// hat wieder zurueckschalten:

					if ( nCurTab != nRefTab )
					{
						pTabViewShell->SetTabNo( nRefTab );
						pTabViewShell->PaintExtras();
					}

					const ScDPObject* pDPObject = pTabViewShell->GetDialogDPObject();
					if ( pDPObject )
					{
						const ScPivotItem* pPItem = (const ScPivotItem*)pItem;
                        bool bSuccess = pTabViewShell->MakePivotTable(
                            pPItem->GetData(), pPItem->GetDestRange(), pPItem->IsNewSheet(), *pDPObject );
                        SfxBoolItem aRet(0, bSuccess);
                        rReq.SetReturnValue(aRet);
					}
					rReq.Done();
				}
				else if (rReq.IsAPI())
					SbxBase::SetError(SbxERR_BAD_PARAMETER);
			}
			break;

		case SID_OPENDLG_PIVOTTABLE:
			{
				ScViewData* pData = GetViewData();
				ScDocument* pDoc = pData->GetDocument();

				ScDPObject* pNewDPObject = NULL;

				// ScPivot is no longer used...
				ScDPObject* pDPObj = pDoc->GetDPAtCursor(
											pData->GetCurX(), pData->GetCurY(),
											pData->GetTabNo() );
				if ( pDPObj )	// on an existing table?
				{
					pNewDPObject = new ScDPObject( *pDPObj );
				}
				else	 		// create new table
				{
					//	select database range or data
					pTabViewShell->GetDBData( sal_True, SC_DB_OLD );
                    ScMarkData& rMark = GetViewData()->GetMarkData();
					if ( !rMark.IsMarked() && !rMark.IsMultiMarked() )
						pTabViewShell->MarkDataArea( sal_False );

					//	output to cursor position for non-sheet data
					ScAddress aDestPos( pData->GetCurX(), pData->GetCurY(),
											pData->GetTabNo() );

					//	first select type of source data

					sal_Bool bEnableExt = ScDPObject::HasRegisteredSources();
					//CHINA001 ScDataPilotSourceTypeDlg* pTypeDlg = new ScDataPilotSourceTypeDlg(
					//CHINA001 							pTabViewShell->GetDialogParent(), bEnableExt );

					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

					AbstractScDataPilotSourceTypeDlg* pTypeDlg = pFact->CreateScDataPilotSourceTypeDlg( pTabViewShell->GetDialogParent(), bEnableExt, RID_SCDLG_DAPITYPE );
					DBG_ASSERT(pTypeDlg, "Dialog create fail!");//CHINA001
					if ( pTypeDlg->Execute() == RET_OK )
					{
						if ( pTypeDlg->IsExternal() )
						{
							uno::Sequence<rtl::OUString> aSources = ScDPObject::GetRegisteredSources();
							//CHINA001 ScDataPilotServiceDlg* pServDlg = new ScDataPilotServiceDlg(
							//CHINA001 					pTabViewShell->GetDialogParent(), aSources );
							AbstractScDataPilotServiceDlg* pServDlg = pFact->CreateScDataPilotServiceDlg( pTabViewShell->GetDialogParent(), aSources, RID_SCDLG_DAPISERVICE );
							DBG_ASSERT(pServDlg, "Dialog create fail!");//CHINA001
							if ( pServDlg->Execute() == RET_OK )
							{
								ScDPServiceDesc aServDesc(
										pServDlg->GetServiceName(),
										pServDlg->GetParSource(),
										pServDlg->GetParName(),
										pServDlg->GetParUser(),
										pServDlg->GetParPass() );
								pNewDPObject = new ScDPObject( pDoc );
								pNewDPObject->SetServiceData( aServDesc );
							}
							delete pServDlg;
						}
						else if ( pTypeDlg->IsDatabase() )
						{
							//CHINA001 ScDataPilotDatabaseDlg* pDataDlg = new ScDataPilotDatabaseDlg(
							//CHINA001 								pTabViewShell->GetDialogParent() );
                            //ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
							DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

							AbstractScDataPilotDatabaseDlg* pDataDlg = pFact->CreateScDataPilotDatabaseDlg( pTabViewShell->GetDialogParent(), RID_SCDLG_DAPIDATA);
							DBG_ASSERT(pDataDlg, "Dialog create fail!");//CHINA001
							if ( pDataDlg->Execute() == RET_OK )
							{
								ScImportSourceDesc aImpDesc;
								pDataDlg->GetValues( aImpDesc );
								pNewDPObject = new ScDPObject( pDoc );
								pNewDPObject->SetImportDesc( aImpDesc );
							}
							delete pDataDlg;
						}
						else		// selection
						{
							//!	use database ranges (select before type dialog?)
							ScRange aRange;
                            ScMarkType eType = GetViewData()->GetSimpleArea(aRange);
                            if ( (eType & SC_MARK_SIMPLE) == SC_MARK_SIMPLE )
							{
                                // Shrink the range to the data area.
                                SCCOL nStartCol = aRange.aStart.Col(), nEndCol = aRange.aEnd.Col();
                                SCROW nStartRow = aRange.aStart.Row(), nEndRow = aRange.aEnd.Row();
                                if (pDoc->ShrinkToDataArea(aRange.aStart.Tab(), nStartCol, nStartRow, nEndCol, nEndRow))
                                {
                                    aRange.aStart.SetCol(nStartCol);
                                    aRange.aStart.SetRow(nStartRow);
                                    aRange.aEnd.SetCol(nEndCol);
                                    aRange.aEnd.SetRow(nEndRow);
                                    rMark.SetMarkArea(aRange);
                                    pTabViewShell->MarkRange(aRange);
                                }

								sal_Bool bOK = sal_True;
								if ( pDoc->HasSubTotalCells( aRange ) )
								{
									//	confirm selection if it contains SubTotal cells

									QueryBox aBox( pTabViewShell->GetDialogParent(),
													WinBits(WB_YES_NO | WB_DEF_YES),
													ScGlobal::GetRscString(STR_DATAPILOT_SUBTOTAL) );
									if (aBox.Execute() == RET_NO)
										bOK = sal_False;
								}
								if (bOK)
								{
									ScSheetSourceDesc aShtDesc;
									aShtDesc.aSourceRange = aRange;
									pNewDPObject = new ScDPObject( pDoc );
									pNewDPObject->SetSheetDesc( aShtDesc );

									//	output below source data
									if ( aRange.aEnd.Row()+2 <= MAXROW - 4 )
										aDestPos = ScAddress( aRange.aStart.Col(),
																aRange.aEnd.Row()+2,
																aRange.aStart.Tab() );
								}
							}
						}
					}
					delete pTypeDlg;

					if ( pNewDPObject )
						pNewDPObject->SetOutRange( aDestPos );

#if 0
					ScDBData*	pDBData = pTabViewShell->GetDBData();
					String		aErrMsg;

					pDBData->GetArea( nTab, nCol1, nRow1, nCol2, nRow2 );

					bAreaOk = sal_True;
					if ( nRow2-nRow1 < 1 )
					{
						// "mindestens eine Datenzeile"
						pTabViewShell->ErrorMessage(STR_PIVOT_INVALID_DBAREA);
						bAreaOk = sal_False;
					}
					else if (!pDBData->HasHeader())
					{
						if ( MessBox( pTabViewShell->GetDialogParent(), WinBits(WB_YES_NO | WB_DEF_YES),
								ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_0 ),		// "StarCalc"
								ScGlobal::GetRscString( STR_MSSG_MAKEAUTOFILTER_0 ) 	// Koepfe aus erster Zeile?
							).Execute() == RET_YES )
						{
							pDBData->SetHeader( sal_True ); 	//! Undo ??
						}
						else
							bAreaOk = sal_False;
					}
#endif
				}

				pTabViewShell->SetDialogDPObject( pNewDPObject );	// is copied
				if ( pNewDPObject )
				{
					//	start layout dialog

					sal_uInt16 nId  = ScPivotLayoutWrapper::GetChildWindowId();
					SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
					SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );
					pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );
				}
				delete pNewDPObject;
			}
			break;

		case SID_DEFINE_DBNAME:
			{

				sal_uInt16			nId  = ScDbNameDlgWrapper::GetChildWindowId();
				SfxViewFrame* pViewFrm = pTabViewShell->GetViewFrame();
				SfxChildWindow* pWnd = pViewFrm->GetChildWindow( nId );

				pScMod->SetRefDialog( nId, pWnd ? sal_False : sal_True );

			}
			break;

		case SID_SELECT_DB:
			{
				if ( pReqArgs )
				{
					const SfxStringItem* pItem =
						(const SfxStringItem*)&pReqArgs->Get( SID_SELECT_DB );

					if( pItem )
					{
						pTabViewShell->GotoDBArea( pItem->GetValue() );
						rReq.Done();
					}
					else
					{
						DBG_ERROR("NULL");
					}
				}
				else
				{
					ScDocument* 	pDoc   = GetViewData()->GetDocument();
					ScDBCollection* pDBCol = pDoc->GetDBCollection();

					if ( pDBCol )
					{
						List			aList;
						sal_uInt16			nDBCount = pDBCol->GetCount();
						ScDBData*		pDbData  = NULL;
						String*			pDBName  = NULL;

						for ( sal_uInt16 i=0; i < nDBCount; i++ )
						{
							pDbData = (ScDBData*)(pDBCol->At( i ));
							if ( pDbData )
							{
								pDBName = new String;
								pDbData->GetName( *pDBName );

								if ( !pDbData->IsInternalUnnamed() )
									aList.Insert( pDBName );
								else
									DELETEZ(pDBName);
							}
						}

						ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
						DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

						AbstractScSelEntryDlg* pDlg = pFact->CreateScSelEntryDlg( pTabViewShell->GetDialogParent(),
																				RID_SCDLG_SELECTDB,
																				String(ScResId(SCSTR_SELECTDB)),
																				String(ScResId(SCSTR_AREAS)),
																				aList,
																				RID_SCDLG_SELECTDB);
						DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
						if ( pDlg->Execute() == RET_OK )
						{
							String aName = pDlg->GetSelectEntry();
							pTabViewShell->GotoDBArea( aName );
							rReq.AppendItem( SfxStringItem( SID_SELECT_DB, aName ) );
							rReq.Done();
						}

						delete pDlg;

						void* pEntry = aList.First();
						while ( pEntry )
						{
							delete (String*) aList.Remove( pEntry );
							pEntry = aList.Next();
						}
					}
				}
			}
			break;

		case FID_VALIDATION:
			{
				const SfxPoolItem* pItem;
				const SfxItemSet* pArgs = rReq.GetArgs();
				if ( pArgs )
				{
					DBG_ERROR("spaeter...");
				}
				else
				{
                    //CHINA001 SfxItemSet aArgSet( GetPool(), ScTPValidationValue::GetRanges() );
					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001
					::GetTabPageRanges ScTPValidationValueGetRanges = pFact->GetTabPageRangesFunc(TP_VALIDATION_VALUES);
					DBG_ASSERT(ScTPValidationValueGetRanges, "TabPage create fail!");//CHINA001
					SfxItemSet aArgSet( GetPool(), (*ScTPValidationValueGetRanges)() );//CHINA001
					ScValidationMode eMode = SC_VALID_ANY;
					ScConditionMode eOper = SC_COND_EQUAL;
					String aExpr1, aExpr2;
					sal_Bool bBlank = sal_True;
                    sal_Int16 nListType = ValidListType::UNSORTED;
					sal_Bool bShowHelp = sal_False;
					String aHelpTitle, aHelpText;
					sal_Bool bShowError = sal_False;
					ScValidErrorStyle eErrStyle = SC_VALERR_STOP;
					String aErrTitle, aErrText;

					ScDocument* pDoc = GetViewData()->GetDocument();
					SCCOL nCurX = GetViewData()->GetCurX();
					SCROW nCurY = GetViewData()->GetCurY();
					SCTAB nTab = GetViewData()->GetTabNo();
					ScAddress aCursorPos( nCurX, nCurY, nTab );
					sal_uLong nIndex = ((SfxUInt32Item*)pDoc->GetAttr(
								nCurX, nCurY, nTab, ATTR_VALIDDATA ))->GetValue();
					if ( nIndex )
					{
						const ScValidationData*	pOldData = pDoc->GetValidationEntry( nIndex );
						if ( pOldData )
						{
							eMode = pOldData->GetDataMode();
							eOper = pOldData->GetOperation();
							sal_uLong nNumFmt = 0;
							if ( eMode == SC_VALID_DATE || eMode == SC_VALID_TIME )
							{
								short nType = ( eMode == SC_VALID_DATE ) ? NUMBERFORMAT_DATE
																		 : NUMBERFORMAT_TIME;
								nNumFmt = pDoc->GetFormatTable()->GetStandardFormat(
																	nType, ScGlobal::eLnge );
							}
							aExpr1 = pOldData->GetExpression( aCursorPos, 0, nNumFmt );
							aExpr2 = pOldData->GetExpression( aCursorPos, 1, nNumFmt );
							bBlank = pOldData->IsIgnoreBlank();
                            nListType = pOldData->GetListType();

							bShowHelp = pOldData->GetInput( aHelpTitle, aHelpText );
							bShowError = pOldData->GetErrMsg( aErrTitle, aErrText, eErrStyle );

                            aArgSet.Put( SfxAllEnumItem( FID_VALID_MODE,        sal::static_int_cast<sal_uInt16>(eMode) ) );
                            aArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE,    sal::static_int_cast<sal_uInt16>(eOper) ) );
							aArgSet.Put( SfxStringItem(	 FID_VALID_VALUE1,		aExpr1 ) );
							aArgSet.Put( SfxStringItem(	 FID_VALID_VALUE2,		aExpr2 ) );
							aArgSet.Put( SfxBoolItem(	 FID_VALID_BLANK,		bBlank ) );
                            aArgSet.Put( SfxInt16Item(   FID_VALID_LISTTYPE,    nListType ) );
							aArgSet.Put( SfxBoolItem(	 FID_VALID_SHOWHELP,	bShowHelp ) );
							aArgSet.Put( SfxStringItem(	 FID_VALID_HELPTITLE,	aHelpTitle ) );
							aArgSet.Put( SfxStringItem(	 FID_VALID_HELPTEXT,	aHelpText ) );
							aArgSet.Put( SfxBoolItem(	 FID_VALID_SHOWERR,		bShowError ) );
                            aArgSet.Put( SfxAllEnumItem( FID_VALID_ERRSTYLE,    sal::static_int_cast<sal_uInt16>(eErrStyle) ) );
							aArgSet.Put( SfxStringItem(  FID_VALID_ERRTITLE,	aErrTitle ) );
							aArgSet.Put( SfxStringItem(  FID_VALID_ERRTEXT,		aErrText ) );
						}
					}

					//CHINA001 ScValidationDlg* pDlg = new ScValidationDlg( NULL, &aArgSet );
					//CHINA001 ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					//CHINA001 DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001
					
					//<!--Modified by PengYunQuan for Validity Cell Range Picker
					//SfxAbstractTabDialog* pDlg = pFact->CreateScValidationDlg( NULL, &aArgSet, TAB_DLG_VALIDATION );
					SfxAbstractTabDialog* pDlg = pFact->CreateScValidationDlg( NULL, &aArgSet, TAB_DLG_VALIDATION, pTabViewShell );
					//-->Modified by PengYunQuan for Validity Cell Range Picker
					DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

					//<!--Modified by PengYunQuan for Validity Cell Range Picker
					//if ( pDlg->Execute() == RET_OK )
					short nResult = pDlg->Execute();
					pTabViewShell->SetTabNo( nTab );//When picking Cell Range ,other Tab may be switched. Need restore the correct tab
					if ( nResult == RET_OK )
					//-->Modified by PengYunQuan for Validity Cell Range Picker
					{
						const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();

						if ( pOutSet->GetItemState( FID_VALID_MODE, sal_True, &pItem ) == SFX_ITEM_SET )
							eMode = (ScValidationMode) ((const SfxAllEnumItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_CONDMODE, sal_True, &pItem ) == SFX_ITEM_SET )
							eOper = (ScConditionMode) ((const SfxAllEnumItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_VALUE1, sal_True, &pItem ) == SFX_ITEM_SET )
                        {
                            String aTemp1 = ((const SfxStringItem*)pItem)->GetValue();
                            if (eMode == SC_VALID_DATE || eMode == SC_VALID_TIME)
                            {
                                sal_uInt32 nNumIndex = 0;
                                double nVal;
                                if (pDoc->GetFormatTable()->IsNumberFormat(aTemp1, nNumIndex, nVal))
                                    aExpr1 =String( ::rtl::math::doubleToUString( nVal,
                                            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                            ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), sal_True));
                                else
                                    aExpr1 = aTemp1;
                            }
                            else
                                aExpr1 = aTemp1;
                        }
						if ( pOutSet->GetItemState( FID_VALID_VALUE2, sal_True, &pItem ) == SFX_ITEM_SET )
                        {
                            String aTemp2 = ((const SfxStringItem*)pItem)->GetValue();
                            if (eMode == SC_VALID_DATE || eMode == SC_VALID_TIME)
                            {
                                sal_uInt32 nNumIndex = 0;
                                double nVal;
                                if (pDoc->GetFormatTable()->IsNumberFormat(aTemp2, nNumIndex, nVal))
                                    aExpr2 =String( ::rtl::math::doubleToUString( nVal,
                                            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
                                            ScGlobal::pLocaleData->getNumDecimalSep().GetChar(0), sal_True));
                                else
                                    aExpr2 = aTemp2;
                            }
                            else
                                aExpr2 = aTemp2;
                        }

						if ( pOutSet->GetItemState( FID_VALID_BLANK, sal_True, &pItem ) == SFX_ITEM_SET )
							bBlank = ((const SfxBoolItem*)pItem)->GetValue();
                        if ( pOutSet->GetItemState( FID_VALID_LISTTYPE, sal_True, &pItem ) == SFX_ITEM_SET )
                            nListType = ((const SfxInt16Item*)pItem)->GetValue();

						if ( pOutSet->GetItemState( FID_VALID_SHOWHELP, sal_True, &pItem ) == SFX_ITEM_SET )
							bShowHelp = ((const SfxBoolItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_HELPTITLE, sal_True, &pItem ) == SFX_ITEM_SET )
							aHelpTitle = ((const SfxStringItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_HELPTEXT, sal_True, &pItem ) == SFX_ITEM_SET )
							aHelpText = ((const SfxStringItem*)pItem)->GetValue();

						if ( pOutSet->GetItemState( FID_VALID_SHOWERR, sal_True, &pItem ) == SFX_ITEM_SET )
							bShowError = ((const SfxBoolItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_ERRSTYLE, sal_True, &pItem ) == SFX_ITEM_SET )
							eErrStyle = (ScValidErrorStyle) ((const SfxAllEnumItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_ERRTITLE, sal_True, &pItem ) == SFX_ITEM_SET )
							aErrTitle = ((const SfxStringItem*)pItem)->GetValue();
						if ( pOutSet->GetItemState( FID_VALID_ERRTEXT, sal_True, &pItem ) == SFX_ITEM_SET )
							aErrText = ((const SfxStringItem*)pItem)->GetValue();

						ScValidationData aData( eMode, eOper, aExpr1, aExpr2, pDoc, aCursorPos );
						aData.SetIgnoreBlank( bBlank );
                        aData.SetListType( nListType );

						aData.SetInput(aHelpTitle, aHelpText);		// sets bShowInput to TRUE
						if (!bShowHelp)
							aData.ResetInput();						// reset only bShowInput

						aData.SetError(aErrTitle, aErrText, eErrStyle);	// sets bShowError to TRUE
						if (!bShowError)
							aData.ResetError();						// reset only bShowError

						pTabViewShell->SetValidation( aData );
						rReq.Done( *pOutSet );
					}
					//<!-- Modified by PengYunQuan for Validity Cell Range Picker
					//after end execute from !IsModalInputMode, it is safer to delay deleting
					//delete pDlg;
					Application::PostUserEvent( Link( pDlg, &DelayDeleteAbstractDialog ) );
					//--> Modified by PengYunQuan for Validity Cell Range Picker
				}
			}
			break;

        case SID_TEXT_TO_COLUMNS:
            {
                ScViewData* pData = GetViewData();
                DBG_ASSERT( pData, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pData is null!" );
                ScRange aRange;

                if ( lcl_GetTextToColumnsRange( pData, aRange ) )
                {
                    ScDocument* pDoc = pData->GetDocument();
                    DBG_ASSERT( pDoc, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDoc is null!" );

                    ScImportExport aExport( pDoc, aRange );
                    aExport.SetExportTextOptions( ScExportTextOptions( ScExportTextOptions::None, 0, false ) );

                    // #i87703# text to columns fails with tab separator
                    aExport.SetDelimiter( static_cast< sal_Unicode >( 0 ) );

                    SvMemoryStream aStream;
                    aStream.SetStreamCharSet( RTL_TEXTENCODING_UNICODE );
                    ScImportExport::SetNoEndianSwap( aStream );
                    aExport.ExportStream( aStream, String(), FORMAT_STRING );

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT( pFact, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pFact is null!" );
                    AbstractScImportAsciiDlg *pDlg = pFact->CreateScImportAsciiDlg(
                        NULL, String(), &aStream, RID_SCDLG_ASCII );
                    DBG_ASSERT( pDlg, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDlg is null!" );
                    pDlg->SetTextToColumnsMode();

                    if ( pDlg->Execute() == RET_OK )
                    {
                        ScDocShell* pDocSh = pData->GetDocShell();
                        DBG_ASSERT( pDocSh, "ScCellShell::ExecuteDB: SID_TEXT_TO_COLUMNS - pDocSh is null!" );

                        String aUndo = ScGlobal::GetRscString( STR_UNDO_TEXTTOCOLUMNS );
                        pDocSh->GetUndoManager()->EnterListAction( aUndo, aUndo );

                        ScImportExport aImport( pDoc, aRange.aStart );
                        ScAsciiOptions aOptions;
                        pDlg->GetOptions( aOptions );
                        aImport.SetExtOptions( aOptions );
                        aImport.SetApi( false );
                        aStream.Seek( 0 );
                        aImport.ImportStream( aStream, String(), FORMAT_STRING );

                        pDocSh->GetUndoManager()->LeaveListAction();
                    }
                    delete pDlg;
                }
            }
            break;
		}
}

void __EXPORT ScCellShell::GetDBState( SfxItemSet& rSet )
{
	ScTabViewShell*	pTabViewShell  	= GetViewData()->GetViewShell();
    ScViewData* pData       = GetViewData();
	ScDocShell*	pDocSh		= pData->GetDocShell();
	ScDocument* pDoc		= pDocSh->GetDocument();
	SCCOL		nPosX		= pData->GetCurX();
	SCROW		nPosY		= pData->GetCurY();
	SCTAB		nTab		= pData->GetTabNo();

    sal_Bool bAutoFilter = sal_False;
	sal_Bool bAutoFilterTested = sal_False;

	SfxWhichIter aIter(rSet);
	sal_uInt16 nWhich = aIter.FirstWhich();
	while (nWhich)
	{
		switch (nWhich)
		{
			case SID_REFRESH_DBAREA:
				{
					//	importierte Daten ohne Selektion
					//	oder Filter,Sortierung,Teilergebis (auch ohne Import)
					sal_Bool bOk = sal_False;
					ScDBData* pDBData = pTabViewShell->GetDBData(sal_False,SC_DB_OLD);
					if (pDBData && pDoc->GetChangeTrack() == NULL)
					{
						if ( pDBData->HasImportParam() )
							bOk = !pDBData->HasImportSelection();
						else
						{
							bOk = pDBData->HasQueryParam() ||
								  pDBData->HasSortParam() ||
								  pDBData->HasSubTotalParam();
						}
					}
					if (!bOk)
						rSet.DisableItem( nWhich );
				}
				break;

			case SID_FILTER:
			case SID_SPECIAL_FILTER:
				{
                    ScRange aDummy;
                    ScMarkType eMarkType = GetViewData()->GetSimpleArea( aDummy);
					if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
					{
						rSet.DisableItem( nWhich );
					}
				}
				break;


				//Bei Redlining und Multiselektion Disablen
			case SID_SORT_ASCENDING:
			case SID_SORT_DESCENDING:
			case SCITEM_SORTDATA:
			case SCITEM_SUBTDATA:
			case SID_OPENDLG_PIVOTTABLE:
				{
					//!	move ReadOnly check to idl flags

					if ( pDocSh->IsReadOnly() || pDoc->GetChangeTrack()!=NULL ||
							GetViewData()->IsMultiMarked() )
					{
						rSet.DisableItem( nWhich );
					}
				}
				break;

			case SID_REIMPORT_DATA:
				{
					//	nur importierte Daten ohne Selektion
					ScDBData* pDBData = pTabViewShell->GetDBData(sal_False,SC_DB_OLD);
					if (!pDBData || !pDBData->HasImportParam() || pDBData->HasImportSelection() ||
						pDoc->GetChangeTrack()!=NULL)
					{
						rSet.DisableItem( nWhich );
					}
				}
				break;

			case SID_VIEW_DATA_SOURCE_BROWSER:
				{
                    if (!SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SDATABASE))
                        rSet.Put(SfxVisibilityItem(nWhich, sal_False));
                    else
					    //	get state (BoolItem) from SfxViewFrame
					    pTabViewShell->GetViewFrame()->GetSlotState( nWhich, NULL, &rSet );
				}
				break;
			case SID_SBA_BRW_INSERT:
				{
					//	SBA will ein sal_Bool-Item, damit ueberhaupt enabled

					sal_Bool bEnable = sal_True;
					rSet.Put(SfxBoolItem(nWhich, bEnable));
				}
				break;

			case SID_AUTO_FILTER:
			case SID_AUTOFILTER_HIDE:
				{
					if (!bAutoFilterTested)
					{
						bAutoFilter = pDoc->HasAutoFilter( nPosX, nPosY, nTab );
						bAutoFilterTested = sal_True;
					}
					if ( nWhich == SID_AUTO_FILTER )
					{
                        ScRange aDummy;
                        ScMarkType eMarkType = GetViewData()->GetSimpleArea( aDummy);
                        if (eMarkType != SC_MARK_SIMPLE && eMarkType != SC_MARK_SIMPLE_FILTERED)
						{
							rSet.DisableItem( nWhich );
						}
						else if (pDoc->GetDPAtBlock(aDummy))
                        {
                            rSet.DisableItem( nWhich );
                        }
                        else
							rSet.Put( SfxBoolItem( nWhich, bAutoFilter ) );
					}
					else
						if (!bAutoFilter)
							rSet.DisableItem( nWhich );
				}
				break;

			case SID_UNFILTER:
				{
                    SCCOL nStartCol, nEndCol;
                    SCROW  nStartRow, nEndRow;
                    SCTAB  nStartTab, nEndTab;
                    sal_Bool bAnyQuery = sal_False;

                    sal_Bool bSelected = (GetViewData()->GetSimpleArea(
                                nStartCol, nStartRow, nStartTab, nEndCol, nEndRow, nEndTab )
                            == SC_MARK_SIMPLE);

					if ( bSelected )
					{
						if (nStartCol==nEndCol && nStartRow==nEndRow)
							bSelected = sal_False;
					}
					else
					{
						nStartCol = GetViewData()->GetCurX();
						nStartRow = GetViewData()->GetCurY();
						nStartTab = GetViewData()->GetTabNo();
					}

					ScDBData* pDBData = bSelected
								? pDoc->GetDBAtArea( nStartTab, nStartCol, nStartRow, nEndCol, nEndRow )
								: pDoc->GetDBAtCursor( nStartCol, nStartRow, nStartTab );

					if ( pDBData )
					{
						ScQueryParam aParam;
						pDBData->GetQueryParam( aParam );
						if ( aParam.GetEntry(0).bDoQuery )
							bAnyQuery = sal_True;
					}

					if ( !bAnyQuery )
						rSet.DisableItem( nWhich );
				}
				break;

            case SID_DEFINE_DBNAME:
                {
                    if ( pDocSh && pDocSh->IsDocShared() )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;

            case SID_TEXT_TO_COLUMNS:
                {
                    ScRange aRange;
                    if ( !lcl_GetTextToColumnsRange( pData, aRange ) )
                    {
                        rSet.DisableItem( nWhich );
                    }
                }
                break;
		}
		nWhich = aIter.NextWhich();
	}
}



