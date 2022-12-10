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
#ifdef _MSC_VER
#pragma optimize("",off)
#endif

//------------------------------------------------------------------



// INCLUDE ---------------------------------------------------------

#include <sfx2/sfxsids.hrc>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <unotools/transliterationwrapper.hxx>

#include "tablink.hxx"

#include "scextopt.hxx"
#include "table.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "globstr.hrc"
#include "undoblk.hxx"
#include "undotab.hxx"
#include "global.hxx"
#include "hints.hxx"
#include "cell.hxx"
#include "dociter.hxx"
#include "formula/opcode.hxx"

struct TableLink_Impl
{
    ScDocShell* m_pDocSh;
    Window*     m_pOldParent;
    Link        m_aEndEditLink;

    TableLink_Impl() : m_pDocSh( NULL ), m_pOldParent( NULL ) {}
};

TYPEINIT1(ScTableLink, ::sfx2::SvBaseLink);

//------------------------------------------------------------------------

ScTableLink::ScTableLink(ScDocShell* pDocSh, const String& rFile,
							const String& rFilter, const String& rOpt,
							sal_uLong nRefresh ):
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
	ScRefreshTimer( nRefresh ),
    pImpl( new TableLink_Impl ),
	aFileName(rFile),
	aFilterName(rFilter),
	aOptions(rOpt),
	bInCreate( sal_False ),
	bInEdit( sal_False ),
	bAddUndo( sal_True ),
	bDoPaint( sal_True )
{
    pImpl->m_pDocSh = pDocSh;
}

ScTableLink::ScTableLink(SfxObjectShell* pShell, const String& rFile,
							const String& rFilter, const String& rOpt,
							sal_uLong nRefresh ):
    ::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
	ScRefreshTimer( nRefresh ),
    pImpl( new TableLink_Impl ),
	aFileName(rFile),
	aFilterName(rFilter),
	aOptions(rOpt),
	bInCreate( sal_False ),
	bInEdit( sal_False ),
	bAddUndo( sal_True ),
	bDoPaint( sal_True )
{
    pImpl->m_pDocSh = static_cast< ScDocShell* >( pShell );
	SetRefreshHandler( LINK( this, ScTableLink, RefreshHdl ) );
    SetRefreshControl( pImpl->m_pDocSh->GetDocument()->GetRefreshTimerControlAddress() );
}

__EXPORT ScTableLink::~ScTableLink()
{
	// Verbindung aufheben

	StopRefreshTimer();
	String aEmpty;
    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
	SCTAB nCount = pDoc->GetTableCount();
	for (SCTAB nTab=0; nTab<nCount; nTab++)
		if (pDoc->IsLinked(nTab) && pDoc->GetLinkDoc(nTab)==aFileName)
			pDoc->SetLink( nTab, SC_LINK_NONE, aEmpty, aEmpty, aEmpty, aEmpty, 0 );
    delete pImpl;
}

void __EXPORT ScTableLink::Edit( Window* pParent, const Link& rEndEditHdl )
{
	//	DefModalDialogParent setzen, weil evtl. aus der DocShell beim ConvertFrom
	//	ein Optionen-Dialog kommt...

    pImpl->m_aEndEditLink = rEndEditHdl;
    pImpl->m_pOldParent = Application::GetDefDialogParent();
	if (pParent)
		Application::SetDefDialogParent(pParent);

	bInEdit = sal_True;
    SvBaseLink::Edit( pParent, LINK( this, ScTableLink, TableEndEditHdl ) );
}

void __EXPORT ScTableLink::DataChanged( const String&,
										const ::com::sun::star::uno::Any& )
{
    sfx2::LinkManager* pLinkManager=pImpl->m_pDocSh->GetDocument()->GetLinkManager();
	if (pLinkManager!=NULL)
	{
		String aFile;
		String aFilter;
		pLinkManager->GetDisplayNames( this,0,&aFile,NULL,&aFilter);

		//	the file dialog returns the filter name with the application prefix
		//	-> remove prefix
		ScDocumentLoader::RemoveAppPrefix( aFilter );

		if (!bInCreate)
			Refresh( aFile, aFilter, NULL, GetRefreshDelay() );	// don't load twice
	}
}

