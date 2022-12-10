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



// INCLUDE ---------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/linkmgr.hxx>
#include <svl/stritem.hxx>
#include <vcl/msgbox.hxx>

#include "arealink.hxx"

#include "tablink.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "rangenam.hxx"
#include "dbcolect.hxx"
#include "undoblk.hxx"
#include "globstr.hrc"
#include "markdata.hxx"
#include "hints.hxx"
#include "filter.hxx"
//CHINA001 #include "linkarea.hxx"			// dialog

#include "attrib.hxx"			// raus, wenn ResetAttrib am Dokument
#include "patattr.hxx"			// raus, wenn ResetAttrib am Dokument
#include "docpool.hxx"			// raus, wenn ResetAttrib am Dokument

#include "sc.hrc" //CHINA001
#include "scabstdlg.hxx" //CHINA001
#include "clipparam.hxx"

struct AreaLink_Impl
{
    ScDocShell* m_pDocSh;
    AbstractScLinkedAreaDlg* m_pDialog;

    AreaLink_Impl() : m_pDocSh( NULL ), m_pDialog( NULL ) {}
};

TYPEINIT1(ScAreaLink,::sfx2::SvBaseLink);

//------------------------------------------------------------------------

ScAreaLink::ScAreaLink( SfxObjectShell* pShell, const String& rFile,
						const String& rFilter, const String& rOpt,
						const String& rArea, const ScRange& rDest,
						sal_uLong nRefresh ) :
	::sfx2::SvBaseLink(sfx2::LINKUPDATE_ONCALL,FORMAT_FILE),
	ScRefreshTimer	( nRefresh ),
    pImpl           ( new AreaLink_Impl() ),
	aFileName		(rFile),
	aFilterName		(rFilter),
	aOptions		(rOpt),
	aSourceArea		(rArea),
	aDestArea		(rDest),
	bAddUndo		(sal_True),
	bInCreate		(sal_False),
	bDoInsert		(sal_True)
{
	DBG_ASSERT(pShell->ISA(ScDocShell), "ScAreaLink mit falscher ObjectShell");
    pImpl->m_pDocSh = static_cast< ScDocShell* >( pShell );
	SetRefreshHandler( LINK( this, ScAreaLink, RefreshHdl ) );
    SetRefreshControl( pImpl->m_pDocSh->GetDocument()->GetRefreshTimerControlAddress() );
}

__EXPORT ScAreaLink::~ScAreaLink()
{
	StopRefreshTimer();
    delete pImpl;
}

void __EXPORT ScAreaLink::Edit(Window* pParent, const Link& /* rEndEditHdl */ )
{
	//	use own dialog instead of SvBaseLink::Edit...
	//	DefModalDialogParent setzen, weil evtl. aus der DocShell beim ConvertFrom
	//	ein Optionen-Dialog kommt...

	ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
	DBG_ASSERT(pFact, "ScAbstractFactory create fail!");//CHINA001

	AbstractScLinkedAreaDlg* pDlg = pFact->CreateScLinkedAreaDlg( pParent, RID_SCDLG_LINKAREA);
	DBG_ASSERT(pDlg, "Dialog create fail!");//CHINA001
	pDlg->InitFromOldLink( aFileName, aFilterName, aOptions, aSourceArea, GetRefreshDelay() );
    pImpl->m_pDialog = pDlg;
    pDlg->StartExecuteModal( LINK( this, ScAreaLink, AreaEndEditHdl ) );
}

void __EXPORT ScAreaLink::DataChanged( const String&,
									   const ::com::sun::star::uno::Any& )
{
	//	bei bInCreate nichts tun, damit Update gerufen werden kann, um den Status im
	//	LinkManager zu setzen, ohne die Daten im Dokument zu aendern

	if (bInCreate)
		return;

    sfx2::LinkManager* pLinkManager=pImpl->m_pDocSh->GetDocument()->GetLinkManager();
	if (pLinkManager!=NULL)
	{
		String aFile;
		String aFilter;
		String aArea;
		pLinkManager->GetDisplayNames( this,0,&aFile,&aArea,&aFilter);

		//	the file dialog returns the filter name with the application prefix
		//	-> remove prefix
		ScDocumentLoader::RemoveAppPrefix( aFilter );

		// #81155# dialog doesn't set area, so keep old one
		if ( !aArea.Len() )
		{
			aArea = aSourceArea;

			// adjust in dialog:
            String aNewLinkName;
            sfx2::MakeLnkName( aNewLinkName, NULL, aFile, aArea, &aFilter );
            SetName( aNewLinkName );
		}

		Refresh( aFile, aFilter, aArea, GetRefreshDelay() );
	}
}

