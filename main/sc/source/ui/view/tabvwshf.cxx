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

#include <boost/scoped_ptr.hpp>

#include "scitems.hxx"
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <basic/sbstar.hxx>
#include <layout/layout.hxx>
#include <svl/languageoptions.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/objface.hxx>
#include <svx/svxdlg.hxx>
#include <editeng/colritem.hxx>

#include "tabvwsh.hxx"
#include "sc.hrc"
#include "docsh.hxx"
#include "document.hxx"
#include "shtabdlg.hxx"
#include "scresid.hxx"
//CHINA001 #include "instbdlg.hxx"
#include "globstr.hrc"
//CHINA001 #include "strindlg.hxx"
//CHINA001 #include "mvtabdlg.hxx"
#include "docfunc.hxx"
#include "eventuno.hxx"

#include "scabstdlg.hxx" //CHINA001

#include "tabbgcolor.hxx"
#include "tabbgcolordlg.hxx"
#include "sccommands.h"

using ::boost::scoped_ptr;
using namespace com::sun::star;

#define IS_AVAILABLE(WhichId,ppItem) \
    (pReqArgs->GetItemState((WhichId), sal_True, ppItem ) == SFX_ITEM_SET)

//------------------------------------------------------------------

void ScTabViewShell::ExecuteTable( SfxRequest& rReq )
{
	ScViewData*	pViewData	= GetViewData();
	ScDocument*	pDoc		= pViewData->GetDocument();

	SCTAB		nCurrentTab	= pViewData->GetTabNo();
	SCTAB		nTabCount	= pDoc->GetTableCount();
	sal_uInt16		nSlot		= rReq.GetSlot();
	const SfxItemSet* pReqArgs = rReq.GetArgs();

	HideListBox();					// Autofilter-DropDown-Listbox

	switch ( nSlot )
	{
		case FID_TABLE_VISIBLE:
			{
				String aName;
				pDoc->GetName( nCurrentTab, aName );

				sal_Bool bVisible=sal_True;
				if( pReqArgs != NULL )
				{
					const SfxPoolItem* pItem;
					if( IS_AVAILABLE( FID_TABLE_VISIBLE, &pItem ) )
						bVisible = ((const SfxBoolItem*)pItem)->GetValue();
				}

				if( ! bVisible )			// ausblenden
				{
					ScMarkData& rMark = pViewData->GetMarkData();
					SCTAB nTabSelCount = rMark.GetSelectCount();
					sal_uInt16 nVis = 0;
					for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
						if (pDoc->IsVisible(i))
							++nVis;
					if ( nVis<2 || !pDoc->IsDocEditable() || nTabSelCount > 1 )
						break;

					SCTAB nHideTab;
					if (pDoc->GetTable( aName, nHideTab ))
						HideTable( nHideTab );
				}
				else						// einblenden
				{
					ShowTable( aName );
				}
			}
			break;

		case FID_TABLE_HIDE:
			{
				ScMarkData& rMark = pViewData->GetMarkData();
				SCTAB nTabSelCount = rMark.GetSelectCount();
				sal_uInt16 nVis = 0;
				for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
					if (pDoc->IsVisible(i))
						++nVis;
				if ( nVis<2 || !pDoc->IsDocEditable() || nTabSelCount > 1 )
					break;


				String aName;
				if( pReqArgs != NULL )
				{
					const SfxPoolItem* pItem;
					if( IS_AVAILABLE( FID_TABLE_HIDE, &pItem ) )
						aName = ((const SfxStringItem*)pItem)->GetValue();
				}

				if (!aName.Len())
				{
					pDoc->GetName( nCurrentTab, aName );		// aktuelle Tabelle
					rReq.AppendItem( SfxStringItem( FID_TABLE_HIDE, aName ) );
				}

				SCTAB nHideTab;
				if (pDoc->GetTable( aName, nHideTab ))
					HideTable( nHideTab );

				if( ! rReq.IsAPI() )
					rReq.Done();
			}
			break;

		case FID_TABLE_SHOW:
			{
				String aName;
				if ( pReqArgs )
				{
					const SfxPoolItem* pItem;
					if( IS_AVAILABLE( FID_TABLE_SHOW, &pItem ) )
					{
						aName = ((const SfxStringItem*)pItem)->GetValue();

						ShowTable( aName );

						if( ! rReq.IsAPI() )
							rReq.Done();
					}
				}
				else
				{
                    //CHINA001 ScShowTabDlg* pDlg = new ScShowTabDlg( GetDialogParent() );
					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

					AbstractScShowTabDlg* pDlg = pFact->CreateScShowTabDlg( GetDialogParent(), RID_SCDLG_SHOW_TAB);
					DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

                    String aTabName;
                    sal_Bool bFirst = sal_True;
					for ( SCTAB i=0; i != nTabCount; i++ )
					{
						if (!pDoc->IsVisible(i))
						{
                            pDoc->GetName( i, aTabName );
                            pDlg->Insert( aTabName, bFirst );
                            bFirst = sal_False;
						}
					}

					if ( pDlg->Execute() == RET_OK )
					{
						sal_uInt16 nCount = pDlg->GetSelectEntryCount();
						for (sal_uInt16 nPos=0; nPos<nCount; nPos++)
						{
							aName = pDlg->GetSelectEntry(nPos);
							ShowTable( aName );
						}
						rReq.AppendItem( SfxStringItem( FID_TABLE_SHOW, aName ) );
						rReq.Done();
					}
					delete pDlg;
				}
			}
			break;

		case FID_INS_TABLE:
        case FID_INS_TABLE_EXT:
			{
				ScMarkData& rMark 	 = pViewData->GetMarkData();
				SCTAB	nTabSelCount = rMark.GetSelectCount();
				SCTAB	nTabNr		 = nCurrentTab;

				if ( !pDoc->IsDocEditable() )
					break;							// gesperrt

				if ( pReqArgs != NULL )				// von Basic
				{
					sal_Bool bOk = sal_False;
					const SfxPoolItem*	pTabItem;
					const SfxPoolItem*	pNameItem;
					String				aName;

					if ( IS_AVAILABLE( FN_PARAM_1, &pTabItem ) &&
						 IS_AVAILABLE( nSlot, &pNameItem ) )
					{
						//	Tabellennr. von Basic: 1-basiert

						aName = ((const SfxStringItem*)pNameItem)->GetValue();
						nTabNr = ((const SfxUInt16Item*)pTabItem)->GetValue() - 1;
						if ( nTabNr < nTabCount )
							bOk = InsertTable( aName, nTabNr );
					}

					if (bOk)
						rReq.Done( *pReqArgs );
					//!	sonst Fehler setzen
				}
				else								// Dialog
				{
//CHINA001					ScInsertTableDlg* pDlg = new ScInsertTableDlg(
//CHINA001					GetDialogParent(),
//CHINA001					*pViewData,nTabSelCount);
					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

					AbstractScInsertTableDlg* pDlg = pFact->CreateScInsertTableDlg( GetDialogParent(), *pViewData,
                                                                                    nTabSelCount, nSlot == FID_INS_TABLE_EXT,
																					RID_SCDLG_INSERT_TABLE);
					DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
					if ( RET_OK == pDlg->Execute() )
					{
						if (pDlg->GetTablesFromFile())
						{
							SCTAB nTabs[MAXTABCOUNT];
							SCTAB nCount = 0;
							sal_uInt16 n = 0;
							const String* pStr = pDlg->GetFirstTable( &n );
							while ( pStr )
							{
								nTabs[nCount++] = static_cast<SCTAB>(n);
								pStr = pDlg->GetNextTable( &n );
							}
							sal_Bool bLink = pDlg->GetTablesAsLink();
							if (nCount != 0)
							{
								if(pDlg->IsTableBefore())
								{
									ImportTables( pDlg->GetDocShellTables(), nCount, nTabs,
												bLink,nTabNr );
								}
								else
								{
									SCTAB	nTabAfter	 = nTabNr+1;

									for(SCTAB j=nCurrentTab+1;j<nTabCount;j++)
									{
										if(!pDoc->IsScenario(j))
										{
											nTabAfter=j;
											break;
										}
									}

									ImportTables( pDlg->GetDocShellTables(), nCount, nTabs,
												bLink,nTabAfter );
								}
							}
						}
						else
						{
							SCTAB nCount=pDlg->GetTableCount();
							if(pDlg->IsTableBefore())
							{
								if(nCount==1 && pDlg->GetFirstTable()->Len()>0)
								{
									rReq.AppendItem( SfxStringItem( FID_INS_TABLE, *pDlg->GetFirstTable() ) );
									rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nTabNr) + 1 ) );		// 1-based
									rReq.Done();

									InsertTable( *pDlg->GetFirstTable(), nTabNr );
								}
								else
									InsertTables( NULL, nTabNr,nCount );
							}
							else
							{
								SCTAB	nTabAfter	 = nTabNr+1;
								SCTAB nSelHigh=0;

								for(SCTAB i=0;i<nTabCount;i++)
								{
									if(rMark.GetTableSelect(i))
									{
										nSelHigh=i;
									}
								}

								for(SCTAB j=nSelHigh+1;j<nTabCount;j++)
								{
									if(!pDoc->IsScenario(j))
									{
										nTabAfter=j;
										break;
									}
                                    else // #101672#; increase nTabAfter, because it is possible that the scenario tables are the last
                                        nTabAfter = j + 1;
								}

								if(nCount==1 && pDlg->GetFirstTable()->Len()>0)
								{
									rReq.AppendItem( SfxStringItem( FID_INS_TABLE, *pDlg->GetFirstTable() ) );
									rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nTabAfter) + 1 ) );		// 1-based
									rReq.Done();

									InsertTable( *pDlg->GetFirstTable(), nTabAfter);
								}
								else
								{
									InsertTables( NULL, nTabAfter,nCount);
								}
							}
						}
					}

					delete pDlg;
				}
			}
			break;

		case FID_TAB_APPEND:
		case FID_TAB_RENAME:
		case FID_TAB_MENU_RENAME:
			{
				//	FID_TAB_MENU_RENAME - "umbenennen" im Menu
				//	FID_TAB_RENAME		- "Name"-Property fuer Basic
				//	Execute ist gleich, aber im GetState wird MENU_RENAME evtl. disabled

				if ( nSlot == FID_TAB_MENU_RENAME )
					nSlot = FID_TAB_RENAME;				// Execute ist gleich

				SCTAB nTabNr = pViewData->GetTabNo();
				ScMarkData& rMark = pViewData->GetMarkData();
				SCTAB nTabSelCount = rMark.GetSelectCount();

				if ( !pDoc->IsDocEditable() )
					break; // alles gesperrt

				if ( nSlot != FID_TAB_APPEND &&
						( pDoc->IsTabProtected( nTabNr ) || nTabSelCount > 1 ) )
					break; // kein Rename

#if 0
				//	ScSbxObject wird nicht mehr benutzt, stattdessen aus dem
				//	ScSbxTable::Notify die richtige Tabelle an der Basic-View eingestellt
				if( rReq.IsAPI() )
				{
					SbxObject* pObj = GetScSbxObject();
					ScSbxTable* pSbxTab = PTR_CAST( ScSbxTable, pObj );
					DBG_ASSERT( pSbxTab, "pSbxTab???" );

					if( pSbxTab )
						nTabNr = pSbxTab->GetTableNr();
				}
#endif

				if( pReqArgs != NULL )
				{
					sal_Bool		bDone	= sal_False;
					const SfxPoolItem* pItem;
					String		aName;

					if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
						nTabNr = ((const SfxUInt16Item*)pItem)->GetValue();

					if( IS_AVAILABLE( nSlot, &pItem ) )
						aName = ((const SfxStringItem*)pItem)->GetValue();

					switch ( nSlot )
					{
						case FID_TAB_APPEND:
							bDone = AppendTable( aName );
							break;
						case FID_TAB_RENAME:
							bDone = RenameTable( aName, nTabNr );
							break;
					}

					if( bDone )
					{
						rReq.Done( *pReqArgs );
					}
				}
				else
				{
					sal_uInt16		nRet	= RET_OK;
					sal_Bool		bDone	= sal_False;
					String		aErrMsg ( ScGlobal::GetRscString( STR_INVALIDTABNAME ) );
					String		aName;
					String		aDlgTitle;
                    const sal_Char* pHelpId = 0;

					switch ( nSlot )
					{
						case FID_TAB_APPEND:
							aDlgTitle = String(ScResId(SCSTR_APDTABLE));
							pDoc->CreateValidTabName( aName );
                            pHelpId = HID_SC_APPEND_NAME;
							break;

						case FID_TAB_RENAME:
							aDlgTitle = String(ScResId(SCSTR_RENAMETAB));
							pDoc->GetName( pViewData->GetTabNo(), aName );
                            pHelpId = HID_SC_RENAME_NAME;
							break;
					}

					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

					AbstractScStringInputDlg* pDlg = pFact->CreateScStringInputDlg( GetDialogParent(),
																					aDlgTitle,
																					String(ScResId(SCSTR_NAME)),
																					aName,
																					GetStaticInterface()->GetSlot(nSlot)->GetCommand(), pHelpId, RID_SCDLG_STRINPUT);
					DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

					while ( !bDone && nRet == RET_OK )
					{
						nRet = pDlg->Execute();

						if ( nRet == RET_OK )
						{
							pDlg->GetInputString( aName );


							switch ( nSlot )
							{
								case FID_TAB_APPEND:
									bDone = AppendTable( aName );
									break;
								case FID_TAB_RENAME:
									bDone = RenameTable( aName, nTabNr );
									break;
							}

							if ( bDone )
							{
								rReq.AppendItem( SfxStringItem( nSlot, aName ) );
								rReq.Done();
							}
							else
							{
								if( rReq.IsAPI() )
								{
									StarBASIC::Error( SbERR_SETPROP_FAILED ); // XXX Fehlerbehandlung???
								}
								else
								{
									nRet = ErrorBox( GetDialogParent(),
													 WinBits( WB_OK | WB_DEF_OK ),
													 aErrMsg
												   ).Execute();
								}
							}
						}
					}
					delete pDlg;
				}
			}
			break;

		case FID_TAB_MOVE:
			{
				if ( pDoc->GetChangeTrack() != NULL )
					break;		// bei aktiviertem ChangeTracking kein TabMove

				sal_Bool   bDoIt = sal_False;
				sal_uInt16 nDoc = 0;
				SCTAB nTab = pViewData->GetTabNo();
				sal_Bool   bCpy = sal_False;
				String aDocName;

				if( pReqArgs != NULL )
				{
					SCTAB nTableCount = pDoc->GetTableCount();
					const SfxPoolItem* pItem;

					if( IS_AVAILABLE( FID_TAB_MOVE, &pItem ) )
						aDocName = ((const SfxStringItem*)pItem)->GetValue();
					if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
					{
						//	Tabelle ist 1-basiert
						nTab = ((const SfxUInt16Item*)pItem)->GetValue() - 1;
						if ( nTab >= nTableCount )
							nTab = SC_TAB_APPEND;
					}
					if( IS_AVAILABLE( FN_PARAM_2, &pItem ) )
						bCpy = ((const SfxBoolItem*)pItem)->GetValue();

					if( aDocName.Len() )
					{
						SfxObjectShell* pSh		= SfxObjectShell::GetFirst();
						ScDocShell*		pScSh	= NULL;
						sal_uInt16			i=0;

						while ( pSh )
						{
							pScSh = PTR_CAST( ScDocShell, pSh );

							if( pScSh )
							{
								pScSh->GetTitle();

								if( pScSh->GetTitle() == aDocName )
								{
									nDoc = i;
									ScDocument* pDestDoc = pScSh->GetDocument();
									nTableCount = pDestDoc->GetTableCount();
									bDoIt = pDestDoc->IsDocEditable();
									break;
								}

								i++;		// nur die ScDocShell's zaehlen
							}
							pSh = SfxObjectShell::GetNext( *pSh );
						}
					}
					else // Kein Dokumentname -> neues Dokument
					{
						nDoc = SC_DOC_NEW;
						bDoIt = sal_True;
					}

					if ( bDoIt && nTab >= nTableCount )		// ggf. anhaengen
						nTab = SC_TAB_APPEND;
				}
				else
				{
					//CHINA001 ScMoveTableDlg* pDlg = new ScMoveTableDlg( GetDialogParent() );
					ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
					DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

					AbstractScMoveTableDlg* pDlg = pFact->CreateScMoveTableDlg( GetDialogParent(), RID_SCDLG_MOVETAB );
					DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001

					SCTAB nTableCount = pDoc->GetTableCount();
					ScMarkData& rMark		= GetViewData()->GetMarkData();
					SCTAB		nTabSelCount = rMark.GetSelectCount();


					if(nTableCount==nTabSelCount)
					{
						pDlg->SetCopyTable();
						pDlg->EnableCopyTable(sal_False);
					}
					if ( pDlg->Execute() == RET_OK )
					{
						nDoc = pDlg->GetSelectedDocument();
						nTab = pDlg->GetSelectedTable();
						bCpy = pDlg->GetCopyTable();
						bDoIt = sal_True;

						String aFoundDocName;
						if ( nDoc != SC_DOC_NEW )
						{
							ScDocShell* pSh = ScDocShell::GetShellByNum( nDoc );
							if (pSh)
							{
								aFoundDocName = pSh->GetTitle();
								if ( !pSh->GetDocument()->IsDocEditable() )
								{
									ErrorMessage(STR_READONLYERR);
									bDoIt = sal_False;
								}
							}
						}
						rReq.AppendItem( SfxStringItem( FID_TAB_MOVE, aFoundDocName ) );
						//	Tabelle ist 1-basiert, wenn nicht APPEND
						SCTAB nBasicTab = ( nTab <= MAXTAB ) ? (nTab+1) : nTab;
						rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, static_cast<sal_uInt16>(nBasicTab) ) );
						rReq.AppendItem( SfxBoolItem( FN_PARAM_2, bCpy ) );
					}
					delete pDlg;
				}

				if( bDoIt )
				{
					rReq.Done();		// aufzeichnen, solange das Dokument noch aktiv ist

					MoveTable( nDoc, nTab, bCpy );
				}
			}
			break;

		case FID_DELETE_TABLE:
			{
				//	Parameter war ueberfluessig, weil die Methode an der Table haengt

				sal_Bool bDoIt = rReq.IsAPI();
				if( !bDoIt )
				{
					//	wenn's nicht von Basic kommt, nochmal nachfragen:

#if ENABLE_LAYOUT
// Using layout::QueryBox without client code modification is
// deprecated, rather add HIG-complient buttons with verbs.
#define QueryBox( parent, winbits, question ) layout::QueryBox (parent, question, ScGlobal::GetRscString (STR_UNDO_DELETE_TAB))
#endif /* ENABLE_LAYOUT */

                        bDoIt = ( RET_YES ==
                                  QueryBox( GetDialogParent(),
                                            WinBits( WB_YES_NO | WB_DEF_YES ),
                                            ScGlobal::GetRscString(STR_QUERY_DELTAB)
                                      ).Execute() );
				}
				if( bDoIt )
				{
					SCTAB nNewTab	= nCurrentTab;
					SCTAB nFirstTab=0;
					sal_Bool   bTabFlag=sal_False;
					ScMarkData& rMark = pViewData->GetMarkData();
					SvShorts TheTabs;
					for(SCTAB i=0;i<nTabCount;i++)
					{
						if(rMark.GetTableSelect(i) &&!pDoc->IsTabProtected(i))
						{
							TheTabs.push_back(i);
							bTabFlag=sal_True;
							if(nNewTab==i) nNewTab++;
						}
						if(!bTabFlag) nFirstTab=i;
					}
					if(nNewTab>=nTabCount) nNewTab=nFirstTab;

					pViewData->SetTabNo(nNewTab);
					DeleteTables(TheTabs);
					TheTabs.clear();
					rReq.Done();
				}
			}
			break;

		case FID_TAB_RTL:
			{
				ScDocShell* pDocSh = pViewData->GetDocShell();
				ScDocFunc aFunc(*pDocSh);
				sal_Bool bSet = !pDoc->IsLayoutRTL( nCurrentTab );

				const ScMarkData& rMark = pViewData->GetMarkData();
				if ( rMark.GetSelectCount() != 0 )
				{
					//	handle several sheets

					::svl::IUndoManager* pUndoManager = pDocSh->GetUndoManager();
					String aUndo = ScGlobal::GetRscString( STR_UNDO_TAB_RTL );
					pUndoManager->EnterListAction( aUndo, aUndo );

					for (SCTAB nTab=0; nTab<nTabCount; nTab++)
						if ( rMark.GetTableSelect(nTab) )
							aFunc.SetLayoutRTL( nTab, bSet, sal_False );

					pUndoManager->LeaveListAction();
				}
				else
					aFunc.SetLayoutRTL( nCurrentTab, bSet, sal_False );
			}
			break;

        case FID_TAB_SET_TAB_BG_COLOR:
        case FID_TAB_MENU_SET_TAB_BG_COLOR:
            {
                if ( nSlot == FID_TAB_MENU_SET_TAB_BG_COLOR )
                    nSlot = FID_TAB_SET_TAB_BG_COLOR;
                SCTAB nTabNr = pViewData->GetTabNo();
                ScMarkData& rMark = pViewData->GetMarkData();
                SCTAB nTabSelCount = rMark.GetSelectCount();
                if ( !pDoc->IsDocEditable() )
                    break;

                if ( pDoc->IsTabProtected( nTabNr ) ) // ||nTabSelCount > 1
                    break;

                if( pReqArgs != NULL )
                {
                    sal_Bool                bDone = sal_False;
                    const SfxPoolItem*  pItem;
                    Color               aColor;
                    if( IS_AVAILABLE( FN_PARAM_1, &pItem ) )
                        nTabNr = ((const SfxUInt16Item*)pItem)->GetValue();

                    if( IS_AVAILABLE( nSlot, &pItem ) )
                        aColor = ((const SvxColorItem*)pItem)->GetValue();

                    if ( nTabSelCount > 1 )
                    {
                        scoped_ptr<ScUndoTabColorInfo::List>
                            pTabColorList(new ScUndoTabColorInfo::List);
                        for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                        {
                            if ( rMark.GetTableSelect(nTab) && !pDoc->IsTabProtected(nTab) )
                            {
                                ScUndoTabColorInfo aTabColorInfo(nTab);
                                aTabColorInfo.maNewTabBgColor = aColor;
                                pTabColorList->push_back(aTabColorInfo);
                            }
                        }
                        bDone = SetTabBgColor( *pTabColorList );
                    }
                    else
                    {
                        bDone = SetTabBgColor( aColor, nCurrentTab ); //ScViewFunc.SetTabBgColor
                    }
                    if( bDone )
                    {
                        rReq.Done( *pReqArgs );
                    }
                }
                else
                {
                    sal_uInt16      nRet    = RET_OK; /// temp
                    sal_Bool        bDone   = sal_False; /// temp
                    Color       aTabBgColor;
                    Color       aNewTabBgColor;

                    aTabBgColor = pDoc->GetTabBgColor( nCurrentTab );
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "ScAbstractFactory create fail!");
                    AbstractScTabBgColorDlg* pDlg = pFact->CreateScTabBgColorDlg(
                                                                GetDialogParent(),
                                                                String(ScResId(SCSTR_SET_TAB_BG_COLOR)),
                                                                String(ScResId(SCSTR_NO_TAB_BG_COLOR)),
                                                                aTabBgColor,
                                                                CMD_FID_TAB_SET_TAB_BG_COLOR,
                                                                RID_SCDLG_TAB_BG_COLOR);
                    while ( !bDone && nRet == RET_OK )
                    {
                        nRet = pDlg->Execute();
                        if( nRet == RET_OK )
                        {
                            Color aSelectedColor;
                            pDlg->GetSelectedColor(aSelectedColor);
                            scoped_ptr<ScUndoTabColorInfo::List>
                                pTabColorList(new ScUndoTabColorInfo::List);
                            if ( nTabSelCount > 1 )
                            {
                                for  (SCTAB nTab=0; nTab<nTabCount; nTab++)
                                {
                                    if ( rMark.GetTableSelect(nTab) && !pDoc->IsTabProtected(nTab) )
                                    {
                                        ScUndoTabColorInfo aTabColorInfo(nTab);
                                        aTabColorInfo.maNewTabBgColor = aSelectedColor;
                                        pTabColorList->push_back(aTabColorInfo);
                                    }
                                }
                                bDone = SetTabBgColor( *pTabColorList );
                            }
                            else
                            {
                                bDone = SetTabBgColor( aSelectedColor, nCurrentTab ); //ScViewFunc.SetTabBgColor
                            }
                            if ( bDone )
                            {
                                rReq.AppendItem( SvxColorItem( aTabBgColor, nSlot ) );
                                rReq.Done();
                            }
                            else
                            {
                                if( rReq.IsAPI() )
                                {
                                    StarBASIC::Error( SbERR_SETPROP_FAILED );
                                }
                            }
                        }
                    }
                    delete( pDlg );
                }
            }
            break;

        case FID_TAB_EVENTS:
            {
                ScDocShell* pDocSh = pViewData->GetDocShell();
                uno::Reference<container::XNameReplace> xEvents( new ScSheetEventsObj( pDocSh, nCurrentTab ) );
                uno::Reference<frame::XFrame> xFrame = GetViewFrame()->GetFrame().GetFrameInterface();
                SvxAbstractDialogFactory* pDlgFactory = SvxAbstractDialogFactory::Create();
                if (pDlgFactory)
                {
                    std::auto_ptr<VclAbstractDialog> pDialog( pDlgFactory->CreateSvxMacroAssignDlg(
                        GetDialogParent(), xFrame, false, xEvents, 0 ) );
                    if ( pDialog.get() && pDialog->Execute() == RET_OK )
                    {
                        // the dialog modifies the settings directly
                    }
                }
            }
            break;

		default:
			DBG_ERROR("Unbekannte Message bei ViewShell");
			break;
	}
}