void __EXPORT ScTableLink::Closed()
{
	// Verknuepfung loeschen: Undo
    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
	sal_Bool bUndo (pDoc->IsUndoEnabled());

	if (bAddUndo && bUndo)
	{
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoRemoveLink( pImpl->m_pDocSh, aFileName ) );

		bAddUndo = sal_False;	// nur einmal
	}

	// Verbindung wird im dtor aufgehoben

	SvBaseLink::Closed();
}

sal_Bool ScTableLink::IsUsed() const
{
    return pImpl->m_pDocSh->GetDocument()->HasLink( aFileName, aFilterName, aOptions );
}

sal_Bool ScTableLink::Refresh(const String& rNewFile, const String& rNewFilter,
							const String* pNewOptions, sal_uLong nNewRefresh )
{
	//	Dokument laden

	if (!rNewFile.Len() || !rNewFilter.Len())
		return sal_False;

    String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pImpl->m_pDocSh ) );
	sal_Bool bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = pImpl->m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
	if (!pFilter)
		return sal_False;

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
	pDoc->SetInLinkUpdate( sal_True );

	sal_Bool bUndo(pDoc->IsUndoEnabled());

	//	wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
	if ( rNewFilter != aFilterName )
		aOptions.Erase();
	if ( pNewOptions )					// Optionen hart angegeben?
		aOptions = *pNewOptions;

	//	ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
	SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
	if ( aOptions.Len() )
		pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );

	SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, sal_False, pFilter, pSet);

	if ( bInEdit )								// only if using the edit dialog,
		pMed->UseInteractionHandler( sal_True );	// enable the filter options dialog

    // aRef->DoClose() will be called explicitly, but it is still more safe to use SfxObjectShellLock here
	ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
    SfxObjectShellLock aRef = pSrcShell;
	pSrcShell->DoLoad(pMed);

	// Optionen koennten gesetzt worden sein
	String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
	if (!aNewOpt.Len())
		aNewOpt = aOptions;

	//	Undo...

	ScDocument* pUndoDoc = NULL;
	sal_Bool bFirst = sal_True;
	if (bAddUndo && bUndo)
		pUndoDoc = new ScDocument( SCDOCMODE_UNDO );

	//	Tabellen kopieren

    ScDocShellModificator aModificator( *pImpl->m_pDocSh );

	sal_Bool bNotFound = sal_False;
	ScDocument* pSrcDoc = pSrcShell->GetDocument();

	//	#74835# from text filters that don't set the table name,
	//	use the one table regardless of link table name
	sal_Bool bAutoTab = (pSrcDoc->GetTableCount() == 1) &&
					ScDocShell::HasAutomaticTableName( rNewFilter );

	SCTAB nCount = pDoc->GetTableCount();
	for (SCTAB nTab=0; nTab<nCount; nTab++)
	{
		sal_uInt8 nMode = pDoc->GetLinkMode(nTab);
		if (nMode && pDoc->GetLinkDoc(nTab)==aFileName)
		{
			String aTabName = pDoc->GetLinkTab(nTab);

			//	Undo

			if (bAddUndo && bUndo)
			{
				if (bFirst)
					pUndoDoc->InitUndo( pDoc, nTab, nTab, sal_True, sal_True );
				else
					pUndoDoc->AddUndoTab( nTab, nTab, sal_True, sal_True );
				bFirst = sal_False;
				ScRange aRange(0,0,nTab,MAXCOL,MAXROW,nTab);
				pDoc->CopyToDocument(aRange, IDF_ALL, sal_False, pUndoDoc);
				pUndoDoc->TransferDrawPage( pDoc, nTab, nTab );
				pUndoDoc->SetLink( nTab, nMode, aFileName, aFilterName,
					aOptions, aTabName, GetRefreshDelay() );
				pUndoDoc->SetTabBgColor( nTab, pDoc->GetTabBgColor(nTab) );
			}

			//	Tabellenname einer ExtDocRef anpassen

			if ( bNewUrlName && nMode == SC_LINK_VALUE )
			{
				String aName;
				pDoc->GetName( nTab, aName );
                if ( ScGlobal::GetpTransliteration()->isEqual(
                        ScGlobal::GetDocTabName( aFileName, aTabName ), aName ) )
				{
					pDoc->RenameTab( nTab,
						ScGlobal::GetDocTabName( aNewUrl, aTabName ),
						sal_False, sal_True );	// kein RefUpdate, kein ValidTabName
				}
			}

			//	kopieren

			SCTAB nSrcTab = 0;
            bool bFound = false;
            /*  #i71497# check if external document is loaded successfully,
                otherwise we may find the empty default sheet "Sheet1" in
                pSrcDoc, even if the document does not exist. */
            if( pMed->GetError() == 0 )
            {
                // no sheet name -> use first sheet
                if ( aTabName.Len() && !bAutoTab )
                    bFound = pSrcDoc->GetTable( aTabName, nSrcTab );
                else
                    bFound = true;
            }

			if (bFound)
				pDoc->TransferTab( pSrcDoc, nSrcTab, nTab, sal_False,		// nicht neu einfuegen
										(nMode == SC_LINK_VALUE) );		// nur Werte?
			else
			{
				pDoc->DeleteAreaTab( 0,0,MAXCOL,MAXROW, nTab, IDF_ALL );

                bool bShowError = true;
                if ( nMode == SC_LINK_VALUE )
                {
                    //  #139464# Value link (used with external references in formulas):
                    //  Look for formulas that reference the sheet, and put errors in the referenced cells.

                    ScRangeList aErrorCells;        // cells on the linked sheets that need error values

                    ScCellIterator aCellIter( pDoc, 0,0,0, MAXCOL,MAXROW,MAXTAB );			// all sheets
                    ScBaseCell* pCell = aCellIter.GetFirst();
                    while (pCell)
                    {
                        if (pCell->GetCellType() == CELLTYPE_FORMULA)
                        {
                            ScFormulaCell* pFCell = static_cast<ScFormulaCell*>(pCell);

                            ScDetectiveRefIter aRefIter( pFCell );
                            ScRange aRefRange;
                            while ( aRefIter.GetNextRef( aRefRange ) )
                            {
                                if ( aRefRange.aStart.Tab() <= nTab && aRefRange.aEnd.Tab() >= nTab )
                                {
                                    // use first cell of range references (don't fill potentially large ranges)

                                    aErrorCells.Join( ScRange( aRefRange.aStart ) );
                                }
                            }
                        }
                        pCell = aCellIter.GetNext();
                    }

                    sal_uLong nRanges = aErrorCells.Count();
                    if ( nRanges )                          // found any?
                    {
                        ScTokenArray aTokenArr;
                        aTokenArr.AddOpCode( ocNotAvail );
                        aTokenArr.AddOpCode( ocOpen );
                        aTokenArr.AddOpCode( ocClose );
                        aTokenArr.AddOpCode( ocStop );

                        for (sal_uLong nPos=0; nPos<nRanges; nPos++)
                        {
                            const ScRange* pRange = aErrorCells.GetObject(nPos);
                            SCCOL nStartCol = pRange->aStart.Col();
                            SCROW nStartRow = pRange->aStart.Row();
                            SCCOL nEndCol = pRange->aEnd.Col();
                            SCROW nEndRow = pRange->aEnd.Row();
                            for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
                                for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
                                {
                                    ScAddress aDestPos( nCol, nRow, nTab );
                                    ScFormulaCell* pNewCell = new ScFormulaCell( pDoc, aDestPos, &aTokenArr );
                                    pDoc->PutCell( aDestPos, pNewCell );
                                }
                        }

                        bShowError = false;
                    }
                    // if no references were found, insert error message (don't leave the sheet empty)
                }

                if ( bShowError )
                {
                    //  Normal link or no references: put error message on sheet.

                    pDoc->SetString( 0,0,nTab, ScGlobal::GetRscString(STR_LINKERROR) );
                    pDoc->SetString( 0,1,nTab, ScGlobal::GetRscString(STR_LINKERRORFILE) );
                    pDoc->SetString( 1,1,nTab, aNewUrl );
                    pDoc->SetString( 0,2,nTab, ScGlobal::GetRscString(STR_LINKERRORTAB) );
                    pDoc->SetString( 1,2,nTab, aTabName );
                }

				bNotFound = sal_True;
			}

			if ( bNewUrlName || rNewFilter != aFilterName ||
					aNewOpt != aOptions || pNewOptions ||
					nNewRefresh != GetRefreshDelay() )
				pDoc->SetLink( nTab, nMode, aNewUrl, rNewFilter, aNewOpt,
					aTabName, nNewRefresh );
		}
	}

	//	neue Einstellungen merken

	if ( bNewUrlName )
		aFileName = aNewUrl;
	if ( rNewFilter != aFilterName )
		aFilterName = rNewFilter;
	if ( aNewOpt != aOptions )
		aOptions = aNewOpt;

	//	aufraeumen