void __EXPORT ScAreaLink::Closed()
{
	// Verknuepfung loeschen: Undo

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();
	sal_Bool bUndo (pDoc->IsUndoEnabled());
	if (bAddUndo && bUndo)
	{
        pImpl->m_pDocSh->GetUndoManager()->AddUndoAction( new ScUndoRemoveAreaLink( pImpl->m_pDocSh,
														aFileName, aFilterName, aOptions,
														aSourceArea, aDestArea, GetRefreshDelay() ) );

		bAddUndo = sal_False;	// nur einmal
	}

    SCTAB nDestTab = aDestArea.aStart.Tab();
    if (pDoc->IsStreamValid(nDestTab))
        pDoc->SetStreamValid(nDestTab, sal_False);

	SvBaseLink::Closed();
}

void ScAreaLink::SetDestArea(const ScRange& rNew)
{
	aDestArea = rNew;			// fuer Undo
}

void ScAreaLink::SetSource(const String& rDoc, const String& rFlt, const String& rOpt,
								const String& rArea)
{
	aFileName	= rDoc;
	aFilterName	= rFlt;
	aOptions	= rOpt;
	aSourceArea	= rArea;

	//	also update link name for dialog
    String aNewLinkName;
    sfx2::MakeLnkName( aNewLinkName, NULL, aFileName, aSourceArea, &aFilterName );
    SetName( aNewLinkName );
}

sal_Bool ScAreaLink::IsEqual( const String& rFile, const String& rFilter, const String& rOpt,
							const String& rSource, const ScRange& rDest ) const
{
	return aFileName == rFile && aFilterName == rFilter && aOptions == rOpt &&
			aSourceArea == rSource && aDestArea.aStart == rDest.aStart;
}

// find a range with name >rAreaName< in >pSrcDoc<, return it in >rRange<
sal_Bool ScAreaLink::FindExtRange( ScRange& rRange, ScDocument* pSrcDoc, const String& rAreaName )
{
	sal_Bool bFound = sal_False;
	ScRangeName* pNames = pSrcDoc->GetRangeName();
	sal_uInt16 nPos;
	if (pNames)			// benannte Bereiche
	{
		if (pNames->SearchName( rAreaName, nPos ))
			if ( (*pNames)[nPos]->IsValidReference( rRange ) )
				bFound = sal_True;
	}
	if (!bFound)		// Datenbankbereiche
	{
		ScDBCollection*	pDBColl = pSrcDoc->GetDBCollection();
		if (pDBColl)
			if (pDBColl->SearchName( rAreaName, nPos ))
			{
                SCTAB nTab;
                SCCOL nCol1, nCol2;
                SCROW nRow1, nRow2;
                (*pDBColl)[nPos]->GetArea(nTab,nCol1,nRow1,nCol2,nRow2);
				rRange = ScRange( nCol1,nRow1,nTab, nCol2,nRow2,nTab );
				bFound = sal_True;
			}
	}
	if (!bFound)		// direct reference (range or cell)
	{
        ScAddress::Details aDetails(pSrcDoc->GetAddressConvention(), 0, 0);
		if ( rRange.ParseAny( rAreaName, pSrcDoc, aDetails ) & SCA_VALID )
			bFound = sal_True;
	}
	return bFound;
}

//	ausfuehren:

sal_Bool ScAreaLink::Refresh( const String& rNewFile, const String& rNewFilter,
							const String& rNewArea, sal_uLong nNewRefresh )
{
	//	Dokument laden - wie TabLink

	if (!rNewFile.Len() || !rNewFilter.Len())
		return sal_False;

    String aNewUrl( ScGlobal::GetAbsDocName( rNewFile, pImpl->m_pDocSh ) );
	sal_Bool bNewUrlName = (aNewUrl != aFileName);

    const SfxFilter* pFilter = pImpl->m_pDocSh->GetFactory().GetFilterContainer()->GetFilter4FilterName(rNewFilter);
	if (!pFilter)
		return sal_False;

    ScDocument* pDoc = pImpl->m_pDocSh->GetDocument();

	sal_Bool bUndo (pDoc->IsUndoEnabled());
	pDoc->SetInLinkUpdate( sal_True );

	//	wenn neuer Filter ausgewaehlt wurde, Optionen vergessen
	if ( rNewFilter != aFilterName )
		aOptions.Erase();

	//	ItemSet immer anlegen, damit die DocShell die Optionen setzen kann
	SfxItemSet* pSet = new SfxAllItemSet( SFX_APP()->GetPool() );
	if ( aOptions.Len() )
		pSet->Put( SfxStringItem( SID_FILE_FILTEROPTIONS, aOptions ) );

	SfxMedium* pMed = new SfxMedium(aNewUrl, STREAM_STD_READ, sal_False, pFilter);

    // aRef->DoClose() will be closed explicitly, but it is still more safe to use SfxObjectShellLock here
	ScDocShell* pSrcShell = new ScDocShell(SFX_CREATE_MODE_INTERNAL);
	SfxObjectShellLock aRef = pSrcShell;
	pSrcShell->DoLoad(pMed);

	ScDocument* pSrcDoc = pSrcShell->GetDocument();

	// Optionen koennten gesetzt worden sein
	String aNewOpt = ScDocumentLoader::GetOptions(*pMed);
	if (!aNewOpt.Len())
		aNewOpt = aOptions;

	// correct source range name list for web query import
	String aTempArea;

	if( rNewFilter == ScDocShell::GetWebQueryFilterName() )
		aTempArea = ScFormatFilter::Get().GetHTMLRangeNameList( pSrcDoc, rNewArea );
	else
		aTempArea = rNewArea;

	// find total size of source area
	SCCOL nWidth = 0;
	SCROW nHeight = 0;
	xub_StrLen nTokenCnt = aTempArea.GetTokenCount( ';' );
	xub_StrLen nStringIx = 0;
	xub_StrLen nToken;

	for( nToken = 0; nToken < nTokenCnt; nToken++ )
	{
		String aToken( aTempArea.GetToken( 0, ';', nStringIx ) );
		ScRange aTokenRange;
		if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
		{
			// columns: find maximum
			nWidth = Max( nWidth, (SCCOL)(aTokenRange.aEnd.Col() - aTokenRange.aStart.Col() + 1) );
			// rows: add row range + 1 empty row
			nHeight += aTokenRange.aEnd.Row() - aTokenRange.aStart.Row() + 2;
		}
	}
	// remove the last empty row
	if( nHeight > 0 )
		nHeight--;

	//	alte Daten loeschen / neue kopieren

	ScAddress aDestPos = aDestArea.aStart;
	SCTAB nDestTab = aDestPos.Tab();
	ScRange aOldRange = aDestArea;
	ScRange aNewRange = aDestArea;			// alter Bereich, wenn Datei nicht gefunden o.ae.
	if (nWidth > 0 && nHeight > 0)
	{
		aNewRange.aEnd.SetCol( aNewRange.aStart.Col() + nWidth - 1 );
		aNewRange.aEnd.SetRow( aNewRange.aStart.Row() + nHeight - 1 );
	}

    //! check CanFitBlock only if bDoInsert is set?
    sal_Bool bCanDo = ValidColRow( aNewRange.aEnd.Col(), aNewRange.aEnd.Row() ) &&
                  pDoc->CanFitBlock( aOldRange, aNewRange );
	if (bCanDo)
	{
        ScDocShellModificator aModificator( *pImpl->m_pDocSh );

		SCCOL nOldEndX = aOldRange.aEnd.Col();
		SCROW nOldEndY = aOldRange.aEnd.Row();
		SCCOL nNewEndX = aNewRange.aEnd.Col();
		SCROW nNewEndY = aNewRange.aEnd.Row();
		ScRange aMaxRange( aDestPos,
					ScAddress(Max(nOldEndX,nNewEndX), Max(nOldEndY,nNewEndY), nDestTab) );

		//	Undo initialisieren

		ScDocument* pUndoDoc = NULL;
		ScDocument* pRedoDoc = NULL;
		if ( bAddUndo && bUndo )
		{
			pUndoDoc = new ScDocument( SCDOCMODE_UNDO );
			if ( bDoInsert )
			{
				if ( nNewEndX != nOldEndX || nNewEndY != nOldEndY )				// Bereich veraendert?
				{
					pUndoDoc->InitUndo( pDoc, 0, pDoc->GetTableCount()-1 );
					pDoc->CopyToDocument( 0,0,0,MAXCOL,MAXROW,MAXTAB,
											IDF_FORMULA, sal_False, pUndoDoc );		// alle Formeln
				}
				else
					pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );				// nur Zieltabelle
                pDoc->CopyToDocument( aOldRange, IDF_ALL & ~IDF_NOTE, sal_False, pUndoDoc );
			}
			else		// ohne Einfuegen
			{
				pUndoDoc->InitUndo( pDoc, nDestTab, nDestTab );				// nur Zieltabelle
                pDoc->CopyToDocument( aMaxRange, IDF_ALL & ~IDF_NOTE, sal_False, pUndoDoc );
			}
		}

		//	Zellen einfuegen / loeschen
		//	DeleteAreaTab loescht auch MERGE_FLAG Attribute

		if (bDoInsert)
			pDoc->FitBlock( aOldRange, aNewRange );			// incl. loeschen
		else
            pDoc->DeleteAreaTab( aMaxRange, IDF_ALL & ~IDF_NOTE );

		//	Daten kopieren

		if (nWidth > 0 && nHeight > 0)
		{
			ScDocument aClipDoc( SCDOCMODE_CLIP );
			ScRange aNewTokenRange( aNewRange.aStart );
			nStringIx = 0;
			for( nToken = 0; nToken < nTokenCnt; nToken++ )
			{
				String aToken( aTempArea.GetToken( 0, ';', nStringIx ) );
				ScRange aTokenRange;
				if( FindExtRange( aTokenRange, pSrcDoc, aToken ) )
				{
					SCTAB nSrcTab = aTokenRange.aStart.Tab();
					ScMarkData aSourceMark;
					aSourceMark.SelectOneTable( nSrcTab );		// selektieren fuer CopyToClip
					aSourceMark.SetMarkArea( aTokenRange );

                    ScClipParam aClipParam(aTokenRange, false);
                    pSrcDoc->CopyToClip(aClipParam, &aClipDoc, &aSourceMark);

					if ( aClipDoc.HasAttrib( 0,0,nSrcTab, MAXCOL,MAXROW,nSrcTab,
											HASATTR_MERGED | HASATTR_OVERLAPPED ) )
					{
						//!	ResetAttrib am Dokument !!!

						ScPatternAttr aPattern( pSrcDoc->GetPool() );
						aPattern.GetItemSet().Put( ScMergeAttr() );				// Defaults
						aPattern.GetItemSet().Put( ScMergeFlagAttr() );
						aClipDoc.ApplyPatternAreaTab( 0,0, MAXCOL,MAXROW, nSrcTab, aPattern );
					}

					aNewTokenRange.aEnd.SetCol( aNewTokenRange.aStart.Col() + (aTokenRange.aEnd.Col() - aTokenRange.aStart.Col()) );
					aNewTokenRange.aEnd.SetRow( aNewTokenRange.aStart.Row() + (aTokenRange.aEnd.Row() - aTokenRange.aStart.Row()) );
					ScMarkData aDestMark;
					aDestMark.SelectOneTable( nDestTab );
					aDestMark.SetMarkArea( aNewTokenRange );
					pDoc->CopyFromClip( aNewTokenRange, aDestMark, IDF_ALL, NULL, &aClipDoc, sal_False );
					aNewTokenRange.aStart.SetRow( aNewTokenRange.aEnd.Row() + 2 );
				}
			}
		}
		else
		{
			String aErr = ScGlobal::GetRscString(STR_LINKERROR);
			pDoc->SetString( aDestPos.Col(), aDestPos.Row(), aDestPos.Tab(), aErr );
		}

		//	Undo eintragen

		if ( bAddUndo && bUndo)
		{
			pRedoDoc = new ScDocument( SCDOCMODE_UNDO );
			pRedoDoc->InitUndo( pDoc, nDestTab, nDestTab );
            pDoc->CopyToDocument( aNewRange, IDF_ALL & ~IDF_NOTE, sal_False, pRedoDoc );

            pImpl->m_pDocSh->GetUndoManager()->AddUndoAction(
                new ScUndoUpdateAreaLink( pImpl->m_pDocSh,
											aFileName, aFilterName, aOptions,
											aSourceArea, aOldRange, GetRefreshDelay(),
											aNewUrl, rNewFilter, aNewOpt,
											rNewArea, aNewRange, nNewRefresh,
											pUndoDoc, pRedoDoc, bDoInsert ) );
		}

		//	neue Einstellungen merken

		if ( bNewUrlName )
			aFileName = aNewUrl;
		if ( rNewFilter != aFilterName )
			aFilterName = rNewFilter;
		if ( rNewArea != aSourceArea )
			aSourceArea = rNewArea;
		if ( aNewOpt != aOptions )
			aOptions = aNewOpt;

		if ( aNewRange != aDestArea )
			aDestArea = aNewRange;

		if ( nNewRefresh != GetRefreshDelay() )
			SetRefreshDelay( nNewRefresh );

		SCCOL nPaintEndX = Max( aOldRange.aEnd.Col(), aNewRange.aEnd.Col() );
		SCROW nPaintEndY = Max( aOldRange.aEnd.Row(), aNewRange.aEnd.Row() );

		if ( aOldRange.aEnd.Col() != aNewRange.aEnd.Col() )
			nPaintEndX = MAXCOL;
		if ( aOldRange.aEnd.Row() != aNewRange.aEnd.Row() )
			nPaintEndY = MAXROW;

        if ( !pImpl->m_pDocSh->AdjustRowHeight( aDestPos.Row(), nPaintEndY, nDestTab ) )
            pImpl->m_pDocSh->PostPaint( aDestPos.Col(),aDestPos.Row(),nDestTab,
									nPaintEndX,nPaintEndY,nDestTab, PAINT_GRID );
		aModificator.SetDocumentModified();
	}
	else
	{
		//	CanFitBlock sal_False -> Probleme mit zusammengefassten Zellen
		//						 oder Tabellengrenze erreicht!
		//!	Zellschutz ???

		//!	Link-Dialog muss Default-Parent setzen
		//	"kann keine Zeilen einfuegen"
		InfoBox aBox( Application::GetDefDialogParent(),
						ScGlobal::GetRscString( STR_MSSG_DOSUBTOTALS_2 ) );
		aBox.Execute();
	}

	//	aufraeumen

	aRef->DoClose();

	pDoc->SetInLinkUpdate( sal_False );

	if (bCanDo)
	{
		//	notify Uno objects (for XRefreshListener)
		//!	also notify Uno objects if file name was changed!
		ScLinkRefreshedHint aHint;
		aHint.SetAreaLink( aDestPos );
		pDoc->BroadcastUno( aHint );
	}

	return bCanDo;
}


IMPL_LINK( ScAreaLink, RefreshHdl, ScAreaLink*, EMPTYARG )
{
    long nRes = Refresh( aFileName, aFilterName, aSourceArea,
        GetRefreshDelay() ) != 0;
    return nRes;
}

IMPL_LINK( ScAreaLink, AreaEndEditHdl, void*, EMPTYARG )
{
    //  #i76514# can't use link argument to access the dialog,
    //  because it's the ScLinkedAreaDlg, not AbstractScLinkedAreaDlg

    if ( pImpl->m_pDialog && pImpl->m_pDialog->GetResult() == RET_OK )
    {
        aOptions = pImpl->m_pDialog->GetOptions();
        Refresh( pImpl->m_pDialog->GetURL(), pImpl->m_pDialog->GetFilter(),
                 pImpl->m_pDialog->GetSource(), pImpl->m_pDialog->GetRefresh() );

        //  copy source data from members (set in Refresh) into link name for dialog
        String aNewLinkName;
        sfx2::MakeLnkName( aNewLinkName, NULL, aFileName, aSourceArea, &aFilterName );
        SetName( aNewLinkName );
    }
    pImpl->m_pDialog = NULL;    // dialog is deleted with parent

    return 0;
}