//------------------------------------------------------------------

void ScTabViewShell::GetStateTable( SfxItemSet& rSet )
{
	ScViewData* pViewData	= GetViewData();
	ScDocument* pDoc		= pViewData->GetDocument();
    ScDocShell* pDocShell   = pViewData->GetDocShell();
	ScMarkData& rMark		= GetViewData()->GetMarkData();
	SCTAB		nTab		= pViewData->GetTabNo();

	SCTAB nTabCount = pDoc->GetTableCount();
	SCTAB nTabSelCount = rMark.GetSelectCount();

	SfxWhichIter	aIter(rSet);
	sal_uInt16			nWhich = aIter.FirstWhich();

	while ( nWhich )
	{
		switch ( nWhich )
		{

			case FID_TABLE_VISIBLE:
				rSet.Put( SfxBoolItem( nWhich, pDoc->IsVisible(nTab) ));
				break;

			case FID_TABLE_HIDE:
				{
					sal_uInt16 nVis = 0;
					for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
						if (pDoc->IsVisible(i))
							++nVis;

					if ( nVis<2 || !pDoc->IsDocEditable() || nTabSelCount > 1 )
						rSet.DisableItem( nWhich );
				}
				break;

			case FID_TABLE_SHOW:
				{
					sal_Bool bHasHidden = sal_False;
					for ( SCTAB i=0; i < nTabCount && !bHasHidden; i++ )
						if (!pDoc->IsVisible(i))
							bHasHidden = sal_True;
					if ( !bHasHidden || pDoc->IsDocProtected() || nTabSelCount > 1 )
						rSet.DisableItem( nWhich );
				}
				break;

			case FID_DELETE_TABLE:
				{
					if ( pDoc->GetChangeTrack() )
						rSet.DisableItem( nWhich );
					else
					{
						sal_uInt16 nVis = 0;
						for ( SCTAB i=0; i < nTabCount && nVis<2; i++ )
							if (pDoc->IsVisible(i))
								++nVis;
						if (   pDoc->IsTabProtected(nTab)
							|| !pDoc->IsDocEditable()
							|| nVis < 2
							|| nTabSelCount == nTabCount)
						rSet.DisableItem( nWhich );
					}
				}
				break;

			case FID_INS_TABLE:
            case FID_INS_TABLE_EXT:
			case FID_TAB_APPEND:
                if ( !pDoc->IsDocEditable() ||
                     nTabCount > MAXTAB ||
                     ( nWhich == FID_INS_TABLE_EXT && pDocShell && pDocShell->IsDocShared() ) )
					rSet.DisableItem( nWhich );
				break;

			case FID_TAB_MOVE:
				if (   !pDoc->IsDocEditable()
					|| pDoc->GetChangeTrack() != NULL
					|| nTabCount > MAXTAB)
					rSet.DisableItem( nWhich );
				break;

			//	FID_TAB_MENU_RENAME - "umbenennen" im Menu
			//	FID_TAB_RENAME		- "Name"-Property fuer Basic

			case FID_TAB_MENU_RENAME:
				if ( !pDoc->IsDocEditable() ||
					 pDoc->IsTabProtected(nTab) ||nTabSelCount > 1 ||
                     ( pDocShell && pDocShell->IsDocShared() ) )
					rSet.DisableItem( nWhich );
				break;

			case FID_TAB_RENAME:
				{
					String aTabName;
					pDoc->GetName( nTab, aTabName );

					rSet.Put( SfxStringItem( nWhich, aTabName ));

				}
				break;

			case FID_TAB_RTL:
				{
					SvtLanguageOptions aLangOpt;
					if ( !aLangOpt.IsCTLFontEnabled() )
						rSet.DisableItem( nWhich );
					else
						rSet.Put( SfxBoolItem( nWhich, pDoc->IsLayoutRTL( nTab ) ) );
				}
				break;

            case FID_TAB_MENU_SET_TAB_BG_COLOR:
                {
                    if ( !pDoc->IsDocEditable()
                        || ( pDocShell && pDocShell->IsDocShared() )
                        || pDoc->IsTabProtected(nTab) )
                        rSet.DisableItem( nWhich );
                }
                break;

            case FID_TAB_SET_TAB_BG_COLOR:
                {
                    Color aColor;
                    aColor = pDoc->GetTabBgColor( nTab );
                    rSet.Put( SvxColorItem( aColor, nWhich ) );
                }
                break;
		}
		nWhich = aIter.NextWhich();
	}
}