//	pSrcShell->DoClose();
	aRef->DoClose();

	//	Undo

	if (bAddUndo && bUndo)
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                    new ScUndoRefreshLink( pImpl->m_pDocSh, pUndoDoc ) );

	//	Paint (koennen mehrere Tabellen sein)

	if (bDoPaint)
	{
        pImpl->m_pDocSh->PostPaint( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB),
								PAINT_GRID | PAINT_TOP | PAINT_LEFT | PAINT_EXTRAS );
		aModificator.SetDocumentModified();
	}

	if (bNotFound)
	{
		//!	Fehler ausgeben ?
	}

	pDoc->SetInLinkUpdate( sal_False );

	//	notify Uno objects (for XRefreshListener)
	//!	also notify Uno objects if file name was changed!
	ScLinkRefreshedHint aHint;
	aHint.SetSheetLink( aFileName );
	pDoc->BroadcastUno( aHint );

	return sal_True;
}

IMPL_LINK( ScTableLink, RefreshHdl, ScTableLink*, EMPTYARG )
{
    long nRes = Refresh( aFileName, aFilterName, NULL, GetRefreshDelay() ) != 0;
    return nRes;
}

IMPL_LINK( ScTableLink, TableEndEditHdl, ::sfx2::SvBaseLink*, pLink )
{
    if ( pImpl->m_aEndEditLink.IsSet() )
        pImpl->m_aEndEditLink.Call( pLink );
    bInEdit = sal_False;
    Application::SetDefDialogParent( pImpl->m_pOldParent );
    return 0;
}

// === ScDocumentLoader ==================================================

String ScDocumentLoader::GetOptions( SfxMedium& rMedium )		// static
{
	SfxItemSet* pSet = rMedium.GetItemSet();
	const SfxPoolItem* pItem;
	if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_FILE_FILTEROPTIONS, sal_True, &pItem ) )
		return ((const SfxStringItem*)pItem)->GetValue();

	return EMPTY_STRING;
}

sal_Bool ScDocumentLoader::GetFilterName( const String& rFileName,
                                    String& rFilter, String& rOptions,
                                    sal_Bool bWithContent, sal_Bool bWithInteraction )  // static
{
	TypeId aScType = TYPE(ScDocShell);
	SfxObjectShell* pDocSh = SfxObjectShell::GetFirst( &aScType );
	while ( pDocSh )
	{
		if ( pDocSh->HasName() )
		{
			SfxMedium* pMed = pDocSh->GetMedium();
			if ( rFileName == pMed->GetName() )
			{
				rFilter = pMed->GetFilter()->GetFilterName();
				rOptions = GetOptions(*pMed);
				return sal_True;
			}
		}
		pDocSh = SfxObjectShell::GetNext( *pDocSh, &aScType );
	}

    INetURLObject aUrl( rFileName );
    INetProtocol eProt = aUrl.GetProtocol();
    if ( eProt == INET_PROT_NOT_VALID )         // invalid URL?
        return sal_False;                           // abort without creating a medium

	//	Filter-Detection

	const SfxFilter* pSfxFilter = NULL;
	SfxMedium* pMedium = new SfxMedium( rFileName, STREAM_STD_READ, sal_False );
	if ( pMedium->GetError() == ERRCODE_NONE )
	{
        if ( bWithInteraction )
            pMedium->UseInteractionHandler(sal_True);   // #i73992# no longer called from GuessFilter

		SfxFilterMatcher aMatcher( String::CreateFromAscii("scalc") );
        if( bWithContent )
            aMatcher.GuessFilter( *pMedium, &pSfxFilter );
        else
            aMatcher.GuessFilterIgnoringContent( *pMedium, &pSfxFilter );
	}

    sal_Bool bOK = sal_False;
	if ( pMedium->GetError() == ERRCODE_NONE )
    {
        if ( pSfxFilter )
            rFilter = pSfxFilter->GetFilterName();
        else
            rFilter = ScDocShell::GetOwnFilterName();		//	sonst Calc-Datei
        bOK = (rFilter.Len()>0);
    }

	delete pMedium;
    return bOK;
}

void ScDocumentLoader::RemoveAppPrefix( String& rFilterName )		// static
{
	String aAppPrefix = String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM( STRING_SCAPP ));
	aAppPrefix.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
	xub_StrLen nPreLen = aAppPrefix.Len();
	if ( rFilterName.Copy(0,nPreLen) == aAppPrefix )
		rFilterName.Erase(0,nPreLen);
}

ScDocumentLoader::ScDocumentLoader( const String& rFileName,
									String& rFilterName, String& rOptions,
									sal_uInt32 nRekCnt, sal_Bool bWithInteraction ) :
        pDocShell(0),
		pMedium(0)
{
	if ( !rFilterName.Len() )
        GetFilterName( rFileName, rFilterName, rOptions, sal_True, bWithInteraction );

	const SfxFilter* pFilter = ScDocShell::Factory().GetFilterContainer()->GetFilter4FilterName( rFilterName );

	//	ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
	SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
	if ( rOptions.Len() )
		pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, rOptions ) );

	pMedium = new SfxMedium( rFileName, STREAM_STD_READ, sal_False, pFilter, pSet );
	if ( pMedium->GetError() != ERRCODE_NONE )
		return ;

	if ( bWithInteraction )
		pMedium->UseInteractionHandler( sal_True );	// to enable the filter options dialog

    pDocShell = new ScDocShell( SFX_CREATE_MODE_INTERNAL );
    aRef = pDocShell;

    ScDocument* pDoc = pDocShell->GetDocument();
	if( pDoc )
	{
		ScExtDocOptions*	pExtDocOpt = pDoc->GetExtDocOptions();
		if( !pExtDocOpt )
		{
			pExtDocOpt = new ScExtDocOptions;
			pDoc->SetExtDocOptions( pExtDocOpt );
		}
        pExtDocOpt->GetDocSettings().mnLinkCnt = nRekCnt;
	}

    pDocShell->DoLoad( pMedium );

	String aNew = GetOptions(*pMedium);			// Optionen werden beim Laden per Dialog gesetzt
	if (aNew.Len() && aNew != rOptions)
		rOptions = aNew;
}

ScDocumentLoader::~ScDocumentLoader()
{
/*  if ( pDocShell )
        pDocShell->DoClose();
*/
	if ( aRef.Is() )
		aRef->DoClose();
	else if ( pMedium )
		delete pMedium;
}

void ScDocumentLoader::ReleaseDocRef()
{
	if ( aRef.Is() )
	{
		//	release reference without calling DoClose - caller must
		//	have another reference to the doc and call DoClose later

        pDocShell = NULL;
		pMedium = NULL;
		aRef.Clear();
	}
}

ScDocument* ScDocumentLoader::GetDocument()
{
    return pDocShell ? pDocShell->GetDocument() : 0;
}

sal_Bool ScDocumentLoader::IsError() const
{
    if ( pDocShell && pMedium )
		return pMedium->GetError() != ERRCODE_NONE;
	else
		return sal_True;
}

String ScDocumentLoader::GetTitle() const
{
    if ( pDocShell )
        return pDocShell->GetTitle();
	else
		return EMPTY_STRING;
}

